#ifndef WHOLTH_VIEW_LIST_H_
#define WHOLTH_VIEW_LIST_H_

#include <string>
#include <string_view>
#include <cstddef>
#include <vector>
#include "wholth/concepts.hpp"

template <typename View>
class ViewList
{
public:
	typedef View value_t;

	constexpr size_t size() const { return this->m_list.size(); }

	const value_t& operator[](size_t pos) const
	{
		return this->m_list[pos];
	}

	const std::vector<value_t>& list() const
	{
		return this->m_list;
	}

	template<typename Query>
	PaginationInfo query_page(sqlw::Connection*, const Query&);

private:
	uint32_t m_buffer_idx {0};
	std::array<std::string, 2> m_buffers;
	std::vector<value_t> m_list;
};

#endif // WHOLTH_VIEW_LIST_H_
