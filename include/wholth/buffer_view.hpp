#ifndef WHOLTH_BUFFER_VIEW_H_
#define WHOLTH_BUFFER_VIEW_H_

#include <string>

namespace wholth
{
template <typename T>
struct BufferView
{
    T view;
    std::string buffer = {};
};
} // namespace wholth

#endif // WHOLTH_BUFFER_VIEW_H_
