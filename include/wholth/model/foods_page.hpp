#ifndef WHOLTH_MODEL_FOODS_PAGE_H_
#define WHOLTH_MODEL_FOODS_PAGE_H_

#include "ui/components/timer.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/page.hpp"
#include "wholth/swappable.hpp"

namespace wholth::model
{
struct FoodsPage
{
    char title_buffer[255]{""};
    size_t title_input_size{0};
    wholth::Page page{20};
    // @todo move timer out of components
    ui::components::Timer input_timer{};
    std::atomic<bool> is_fetching{false};
    ExpandedFood expanded_food;
    Swappable<BufferView<std::array<wholth::entity::shortened::Food, 20>>>
        swappable_list;
};
} // namespace wholth::model

#endif // WHOLTH_MODEL_FOODS_PAGE_H_
