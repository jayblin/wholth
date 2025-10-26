#ifndef WHOLTH_APP_C_H_
#define WHOLTH_APP_C_H_

#include <stdbool.h>
#include "wholth/c/forward.h"

// DONT REMOVE THESE INCLUDES
#include "wholth/c/buffer.h"
#include "wholth/c/entity/consumption_log.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/entity/ingredient.h"
#include "wholth/c/entity/locale.h"
#include "wholth/c/entity/nutrient.h"
#include "wholth/c/entity/recipe_step.h"
#include "wholth/c/entity_manager/consumption_log.h"
#include "wholth/c/entity_manager/food.h"
#include "wholth/c/entity_manager/food_nutrient.h"
#include "wholth/c/entity_manager/ingredient.h"
#include "wholth/c/entity_manager/recipe_step.h"
#include "wholth/c/food_details.h"
#include "wholth/c/forward.h"
#include "wholth/c/pages/consumption_log.h"
#include "wholth/c/pages/food.h"
#include "wholth/c/pages/food_nutrient.h"
#include "wholth/c/pages/ingredient.h"
#include "wholth/c/pages/nutrient.h"
#include "wholth/c/pages/recipe_step.h"
#include "wholth/c/pages/utils.h"
#include "wholth/c/string_view.h"

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

    typedef struct wholth_AppSetupArgs
    {
        // todo change db_path to wholth_StringView ???
        const char* db_path;
    } wholth_AppSetupArgs;

    wholth_Error wholth_app_setup(const wholth_AppSetupArgs* const);

    // todo move to em
    void wholth_user_locale_id(const wholth_StringView);

    void eueu(wholth_StringView sv);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // WHOLTH_APP_C_H_
