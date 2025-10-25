#include "wholth/status.hpp"
#include "db/db.hpp"
#include "wholth/pages/code.hpp"
#include <cassert>

namespace wholth::status
{
struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "database";
    }

    std::string message(int ev) const override final
    {
        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        /* case Code::SQL_STATEMENT_ERROR: */
        /*     return "error in SQL statement"; */
        // case Code::ENTITY_NOT_FOUND:
        // return "entity was not found";
        case Code::INVALID_LOCALE_ID:
            return "provided ill-formed locale id";
        // case Code::INVALID_FOOD_ID:
        //     return "provided ill-formed food id";
        // case Code::EMPTY_FOOD_TITLE:
        //     return "provided empty food title";
        // case Code::UNCHANGED_FOOD_TITLE:
        //     return "provided same food title";
        // case Code::UNCHANGED_FOOD_DESCRIPTION:
        //     return "provided same food description";
        case Code::INVALID_DATE:
            return "provided ill-formed date";
        case Code::INVALID_MASS:
            return "provided ill-formed mass";
        }

        return "(unrecognized error)";
    }
};

struct ConditionCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "wholth condition";
    }

    std::string message(int ev) const override final
    {
        switch (static_cast<Condition>(ev))
        {
        case Condition::ERROR:
            return "error";
        case Condition::OK:
            return "no error";
        }

        return "(unrecognized error)";
    }

    bool equivalent(const std::error_code& ec, int cond)
        const noexcept override final
    {
        const std::error_category& pages_error_category =
            std::error_code{wholth::pages::Code{}}.category();

        if (ec.category() == pages_error_category)
        {
            return wholth::pages::Code::OK ==
                   static_cast<wholth::pages::Code>(ec.value());
        }

        const std::error_category& wholth_error_category =
            std::error_code{Code{}}.category();

        if (ec.category() != wholth_error_category)
        {
            return db::status::Condition::OK == ec;
        }

        // todo WTF is this?
        // test this shit
        switch (static_cast<Condition>(cond))
        {
        case Condition::OK:
            return static_cast<Code>(ec.value()) == wholth::status::Code::OK;
        case Condition::ERROR:
            return static_cast<Code>(ec.value()) != wholth::status::Code::OK;
        default:
            return false;
        }
    }
};

const ErrorCategory error_category{};
const ConditionCategory condition_category{};
} // namespace wholth::status

std::error_code wholth::status::make_error_code(wholth::status::Code e)
{
    return {static_cast<int>(e), wholth::status::error_category};
}

std::error_condition wholth::status::make_error_condition(
    wholth::status::Condition e)
{
    return {static_cast<int>(e), wholth::status::condition_category};
}
