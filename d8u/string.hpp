/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "memory.hpp"

namespace d8u
{
	namespace util
	{
		constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
									'8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

		template <typename T> std::string to_hex(const T& d)
		{
			std::string s(d.size() * 2, ' ');
			for (size_t i = 0; i < d.size(); ++i)
			{
				s[2 * i] = hexmap[(d[i] & 0xF0) >> 4];
				s[2 * i + 1] = hexmap[d[i] & 0x0F];
			}
			return s;
		}

		std::vector<uint8_t> to_bin(std::string_view v)
		{
			std::vector<uint8_t> result; result.reserve(v.size() / 2 + 1);
			auto ctoi = [](char c)
			{
				if (c >= '0' && c <= '9')
					return c - '0';
				if (c >= 'A' && c <= 'F')
					return c - 'A' + 10;
				if (c >= 'a' && c <= 'f')
					return c - 'a' + 10;

				return 0;
			};

			for (auto c = v.begin(); c < v.end(); c += 2)
				result.push_back((ctoi(*c) << 4) + ctoi(*(c + 1)));

			return result;
		}

		sse_vector to_bin_sse(std::string_view v)
		{
			sse_vector result; result.reserve(v.size() / 2 + 1);
			auto ctoi = [](char c)
			{
				if (c >= '0' && c <= '9')
					return c - '0';
				if (c >= 'A' && c <= 'F')
					return c - 'A' + 10;
				if (c >= 'a' && c <= 'f')
					return c - 'a' + 10;

				return 0;
			};

			for (auto c = v.begin(); c < v.end(); c += 2)
				result.push_back((ctoi(*c) << 4) + ctoi(*(c + 1)));

			return result;
		}

		template < typename T > T to_bin_t(std::string_view v)
		{
			T result;

			auto vb = d8u::util::to_bin(v);
			if (vb.size() != sizeof(T))
				throw std::runtime_error("Bad input key");

			std::copy(vb.begin(), vb.end(), (uint8_t*)&result);

			return result;
		}

		std::wstring to_wide(std::string_view s)
		{
			//This Method, is so slow and uses thread locks
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//return converter.from_bytes(string(s));

			//Back to the basics, no good deed unpunished.
			return std::wstring(s.begin(), s.end());
		}

		std::string to_narrow(std::wstring_view s)
		{
			//This Method, is so slow and uses thread locks
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//return converter.to_bytes(wstring(s));

			//Back to the basics, no good deed unpunished.
			return std::string(s.begin(), s.end());
		}
	}
}