#ifndef WHOLTH_CONTEXT_H_
#define WHOLTH_CONTEXT_H_

#include "sqlw/connection.hpp"
#include "db/db.hpp"
#include "wholth/scheduler.hpp"
#include <algorithm>
#include <mutex>
#include <tuple>
#include <vector>

// todo move to other namespace
namespace wholth
{

// @todo
// - do i even need locale_id as a member?
// - make members private?
/* template <typename F = wholth::entity::shortened::Food> */
class Context
{
  public:
    Context(){};

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    Context(Context&& other) {
        *this = std::move(other);
    };
    Context& operator=(Context&& other) {
        if (&other == this) {
            return other;
        }

        this->exception_message = std::move(other.exception_message);
        this->sql_errors = std::move(other.sql_errors);
        this->connection = std::move(other.connection);
        this->db_path = std::move(other.db_path);
        this->m_locale_id = std::move(other.m_locale_id);
        /* this->scheduler */
        return *this;
    };

    sqlw::Connection connection;
    // @todo убрать из контекста
    /* ui::Style style{}; */
    /* char foods_title_buffer[255] {""}; */
    /* size_t foods_title_input_size {0}; */

    // @todo Rename to handle_tasks???
    /* template <typename... M, typename... C> */
    template <typename... C>
    auto update(
        const std::chrono::duration<double>& delta,
        std::tuple<C...>& controllers) -> void;

    auto locale_id(std::string new_locale_id) -> void;
    auto locale_id() const -> std::string_view
    {
        return m_locale_id;
    }

    std::string_view db_path;

    /* auto task_list() const -> const TaskList& */
    /* { */
    /*     return m_task_list; */
    /* } */

    /* auto foods_page() const -> const wholth::model::FoodsPage& */
    /* { */
    /*     return m_foods_page; */
    /* } */

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

    /* db::migration::MigrateResult migrate_result; */
    std::vector<std::string> sql_errors;
    std::string exception_message;

    /* wholth::controller::FoodsPage foods_page_ctrl{m_foods_page, m_task_list};
     */
    /* wholth::controller::FoodsPage foods_page_ctrl{m_task_list}; */
    // @todo make this member const ???
    /* std::tuple< */
    /*     wholth::controller::FoodsPage, */
    /*     wholth::controller::ExpandedFood */
    /* > controllers {}; */

    /* std::tuple< */
    /*     wholth::model::FoodsPage<>, */
    /*     wholth::model::ExpandedFood */
    /* > models {}; */

    /* wholth::Scheduler scheduler{}; */

  private:
    std::string m_locale_id{"1"};
    /* std::mutex m_task_mutex; */

    /* wholth::model::FoodsPage m_foods_page{}; */
    /* TaskList m_task_list{}; */
};
} // namespace wholth

#endif // WHOLTH_CONTEXT_H_
