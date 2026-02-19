#include "db/db.hpp"
#include "wholth/c/pages/utils.h"
#include "wholth/c/error.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/pages/code.hpp"
#include "wholth/pages/consumption_log.hpp"
#include "wholth/pages/food.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/pages/food_nutrient.hpp"
#include "wholth/pages/nutrient.hpp"
#include "wholth/pagination.hpp"
#include "wholth/status.hpp"
#include "wholth/utils/length_container.hpp"
#include <atomic>
#include <charconv>
#include <cstdint>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <variant>
#include <cassert>

using wholth::pages::internal::PageType;

static std::string breaker = "";
static constexpr auto page_data_buffer(wholth_Page& p) -> std::string&
{
    return std::visit(
        [](auto&& data) -> std::string& {
            using T = std::remove_cvref_t<decltype(data)>;

            if constexpr (not std::is_same_v<T, std::monostate>)
            {
                return data.container.buffer;
            }

            assert(false);

            // I am fighting the compiler here. This code should be unreachable. 
            return breaker;
        },
        p.data);
}

static constexpr auto page_data_container_size(const wholth_Page& p) -> size_t
{
    return std::visit(
        [](auto&& data) -> size_t {
            using T = std::remove_cvref_t<decltype(data)>;

            if constexpr (not std::is_same_v<T, std::monostate>)
            {
                return data.container.view.size();
            }

            assert(false);

            // I am fighting the compiler here. This code should be unreachable. 
            return 0;
        },
        p.data);
}

// get from abstract_page.hpp
// template <typename T>
// concept has_container = requires(T t) { t.container; };
//
// template <typename T>
// concept has_swappable_container =
//     requires(T t) { t.container.swappable_buffer_views; };

auto fill_pages_data_prepare_stmt(sqlw::Statement& stmt, wholth_Page& page)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    switch (static_cast<PageType>(page.data.index()))
    {
    case PageType::FOOD:
        return wholth::pages::prepare_food_stmt(
            stmt, std::get<PageType::FOOD>(page.data).query, page.pagination);
    case PageType::FOOD_NUTRIENT:
        return wholth::pages::prepare_food_nutrient_stmt(
            stmt,
            std::get<PageType::FOOD_NUTRIENT>(page.data).query,
            page.pagination);
    case PageType::NUTRIENT:
        return wholth::pages::prepare_nutrient_stmt(
            stmt,
            std::get<PageType::NUTRIENT>(page.data).query,
            page.pagination);
    case PageType::INGREDIENT:
        return wholth::pages::prepare_ingredient_stmt(
            stmt,
            std::get<PageType::INGREDIENT>(page.data).query,
            page.pagination);
    case PageType::CONSUMPTION_LOG:
        return wholth::pages::prepare_consumption_log_stmt(
            stmt,
            std::get<PageType::CONSUMPTION_LOG>(page.data).query,
            page.pagination);
    case PageType::RECIPE_STEP:
        return wholth::pages::prepare_recipe_step_stmt(
            stmt,
            std::get<PageType::RECIPE_STEP>(page.data).query,
            page.pagination);
    case PageType::NONE:
    case PageType::_COUNT_:
        assert(false);
        break;
    }

    // I am fighting the compiler here. This code should be unreachable. 
    return {
        wholth::entity::LengthContainer{},
        std::make_error_code(std::io_errc::stream)};
}

auto check_pagination_constraints(const wholth_Page& page) -> std::error_code
{
    using SC = wholth::pages::Code;

    const size_t size = page_data_container_size(page);

    // todo return error_code instead of assert.
    assert(
        "Can't fill span, because it's size is smaller "
        "than queruied page size!" &&
        size >= page.pagination.per_page());

    if (size > std::numeric_limits<int>::max())
    {
        return SC::SPAN_SIZE_TOO_BIG;
    }

    const auto offset =
        page.pagination.per_page() * page.pagination.current_page();
    if (offset > std::numeric_limits<int>::max() ||
        (page.pagination.current_page() > 0 &&
         offset < page.pagination.per_page()))
    {
        return SC::QUERY_OFFSET_TOO_BIG;
    }

    return SC::OK;
}

static auto wholth_pages_fetch_exec_stmt(
    sqlw::Statement& stmt,
    uint64_t& new_count,
    uint64_t& new_span_size,
    std::string& result,
    wholth::entity::LengthContainer& lc,
    const wholth::Pagination& pagination) -> std::error_code
{
    const size_t field_count = lc.lengths.capacity() / pagination.per_page();

    assert(field_count > 0);

    std::stringstream buffer_stream;
    uint64_t i = 0;

    const auto ec = stmt([&buffer_stream, &lc, &i, &new_count, field_count](
                             sqlw::Statement::ExecArgs e) {
        i++;

        // first result should always be the total count of elements.
        if (1 == i)
        {
            auto res = std::from_chars(
                e.column_value.data(),
                e.column_value.data() + e.column_value.size(),
                new_count);
            if (res.ec != std::errc())
            {
                // todo ????
                new_count = 0;
            }
        }

        // skip first row because only first column in this row contains
        // payload (and that would be the total element count)
        if (field_count >= i)
        {
            return;
        }

        buffer_stream << e.column_value;

        wholth::utils::add(lc, e.column_value.size());
    });

    if (0 == new_count)
    {
        new_span_size = 0;
    }
    else
    {
        // todo test this
        new_span_size = (i - field_count) / field_count;
    }

    if (sqlw::status::Condition::OK != ec)
    {
        return ec;
    }

    if (buffer_stream.rdbuf()->in_avail() <= 0)
    {
        return wholth::pages::Code::NOT_FOUND;
    }

    result = buffer_stream.str();

    return ec;
}

static std::error_code wholth_pages_fetch_impl(wholth_Page& page)
{
    using SC = wholth::pages::Code;

    auto& connection = db::connection();

    if (sqlw::status::Condition::OK != connection.status())
    {
        // todo panic here maybe?
        return connection.status();
    }

    if (page.pagination.current_page() > std::numeric_limits<int>::max())
    {
        return SC::QUERY_PAGE_TOO_BIG;
    }

    std::error_code ec = check_pagination_constraints(page);

    if (wholth::status::Condition::OK != ec)
    {
        return ec;
    }

    size_t iters = 0;
    std::tuple lc_and_ec{wholth::entity::LengthContainer{0}, std::error_code{}};
    uint64_t new_count = 0;
    uint64_t new_span_size = 0;

    do
    {
        iters++;

        sqlw::Statement stmt{&connection};
        lc_and_ec = fill_pages_data_prepare_stmt(stmt, page);

        if (wholth::status::Condition::OK != std::get<1>(lc_and_ec))
        {
            return std::get<1>(lc_and_ec);
        }

        std::string& buffer = page_data_buffer(page);

        ec = wholth_pages_fetch_exec_stmt(
            stmt,
            new_count,
            new_span_size,
            buffer,
            std::get<0>(lc_and_ec),
            page.pagination);

        page.pagination.count(new_count);
        page.pagination.span_size(new_span_size);
        page.pagination.update();
    } while (iters < 2 && wholth::pages::Code::NOT_FOUND == ec &&
             page.pagination.current_page() > page.pagination.max_page() &&
             page.pagination.skip_to(page.pagination.max_page()));

    assert(new_span_size <= page.pagination.per_page());

    for (size_t element_idx = 0; element_idx < new_span_size; element_idx++)
    {
        std::visit(
            [element_idx, &lc_and_ec](auto&& data) -> void {
                using T = std::remove_cvref_t<decltype(data)>;

                if constexpr (not std::is_same_v<T, std::monostate>)
                {
                    return wholth::pages::hydrate(
                        data, element_idx, std::get<0>(lc_and_ec));
                }

                assert(false);
            },
            page.data);
    }

    return ec;
}

extern "C" wholth_Error wholth_pages_fetch(wholth_Page* const page)
{
    assert("wholth_pages_fetch PRECONDITION" && nullptr != page);

    if (page->data.index() == PageType::NONE)
    {
        // think aboit dis
        return wholth_Error_OK;
    }

    page->state.store(wholth_Page::State::FETCHING, std::memory_order_seq_cst);

    const auto ec = wholth_pages_fetch_impl(*page);

    // @todo learn about memeory order
    page->state.store(wholth_Page::State::OCCUPIED, std::memory_order_seq_cst);

    if (wholth::status::Condition::OK != ec)
    {
        auto& buffer = page_data_buffer(*page);
        buffer = ec.message();

        return {
            .code = static_cast<wholth_ErrorCode>(ec.value()), // uh oh
            // .message = push_error(ec.message()),
            .message =
                {
                    .data = buffer.data(),
                    .size = buffer.size(),
                },
        };
    }

    return wholth_Error_OK;
}

extern "C" bool wholth_pages_advance(wholth_Page* const p, uint64_t by)
{
    if (nullptr == p)
    {
        return false;
    }

    return wholth_Page::State::FETCHING != p->state &&
           p->pagination.advance(by);
}

extern "C" bool wholth_pages_retreat(wholth_Page* const p, uint64_t by)
{
    if (nullptr == p)
    {
        return false;
    }

    return wholth_Page::State::FETCHING != p->state &&
           p->pagination.retreat(by);
}

extern "C" bool wholth_pages_skip_to(wholth_Page* const p, uint64_t page_number)
{
    if (nullptr == p)
    {
        return false;
    }

    return wholth_Page::State::FETCHING != p->state &&
           p->pagination.skip_to(page_number);
}

extern "C" uint64_t wholth_pages_current_page_num(const wholth_Page* const p)
{
    if (nullptr == p)
    {
        return 0;
    }

    return p->pagination.current_page();
}

extern "C" uint64_t wholth_pages_max(const wholth_Page* const p)
{
    if (nullptr == p)
    {
        return 0;
    }

    return p->pagination.max_page();
}

extern "C" uint64_t wholth_pages_count(const wholth_Page* const p)
{
    if (nullptr == p)
    {
        return 0;
    }

    return p->pagination.count();
}

extern "C" uint64_t wholth_pages_span_size(const wholth_Page* const p)
{
    if (nullptr == p)
    {
        return 0;
    }

    return p->pagination.span_size();
}

extern "C" bool wholth_pages_is_fetching(const wholth_Page* const p)
{
    if (nullptr == p)
    {
        return false;
    }

    return wholth_Page::State::FETCHING == p->state;
}

extern "C" uint64_t wholth_pages_array_size(const wholth_Page* const p)
{
    return nullptr == p ? 0 : p->pagination.span_size();
}

extern "C" wholth_Error wholth_pages_at(
    const wholth_Page* const page,
    void* entity,
    uint64_t at)
{
    assert(nullptr != page && "wholth_pages_at - null page");
    // assert(
    //     nullptr != entity &&
    //     "wholth_pages_at - null pointer to pointer of entity");
    // assert(nullptr == *entity && "wholth_pages_at - unresponsible!");
    assert(
        at < page->pagination.span_size() &&
        "wholth_pages_at - overbound access");

    std::visit(
        [at, &entity](auto&& data) -> void {
            using T = std::remove_cvref_t<decltype(data)>;

            if constexpr (not std::is_same_v<T, std::monostate>)
            {
                auto& ent = data.container.view[at];
                *(reinterpret_cast<
                    typename decltype(data.container.view)::value_type*>(
                    entity)) = ent;
                return;
            }

            assert(false);
        },
        page->data);

    // assert(nullptr != *entity && "wholth_pages_at entity sho);
    // if (nullptr == *entity)
    // {
    //     constexpr std::string_view msg =
    //         "wholth_pages_at could not fill entity pointer!";
    //     return wholth_Error{.code = 942, .message{msg.data(), msg.size()}};
    // }

    return wholth_Error_OK;
}

typedef uint8_t ring_pool_idx_t;
static std::atomic<ring_pool_idx_t> g_idx = 0;
static constexpr auto g_size = std::numeric_limits<ring_pool_idx_t>::max();
// static wholth_Page a[g_size];
// constexpr auto s = sizeof(a);

static auto ring_pool() -> std::vector<wholth_Page>&
{
    static std::vector<wholth_Page> g_buffer_pool(g_size);
    return g_buffer_pool;
}

extern "C" auto wholth_pages_new(wholth_Page** page) -> wholth_Error
{
    assert(nullptr != page && "wholth_pages_new - pointer to pointer is null!");
    assert(nullptr == *page && "wholth_pages_new - unresponsible!");

    *page = &ring_pool()[g_idx];

    const auto state = (*page)->state.load(std::memory_order_seq_cst);
    if (wholth_Page::State::FREE != state)
    {
        std::string_view msg =
            "wholth_pages_new - page is not free! use it at your own risk.";
        return wholth_Error{
            // todo enum
            .code = 58,
            .message = {
                .data = msg.data(),
                .size = msg.size(),
            }};
    }

    // (*page)->state.wait(wholth_Page::State::FREE, std::memory_order_seq_cst);
    (*page)->state.store(
        wholth_Page::State::OCCUPIED, std::memory_order_seq_cst);
    (*page)->pagination.reset();

    g_idx = (g_idx + 1) % g_size;

    return wholth_Error_OK;
}

extern "C" auto wholth_pages_del(wholth_Page* page) -> wholth_Error
{
    if (nullptr == page)
    {
        constexpr std::string_view msg = "Buffer is null!";
        // todo add code enum
        return {.code = 1, .message = {.data = msg.data(), .size = msg.size()}};
    }

    page->state.store(wholth_Page::State::FREE, std::memory_order_seq_cst);
    page->state.notify_all();
    page = nullptr;

    return wholth_Error_OK;
}
