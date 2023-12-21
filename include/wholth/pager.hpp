#ifndef WHOLTH_PAGER_H_
#define WHOLTH_PAGER_H_

#include <span>
#include <string>
#include <string_view>
#include <cstddef>
#include <vector>
#include "wholth/concepts.hpp"

namespace wholth {

template <typename View>
class Pager
{
public:
	typedef View value_t;

	template<typename Query>
	PaginationInfo query_page(
		std::span<View>,
		sqlw::Connection*,
		const Query&
	);

private:
	uint32_t m_buffer_idx {0};
	std::array<std::string, 2> m_buffers;
};

}

#endif // WHOLTH_PAGER_H_
