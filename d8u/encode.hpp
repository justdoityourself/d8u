/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace d8u
{
	namespace encode
	{
		template <typename T> std::string base64(const T& input)
		{
			std::string out;
			out.reserve(input.size() * 4 / 3 + 4);

			int val = 0, valb = -6;
			for (uint8_t c : input)
			{
				val = (val << 8) + c;
				valb += 8;
				while (valb >= 0)
				{
					out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
					valb -= 6;
				}
			}

			if (valb > -6)
				out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);

			while (out.size() % 4) out.push_back('=');

			return out;
		}

		template <typename T> std::string url(const T& input)
		{
			std::string out;
			out.reserve(input.size() * 4 / 3 + 4);

			int val = 0, valb = -6;
			for (uint8_t c : input)
			{
				val = (val << 8) + c;
				valb += 8;
				while (valb >= 0)
				{
					out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]"[(val >> valb) & 0x3F]);
					valb -= 6;
				}
			}

			if (valb > -6)
				out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]"[((val << 8) >> (valb + 8)) & 0x3F]);

			while (out.size() % 4) out.push_back('-');

			return out;
		}
	}

	namespace decode
	{
		template<typename R> R base64_t(const std::string_view in)
		{
			R out;
			auto itr = (uint8_t*)&out;

			std::array<int, 256> T = { -1 };
			for (int i = 0; i < 64; i++)
				T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

			int val = 0, dx = 0, valb = -8;
			for (uint8_t c : in)
			{
				if (T[c] == -1 || dx == sizeof(R)) break;

				val = (val << 6) + T[c];
				valb += 6;

				if (valb >= 0)
				{
					dx++;
					*itr++ = (char((val >> valb) & 0xFF));
					valb -= 8;
				}
			}

			return out;
		}

		inline std::vector<uint8_t> base64(const std::string_view in)
		{
			std::vector<uint8_t> out;
			out.reserve(in.size() * 3 / 4 + 4);

			std::array<int, 256> T;
			T.fill(-1);
			for (int i = 0; i < 64; i++)
				T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

			int val = 0, valb = -8;
			for (uint8_t c : in)
			{
				if (T[c] == -1) 
					break;

				val = (val << 6) + T[c];
				valb += 6;
				if (valb >= 0) 
				{
					out.push_back(char((val >> valb) & 0xFF));
					valb -= 8;
				}
			}
			return out;
		}

		template<typename R> R url_t(const std::string_view in)
		{
			R out;
			auto itr = (uint8_t*)&out;

			std::array<int, 256> T;
			T.fill(-1);
			for (int i = 0; i < 64; i++)
				T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]"[i]] = i;

			int val = 0, dx = 0, valb = -8;
			for (uint8_t c : in)
			{
				if (T[c] == -1 || dx == sizeof(R)) break;

				val = (val << 6) + T[c];
				valb += 6;

				if (valb >= 0)
				{
					dx++;
					*itr++ = (char((val >> valb) & 0xFF));
					valb -= 8;
				}
			}

			return out;
		}

		inline std::vector<uint8_t> url(const std::string_view in)
		{
			std::vector<uint8_t> out;
			out.reserve(in.size() * 3 / 4 + 4);

			std::array<int, 256> T;
			T.fill(-1);
			for (int i = 0; i < 64; i++)
				T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]"[i]] = i;

			int val = 0, valb = -8;
			for (uint8_t c : in)
			{
				if (T[c] == -1)
					break;

				val = (val << 6) + T[c];
				valb += 6;

				if (valb >= 0)
				{
					out.push_back(char((val >> valb) & 0xFF));
					valb -= 8;
				}
			}
			return out;
		}
	}
}