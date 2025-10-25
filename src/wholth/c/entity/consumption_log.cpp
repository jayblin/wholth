#include "wholth/c/entity/consumption_log.h"

wholth_ConsumptionLog wholth_entity_consumption_log_init()
{
    return {
        .id = wholth_StringView_default,
        .food_id = wholth_StringView_default,
        .mass = wholth_StringView_default,
        .consumed_at = wholth_StringView_default,
        .food_title = wholth_StringView_default,
    };
}
