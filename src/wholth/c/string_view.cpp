#include "wholth/c/string_view.h"

extern "C" wholth_StringView wholth_default_string_view()
{
    return {.data = nullptr, .size = 0};
}
