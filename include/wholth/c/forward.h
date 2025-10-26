#ifndef WHOLTH_C_FORWARD_H_
#define WHOLTH_C_FORWARD_H_

#include <stdbool.h>

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */

#include <cstddef>
extern "C"
{

#endif

#ifndef NULL
#define NULL 0
#endif

    // typedef wholth_StringView wholth_ErrorMessage;

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // WHOLTH_C_FORWARD_H_
