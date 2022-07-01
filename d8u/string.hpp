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
		template < typename F > size_t split(const std::string_view target, std::string_view delimiter, F f)
		{
			size_t pre = 0, i = 0;

			for (size_t pos = 0; (pos = target.find(delimiter, pre)) != std::string_view::npos; i++, pre=pos+1)
				f(target.substr(pre, pos-pre), i);

			if(pre) f(target.substr(pre, target.size()-pre), i);

			return i;
		}

		template <size_t N> std::string_view split_at(const std::string_view target, std::string_view delimiter)
		{
			std::string_view result;
			d8u::util::split(target, delimiter, [&](auto s, auto i) {
				switch (i) {
				case N: result = s;
				default: return;
				}
			});

			return result;
		}

		template <size_t ...N> auto split_n(const std::string_view target, std::string_view delimiter)
		{
			size_t pre = 0, i = 0,pos = 0;

			auto next = [&](size_t n) {
				for (; (pos = target.find(delimiter, pre)) != std::string_view::npos; i++, pre = pos + 1)
				{
					if (n == i)
					{
						auto result = target.substr(pre, pos - pre);
						return i++, pre = pos + 1, result;
					}
				}

				return target.substr(pre, pos - pre);
			};

			return std::make_tuple(next(N)...);
		}



		inline auto split_pair(const std::string_view target, std::string_view delimiter)
		{
			size_t del = target.find(delimiter);

			return del == -1 
				? std::make_pair(target,"")
				: std::make_pair(std::string_view(target.data(),del),std::string_view(target.data()+del+1,target.size()-(del+1)));
		}

		inline auto split_first(const std::string_view target, std::string_view delimiter)
		{
			size_t del = target.find(delimiter);

			return del == -1
				? target
				: std::string_view(target.data(), del);
		}


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

		template <typename T> std::string to_hex_t(const T& _d)
		{
			std::string s(sizeof(T) * 2, ' ');
			auto d = (uint8_t*)&_d;
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				s[2 * i] = hexmap[(d[i] & 0xF0) >> 4];
				s[2 * i + 1] = hexmap[d[i] & 0x0F];
			}
			return s;
		}

		inline std::vector<uint8_t> to_bin(std::string_view v)
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

		inline sse_vector to_bin_sse(std::string_view v)
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

		inline std::wstring to_wide(std::string_view s)
		{
			//This Method, is so slow and uses thread locks
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//return converter.from_bytes(string(s));

			//Back to the basics, no good deed unpunished.
			return std::wstring(s.begin(), s.end());
		}

		inline std::string to_narrow(std::wstring_view s)
		{
			//This Method, is so slow and uses thread locks
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//return converter.to_bytes(wstring(s));

			//Back to the basics, no good deed unpunished.
			return std::string(s.begin(), s.end());
		}
	}
}