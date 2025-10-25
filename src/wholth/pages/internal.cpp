#include "wholth/pages/internal.hpp"
#include "wholth/pages/code.hpp"
#include <system_error>
#include <variant>

namespace wholth::pages
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "wholth::pages";
    }

    std::string message(int ev) const override final
    {
        using Code = wholth::pages::Code;

        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case Code::SPAN_SIZE_TOO_BIG:
            return "SPAN_SIZE_TOO_BIG";
        case Code::QUERY_PAGE_TOO_BIG:
            return "QUERY_PAGE_TOO_BIG";
        case Code::QUERY_OFFSET_TOO_BIG:
            return "QUERY_OFFSET_TOO_BIG";
        case Code::NOT_FOUND:
            return "NOT_FOUND";
        case Code::INVALID_LOCALE_ID:
            return "INVALID_LOCALE_ID";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory error_category{};
} // namespace wholth::pages

std::error_code wholth::pages::make_error_code(wholth::pages::Code e)
{
    return {static_cast<int>(e), wholth::pages::error_category};
}

wholth_Page::wholth_Page_t()
    : pagination(0), is_fetching(false), data(std::monostate{})
{
}

wholth_Page::wholth_Page_t(
    wholth::Pagination::per_page_t _per_page,
    wholth::pages::internal::PageData _data)
    : pagination(_per_page), is_fetching(false), data(_data)
{
}

wholth_Page& wholth_Page::operator=(wholth_Page&& other)
{
    if (this != &other)
    {
        this->is_fetching = false;
        this->pagination = std::move(other.pagination);
        this->data = std::move(other.data);

        other.data = std::monostate{};
    }

    return *this;
}
