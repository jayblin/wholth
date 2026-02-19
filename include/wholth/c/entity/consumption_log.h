#ifndef WHOLTH_C_ENTITY_CONSUMPTION_LOG_H_
#define WHOLTH_C_ENTITY_CONSUMPTION_LOG_H_

#include "wholth/c/string_view.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct wholth_ConsumptionLog_t
    {
        wholth_StringView id;
        wholth_StringView food_id;
        wholth_StringView mass;
        wholth_StringView nutrient_amount;
        wholth_StringView consumed_at;
        wholth_StringView food_title;
    } wholth_ConsumptionLog;

    wholth_ConsumptionLog wholth_entity_consumption_log_init();

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_ENTITY_CONSUMPTION_LOG_H_
