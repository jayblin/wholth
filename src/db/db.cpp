#include "db/db.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "wholth/utils.hpp"
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace db::status
{
struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "database";
    }

    std::string message(int ev) const override final
    {
        switch (static_cast<Code>(ev))
        {
        case Code::MIGRATION_TABLE_DOES_NOT_EXIST:
            return "migration table does not exist";
        case Code::MIGRATION_FAILED:
            return "error while executing a migration";
        case Code::MIGRATION_LOG_FAILED:
            return "error while trying to log info about a migration";
        case Code::OK:
            return "no error";
        }

        return "(unrecognized error)";
    }
};

struct ConditionCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "database condition";
    }

    std::string message(int ev) const override final
    {
        switch (static_cast<Condition>(ev))
        {
        case Condition::OK:
            return "ok";
        }

        return "(unrecognized error)";
    }

    bool equivalent(const std::error_code& ec, int cond)
        const noexcept override final
    {
        const std::error_category& db_error_category =
            std::error_code{Code{}}.category();

        if (ec.category() != db_error_category)
        {
            return sqlw::status::Condition::OK == ec;
        }

        switch (static_cast<Condition>(cond))
        {
        case Condition::OK:
            return static_cast<Code>(ec.value()) == Code::OK;
        default:
            return false;
        }
    }
};

const ErrorCategory error_category{};
const ConditionCategory condition_category{};
} // namespace db::status

std::error_code db::status::make_error_code(db::status::Code e)
{
    return {static_cast<int>(e), db::status::error_category};
}

std::error_condition db::status::make_error_condition(db::status::Condition e)
{
    return {static_cast<int>(e), db::status::condition_category};
}

static std::error_code log_migration(
    sqlw::Connection* con,
    const std::filesystem::directory_entry& entry)
{
    return sqlw::Transaction{con}(
        "INSERT INTO migration (filename, executed_at) "
        "VALUES (:1, :2)",
        std::array<sqlw::Statement::bindable_t, 2>{
            {{entry.path().filename().string(), sqlw::Type::SQL_TEXT},
             {wholth::utils::current_time_and_date(), sqlw::Type::SQL_TEXT}}});
}

static std::error_code does_migration_table_exist(sqlw::Connection* con)
{
    // get list of executed migrations
    bool result = false;

    sqlw::Statement stmt{con};
    const auto ec = stmt(
        "SELECT name "
        "FROM sqlite_master "
        "WHERE type='table' AND name='migration'",
        [&result](sqlw::Statement::ExecArgs _) { result = true; });

    return result ? ec : db::status::Code::MIGRATION_TABLE_DOES_NOT_EXIST;
}

static std::tuple<std::error_code, std::vector<std::string>>
find_already_executed_migrations(sqlw::Connection* con)
{
    std::vector<std::string> result;
    const std::error_code ec = sqlw::Statement{con}(
        "SELECT filename FROM migration",
        [&result](sqlw::Statement::ExecArgs args) {
            result.push_back(std::string{args.column_value});
        });

    return {ec, result};
}

static bool is_migrated_already(
    const std::filesystem::directory_entry& entry,
    const std::vector<std::string>& executed_migrations)
{
    const auto it = std::find_if(
        executed_migrations.begin(),
        executed_migrations.end(),
        [&entry](const std::string& name) {
            return name == entry.path().filename();
        });

    return it != executed_migrations.end();
}

static std::error_code execute_migration(
    sqlw::Connection* con,
    const std::filesystem::directory_entry& entry) noexcept
{
    std::ifstream fstream{entry.path()};
    std::string sql(
        (std::istreambuf_iterator<char>(fstream)),
        (std::istreambuf_iterator<char>()));

    return sqlw::Transaction{con}(sql);
}

std::error_code db::migration::make_migration_table(
    sqlw::Connection* con) noexcept
{
    return sqlw::Transaction{con}(
        R"SQL(
        CREATE TABLE IF NOT EXISTS migration (
            filename TEXT,
            executed_at TEXT,
            UNIQUE (filename)
        ) STRICT
        )SQL");
}

std::vector<std::filesystem::directory_entry> db::migration::
    list_sorted_migrations(std::filesystem::path path)
{
    assert(std::filesystem::exists(path));

    std::set<std::filesystem::directory_entry> sorted_by_name;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        sorted_by_name.insert(entry);
    }

    return {sorted_by_name.begin(), sorted_by_name.end()};
}

db::migration::MigrateResult db::migration::migrate(MigrateArgs args) noexcept
{
    db::migration::MigrateResult result{};

    result.error_code = does_migration_table_exist(args.con);
    auto ec = result.error_code;

    if (db::status::Code::MIGRATION_TABLE_DOES_NOT_EXIST == result.error_code)
    {

        ec = db::migration::make_migration_table(&db::connection());
    }

    if (db::status::Condition::OK != result.error_code)
    {
        return result;
    }

    auto faem_result = find_already_executed_migrations(args.con);

    result.error_code = std::get<std::error_code>(faem_result);

    if (db::status::Condition::OK != result.error_code)
    {
        return result;
    }

    sqlw::Statement stmt{args.con};

    for (const auto& entry : args.migrations)
    {
        if (is_migrated_already(entry, std::get<1>(faem_result)))
        {
            continue;
        }

        result.error_code = stmt("BEGIN EXCLUSIVE TRANSACTION");

        if (sqlw::status::Condition::OK != result.error_code)
        {
            return result;
        }

        result.error_code = execute_migration(args.con, entry);

        if (sqlw::status::Condition::OK != result.error_code)
        {
            ec = stmt("ROLLBACK");

            if (sqlw::status::Condition::OK != ec)
            {
                result.error_code = sqlw::status::Code::ROLLBACK_ERROR;
                return result;
            }

            result.error_code = db::status::Code::MIGRATION_FAILED;
            result.problematic_migration = entry.path().filename();

            return result;
        }

        result.error_code = log_migration(args.con, entry);

        if (sqlw::status::Condition::OK != result.error_code)
        {
            ec = stmt("ROLLBACK");

            if (sqlw::status::Condition::OK != ec)
            {
                result.error_code = sqlw::status::Code::ROLLBACK_ERROR;
                return result;
            }

            result.error_code = db::status::Code::MIGRATION_LOG_FAILED;

            return result;
        }

        ec = stmt("COMMIT");

        if (sqlw::status::Condition::OK != ec)
        {
            // todo remane to COMMIT_ERROR
            result.error_code = sqlw::status::Code::RELEASE_ERROR;
            return result;
        }

        result.error_code = db::status::Code::OK;
        result.executed_migrations.push_back(entry.path().filename());
    }

    return result;
}

void db::setup_logger() noexcept
{
    static bool db_is_logger_set = false;

    if (!db_is_logger_set)
    {
        const std::error_code ec = sqlw::status::Code{
            sqlite3_config(SQLITE_CONFIG_LOG, db::user_defined::log, nullptr)};
        assert(
            sqlw::status::Condition::OK == ec &&
            "Error during sqlite3_config()");
        db_is_logger_set = true;
        // const std::error_code ec =
        // sqlw::status::Code{sqlite3_config(SQLITE_CONFIG_LOG,
        // error_log_callback, &ctx)};
    }
}

static sqlw::Connection global_db_connection;

sqlw::Connection& db::connection() noexcept
{
    assert(
        global_db_connection.handle() != nullptr &&
        "Connection is not initialized!");
    return global_db_connection;
}

static bool does_db_file_exist(std::string_view db_path) noexcept
{
    /* auto path = get_db_filepath(); */
    /* return std::filesystem::exists(path); */
    return std::filesystem::exists(db_path);
}

static void cleanup_db(std::string_view db_path) noexcept(false)
{
    if (std::filesystem::exists(db_path))
    {
        std::filesystem::remove(db_path);
    }

    assert(!std::filesystem::exists(db_path));
}

void db::init(std::string_view db_path) noexcept
{
    assert(db_path.size() > 0 && "Path to the database can't be empty!");

    static bool db_is_initialized = false;

    assert(!db_is_initialized && "Can't initialize database more than once!");

    sqlw::Connection con;

    const auto existed = does_db_file_exist(db_path);

    con = sqlw::Connection(db_path);

    // todo log error code via logger function ???
    if (sqlw::status::Condition::OK != con.status())
    {
        if (!existed) {
            cleanup_db(db_path);
        }
        // throw std::system_error(con.status());
        assert(false && "Couldn't create database file");
    }

    global_db_connection = std::move(con);
}
