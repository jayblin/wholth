#ifndef WHOLTH_CONTEXT_H_
#define WHOLTH_CONTEXT_H_

#include "sqlw/connection.hpp"
#include "ui/components/timer.hpp"
#include "ui/style.hpp"
#include "db/db.hpp"
#include "entity/locale.hpp"
#include "utils/serializer.hpp"
#include "wholth/controller/foods_page.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list/food.hpp"
#include "wholth/model/foods_page.hpp"
#include "wholth/task_list.hpp"
#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <span>
#include <thread>
#include <vector>

// todo move to other namespace
namespace wholth
{

// @todo
// - do i even need locale_id as a member?
class Context
{
  public:
    sqlw::Connection connection;
    ui::Style style{};
    /* char foods_title_buffer[255] {""}; */
    /* size_t foods_title_input_size {0}; */

    auto update(const std::chrono::duration<double>& delta) -> void;

    auto locale_id(std::string new_locale_id) -> void;
    auto locale_id() const -> std::string_view
    {
        return m_locale_id;
    }

    // __________INGREDIENTS__________
    /* Swappable<Page<wholth::entity::shortened::Food, 5>> m_ingredient_pages;
     */
    /* char m_ingredient_title_buffer[255] {""}; */

    /* std::array< */
    /*     std::array<wholth::entity::shortened::Food, 5>, */
    /*     2 */
    /* > ingredients; */
    /* std::string ingredients_buffer; */
    /* uint64_t ingredients_cur_page; */
    /* uint64_t ingredients_max_page; */

    db::migration::MigrateResult migrate_result;
    std::vector<std::string> sql_errors;
    std::string exception_message;

    template <typename Serializer>
    auto serialize(Serializer& serializer) const noexcept -> void
    {
        serializer << NVP(connection.status())
                   /* << NVP(style) */  // this segfaults!
                   << NVP(m_locale_id)  //
                   << NVP(m_task_list)      //
                   << NVP(m_foods_page) //
                   /* << NVP(m_task_mutex) */
                   << NVP(migrate_result) << NVP(sql_errors)
                   << NVP(exception_message);
    }
    wholth::controller::FoodsPage foods_page_ctrl{m_foods_page, m_task_list};

  private:
    std::string m_locale_id{"1"};
    std::mutex m_task_mutex;
    wholth::model::FoodsPage m_foods_page{};
    TaskList m_task_list{};
};
} // namespace wholth

#endif // WHOLTH_CONTEXT_H_
