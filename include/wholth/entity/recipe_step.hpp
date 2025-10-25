#ifndef WHOLTH_ENTITY_RECIPE_STEP_H_
#define WHOLTH_ENTITY_RECIPE_STEP_H_

#include <string_view>

namespace wholth::entity::recipe_step
{
	typedef std::string_view id_t;
	typedef std::string_view time_t;
	// typedef std::string_view note_t;
	typedef std::string_view description_t;
};

namespace wholth::entity
{
    struct RecipeStep
    {
        recipe_step::id_t id {""};
        recipe_step::time_t time {""};
        recipe_step::description_t description {""};
    };
}

#endif // WHOLTH_ENTITY_RECIPE_STEP_H_
