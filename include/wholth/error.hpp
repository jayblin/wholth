#ifndef WHOLTH_ERROR_H_
#define WHOLTH_ERROR_H_

#include <type_traits>
#include <system_error>

namespace wholth::error
{

template <typename T>
struct is_error_code_enum : std::false_type
{
};

struct Category : std::error_category
{
    const char* name() const noexcept override final;

    std::string message(int ev) const override final;
};

const Category& category();

std::string_view message(int error_value);

} // namespace wholth::error

template <typename T>
    requires wholth::error::is_error_code_enum<T>::value
struct std::is_error_code_enum<T> : std::true_type
{
};

template <typename T>
    requires wholth::error::is_error_code_enum<T>::value
std::error_code make_error_code(T e)
{
    return {static_cast<int>(e), wholth::error::category()};
}

#endif // WHOLTH_ERROR_H_
