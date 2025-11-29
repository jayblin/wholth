#ifndef WHOLTH_CONTEXT_H_
#define WHOLTH_CONTEXT_H_

#include "sqlw/connection.hpp"
#include <concepts>
#include <vector>

// todo move to other namespace
namespace wholth
{

// todo remake to struct
class Context
{
  public:
    Context(){};

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    Context(Context&& other) {
        *this = std::move(other);
    };
    Context& operator=(Context&& other) {
        if (&other == this) {
            return other;
        }

        this->exception_message = std::move(other.exception_message);
        this->sql_errors = std::move(other.sql_errors);
        // this->connection = std::move(other.connection);

        return *this;
    };

    // sqlw::Connection connection;

    // todo move this outside of context
    std::vector<std::string> sql_errors;
    std::string exception_message;
    std::string password_encryption_secret {""};
};

} // namespace wholth

namespace wholth::concepts
{
    template <typename T>
    concept is_context_aware = requires(T t)
    {
        t.ctx;
        requires std::same_as<decltype(t.ctx), const wholth::Context&>;
    };
};

#endif // WHOLTH_CONTEXT_H_
