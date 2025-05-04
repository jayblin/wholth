#ifndef WHOLTH_CONTEXT_H_
#define WHOLTH_CONTEXT_H_

#include "sqlw/connection.hpp"
#include <concepts>
#include <tuple>
#include <vector>

// todo move to other namespace
namespace wholth
{

// todo remake to struct
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
        this->locale_id = std::move(other.locale_id);

        return *this;
    };

    sqlw::Connection connection;

    // @todo Rename to handle_tasks???
    /* template <typename... M, typename... C> */
    template <typename... C>
    auto update(
        const std::chrono::duration<double>& delta,
        std::tuple<C...>& controllers) -> void;


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
    // todo move this outside of context
    std::vector<std::string> sql_errors;
    std::string exception_message;
    std::string locale_id{"1"};

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
};

} // namespace wholth

namespace wholth::concepts
{
    template <typename T>
    concept is_context_aware = requires(T t)
    {
        t.ctx;
        requires std::same_as<decltype(t.ctx), const wholth::Context&>;
    };
};

#endif // WHOLTH_CONTEXT_H_
