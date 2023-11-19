#ifndef DB_H_
#define DB_H_

#include "sqlw/connection.hpp"
#include <filesystem>
#include <string_view>

namespace db::migration
{
	int migrate(
		sqlw::Connection* con,
		std::filesystem::path migrations_dir,
		bool log = true
	);
}

#endif // DB_H_
