#ifndef WHOLTH_INTERNAL_RING_POOL_H_
#define WHOLTH_INTERNAL_RING_POOL_H_

#include "wholth/c/error.h"
#include <atomic>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>
#include <cassert>

namespace wholth::internal::ring_pool
{

constexpr size_t DEFAULT_SIZE = 255;

enum class EntryState : int
{
    FREE = 0,
    OCCUPIED,
};

template <typename T>
concept is_container = requires(T t) {
    t.data;
    t.state;
    requires std::same_as<std::atomic<EntryState>, decltype(t.state)>;
};

template <typename T>
struct Index
{
    std::atomic<uint64_t> value = 0;
};

template <typename T, size_t Size = DEFAULT_SIZE>
static auto ring_pool() -> std::vector<T>&
{
    static std::vector<T> g_buffer_pool(Size);
    return g_buffer_pool;
}

template <typename T, size_t Size = DEFAULT_SIZE>
    requires is_container<T>
wholth_Error fetch(T** container)
{
    static Index<T> idx{};

    assert(
        nullptr != container && nullptr == *container &&
        "NULL_RING_POOL_ELEMENT_CONTAINER_PROVIDED");

    T&   entry = ring_pool<T>()[idx.value];
    auto err = wholth_Error_OK;

    if (EntryState::FREE != entry.state)
    {
        // return "Ring pool element is not free!";
        constexpr std::string_view msg = "Ring pool element is not free!";
        // todo add code enum
        err = {.code = 1, .message = {.data = msg.data(), .size = msg.size()}};
    }
    else
    {
        entry.state.store(EntryState::OCCUPIED, std::memory_order_seq_cst);
        entry.data = {};

        *container = &entry;
    }

    idx.value = (idx.value + 1) % Size;

    return err;
    // return "";
}

template <typename T, size_t Size = DEFAULT_SIZE>
    requires is_container<T>
wholth_Error unfetch(T* buf)
{
    if (nullptr == buf)
    {
        constexpr std::string_view msg = "Ring pool element container is null!";
        // todo add code enum
        return {.code = 1, .message = {.data = msg.data(), .size = msg.size()}};
    }

    buf->state.store(EntryState::FREE, std::memory_order_seq_cst);

    return wholth_Error_OK;
}

} // namespace wholth::internal::ring_pool

#endif // WHOLTH_INTERNAL_RING_POOL_H_
