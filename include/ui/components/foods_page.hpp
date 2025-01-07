#ifndef UI_COMPONENTS_FOODS_TAB_H_
#define UI_COMPONENTS_FOODS_TAB_H_

#include "ui/style.hpp"
#include "wholth/controller/foods_page.hpp"

namespace ui::components
{
class Foods
{
  public:
    auto render(
        wholth::controller::FoodsPage&,
        const std::chrono::duration<double>& delta,
        const ui::Style&) -> void;
};
} // namespace ui::components

#endif // UI_COMPONENTS_FOODS_TAB_H_
