#include <filesystem>
#include <gsl/assert>
#include <gsl/gsl>
#include <memory>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <vector>
#include "fmt/color.h"
#include "fmt/core.h"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "db/db.hpp"
#include "wholth/app.hpp"
#include "wholth/cmake_vars.h"
#include "wholth/context.hpp"
#include "wholth/utils.hpp"

#include "wholth/app.hpp"

using namespace std::chrono_literals;

// static void error_log_callback(void* pArg, int iErrCode, const char* zMsg)
// {
//     wholth::Context* ctx = static_cast<wholth::Context*>(pArg);
//     // todo remake to array
//     ctx->sql_errors.emplace_back(fmt::format("[{}] {}", iErrCode, zMsg));
// }

void db::user_defined::log(void *pArg, int iErrCode, const char *zMsg)
{
	// std::cout << '[' << iErrCode << "] " << zMsg << '\n';
    fmt::print(fmt::fg(fmt::color::medium_violet_red), "[{}] {}\n", iErrCode, zMsg);
}

/* static void glfw_error_callback(int error_code, const char* description) */
/* { */
/*     fmt::print(fmt::fg(fmt::color::red), "GLFW error [{}]: {}\n", error_code,
 * description); */
/* } */

bool is_app_version_equal_db(sqlw::Connection& con) noexcept(false)
{
    using C = sqlw::status::Condition;
    bool are_versions_same = false;
    std::error_code ec =
        (sqlw::Statement{&con})
            .prepare("SELECT value FROM app_info WHERE field = 'version'")
            .exec([&are_versions_same](sqlw::Statement::ExecArgs e) {
                are_versions_same = e.column_value == WHOLTH_APP_VERSION;
            })
            .status();

    if (C::OK != ec)
    {
        throw std::system_error(ec);
    }

    return are_versions_same;
}

/* static std::filesystem::path get_db_filepath() */
/* { */
/*     std::filesystem::path path {DATABASE_DIR}; */
/*     path /= DATABASE_NAME; */

/*     return path; */
/* } */

static bool does_db_file_exist(std::string_view db_path) noexcept
{
    /* auto path = get_db_filepath(); */
    /* return std::filesystem::exists(path); */
    return std::filesystem::exists(db_path);
}

static db::migration::MigrateResult migrate(
    /* static std::error_code migrate( */
    /* wholth::Context& ctx, */
    sqlw::Connection& con
    /* ) noexcept(false) */
    ) noexcept
{
    /* assert(sqlw::status::Condition::OK == con.status()); */

    const auto list = db::migration::list_sorted_migrations(MIGRATIONS_DIR);

    /* ctx.migrate_result = db::migration::migrate({ */
    /*     .con = &con, */
    /*     .migrations = list, */
    /*     .log = false, */
    /* }); */

    return db::migration::migrate({
        /* const auto mr = db::migration::migrate({ */
        .con = &con,
        .migrations = list,
        .log = false,
    });

    /* if (db::status::Condition::OK != mr.error_code) { */
    /*     throw std::system_error(mr.error_code, mr.problematic_migration); */
    /* } */
}

/* static void bump_app_version_in_db(sqlw::Connection& con) noexcept(false) */
static std::error_code bump_app_version_in_db(sqlw::Connection& con) noexcept
{
    /* auto ec = (sqlw::Statement {&con}) */
    return (sqlw::Statement{&con})
        .prepare("INSERT OR REPLACE INTO app_info (field, value) "
                 " VALUES ('version', :1)")
        .bind(1, WHOLTH_APP_VERSION, sqlw::Type::SQL_TEXT)
        .exec()
        .status();

    /* if (sqlw::status::Condition::OK != ec) { */
    /*     throw std::system_error(ec); */
    /* } */
}

static void setup_db_cleanup(std::string_view db_path) noexcept(false)
{
    if (std::filesystem::exists(db_path))
    {
        std::filesystem::remove(db_path);
    }

    assert(!std::filesystem::exists(db_path));
}

/* static sqlw::Connection connect_to_db() noexcept(false) */
/* static sqlw::Connection connect_to_db(std::string_view db_path) noexcept */
/* { */
/*     using C = sqlw::status::Condition; */

/*     /1* const auto path = get_db_filepath(); *1/ */
/*     /1* sqlw::Connection con {path.string()}; *1/ */
/*     sqlw::Connection con {db_path}; */

/*     /1* if (C::OK != con.status()) *1/ */
/*     /1* { *1/ */
/*     /1*     throw std::system_error(con.status()); *1/ */
/*     /1* } *1/ */

/*     return con; */
/* } */

// static void setup_db_global_options(wholth::Context& ctx)
// {
//     static bool did_global_db_setup = false;
//
//     if (!did_global_db_setup)
//     {
//         const std::error_code ec = sqlw::status::Code{
//             sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, &ctx)};
//         assertm(
//             sqlw::status::Condition::OK == ec, "Error during sqlite3_config()");
//         did_global_db_setup = true;
//     }
// }

static void setup_db_instance_options(sqlw::Connection& con) noexcept(false)
{
    sqlw::Statement stmt{&con};
    std::error_code ec;

    ec = stmt("PRAGMA foreign_keys = ON");

    if (sqlw::status::Condition::OK != ec)
    {
        throw std::system_error{ec};
    }

    ec = stmt("PRAGMA automatic_index = OFF");

    if (sqlw::status::Condition::OK != ec)
    {
        throw std::system_error{ec};
    }

    ec = sqlw::status::Code{sqlite3_create_function_v2(
        con.handle(),
        "seconds_to_readable_time",
        1,
        SQLITE_DETERMINISTIC | SQLITE_DIRECTONLY,
        nullptr,
        wholth::utils::sqlite::seconds_to_readable_time,
        nullptr,
        nullptr,
        nullptr)};

    if (sqlw::status::Condition::OK != ec)
    {
        throw std::system_error{ec};
    }
}

// @todo test this workflow
// static sqlw::Connection setup_db(
//     /* static std::error_code setup_db( */
//     /* wholth::Context& ctx */
//     std::string_view db_path) noexcept(false)
// {
//     assert(db_path.size() > 0);
//
//     sqlw::Connection con;
//     std::error_code ec;
//
//     // todo check db_path
//     if (!does_db_file_exist(db_path))
//     {
//         /* con = connect_to_db(ctx.db_path); */
//         con = sqlw::Connection(db_path);
//
//         if (sqlw::status::Condition::OK != con.status())
//         {
//             setup_db_cleanup(db_path);
//             throw std::system_error(con.status());
//         }
//
//         ec = db::migration::make_migration_table(&con);
//
//         if (db::status::Condition::OK != ec)
//         {
//             setup_db_cleanup(db_path);
//             throw std::system_error(ec);
//         }
//
//         const auto migrate_result = migrate(con);
//
//         if (db::status::Condition::OK != migrate_result.error_code)
//         {
//             setup_db_cleanup(db_path);
//             throw std::system_error(
//                 migrate_result.error_code,
//                 migrate_result.problematic_migration);
//         }
//
//         ec = bump_app_version_in_db(con);
//
//         if (db::status::Condition::OK != ec)
//         {
//             setup_db_cleanup(db_path);
//             throw std::system_error(ec);
//         }
//     }
//     else
//     {
//         /* con = connect_to_db(ctx.db_path); */
//         con = sqlw::Connection(db_path);
//
//         if (!is_app_version_equal_db(con))
//         {
//             /* migrate(con); */
//             const auto migrate_result = migrate(con);
//
//             if (db::status::Condition::OK != migrate_result.error_code)
//             {
//                 throw std::system_error(
//                     migrate_result.error_code,
//                     migrate_result.problematic_migration);
//             }
//
//             ec = bump_app_version_in_db(con);
//
//             if (db::status::Condition::OK != ec)
//             {
//                 throw std::system_error(ec);
//             }
//         }
//     }
//
//     setup_db_instance_options(con);
//
//     /* ctx.connection = std::move(con); */
//     return con;
// }

/* template <typename... C, typename... M> */
void wholth::app::setup(
    std::string_view db_path,
    wholth::Context& ctx
    /* std::tuple<C...> controllers, */
    /* std::tuple<M...> models */
)
{
    // everything below may throw.
    db::setup_logger();
    db::init(db_path);

    auto& db_con = db::connection();

    setup_db_instance_options(db_con);

    // todo rename "MIGRATIONS_DIR" to "WHOLTH_MIGRATIONS_DIR".
    const auto migrations =
        db::migration::list_sorted_migrations(MIGRATIONS_DIR);

    auto ec = db::migration::make_migration_table(&db_con);

    if (db::status::Condition::OK != ec)
    {
        // setup_db_cleanup(db_path);
        throw std::system_error(ec);
    }

    const auto migrate_result = db::migration::migrate({
        .con = &db_con,
        .migrations = migrations,
    });

    if (db::status::Condition::OK != migrate_result.error_code)
    {
        throw std::system_error(
            migrate_result.error_code, migrate_result.problematic_migration);
    }

    ec = bump_app_version_in_db(db::connection());

    if (db::status::Condition::OK != ec)
    {
        throw std::system_error(ec);
    }

    // if (nullptr == ctx.connection.handle()) {
    //     setup_db_global_options(ctx);
    //
    //     ctx.connection = setup_db(ctx.db_path);
    // }

    /* ctx.foods_page_ctrl.fetch(ctx.locale_id(), ctx.connection); */
    /* std::get<wholth::controller::FoodsPage>(controllers). */
}
