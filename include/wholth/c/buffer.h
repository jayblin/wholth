#ifndef WHOLTH_C_BUFFER_H_
#define WHOLTH_C_BUFFER_H_

#include "wholth/c/forward.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct wholth_Buffer_t;
    typedef struct wholth_Buffer_t wholth_Buffer;

    /**
     * Returns a buffer that should not be considered valid after caller's scope
     * ends. Pointer should not be freed.
     */
    wholth_Buffer* wholth_buffer_ring_pool_element();
    void wholth_buffer_move_data_to(wholth_Buffer* const, void* data);
    const wholth_StringView wholth_buffer_view(const wholth_Buffer* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_BUFFER_H_
