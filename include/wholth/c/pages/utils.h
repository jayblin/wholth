#ifndef WHOLTH_C_PAGES_UTILS_H_
#define WHOLTH_C_PAGES_UTILS_H_

#include "wholth/c/error.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct wholth_Page_t;
    typedef struct wholth_Page_t wholth_Page;

    wholth_Error wholth_pages_new(wholth_Page**);
    wholth_Error wholth_pages_del(wholth_Page*);

    wholth_Error wholth_pages_fetch(wholth_Page* const);
    typedef uint64_t wholth_PageStep;
    typedef uint64_t wholth_PageNumber;

    bool wholth_pages_advance(wholth_Page* const, wholth_PageStep by);
    bool wholth_pages_retreat(wholth_Page* const, wholth_PageStep by);
    bool wholth_pages_skip_to(
        wholth_Page* const,
        wholth_PageNumber page_number);

    wholth_PageNumber wholth_pages_current_page_num(const wholth_Page* const);
    wholth_PageNumber wholth_pages_max(const wholth_Page* const);
    uint64_t wholth_pages_count(const wholth_Page* const);
    uint64_t wholth_pages_span_size(const wholth_Page* const);
    bool wholth_pages_is_fetching(const wholth_Page* const);

    uint64_t wholth_pages_array_size(const wholth_Page* const);
    wholth_Error wholth_pages_at(
        const wholth_Page* const,
        void* entity,
        uint64_t at);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_PAGES_UTILS_H_
