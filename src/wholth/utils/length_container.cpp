#include "wholth/entity/length_container.hpp"
#include "wholth/c/forward.h"
#include "wholth/utils/length_container.hpp"

wholth::entity::LengthContainer::LengthContainer(size_t initial_vector_size)
{
    this->lengths.reserve(initial_vector_size);
}

wholth::entity::LengthContainer::LengthContainer(LengthContainer&& other)
{
    *this = std::move(other);
}

wholth::entity::LengthContainer& wholth::entity::LengthContainer::operator=(LengthContainer&& other)
{
    this->lengths = std::move(other.lengths);
    this->i1 = other.i1;
    this->i2 = other.i2;
    this->offset = other.offset;

    return *this;
}

auto wholth::utils::add(wholth::entity::LengthContainer& lc, size_t length)
    -> void
{
    if (lc.i1 < lc.lengths.capacity())
    {
        lc.lengths[lc.i1] = length;
        lc.i1++;
    }
}

auto wholth::utils::clear(wholth::entity::LengthContainer& lc) -> void
{
    lc.lengths.clear();
    lc.i1 = 0;
    lc.i2 = 0;
    lc.offset = 0;
}

template <>
auto wholth::utils::next(
    wholth::entity::LengthContainer& lc,
    const std::string& buffer) -> wholth_StringView
{
    if (lc.i2 >= lc.lengths.capacity())
    {
        return {nullptr, 0};
    }

    wholth_StringView result{buffer.data() + lc.offset, lc.lengths[lc.i2]};

    lc.offset += lc.lengths[lc.i2];
    lc.i2++;

    return result;
}

// template <>
// auto wholth::utils::extract(
//     wholth_StringView& to,
//     wholth::entity::LengthContainer& from,
//     const std::string& buffer)
// {
//     to = wholth::utils::next<wholth_StringView>(from, buffer);
// }
