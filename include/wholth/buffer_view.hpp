#ifndef WHOLTH_BUFFER_VIEW_H_
#define WHOLTH_BUFFER_VIEW_H_

#include <string>

namespace wholth::concepts
{
    template <typename T>
    concept is_buffer_view = requires (T t)
    {
        /* T::value_t; */
        t.view;
        t.buffer;
    };
};

namespace wholth
{

template <typename T>
struct BufferView
{
    using value_t = T;

    T view;
    std::string buffer = {};
};

} // namespace wholth

#endif // WHOLTH_BUFFER_VIEW_H_
