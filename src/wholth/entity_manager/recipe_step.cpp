#include "wholth/entity_manager/recipe_step.hpp"

namespace wholth::entity_manager::recipe_step
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "entity_manager::recipe_step";
    }

    std::string message(int ev) const override final
    {
        using Code = wholth::entity_manager::recipe_step::Code;

        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case Code::RECIPE_STEP_ID_MISMATCH:
            return "RECIPE_STEP_ID_MISMATCH";
        case Code::RECIPE_STEP_INSERT_FAILURE:
            return "RECIPE_STEP_INSERT_FAILURE";
        case Code::RECIPE_STEP_INSERT_FAILURE_NO_RET_ID:
            return "RECIPE_STEP_INSERT_FAILURE_NO_RET_ID";
        case Code::RECIPE_STEP_UPDATE_FAILURE:
            return "RECIPE_STEP_UPDATE_FAILURE";
        case Code::RECIPE_STEP_LOCALISATION_UPDATE_FAILURE:
            return "RECIPE_STEP_LOCALISATION_UPDATE_FAILURE";
        case Code::RECIPE_STEP_ALREADY_EXISTS:
            return "RECIPE_STEP_ALREADY_EXISTS";
        case Code::RECIPE_STEP_INVALID_ID:
            return "RECIPE_STEP_INVALID_ID";
        case Code::RECIPE_STEP_NULL:
            return "RECIPE_NULL_STEP";
        case Code::RECIPE_STEP_NULL_TIME:
            return "RECIPE_STEP_NULL_TIME";
        case Code::RECIPE_STEP_NULL_DESCRIPTION:
            return "RECIPE_STEP_NULL_DESCRIPTION";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory error_category{};
} // namespace wholth::entity_manager::recipe_step

std::error_code wholth::entity_manager::recipe_step::make_error_code(
    wholth::entity_manager::recipe_step::Code e)
{
    return {
        static_cast<int>(e),
        wholth::entity_manager::recipe_step::error_category};
}
