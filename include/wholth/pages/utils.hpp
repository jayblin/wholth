#ifndef WHOLTH_CONTROLLER_ABSTRACT_PAGE_H_
#define WHOLTH_CONTROLLER_ABSTRACT_PAGE_H_

#include "sqlw/statement.hpp"
#include "wholth/entity/length_container.hpp"
#include "wholth/pagination.hpp"
#include <system_error>
#include <tuple>
// #include "wholth/model/abstract_page.hpp"
// #include <cstdint>

// todo remove
namespace wholth::pages
{

// template <typename T>
//     requires wholth::concepts::is_fetch_aware<T> &&
//              wholth::concepts::has_pagination<T>
// bool advance(T& model, uint64_t by)
// {
//     return !model.is_fetching && model.pagination.advance(by);
// }
//
// template <typename T>
//     requires wholth::concepts::is_fetch_aware<T> &&
//              wholth::concepts::has_pagination<T>
// bool retreat(T& model, uint64_t by)
// {
//     return !model.is_fetching && model.pagination.retreat(by);
// }
//
// template <typename T>
//     requires wholth::concepts::is_fetch_aware<T> &&
//              wholth::concepts::has_pagination<T>
// bool skip_to(T& model, uint64_t page)
// {
//     return !model.is_fetching && model.pagination.skip_to(page);
// }

// template <typename T>
// constexpr auto count_fields() -> size_t;

// template <typename T>
// auto hydrate(const std::string&, wholth::entity::LengthContainer&) -> T;

// template <typename T>
// auto prepare_stmt(
//     sqlw::Statement& stmt,
//     const T& query,
//     const wholth::Pagination& pagination)
//     // -> std::variant<wholth::entity::LengthContainer, std::error_code>;
//     -> std::tuple<wholth::entity::LengthContainer, std::error_code>;

} // namespace wholth::pages

#endif // WHOLTH_CONTROLLER_ABSTRACT_PAGE_H_
