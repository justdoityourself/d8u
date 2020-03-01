#pragma once

#include <cstdint>
#include <string_view>

namespace d8u
{
	uint64_t constexpr __mix(char m, uint64_t s)
	{
		return ((s << 7) + ~(s >> 3)) + ~m;
	}

	uint64_t constexpr _switch_t(const char* m)
	{
		return (*m) ? __mix(*m, _switch_t(m + 1)) : 0;
	}

	uint64_t constexpr _switch_t(const char* m, size_t l)
	{
		return (l) ? __mix(*m, _switch_t(m + 1, l - 1)) : 0;
	}

	uint64_t constexpr switch_t(std::string_view t)
	{
		return _switch_t((const char*)t.data(), t.size());
	}

	uint64_t constexpr switch_t(const char * s)
	{
		std::string_view t(s);
		return _switch_t((const char*)t.data(), t.size());
	}
}