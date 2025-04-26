#ifndef WHOLTH_MODEL_FOODS_PAGE_H_
#define WHOLTH_MODEL_FOODS_PAGE_H_

#include "wholth/buffer_view.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/page.hpp"
#include "wholth/swappable.hpp"

namespace wholth::model
{
template <typename T = wholth::entity::shortened::Food>
struct FoodsPage
{
    FoodsPage(){};
    /* char title_buffer[255]{""}; */
    /* size_t title_input_size{0}; */
    wholth::Page page{20};
    /* ui::components::Timer input_timer{}; */
    std::atomic<bool> is_fetching{false};
    /* ExpandedFood expanded_food {}; */
    Swappable<BufferView<std::array<T, 20>>>
        swappable_list;
};
} // namespace wholth::model

#endif // WHOLTH_MODEL_FOODS_PAGE_H_
