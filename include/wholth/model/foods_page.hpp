#ifndef WHOLTH_MODEL_FOODS_PAGE_H_
#define WHOLTH_MODEL_FOODS_PAGE_H_

#include "wholth/context.hpp"
#include "wholth/hydrate.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/page.hpp"

namespace wholth::model
{

struct FoodsPage
{
    const wholth::Context& ctx;
    wholth::Pagination pagination;
    std::atomic<bool> is_fetching{false};
    std::string title;
    std::string ingredients;
};

template <wholth::concepts::is_food T, size_t Size = 20>
using FoodsContainer = SwappableBufferViewsAwareContainer<T, Size>;

} // namespace wholth::model

#endif // WHOLTH_MODEL_FOODS_PAGE_H_
