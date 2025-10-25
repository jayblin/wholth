#include "wholth/controller/update_food.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "wholth/status.hpp"
#include "wholth/utils/is_valid_id.hpp"

auto wholth::controller::update(
    const wholth::entity::Food& food,
    sqlw::Connection& db_con,
    const wholth::Context& ctx,
    const wholth::entity::food::Details& details) -> std::error_code
{
    if (!is_valid_id(food.id))
    {
        return wholth::status::Code::INVALID_FOOD_ID;
    }

    if (!is_valid_id(ctx.locale_id))
    {
        return wholth::status::Code::INVALID_LOCALE_ID;
    }

    const std::string now = wholth::utils::current_time_and_date();

    const std::array<sqlw::Statement::bindable_t, 4> params{{
        {food.title, sqlw::Type::SQL_TEXT},
        {details.description, sqlw::Type::SQL_TEXT},
        {ctx.locale_id, sqlw::Type::SQL_INT},
        {food.id, sqlw::Type::SQL_INT},
    }};

    const auto ec = sqlw::Transaction{&db_con}(
        R"sql(
        INSERT INTO food_localisation (food_id, locale_id, title, description) 
        VALUES (?4, ?3, lower(trim(?1)), ?2)
        ON CONFLICT DO UPDATE SET
            title = lower(trim(?1)),
            description = ?2
        WHERE locale_id = ?3 AND food_id = ?4
        )sql",
        params);

    return ec;
}
