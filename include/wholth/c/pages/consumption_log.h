#ifndef WHOLTH_C_PAGES_CONSUMPTION_LOG_H_
#define WHOLTH_C_PAGES_CONSUMPTION_LOG_H_

#include "wholth/c/array.h"
#include "wholth/c/entity/consumption_log.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif
    ARRAY_T(wholth_ConsumptionLog_t, wholth_ConsumptionLogArray);

    wholth_Page* wholth_pages_consumption_log(uint64_t per_page, bool reset);
    const wholth_ConsumptionLogArray wholth_pages_consumption_log_array(
        const wholth_Page* const);
    wholth_Error wholth_pages_consumption_log_period(
        wholth_Page* const,
        wholth_StringView from,
        wholth_StringView to);
    wholth_Error wholth_pages_consumption_log_user_id(
        wholth_Page* const,
        wholth_StringView user_id);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_PAGES_CONSUMPTION_LOG_H_
