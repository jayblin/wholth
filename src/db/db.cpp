#include "db/db.hpp"
#include "fmt/color.h"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/utils.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
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
                case Condition::ERROR:
                    return "error";
                case Condition::OK:
                    return "no error";
            }

            return "(unrecognized error)";
        }

        bool equivalent(const std::error_code& ec, int cond) const noexcept override final
        {
            const std::error_category& db_error_category = std::error_code{Code{}}.category();

            if (
                ec.category() != db_error_category
            ) {
                return sqlw::status::Condition::OK == ec;
            }

            switch (static_cast<Condition>(cond))
            {
                case Condition::OK:
                    return static_cast<Code>(ec.value()) == Code::OK;
				case Condition::ERROR:
                    return static_cast<Code>(ec.value()) != Code::OK;
                default:
                    return false;
			}
		}
    };

    const ErrorCategory error_category {};
    const ConditionCategory condition_category {};
}

std::error_code db::status::make_error_code(db::status::Code e)
{
    return {static_cast<int>(e), db::status::error_category};
}

std::error_condition db::status::make_error_condition(db::status::Condition e)
{
    return {static_cast<int>(e), db::status::condition_category};
}

static std::error_code log_migration(
    sqlw::Statement& stmt,
    const std::filesystem::directory_entry& entry
)
{
    stmt("SAVEPOINT migration_log_sp");

    stmt.prepare(
        "INSERT INTO migration (filename, executed_at) "
        "VALUES (:1, :2)"
    )
        .bind(1, entry.path().filename().string(), sqlw::Type::SQL_TEXT)
        .bind(2, wholth::utils::current_time_and_date(), sqlw::Type::SQL_TEXT)
        .exec();

    if (sqlw::status::Condition::OK != stmt.status())
    {
        stmt("ROLLBACK TO migration_log_sp");
    }
    else {
        stmt("RELEASE migration_log_sp");
    }

    return stmt.status();
}

static std::error_code does_migration_table_exist(sqlw::Connection* con)
{
	// get list of executed migrations
	bool result = false;

	sqlw::Statement stmt {con};
    stmt(
	    "SELECT name "
	    "FROM sqlite_master "
	    "WHERE type='table' AND name='migration'",
	    [&result](sqlw::Statement::ExecArgs _)
	    {
		    result = true;
	    }
	);

    return result ?
        stmt.status() :
        db::status::Code::MIGRATION_TABLE_DOES_NOT_EXIST;
}

static std::tuple<std::error_code, std::vector<std::string>> find_executed_migrations(sqlw::Connection* con)
{
	std::vector<std::string> result;
	const std::error_code ec = does_migration_table_exist(con);

	if (
        db::status::Condition::OK != ec
        /* || sqlw::status::Condition::OK != ec */
        /* db::status::Code::MIGRATION_TABLE_DOES_NOT_EXIST == ec */
        /* || sqlw::status::Condition::OK != con->status() */
    )
	{
		return {ec, result};
	}

	sqlw::Statement {con}(
	    "SELECT filename FROM migration",
	    [&result](sqlw::Statement::ExecArgs args)
	    {
		    result.push_back(std::string {args.column_value});
	    }
	);

	return {con->status(), result};
}

static bool is_migrated_already(
    const std::filesystem::directory_entry& entry,
    const std::vector<std::string>& executed_migrations
)
{
	const auto it = std::find_if(
	    executed_migrations.begin(),
	    executed_migrations.end(),
	    [&entry](const std::string& name)
	    {
		    return name == entry.path().filename();
	    }
	);

	return it != executed_migrations.end();
}

static std::error_code execute_migration(
    sqlw::Connection* con,
    const std::filesystem::directory_entry& entry
) noexcept
{
	const auto fstream = std::ifstream {entry.path()};
	auto sstream = std::stringstream {};
	sstream << fstream.rdbuf();

	sqlw::Statement stmt {con};
	std::string sql = sstream.str();

    stmt("SAVEPOINT actual_migration_sp;");

	stmt.prepare(sql);

    if (sqlw::status::Condition::OK != stmt.status())
    {
        stmt("ROLLBACK TO actual_migration_sp");
        return stmt.status();
    }

	stmt.exec();

    if (sqlw::status::Condition::OK != stmt.status())
    {
        stmt("ROLLBACK TO actual_migration_sp");
    }
    else {
        stmt("RELEASE actual_migration_sp");
    }

    return stmt.status();
}

db::migration::MigrateResult db::migration::migrate(MigrateArgs args) noexcept
{
	const auto [fem_ec, executed_migrations] = find_executed_migrations(args.con);

    db::migration::MigrateResult result {
        .error_code = fem_ec,
        .executed_migrations = executed_migrations
    };

    if (
        db::status::Code::MIGRATION_TABLE_DOES_NOT_EXIST != result.error_code
        && db::status::Condition::OK != result.error_code
        /* db::status::Code::MIGRATION_TABLE_DOES_NOT_EXIST != result.error_code */
        /* && sqlw::status::Condition::OK != result.error_code */
    ) {
        return result;
    }

	sqlw::Statement stmt {args.con};

	const auto path = std::filesystem::path {args.migrations_dir};

	std::set<std::filesystem::directory_entry> sorted_by_name;

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		sorted_by_name.insert(entry);
	}
	
	for (const auto& entry : sorted_by_name)
	{
		if (
			is_migrated_already(entry, executed_migrations)
			|| (args.filter && !args.filter(entry))
		)
		{
			continue;
		}

		stmt("BEGIN EXCLUSIVE TRANSACTION");

		result.error_code = execute_migration(args.con, entry);

        if (sqlw::status::Condition::OK != result.error_code)
        {
            stmt("ROLLBACK");

            result.problematic_migration = entry.path();

            return result;
        }

		result.error_code = log_migration(stmt, entry);

		if (sqlw::status::Condition::OK != result.error_code)
		{
			stmt("ROLLBACK");

            return result;
		}

		stmt("COMMIT");
	}

    return result;
}
