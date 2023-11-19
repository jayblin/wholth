#ifndef WHOLTH_ENTITY_UTILS_H_
#define WHOLTH_ENTITY_UTILS_H_

#include <cstddef>
#include <tuple>
#include "gsl/gsl"
#include "wholth/concepts.hpp"

template <size_t Size>
struct TextInput
{
	const gsl::czstring label;
	char value[Size] {""};

	TextInput() = delete;
	TextInput(gsl::czstring a_label) : label(a_label)
	{}

	constexpr size_t size() const { return Size; };
};

struct FloatInput
{
	const gsl::czstring label;
	float value {0.0f};

	FloatInput() = delete;
	FloatInput(gsl::czstring a_label) : label(a_label)
	{}
};

template<typename T>
concept has_static_index = requires()
{
	T::index;
};

template<typename T, size_t Index>
struct TupleElement
{
	typedef T value_type;
	static constexpr size_t index {Index};
};

namespace entity
{
	template<typename T>
	requires has_static_index<T>
	const auto& get(const auto& entry)
	{
		return std::get<T::index>(entry);
	}
}

#endif // WHOLTH_ENTITY_UTILS_H_
