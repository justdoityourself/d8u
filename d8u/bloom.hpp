/* Copyright (C) 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include "compression.hpp"
#include "encode.hpp"

namespace d8u
{
	
	template <size_t byte_c, typename hash_t = uint16_t, size_t hash_max_c = 0> class Bloom : public std::array<uint8_t, byte_c>
	{
		static const size_t lim_c = byte_c * 8;

		inline void set(size_t n) {
			(*this)[n / 8] |= 1 << n % 8;
		}

		inline bool test(size_t n) {
			return (bool) ((*this)[n / 8] & 1 << n % 8);
		}
	public:

		void Reset()
		{
			std::fill(std::array<uint8_t, byte_c>::begin(), std::array<uint8_t, byte_c>::end(), 0);
		}

		// Assumes that t is pre-distributed data.
		// t.size() controls hash count.
		// remainder % sizeof(hash_t) is discarded.
		template < typename T> void Set(const T & t)
		{
			auto l = t.size() / sizeof(hash_t);
			if (hash_max_c && l > hash_max_c)
				l = hash_max_c;

			auto* p = (hash_t*)t.data();

			for (size_t i = 0; i < l; i++, p++)
				set(*p % lim_c);
		}

		template < typename T> bool Test(const T& t)
		{
			auto l = t.size() / sizeof(hash_t);
			if (hash_max_c && l > hash_max_c)
				l = hash_max_c;

			auto* p = (hash_t*)t.data();

			for (size_t i = 0; i < l; i++, p++) {
				if (!test(*p % lim_c))
					return false;
			}

			return true;
		}
	};

	template <size_t byte_c, typename hash_t = uint16_t, size_t hash_max_c = 0> class UniqueTally : public Bloom< byte_c, hash_t, hash_max_c>
	{
		size_t tally = 0;
	public:
		void Reset()
		{
			Bloom< byte_c, hash_t>::Reset();
			tally = 0;
		}

		template < typename T> void Tally(const T& t)
		{
			if (Bloom< byte_c, hash_t, hash_max_c>::Test(t))
				return;

			tally++;
			Bloom< byte_c, hash_t, hash_max_c>::Set(t);
		}

		size_t Tally()
		{
			return tally;
		}

		std::string b64()
		{
			return encode::base64(compress(*this));
		}

		void b64(std::string_view v, size_t _tally=0)
		{
			auto bin = decompress(decode::base64(v));
			copy(bin.begin(), bin.end(), Bloom< byte_c, hash_t, hash_max_c>::begin());
			tally = _tally;
		}
	};
}