#ifndef WHOLTH_UTILS_LENGTH_CONTAINER_H_
#define WHOLTH_UTILS_LENGTH_CONTAINER_H_

#include "wholth/entity/length_container.hpp"
#include <string>

namespace wholth::utils
{

auto add(wholth::entity::LengthContainer&, size_t length) -> void;

template <typename T>
auto next(wholth::entity::LengthContainer& lc, const std::string& buffer) -> T;

template <typename T>
auto extract(
    T& to,
    wholth::entity::LengthContainer& from,
    const std::string& buffer)
{
    to = wholth::utils::next<T>(from, buffer);
}

auto clear(wholth::entity::LengthContainer&) -> void;

} // namespace wholth::utils

#endif // WHOLTH_UTILS_LENGTH_CONTAINER_H_
