#ifndef UTILS_SERIALIZER_H_
#define UTILS_SERIALIZER_H_

#include <array>
#include <concepts>
#include <string_view>
#include <tuple>
#include <type_traits>

#define NVP(x) ::serialization::NameValuePair{#x, x}
#define ONVP(x) ::serialization::NameValuePair{#x, this->o.x}

namespace serialization
{
    template <typename T>
    concept is_std_container = requires (T t)
    {
        t.begin();
        t.end();
        t.data();
    };

    template <typename T>
    concept is_std_vector = is_std_container<T> && requires (T t)
    {
        t.emplace_back();
    };

    // @todo understand this
    template<class T, std::size_t N>
    concept has_tuple_element = requires(T t) {
        typename std::tuple_element_t<N, std::remove_const_t<T>>;
        { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
    };

    template<class T>
    concept is_tuple_like = !std::is_reference_v<T> 
        && requires(T t) { 
            typename std::tuple_size<T>::type; 
            requires std::derived_from<
                std::tuple_size<T>, 
                std::integral_constant<std::size_t, std::tuple_size_v<T>>
            >;
        } && []<std::size_t... N>(std::index_sequence<N...>) { 
            return (has_tuple_element<T, N> && ...); 
        }(std::make_index_sequence<std::tuple_size_v<T>>());

    template <typename Value>
    class NameValuePair
    {
    public:
        NameValuePair(std::string_view name, const Value& value)
            : m_pair(name, value)
        {
        }

        constexpr std::string_view name() const
        {
            return m_pair.first;
        }

        constexpr const Value& value() const
        {
            return m_pair.second;
        }

    private:
        std::pair<std::string_view, const Value& > m_pair;
    };

    template <typename T>
    struct Ref
    {
        const T& o;
    };

    template <typename Value>
    struct Serializable : std::false_type
    {
        template <typename Serializer>
        auto serialize(Serializer& serializer) -> void;
    };

    template <typename Value, typename Serializer>
    concept is_inherently_serializable = requires(Value v, Serializer& serializer_ref)
    {
        v.serialize(serializer_ref);
    };

    template <typename Value, typename Serializer>
    concept is_convertably_serializable = requires(const Value& value_ref, Serializer& serializer_ref, Serializable<Value> si)
    {
        si.serialize(serializer_ref);
    } 
    && std::is_base_of_v<std::true_type, Serializable<Value>>;
}

#endif // UTILS_SERIALIZER_H_

