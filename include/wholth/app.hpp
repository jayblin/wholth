#ifndef WHOLTH_APP_H_
#define WHOLTH_APP_H_

#include "wholth/context.hpp"
#include <tuple>

namespace wholth::app
{
/* template <typename... C, typename... M> */
void setup(
    std::string_view db_path,
    wholth::Context&
    /* std::tuple<C...> controllers, */
    /* std::tuple<M...> models */
);
}

#endif // WHOLTH_APP_H_
