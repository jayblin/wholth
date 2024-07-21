#ifndef DB_H_
#define DB_H_

#include "sqlw/connection.hpp"
#include "utils/serializer.hpp"
#include <filesystem>
#include <functional>
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
        };

        enum class Condition : int
        {
            OK = 0,
            ERROR,
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
		std::filesystem::path migrations_dir;
		std::function<bool (const std::filesystem::directory_entry &)> filter;
		bool log {true};
	};

    struct MigrateResult
    {
        std::error_code error_code;
        std::vector<std::string> executed_migrations;
        std::string problematic_migration;

        template <typename Serializer>
        auto serialize(Serializer& serializer) const noexcept  -> void
        {
            serializer 
                << NVP(error_code)
                << NVP(executed_migrations)
                << NVP(problematic_migration)
                ;
        }
    };

    MigrateResult migrate(MigrateArgs) noexcept;
}

#endif // DB_H_
