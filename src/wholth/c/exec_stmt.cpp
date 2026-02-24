#include "wholth/c/exec_stmt.h"
#include "db/db.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/error.h"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_error.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include "wholth/cmake_vars.h"

static wholth_StringView to_wholth_str_view(std::string_view sv)
{
    return {.data = sv.data(), .size = sv.size()};
}

static constexpr sqlw::Type to_sqlw_type(std::string_view t)
{
    if /*constexpr*/ ("INT" == t)
    {
        return sqlw::Type::SQL_INT;
    }

    return sqlw::Type::SQL_NULL;
}

struct Bindable
{
    size_t      index{0};
    sqlw::Type  type{sqlw::Type::SQL_NULL};
    std::string validation_error_msg{};
    bool (*validator_func)(std::string_view){nullptr};
    // std::string_view validator_func_name{};
    // std::function<bool(std::string_view)> validator_func{};
    // std::string_view validator_func_name{};
    // std::string_view validation_error_msg{};
};

static std::pair<Bindable, wholth_Error> bindable_from_line(
    std::string_view line)
{
    Bindable result{};

    std::string_view bind_num{};
    std::string_view bind_type{};
    std::string_view validator_func_name{};
    std::string_view validation_error_msg{};
    bool             did_determine_type = false;

    for (size_t i = 0; i < line.size(); i++)
    {
        const auto ch = line[i];

        if (';' != ch)
        {
            continue;
        }

        if (bind_num.empty())
        {
            bind_num = line.substr(4, i - 4);
        }
        else if (bind_type.empty())
        {
            if (line.size() <= 4 + bind_num.size() + 1)
            {
                return {
                    result,
                    {
                        .code = wholth_exec_stmt_Code_SQL_FILE_ARG_MISCONF,
                        .message = to_wholth_str_view(
                            "В sql-скрипте не удалось определить номер "
                            "аргумента!"),
                    }};
            }

            bind_type = line.substr(
                // TODO check bounds
                4 + bind_num.size() + 1,
                i - (4 + bind_num.size() + 1));

            result.type = to_sqlw_type(bind_type);
            did_determine_type = true;
        }
        else
        {
            if (line.size() <= 4 + bind_num.size() + bind_type.size() + 2)
            {
                return {
                    result,
                    {
                        .code =
                            wholth_exec_stmt_Code_SQL_FILE_VALIDATOR_MISCONF,
                        .message = to_wholth_str_view(
                            "В sql-скрипте не удалось определить названии "
                            "валидирующей ф-ции!"),
                    }};
            }

            validator_func_name = line.substr(
                4 + bind_num.size() + bind_type.size() + 2,
                i - (4 + bind_num.size() + bind_type.size() + 2));

            // TODO check bounds
            validation_error_msg = line.substr(i + 1);

            // result.validator_func_name = validator_func_name;
            result.validation_error_msg = validation_error_msg;

            if ("is_valid_id" == validator_func_name)
            {
                result.validator_func = wholth::utils::is_valid_id;
            }
            else
            {
                return {
                    result,
                    {
                        .code =
                            wholth_exec_stmt_Code_SQL_FILE_VALIDATOR_UNKNOWN,
                        .message = to_wholth_str_view(
                            "Указана неизвестная вилидирующая ф-ция!"),
                    }};
            }
        }
    }

    if (!did_determine_type)
    {
        return {
            result,
            {
                .code = wholth_exec_stmt_Code_SQL_FILE_BINDABLE_TYPE_MISCONF,
                .message = to_wholth_str_view(
                    "В sql-скрипте не указан тип аргумента!"),
            }};
    }

    return {result, wholth_Error_OK};
}

struct MapEntry
{
    std::string           filename;
    std::string           sql;
    wholth_exec_stmt_Task task{wholth_exec_stmt_Task_NOOP};
    std::vector<Bindable> bindables;
};
// TODO check if needed
struct CompareToStringView
{
    bool operator()(std::string& s1, std::string_view s2) const
    {
        if (s1.data() == s2.data())
        {
            return true;
        }

        if (s1.size() != s2.size())
        {
            return false;
        }

        for (size_t i = 0; i < s1.size(); i++)
        {
            if (s1[i] != s2[i])
            {
                return false;
            }
        }

        return true;
    }
};
std::unordered_map<std::string_view, MapEntry /*, CompareToStringView*/>
    g_cache;

static wholth_exec_stmt_Task task_from_line(std::string_view line)
{
    if ("-- wholth_exec_stmt_Task_DELETE" == line)
    {
        return wholth_exec_stmt_Task_DELETE;
    }

    return wholth_exec_stmt_Task_NOOP;
}

// ptr -> ptr -> value
static wholth_Error get_entry(
    const MapEntry** result,
    std::string_view filename)
{
    if (!g_cache.contains(filename))
    {
        // std::cout << "<<<<<<<<<<<<<<<<<<<<<<\nDOES NOT CONTAINS!!!!\n"
        //           << __FILE__ << ":" << __LINE__ << "\n"
        //           << filename << "\n";

        MapEntry entry{};

        std::stringstream     sql_stream{};
        std::filesystem::path filepath = SQL_STATEMENTS_DIR;
        filepath /= filename;

        if (!std::filesystem::exists(filepath))
        {
            return {
                .code = wholth_exec_stmt_Code_SQL_FILE_DOES_NOT_EXIST,
                .message =
                    to_wholth_str_view("Запрошенный sql-скрипт не существует!"),
            };
        }
        std::ifstream fin{filepath};

        std::string line;
        size_t      line_number = 0;
        size_t      bind_idx = 0;

        while (std::getline(fin, line))
        {
            line_number++;
            std::string_view sv{line};

            if (1 == line_number)
            {
                entry.task = task_from_line(sv);
                continue;
            }

            if (wholth_exec_stmt_Task_NOOP == entry.task)
            {
                return {
                    .code = wholth_exec_stmt_Code_NOOP,
                    .message =
                        to_wholth_str_view("Не удалось определить тип задачи!"),
                };
            }

            if (sv.size() > 4 && "-- ?" == sv.substr(0, 4))
            {
                bind_idx++;

                auto [bindable, err] = bindable_from_line(sv);

                if (!wholth_error_ok(&err))
                {
                    return err;
                }

                bindable.index = bind_idx;

                entry.bindables.push_back(bindable);
            }
            else
            {
                sql_stream << line << "\n";
            }
        }

        entry.sql = sql_stream.str();
        entry.filename = filename;

        g_cache[filename] = std::move(entry);
    }

    const auto& r = g_cache[filename];
    *result = &r;

    return wholth_Error_OK;
}
/**
 * Look into `???????` directory to find out what script does.
 */
// extern "C" wholth_exec_stmt_Result wholth_exec_stmt(
extern "C" wholth_Error wholth_exec_stmt(
    const wholth_exec_stmt_Args* const args)
{
    // TODO check nullptr != args
    if (nullptr == args)
    {
        return {
            .code = wholth_exec_stmt_Code_ARGS_INVALID,
            .message = to_wholth_str_view("Передан nullptr как аргумент!"),
        };
    }

    if (nullptr == args->binds && args->binds_size > 0)
    {
        return {
            .code = wholth_exec_stmt_Code_BINDS_MISCOUNT,
            .message = to_wholth_str_view(
                "Указанное кол-во аргументов не соотв. действительному!"),
        };
    }

    const auto filename = wholth::utils::to_string_view(args->sql_file);

    const MapEntry* entry{nullptr};
    const auto      err = get_entry(&entry, filename);

    if (!wholth_error_ok(&err))
    {
        return err;
    }

    if (nullptr == entry)
    {
        return {
            .code = wholth_exec_stmt_Code_SQL_FILE_MISCONF,
            .message = to_wholth_str_view(
                "Не удалось сгенерировать конфигурацию для sql-скрипта!")};
    }

    if (args->binds_size != entry->bindables.size())
    {
        return {
            .code = wholth_exec_stmt_Code_SQL_FILE_BINDABLE_MISCOUNT,
            .message = to_wholth_str_view(
                "Кол-во аргументов, указанных в sql-скрипте, не соответствует "
                "кол-ву указанному в пользователем!")};
    }

    // args.binds_count
    // TODO check overflow
    for (uint64_t i = 0; i < args->binds_size; i++)
    {
        const auto& bindable = entry->bindables[i];
        const auto  v = wholth::utils::to_string_view(args->binds[i].value);

        if (nullptr != bindable.validator_func)
        {
            if (!bindable.validator_func(v))
            {
                return {
                    .code = 1,
                    .message =
                        to_wholth_str_view(bindable.validation_error_msg)};
            }
        }
    }

    auto&           con = db::connection();
    sqlw::Statement stmt{&con};

    stmt.prepare(entry->sql);

    if (sqlw::status::Condition::OK != stmt.status())
    {
        return wholth::utils::from_error_code(stmt.status());
    }

    // assert bind not nullptr
    for (uint64_t i = 0; i < args->binds_size; i++)
    {
        const auto v = args->binds[i].value;
        stmt.bind(i + 1, {v.data, v.size}, entry->bindables[i].type);

        if (sqlw::status::Condition::OK != stmt.status())
        {
            return wholth::utils::from_error_code(stmt.status());
        }
    }

    std::stringstream buffer_stream{};

    switch (entry->task)
    {
    case wholth_exec_stmt_Task_DELETE:
        stmt.exec([&buffer_stream](sqlw::Statement::ExecArgs e) {
            buffer_stream << e.column_value;
        });
        break;
    case wholth_exec_stmt_Task_NOOP:
        assert(false);
    }

    if (sqlw::status::Condition::OK != stmt.status())
    {
        return wholth::utils::from_error_code(stmt.status());
    }

    return wholth_Error_OK;
}
