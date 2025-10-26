#ifndef WHOLTH_C_EM_FOOD_H_
#define WHOLTH_C_EM_FOOD_H_

#include "wholth/c/buffer.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

    wholth_Error wholth_em_food_insert(
        wholth_Food* const food,
        wholth_Buffer* const scratch);
    wholth_Error wholth_em_food_update(
        const wholth_Food* const food,
        wholth_Buffer* const scratch);
    wholth_Error wholth_em_food_delete(
        const wholth_Food* const food,
        wholth_Buffer* const scratch);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EM_FOOD_H_
