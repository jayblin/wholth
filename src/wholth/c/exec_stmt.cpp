#include "wholth/c/exec_stmt.h"
#include "db/db.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "wholth/c/error.h"
#include "wholth/internal/ring_pool.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_error.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include "wholth/cmake_vars.h"

struct wholth_exec_stmt_Result_t
{
    using State = wholth::internal::ring_pool::EntryState;
    struct Data
    {
        uint64_t              column_count{0};
        std::string           sql_output{""};
        std::vector<uint64_t> sizes{};
    };

    std::atomic<State> state{State::FREE};
    Data               data{};
};

wholth_Error wholth_exec_stmt_Result_new(wholth_exec_stmt_Result** res)
{
    return wholth::internal::ring_pool::fetch<wholth_exec_stmt_Result>(res);
}

wholth_Error wholth_exec_stmt_Result_del(wholth_exec_stmt_Result* res)
{
    return wholth::internal::ring_pool::unfetch<wholth_exec_stmt_Result>(res);
}

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

    if /*constexpr*/ ("TEXT" == t)
    {
        return sqlw::Type::SQL_TEXT;
    }

    if /*constexpr*/ ("DOUBLE" == t)
    {
        return sqlw::Type::SQL_DOUBLE;
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

    if ("-- wholth_exec_stmt_Task_INSERT" == line)
    {
        return wholth_exec_stmt_Task_INSERT;
    }

    if ("-- wholth_exec_stmt_Task_SELECT" == line)
    {
        return wholth_exec_stmt_Task_SELECT;
    }

    if ("-- wholth_exec_stmt_Task_UPDATE" == line)
    {
        return wholth_exec_stmt_Task_UPDATE;
    }

    return wholth_exec_stmt_Task_NOOP;
}

static wholth_Error get_entry(
    const MapEntry** result,
    std::string_view filename,
    bool             skip_cache)
{
    if (true == skip_cache || !g_cache.contains(filename))
    {
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

extern "C" wholth_Error wholth_exec_stmt(
    const wholth_exec_stmt_Args* const args,
    wholth_exec_stmt_Result*           result)
{
    if (nullptr == args)
    {
        return {
            .code = wholth_exec_stmt_Code_ARGS_NULLPTR,
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
    const auto      err = get_entry(&entry, filename, args->skip_cache);

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

    std::vector<sqlw::Statement::bindable_t> binds(args->binds_size);

    for (uint64_t i = 0; i < args->binds_size; i++)
    {
        if (nullptr == args->binds[i].value.data &&
            args->binds[i].value.size > 0)
        {
            return {
                // TODO change code
                .code =
                    wholth_exec_stmt_Code_BINDABLE_VALUE_IS_NULLPTR_BUT_SIZE_IS_GT_0,
                .message = to_wholth_str_view(
                    "BINDABLE_VALUE_IS_NULLPTR_BUT_SIZE_IS_GT_0")};
        }

        // TODO check overflow
        const auto& bindable = entry->bindables[i];

        const auto v = wholth::utils::to_string_view(args->binds[i].value);
        binds[i] = {v, bindable.type};

        if (nullptr != bindable.validator_func)
        {
            if (!bindable.validator_func(v))
            {
                return {
                    .code = wholth_exec_stmt_Code_BINDABLE_VALIDATION_FAIL,
                    .message =
                        to_wholth_str_view(bindable.validation_error_msg)};
            }
        }
    }

    auto&             con = db::connection();
    std::stringstream buffer_stream{};
    std::error_code   ec{};

    wholth_exec_stmt_Result::Data result_data{};

    switch (entry->task)
    {
    case wholth_exec_stmt_Task_DELETE:
    case wholth_exec_stmt_Task_INSERT:
    case wholth_exec_stmt_Task_UPDATE:
    case wholth_exec_stmt_Task_SELECT: {
        sqlw::Transaction transaction{&con};
        uint64_t          sql_output_size = 0;
        ec = transaction(
            entry->sql,
            [&buffer_stream, &result_data, &sql_output_size](
                sqlw::Statement::ExecArgs e) {
                buffer_stream << e.column_value;
                result_data.column_count = e.column_count;
                // todo check int bounds
                sql_output_size += e.column_value.size();
                result_data.sizes.push_back(sql_output_size);
                // result_data.offsets.push_back(e.column_value.size());
            },
            binds);
        break;
    }
    case wholth_exec_stmt_Task_NOOP:
        assert(false);
    }

    // if (sqlw::status::Condition::OK != stmt.status())
    if (sqlw::status::Condition::OK != ec)
    {
        // return wholth::utils::from_error_code(stmt.status());
        return wholth::utils::from_error_code(ec);
    }

    result_data.sql_output = buffer_stream.str();

    if (nullptr != result)
    {
        result->data = std::move(result_data);
    }

    return wholth_Error_OK;
}

constexpr std::string_view _empty_str = "";
extern "C" const wholth_StringView wholth_exec_stmt_Result_at(
    const wholth_exec_stmt_Result* result,
    unsigned long long             row,
    unsigned long long             column)
{
    if (nullptr == result)
    {
        return to_wholth_str_view(_empty_str);
    }

    if (0 == result->data.column_count)
    {
        return to_wholth_str_view(_empty_str);
    }

    if (column >= result->data.column_count)
    {
        return to_wholth_str_view(_empty_str);
    }

    const auto sz = result->data.sizes.size();
    const auto idx = row * result->data.column_count + column;

    if (idx < row * result->data.column_count || idx >= sz)
    {
        return to_wholth_str_view(_empty_str);
    }

    // const auto offset = idx > 0 ? result->data.sizes[idx - 1] : 0;
    if (0 == idx)
    {
        return {
            .data = result->data.sql_output.data(),
            .size = result->data.sizes[idx]};
    }

    const auto offset = result->data.sizes[idx - 1];
    const auto size = result->data.sizes[idx] - offset;

    return {.data = result->data.sql_output.data() + offset, .size = size};
}

extern "C" unsigned long long wholth_exec_stmt_Result_row_count(
    const wholth_exec_stmt_Result* res)
{
    if (nullptr == res || 0 == res->data.column_count)
    {
        return 0;
    }

    return res->data.sizes.size() / res->data.column_count;
}
