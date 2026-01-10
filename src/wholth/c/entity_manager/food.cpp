#include "wholth/c/entity/food.h"
#include "db/db.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity_manager/food.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/food.hpp"
#include "wholth/error.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/prepend_sql_params.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <span>
#include <sstream>

using wholth::c::internal::ec_to_error;
using wholth::entity_manager::food::Code;
using wholth::utils::current_time_and_date;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;
using bind_t = sqlw::Statement::bindable_t;

template <>
struct wholth::error::is_error_code_enum<wholth_em_food_Code> : std::true_type
{
};

static auto& g_context = wholth::c::internal::global_context();

// todo: move erro_code stuff to wholth/entity/manager/food.cpp
namespace wholth::entity_manager::food
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "entity_manager::food";
    }

    std::string message(int ev) const override final
    {
        using Code = wholth::entity_manager::food::Code;

        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case food::Code::FOOD_INVALID_ID:
            return "INVALID_FOOD_ID";
        case food::Code::FOOD_NULL:
            return "NULL_FOOD";
        // case food::Code::NULL_DETAILS:
        //     return "NULL_DETAILS";
        case Code::FOOD_NULL_TITLE:
            return "FOOD_NULL_TITLE";
        case Code::FOOD_NULL_DESCRIPTION:
            return "FOOD_NULL_DESCRIPTION";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory error_category{};
} // namespace wholth::entity_manager::food

std::error_code wholth::entity_manager::food::make_error_code(
    wholth::entity_manager::food::Code e)
{
    return {static_cast<int>(e), wholth::entity_manager::food::error_category};
}

extern "C" auto wholth_em_food_insert(
    wholth_Food* const food,
    wholth_StringView locale_id,
    wholth_Buffer* const buffer) -> wholth_Error
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == food)
    {
        return ec_to_error(Code::FOOD_NULL, buffer);
    }

    auto _locale_id = to_string_view(locale_id);
    if (!is_valid_id(_locale_id))
    {
        return ec_to_error(wholth_em_food_Code::FOOD_EM_BAD_LOCALE_ID, buffer);
    }

    if (nullptr == food->title.data || 0 == food->title.size)
    {
        return ec_to_error(Code::FOOD_NULL_TITLE, buffer);
    }

    const std::string now = current_time_and_date();

    std::string result_id;
    bind_t bind_now{now, sqlw::Type::SQL_TEXT};
    bind_t bind_locale_id{_locale_id, sqlw::Type::SQL_INT};
    bind_t bind_title{to_string_view(food->title), sqlw::Type::SQL_TEXT};
    bind_t bind_description{
        to_string_view(food->description),
        (nullptr == food->description.data || 0 == food->description.size)
            ? sqlw::Type::SQL_NULL
            : sqlw::Type::SQL_TEXT};

    std::error_code ec{};

    std::string rowid = "";
    ec = sqlw::Statement{&db::connection()}(
        "SELECT fl_fts5.rowid "
        "FROM food_localisation_fts5 fl_fts5 "
        "INNER JOIN food_localisation fl "
        " ON fl_fts5.rowid = fl.fl_fts5_rowid "
        "    AND fl.locale_id = ?2 "
        "WHERE food_localisation_fts5 MATCH ?1",
        [&rowid](auto e) { rowid = e.column_value; },
        std::array<bind_t, 2>{bind_title, bind_locale_id});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    if (!rowid.empty())
    {
        return ec_to_error(FOOD_EM_DUPLICATE_ENTRY, buffer);
    }

    const std::array<bind_t, 4> params{{
        bind_now,

        bind_title,
        bind_description,

        bind_locale_id,
    }};

    ec = sqlw::Transaction{&db::connection()}(
        R"sql(
        CREATE TEMP TABLE vars (id,value);

        INSERT INTO food (created_at)
            VALUES (?1);
        INSERT INTO vars VALUES ('food_id', last_insert_rowid());

        INSERT INTO food_localisation_fts5 (title, description)
            VALUES (trim(?1), ?2);

        INSERT INTO food_localisation (food_id, locale_id, fl_fts5_rowid) 
            SELECT value, ?1, last_insert_rowid()
            FROM vars
            WHERE id = 'food_id'
        RETURNING food_id
        )sql",
        [&](auto e) { result_id = e.column_value; },
        params);

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    wholth_buffer_move_data_to(buffer, &result_id);

    food->id = wholth_buffer_view(buffer);
    // wholth_StringView{.data = result_id.data(), .size = result_id.size()};

    return wholth_Error_OK;
}

extern "C" auto wholth_em_food_update(
    const wholth_Food* const food,
    wholth_StringView locale_id,
    wholth_Buffer* const buffer) -> wholth_Error
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == food)
    {
        return ec_to_error(Code::FOOD_NULL, buffer);
    }

    auto _locale_id = to_string_view(locale_id);
    if (!is_valid_id(_locale_id))
    {
        return ec_to_error(wholth_em_food_Code::FOOD_EM_BAD_LOCALE_ID, buffer);
    }

    const auto food_id = to_string_view(food->id);

    if (!is_valid_id(food_id))
    {
        return wholth::c::internal::ec_to_error(Code::FOOD_INVALID_ID, buffer);
    }

    std::error_code ec;

    sqlw::Statement stmt{&db::connection()};

    // std::vector<bind_t> params;
    // params.reserve(4);
    // params.emplace_back(food_id, sqlw::Type::SQL_INT);
    // params.emplace_back(_locale_id, sqlw::Type::SQL_INT);

    std::string rowid = "";
    ec = stmt(
        "SELECT fl_fts5_rowid "
        "FROM food_localisation "
        "WHERE food_id = ?1 AND locale_id = ?2",
        [&rowid](auto e) { rowid = e.column_value; },
        // std::span<bind_t>(params).subspan(0, 2));
        std::array<bind_t, 2>{{
            {food_id, sqlw::Type::SQL_INT},
            {_locale_id, sqlw::Type::SQL_INT},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    // if (rowid.empty())
    // {
    std::vector<bind_t> params{};

    const bind_t bind_title{
        to_string_view(food->title),
        nullptr == food->title.data ? sqlw::Type::SQL_NULL
                                    : sqlw::Type::SQL_TEXT};
    const bind_t bind_description{
        to_string_view(food->description),
        nullptr == food->description.data ? sqlw::Type::SQL_NULL
                                          : sqlw::Type::SQL_TEXT};

    if (rowid.empty())
    {
        params = {{
            {food_id, sqlw::Type::SQL_INT},
            {_locale_id, sqlw::Type::SQL_INT},

            bind_title,
            bind_description,

            {food_id, sqlw::Type::SQL_INT},
            {_locale_id, sqlw::Type::SQL_INT},
        }};
    }
    else
    {
        params = {{
            {food_id, sqlw::Type::SQL_INT},
            {_locale_id, sqlw::Type::SQL_INT},

            bind_title,
            bind_description,
            {rowid, sqlw::Type::SQL_INT},

            {food_id, sqlw::Type::SQL_INT},
            {_locale_id, sqlw::Type::SQL_INT},
            {rowid, sqlw::Type::SQL_INT},
        }};
    }

    // ec = t("UPDATE food_localisation_fts5  ", params);
    constexpr std::string_view sql_insert_fts5 =
        "INSERT INTO food_localisation_fts5 (title, description) "
        "   VALUES (trim(?1), ?2); ";
    constexpr std::string_view sql_update_fts5 =
        "UPDATE food_localisation_fts5 "
        " SET "
        " title = CASE WHEN ?1 IS NULL THEN title ELSE trim(?1) END, "
        " description = CASE WHEN ?2 IS NULL THEN description ELSE ?2 END "
        "WHERE rowid = ?3;";
    constexpr std::string_view sql =
        "INSERT INTO food_localisation (food_id, locale_id) "
        " VALUES (?1, ?2) "
        " ON CONFLICT (food_id, locale_id) DO UPDATE "
        " SET fl_fts5_rowid = NULL; "

        " {0} "

        "UPDATE food_localisation "
        "SET fl_fts5_rowid = {1} "
        "WHERE food_id = ?1 AND locale_id = ?2 ";
    sqlw::Transaction t{&db::connection()};
    ec =
        t(fmt::format(
              sql,
              rowid.empty() ? sql_insert_fts5 : sql_update_fts5,
              rowid.empty() ? "last_insert_rowid()" : "?3"),
          params);
    // }
    // else
    // {
    //
    //     std::stringstream ss;
    //     ss << "UPDATE food_localisation_fts5 SET ";
    //
    //     wholth::utils::prepend_sql_params(
    //         std::array<wholth::utils::extended_param_t, 2>{{
    //             {"title", food->title, sqlw::Type::SQL_TEXT, "trim(?{})"},
    //             {"description", food->description, sqlw::Type::SQL_TEXT, {}},
    //         }},
    //         params,
    //         ss);
    //
    //     ss << " WHERE rowid IN "
    //           "(SELECT fl_fts5_rowid "
    //           " FROM food_localisation "
    //           " WHERE food_id = ?1 AND locale_id = ?2)";
    //
    //     sqlw::Transaction transaction{&db::connection()};
    //     ec = transaction(ss.str(), params);
    // }

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}

extern "C" auto wholth_em_food_delete(
    const wholth_Food* const food,
    wholth_Buffer* const buffer) -> wholth_Error
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == food)
    {
        return ec_to_error(Code::FOOD_NULL, buffer);
    }

    const auto id = to_string_view(food->id);

    if (!is_valid_id(id))
    {
        return wholth::c::internal::ec_to_error(Code::FOOD_INVALID_ID, buffer);
    }

    const auto ec = sqlw::Transaction{&db::connection()}(
        "DELETE FROM food WHERE id = ?1",
        std::array<sqlw::Statement::bindable_t, 1>{
            {{id, sqlw::Type::SQL_INT}}});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}
