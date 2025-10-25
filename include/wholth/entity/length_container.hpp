#ifndef WHOLTH_ENTITY_LENGTH_CONTAINER_H_
#define WHOLTH_ENTITY_LENGTH_CONTAINER_H_

#include <cstddef>
#include <vector>

namespace wholth::entity
{

struct LengthContainer
{
    LengthContainer(size_t initial_vector_size);

    LengthContainer(const LengthContainer&) = delete;
    LengthContainer& operator=(const LengthContainer&) = delete;

    LengthContainer(LengthContainer&&);
    LengthContainer& operator=(LengthContainer&&);

    std::vector<size_t> lengths{};
    size_t i1{0};
    size_t i2{0};
    size_t offset{0};
};

} // namespace wholth::entity

#endif // WHOLTH_ENTITY_LENGTH_CONTAINER_H_
