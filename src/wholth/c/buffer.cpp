#include "wholth/c/buffer.h"
#include <atomic>
#include <vector>
#include <string>

struct wholth_Buffer_t
{
    std::string data{};
};

static std::atomic<int64_t> g_idx = -1;
constexpr auto g_size = 100;

static auto ring_pool() -> std::vector<wholth_Buffer>&
{
    static std::vector<wholth_Buffer> g_buffer_pool(g_size);
    return g_buffer_pool;
}

// wholth_Error wholth_buffer_create(wholth_Buffer* handle)
// {
//     g_idx = (g_idx + 1) % g_size;
//
//     handle = &pool()[g_idx];
//
//     return wholth_Error_OK;
// }

auto wholth_buffer_ring_pool_element() -> wholth_Buffer*
{
    g_idx = (g_idx + 1) % g_size;
    return &ring_pool()[g_idx];
}

auto wholth_buffer_move_data_to(wholth_Buffer* const handle, void* data) -> void
{
    if (nullptr != handle && nullptr != data)
    {
        handle->data = std::move(*static_cast<std::string*>(data));
    }
}

auto wholth_buffer_view(const wholth_Buffer* const handle)
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
