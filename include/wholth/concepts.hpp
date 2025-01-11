#ifndef WHOLTH_CONCEPTS_H_
#define WHOLTH_CONCEPTS_H_

#include <tuple>
#include <type_traits>
#include <concepts>

template<class T, size_t Index>
concept has_tuple_get = requires(T t)
{
    typename std::tuple_element_t<Index, std::remove_const_t<T>>;
    { get<Index>(t) } -> std::convertible_to<const std::tuple_element_t<Index, T>&>;
};

// @todo is this a duplicate of `serialization::is_tuple_like`?
template<class T>
concept is_tuple_like = !std::is_reference_v<T>
	&& requires(T t)
	{
		typename std::tuple_size<T>::type;
		requires std::derived_from<
		  std::tuple_size<T>,
		  std::integral_constant<std::size_t, std::tuple_size_v<T>>
		>;
	}
	&& []<std::size_t... Index>(std::index_sequence<Index...>)
	{
		return (has_tuple_get<T, Index> && ...);
	}(std::make_index_sequence<std::tuple_size_v<T>>());

template<typename T>
concept is_view_list = is_tuple_like<typename T::value_t>
&& requires(T t)
{
	typename T::value_t;

	std::is_reference_v<typename T::reference>;

	{ t.clear() } -> std::same_as<void>;
	{ t.operator[](size_t{}) } -> std::same_as<T&>;
	{ t.size() } -> std::same_as<size_t>;
};

#endif // WHOLTH_CONCEPTS_H_
