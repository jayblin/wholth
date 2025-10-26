#ifndef WHOLTH_C_ARRAY_H_
#define WHOLTH_C_ARRAY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define ARRAY_T(type, name) \
    typedef struct name\
    {\
        const struct type* const data;\
        const unsigned long size;\
    } name

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_ARRAY_H_
