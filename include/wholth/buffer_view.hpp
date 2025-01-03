#ifndef WHOLTH_BUFFER_VIEW_H_
#define WHOLTH_BUFFER_VIEW_H_

#include "utils/serializer.hpp"
#include <string>

namespace wholth
{
template <typename T> struct BufferView
{
    T view;
    std::string buffer;

    template <typename Serializer>
    auto serialize(Serializer& serializer) const noexcept -> void
    {
        serializer << NVP(view) << NVP(buffer);
    }
};
} // namespace wholth

#endif // WHOLTH_BUFFER_VIEW_H_
