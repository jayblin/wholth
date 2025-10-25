#include "wholth/entity_manager/nutrient.hpp"

namespace wholth::entity_manager::nutrient
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "entity_manager::nutrient";
    }

    std::string message(int ev) const override final
    {
        using Code = wholth::entity_manager::nutrient::Code;

        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case Code::NUTRIENT_INVALID_ID:
            return "NUTRIENT_INVALID_ID";
        case Code::NUTRIENT_INVALID_VALUE:
            return "NUTRIENT_INVALID_VALUE";
        case Code::NUTRIENT_NULL:
            return "NUTRIENT_NULL";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory error_category{};
} // namespace wholth::entity_manager::nutrient

std::error_code wholth::entity_manager::nutrient::make_error_code(
    wholth::entity_manager::nutrient::Code e)
{
    return {
        static_cast<int>(e), wholth::entity_manager::nutrient::error_category};
}
