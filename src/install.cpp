#include "db/db.hpp"
#include "local/cmake_vars.h"

int main()
{
	return db::migration::migrate(
	    std::filesystem::path {DATABASE_DIR} / DATABASE_NAME,
	    MIGRATIONS_DIR
	);
}
