#ifndef WHOLTH_UTILS_CONVERT_H_
#define WHOLTH_UTILS_CONVERT_H_

#include "wholth/entity/food.hpp"
#include "wholth/hydrate_food.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <string_view>

namespace wholth::utils
{

template <wholth::concepts::is_food T>
auto convert(const T& food) -> wholth::entity::Food
{
    return {
        .id = wholth::utils::to_string_view(food.id),
        .title = wholth::utils::to_string_view(food.title),
        .top_nutrient = wholth::utils::to_string_view(food.top_nutrient),
        .preparation_time =
            wholth::utils::to_string_view(food.preparation_time),
    };
}

struct Sup
{
    std::string_view help(
        const std::string& buffer,
        wholth::utils::LengthContainer& lc)
    {
        return lc.next(buffer);
    }
};

struct LengthContainerSupporter
{
    LengthContainer& lc;
    const std::string& buffer;

    template <typename T>
    std::string_view help(T value)
    {
        return wholth::utils::to_string_view(value);
    }
};

struct NoopSupport
{
    template <typename T>
    std::string_view help(T value)
    {
        return {};
    }
};

template <wholth::concepts::is_food T, typename Support>
auto convert(const T& thing, const Support& s) -> wholth::entity::Food
{
    wholth::entity::Food result;
    result.id = s.help(thing.id);
    result.title = s.help(thing.title);
    result.top_nutrient = s.help(thing.top_nutrient);
    result.preparation_time = s.help(thing.preparation_time);
    return result;
}

} // namespace wholth::utils

#endif // WHOLTH_UTILS_CONVERT_H_
