#ifndef UTILS_JSON_SERIALIZER_H_
#define UTILS_JSON_SERIALIZER_H_

#include "utils/serializer.hpp"
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <span>

template <typename T>
concept is_span = /*::utils::is_std_container<T> &&*/
requires(T t) {
    t.subspan(size_t{}, size_t{});
};

namespace utils {
    class JsonSerializer
    {
    public:
        JsonSerializer()
        {
        }

        template <typename Value>
        requires utils::is_serializable<Value, utils::JsonSerializer>
        std::string serialize(const Value& value) noexcept
        {
            begin_object();
            descend();

            value.serialize(*this);

            ascend();
            new_line();
            end_object();

            return m_output_stream.str();
        }

        template<typename Value>
        JsonSerializer& operator<<(const NameValuePair<Value>& nvp)
        {
            if (should_delimit()) {
                delimit();
            }

            new_line();

            tabulate();

            /* m_output_stream << m_upstairs_neighbor_depth << ":" << m_depth << '"' << nvp.name() << "\":"; */
            m_output_stream << '"' << nvp.name() << "\":";

            recurse(nvp.value());

            return *this;
        }

    template <typename Value>
    void recurse(const Value& value)
    {
        if (depth() > 100) {
            // @todo
            // - handle depth limit;
            // - handle references;
            m_output_stream << "\"<too_deep>\"";
            return;
        }

        m_upstairs_neighbor_depth = m_depth;

        if constexpr (utils::is_serializable<Value, utils::JsonSerializer>) {
            begin_object();
            descend();

            value.serialize(*this);

            new_line();
            ascend();
            tabulate();
            end_object();
        }
        else if constexpr (utils::is_std_vector<Value> || is_span<Value>) {
            begin_array();

            if (value.size() > 0) {
                descend();

                for (const auto& e : value)
                {
                    new_line();
                    tabulate();

                    recurse(e);

                    if (&e != &value.back()) {
                        delimit();
                    }
                }

                new_line();
                ascend();
                tabulate();
            }

            end_array();
        }
        else if constexpr (utils::is_tuple_like<Value>) {
            begin_array();
            descend(),
            new_line();
            constexpr size_t size = std::tuple_size_v<Value>;

            std::apply(
                [this] (auto&&... args) {
                    size_t i = 0;
                    auto print_elem = [this, &i] (const auto& x) {
                        i++;
                        tabulate();
                        recurse(x);

                        if (i < size) {
                            delimit();
                        }

                        new_line();
                    };
                    ((print_elem(args)), ...);
                },
                value
            );

            ascend();
            tabulate();
            end_array();
        }
        else if constexpr (std::is_arithmetic_v<Value>) {
            m_output_stream << static_cast<uint64_t>(value);
        }
        else {
            auto start_pos = m_output_stream.tellp();

            m_output_stream << '"' << value << '"';

            // replace " to '
            auto end_pos = m_output_stream.tellp();

            int64_t total_chars_put = end_pos - start_pos - 2;
            for (int64_t i = 1; i <= total_chars_put; i++)
            {
                m_output_stream.seekg(start_pos + i);
                const char ch = m_output_stream.get();

                if (ch == '"') {
                    m_output_stream.seekp(start_pos + i);
                    m_output_stream.put('\'');
                    m_output_stream.seekp(end_pos);
                }
                else if (ch == '\n') {
                    m_output_stream.seekp(start_pos + i);
                    m_output_stream.put(' ');
                    m_output_stream.seekp(end_pos);
                }
            }

            m_output_stream.clear();
        }
    }

    private:
        std::stringstream m_output_stream;
        int64_t m_depth {0};
        int64_t m_upstairs_neighbor_depth {0};

        auto begin_object() -> void;
        auto end_object() -> void;
        auto begin_array() -> void;
        auto end_array() -> void;
        auto tabulate() -> void;
        auto new_line() -> void;
        auto should_delimit() -> bool;
        auto delimit() -> void;
        auto sanitize_string(std::string&) -> void;

        int64_t depth()
        {
            return m_depth;
        }

        void descend()
        {
            m_depth++;
        }

        void ascend()
        {
            if (m_depth > 0) {
                m_depth--;
                m_upstairs_neighbor_depth = m_depth;
            }
        }
    };
}

#endif // UTILS_JSON_SERIALIZER_H_
