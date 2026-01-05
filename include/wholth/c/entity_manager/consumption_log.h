#ifndef WHOLTH_C_EM_CONSUMPTION_LOG_H_
#define WHOLTH_C_EM_CONSUMPTION_LOG_H_

#include "wholth/c/buffer.h"
#include "wholth/c/entity/consumption_log.h"
#include "wholth/c/entity/user.h"
#include "wholth/c/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum wholth_em_consumption_log_Code
    {
        CONSUMPTION_LOG_NULL = 101,
        CONSUMPTION_LOG_INVALID_ID,
        CONSUMPTION_LOG_INVALID_FOOD_ID,
        CONSUMPTION_LOG_INVALID_MASS,
        CONSUMPTION_LOG_INVALID_DATE,
    };

    wholth_Error wholth_em_consumption_log_insert(
        wholth_ConsumptionLog* const,
        wholth_User* const,
        wholth_Buffer* const);

    wholth_Error wholth_em_consumption_log_update(
        const wholth_ConsumptionLog* const,
        wholth_User* const,
        wholth_Buffer* const);

    wholth_Error wholth_em_consumption_log_delete(
        const wholth_ConsumptionLog* const,
        wholth_User* const,
        wholth_Buffer* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EM_CONSUMPTION_LOG_H_
