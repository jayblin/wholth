#ifndef WHOLTH_C_PAGES_UTILS_H_
#define WHOLTH_C_PAGES_UTILS_H_

#include "wholth/c/forward.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct wholth_Page_t;
    typedef struct wholth_Page_t wholth_Page;

    wholth_Error wholth_pages_fetch(wholth_Page* const);

    bool wholth_pages_advance(wholth_Page* const, uint64_t by);
    bool wholth_pages_retreat(wholth_Page* const, uint64_t by);
    bool wholth_pages_skip_to(wholth_Page* const, uint64_t page_number);

    uint64_t wholth_pages_current_page_num(const wholth_Page* const);
    uint64_t wholth_pages_max(const wholth_Page* const);
    uint64_t wholth_pages_count(const wholth_Page* const);
    uint64_t wholth_pages_span_size(const wholth_Page* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_PAGES_UTILS_H_
