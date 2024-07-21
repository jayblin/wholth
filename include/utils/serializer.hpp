#ifndef UTILS_SERIALIZER_H_
#define UTILS_SERIALIZER_H_

#include <concepts>
#include <string_view>
#include <type_traits>

#define NVP(x) utils::NameValuePair{#x, x}

namespace utils
{
    template <typename Value, typename Stream>
    concept is_serializable = requires(Value v, Stream& stream)
    {
        { v.serialize(stream) } -> std::same_as<void>;
    };

    template <typename T>
    concept is_std_container = requires (T t)
    {
        t.begin();
        t.end();
        t.data();
        t.emplace_back();
    };

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
}

#endif // UTILS_SERIALIZER_H_

