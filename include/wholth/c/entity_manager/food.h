#ifndef WHOLTH_C_EM_FOOD_H_
#define WHOLTH_C_EM_FOOD_H_

#include "wholth/c/buffer.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum wholth_em_food_Code
    {
        wholth_em_food_Code_FIRST_ = 100,
        wholth_em_food_Code_BAD_LOCALE_ID,
        wholth_em_food_Code_DUPLICATE_ENTRY,
        wholth_em_food_Code_LAST_,
        wholth_em_food_Code_COUNT_ =
            wholth_em_food_Code_LAST_ - wholth_em_food_Code_FIRST_ - 1,
    };

    wholth_Error wholth_em_food_insert(
        wholth_Food* const   food,
        wholth_StringView    locale_id,
        wholth_Buffer* const scratch);
    wholth_Error wholth_em_food_update(
        const wholth_Food* const food,
        wholth_StringView        locale_id,
        wholth_Buffer* const     scratch);
    wholth_Error wholth_em_food_delete(
        const wholth_Food* const food,
        wholth_Buffer* const     scratch);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EM_FOOD_H_
