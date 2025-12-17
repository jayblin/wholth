#include "wholth/c/buffer.h"
#include "wholth/c/error.h"
#include <atomic>
#include <vector>
#include <string>

struct wholth_Buffer_t
{
    enum class State : int
    {
        FREE = 0,
        OCCUPIED,
    };

    std::atomic<State> state{State::FREE};
    std::string data{};
};

typedef uint8_t ring_pool_idx_t;
static std::atomic<ring_pool_idx_t> g_idx = 0;
static constexpr auto g_size = std::numeric_limits<ring_pool_idx_t>::max();

static auto ring_pool() -> std::vector<wholth_Buffer>&
{
    static std::vector<wholth_Buffer> g_buffer_pool(g_size);
    return g_buffer_pool;
}

extern "C" auto wholth_buffer_ring_pool_element() -> wholth_Buffer*
{
    wholth_Buffer* buf = nullptr;
    wholth_buffer_new(&buf);
    return buf;
}

extern "C" auto wholth_buffer_new(wholth_Buffer** buf) -> wholth_Error
{
    assert(
        nullptr != buf &&
        nullptr == *buf &&
        "wholth_buffer_new - unresponsible!");

    *buf = &ring_pool()[g_idx];
    auto err = wholth_Error_OK;

    if (wholth_Buffer::State::FREE != (*buf)->state)
    {
        constexpr std::string_view msg = "Buffer is not free!";
        // todo add code enum
        err = {.code = 1, .message = {.data = msg.data(), .size = msg.size()}};
    }
    else
    {
        (*buf)->state.store(
            wholth_Buffer::State::OCCUPIED, std::memory_order_seq_cst);
    }

    g_idx = (g_idx + 1) % g_size;

    return err;
}

extern "C" wholth_Error wholth_buffer_del(wholth_Buffer* buf)
{
    if (nullptr == buf)
    {
        constexpr std::string_view msg = "Buffer is null!";
        // todo add code enum
        return {.code = 1, .message = {.data = msg.data(), .size = msg.size()}};
    }

    buf->state.store(wholth_Buffer::State::FREE, std::memory_order_seq_cst);

    return wholth_Error_OK;
}

extern "C" auto wholth_buffer_move_data_to(
    wholth_Buffer* const handle,
    void* data) -> void
{
    if (nullptr != handle && nullptr != data)
    {
        handle->data = std::move(*static_cast<std::string*>(data));
    }
}

extern "C" auto wholth_buffer_view(const wholth_Buffer* const handle)
    -> const wholth_StringView
{
    if (nullptr == handle)
    {
        return {
            .data = nullptr,
            .size = 0,
        };
    }

    return {
        .data = handle->data.data(),
        .size = handle->data.size(),
    };
}
