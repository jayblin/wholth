#ifndef DB_H_
#define DB_H_

#include "sqlw/connection.hpp"
#include <filesystem>
#include <functional>
#include <string_view>

namespace db::migration
{
	struct MigrateArgs
	{
		sqlw::Connection* con;
		std::filesystem::path migrations_dir;
		std::function<bool (const std::filesystem::directory_entry &)> filter;
		bool log {true};
	};

	int migrate(MigrateArgs);
}

#endif // DB_H_
