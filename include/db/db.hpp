#ifndef DB_H_
#define DB_H_

#include "sqlw/connection.hpp"
#include <filesystem>
#include <span>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

namespace db
{
    namespace status
    {
        enum class Code : int
        {
            OK = 0,
            MIGRATION_TABLE_DOES_NOT_EXIST,
            MIGRATION_FAILED,
            MIGRATION_LOG_FAILED,
        };

        enum class Condition : int
        {
            OK = 0,
        };

        std::error_code make_error_code(Code);
        std::error_condition make_error_condition(Condition);
    }
}

namespace std
{
    template <>
    struct is_error_code_enum<db::status::Code> : true_type {};

    template <>
    struct is_error_condition_enum<db::status::Condition> : true_type {};
}

namespace db::migration
{
	struct MigrateArgs
	{
		sqlw::Connection* con;
        std::span<const std::filesystem::directory_entry> migrations;
        // todo remove???
		bool log {true};
	};

    struct MigrateResult
    {
        std::error_code error_code;
        std::vector<std::string> executed_migrations;
        std::string problematic_migration;
    };

    std::error_code make_migration_table(sqlw::Connection*) noexcept;
    std::vector<std::filesystem::directory_entry> list_sorted_migrations(std::filesystem::path);

    MigrateResult migrate(MigrateArgs) noexcept;
}

#endif // DB_H_
