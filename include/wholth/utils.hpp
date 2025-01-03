#ifndef WHOLTH_UTILS_H_
#define WHOLTH_UTILS_H_

#include "fmt/color.h"
#include "sqlite3.h"
#include <array>
#include <gsl/gsl>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

// @todo - remove LOG_* functions.
#define LOG_RED(x) std::cout << "\033[1;31m" << x << "\033[0m\n"
#define LOG_GREEN(x) std::cout << "\033[1;32m" << x << "\033[0m\n"
#define LOG_YELLOW(x) std::cout << "\033[1;33m" << x << "\033[0m\n"
#define LOG_BLUE(x) std::cout << "\033[1;34m" << x << "\033[0m\n"
#define LOG_MAGENTA(x) std::cout << "\033[1;35m" << x << "\033[0m\n"
#define LOG_CYAN(x) std::cout << "\033[1;36m" << x << "\033[0m\n"
#define LOG_WHITE(x) std::cout << "\033[1;37m" << x << "\033[0m\n"
#define LOG(x) std::cout << x << '\n'

namespace wholth::utils
{
	/* constexpr std::string_view NIL = "<NIL>"; */
	const gsl::czstring NIL = "<NIL>";

	// YYYY-MM-DDTHH:MM:SS
	std::string current_time_and_date();

	template<size_t Size, size_t Offset = 0>
	class IndexSequence
	{
	public:
		IndexSequence()
		{
			this->reset();
		}

		size_t operator[](size_t i) const noexcept
		{
			return m_idxs[i];
		}

		void advance()
		{
			for (size_t i = 0; i < Size; i++)
			{
				m_idxs[i] += Size;
			}
		}

		void reset()
		{
			for (size_t i = 0; i < Size; i++)
			{
				m_idxs[i] = i + Offset;
			}
		}

	private:
		std::array<size_t, Size> m_idxs;
	};

	class LengthContainer
	{
	public:
		LengthContainer(size_t size)
		{
			m_lengths.resize(size);
		};

		void add(size_t length)
		{
			if (m_i1 < m_lengths.size())
			{
				m_lengths[m_i1] = length;
				m_i1++;
			}
		}

		std::string_view next(const std::string& buffer)
		{
			if (m_i2 < m_lengths.size())
			{
				std::string_view result {
					buffer.data() + m_offset,
					m_lengths[m_i2]
				};
				m_offset += m_lengths[m_i2];
				m_i2++;

				return result;
			}

			return NIL;
		}

	private:
		std::vector<size_t> m_lengths {};
		size_t m_i1 {0};
		size_t m_i2 {0};
		size_t m_offset {0};
	};

	class BufferSwapper
	{
	public:
		auto swap() -> void
		{
			m_buffer_idx = (m_buffer_idx + 1) % BUFF_COUNT;
		}

		auto set_next(std::string&& str) -> std::string&
		{
			m_buffers[(m_buffer_idx + 1) % BUFF_COUNT] = std::move(str);

			return m_buffers[(m_buffer_idx + 1) % BUFF_COUNT];
		}

		/* auto operator[](size_t idx) -> std::string& */
		/* { */
		/* 	assert(idx < BUFF_COUNT); */

		/* 	return m_buffers[idx]; */
		/* } */

	private:
		static constexpr size_t BUFF_COUNT {2};
		uint32_t m_buffer_idx {0};
		std::array<std::string, BUFF_COUNT> m_buffers {};
	};

	namespace sqlite
	{
		void seconds_to_readable_time(
			sqlite3_context* ctx,
			int argc,
			sqlite3_value** argv
		);
	}
}

#endif // WHOLTH_UTILS_H_
