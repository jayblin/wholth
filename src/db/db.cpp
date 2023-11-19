#include "db/db.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

static void log_migration(
    sqlw::Connection* con,
    const std::filesystem::directory_entry& entry
)
{
	std::stringstream insert;
	const auto timestamp = std::time(nullptr);

	/**
	 * @todo: rework executed_at to work with string-datetime representation.
	 */
	insert << "INSERT INTO migration (filename, executed_at) "
	       << "VALUES ('" << entry.path().filename().string() << "', '"
	       << timestamp << "')";

	sqlw::Statement {con}(insert.str());
}

static bool check_meta_table_exists(sqlw::Connection* con)
{
	// get list of executed migrations
	bool result = false;

	sqlw::Statement {con}(
	    "SELECT name "
	    "FROM sqlite_master "
	    "WHERE type='table' AND name='migration'",
	    [&result](sqlw::Statement::ExecArgs _)
	    {
		    result = true;
	    }
	);

	return result;
}

static std::vector<std::string> find_executed_migrations(sqlw::Connection* con)
{
	std::vector<std::string> result;
	const auto meta_table_exists = check_meta_table_exists(con);

	if (!meta_table_exists || !sqlw::status::is_ok(con->status()))
	{
		return result;
	}

	sqlw::Statement {con}(
	    "SELECT filename FROM migration",
	    [&result](sqlw::Statement::ExecArgs args)
	    {
		    result.push_back(std::string {args.column_value});
	    }
	);

	return result;
}

static bool check_if_migrated_already(
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

static void execute_migration(
    sqlw::Connection* con,
    const std::filesystem::directory_entry& entry
)
{
	const auto fstream = std::ifstream {entry.path()};
	auto sstream = std::stringstream {};
	sstream << fstream.rdbuf();

	sqlw::Statement stmt {con};
	std::string sql = sstream.str();

	stmt.prepare(sql);
	stmt();
}

static void print_sql_error(const sqlw::Connection* con)
{
	std::cout << '(' << static_cast<int>(con->status()) << ") "
	          << sqlw::status::verbose(con->status()) << '\n';
}

/**
 * @todo
 * 	- Add return codes;
 * 	- Add tests for return codes.
 */
int db::migration::migrate(
    /* std::filesystem::path database_path, */
	sqlw::Connection* con,
    std::filesystem::path migrations_dir,
	bool log
)
{
	/* if ( */
	/* 	database_path.compare(":memory:") != 0 */
	/* 	&& ( */
	/* 		!std::filesystem::exists(database_path.parent_path()) */
	/* 		|| !std::filesystem::exists(migrations_dir)) */
	/* 	) */
	/* { */
	/* 	return 10; */
	/* } */

	/* sqlw::Connection con {database_path.string()}; */
	const auto executed_migrations = find_executed_migrations(con);

	if (log)
	{
		std::cout << "A total of " << executed_migrations.size()
				  << " already executed migrations found\n";
	}

	if (!sqlw::status::is_ok(con->status()))
	{
		print_sql_error(con);
		return 1;
	}

	sqlw::Statement stmt {con};

	const auto path = std::filesystem::path {migrations_dir};

	std::set<std::filesystem::directory_entry> sorted_by_name;

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		sorted_by_name.insert(entry);
	}
	
	for (const auto& entry : sorted_by_name)
	{
		if (check_if_migrated_already(entry, executed_migrations))
		{
			continue;
		}

		stmt("BEGIN EXCLUSIVE TRANSACTION;"
		     "SAVEPOINT actual_migration_sp;");

		if (log)
		{
			std::cout << "migrating " << entry.path() << "\n";
		}

		execute_migration(con, entry);

		if (!sqlw::status::is_ok(con->status()))
		{
			print_sql_error(con);

			stmt("ROLLBACK TO actual_migration_sp;"
			     "ROLLBACK;");

			return 1;
		}

		stmt("RELEASE actual_migration_sp");

		stmt("SAVEPOINT migration_log_sp");

		if (log)
		{
			std::cout << "logging migration " << entry.path().filename().string()
		          << "\n";
		}
		log_migration(con, entry);

		if (!sqlw::status::is_ok(con->status()))
		{
			print_sql_error(con);

			stmt("ROLLBACK TO migration_log_sp;"
			     "ROLLBACK;");

			return 1;
		}

		stmt("RELEASE migration_log_sp;"
		     "COMMIT;");
	}

	return 0;
}
