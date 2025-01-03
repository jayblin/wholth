#ifndef UI_COMPONENTS_FOOD_EDIT_MODAL_H_
#define UI_COMPONENTS_FOOD_EDIT_MODAL_H_

#include "ui/components/food_list.hpp"
#include "ui/components/search_bap.hpp"
#include "wholth/context.hpp"

namespace ui::components
{
    class FoodEditModal
    {
    public:
      FoodEditModal(wholth::Context& ctx)
          : m_ctx(ctx)
      {
      }

      void render(const std::chrono::duration<double>& delta);

    private:
        wholth::Context& m_ctx;
        /* const Style& m_style; */
        /* sqlw::Connection& m_con; */
        
        char m_title_buffer[255] {""};
        char m_description_buffer[1024] {""};
        char m_recipe_buffer[1024 * 10] {""};
        uint8_t m_hours {0};
        uint8_t m_minutes {0};
        uint8_t m_seconds {0};

        SearchBar<1024> m_search_bar {"Search"};
        /* FoodList<5> m_ingredient_list {m_ctx}; */
    };
}

#endif // UI_COMPONENTS_FOOD_EDIT_MODAL_H_
