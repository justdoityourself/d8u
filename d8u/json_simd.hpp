/* Copyright (C) 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <immintrin.h>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#include <string_view>

namespace d8u
{
	size_t find_avx2(const std::string_view sv, const char ch)
	{
		auto end = sv.data() + sv.size();
		const __m256i tocmp = _mm256_set1_epi8(ch);
		for (auto data = sv.data(); data < end; data += 32) 
		{
			const __m256i cmp = _mm256_cmpeq_epi8(*(__m256i*)data, tocmp);
			const int z = _mm256_movemask_epi8(cmp);
			if (z)
			{
				const int first_1_pos = _tzcnt_u32(z);
				return (data + first_1_pos) - sv.data();
			}
		}
	}

	size_t find_avx2_unaligned(const char* start, const char ch)
	{
		size_t align = (size_t)start % 32;
		const char* data = start - align;
		const __m256i tocmp = _mm256_set1_epi8(ch);

		const __m256i cmp = _mm256_cmpeq_epi8(*(__m256i*)data, tocmp);
		const int z = _mm256_movemask_epi8(cmp) >> align;
		if (z)
			return _tzcnt_u32(z);

		data += 32;

		for (; true; data += 32)
		{
			const __m256i cmp = _mm256_cmpeq_epi8(*(__m256i*)data, tocmp);
			const int z = _mm256_movemask_epi8(cmp);
			if (z)
				return (data + _tzcnt_u32(z)) - start;
		}
	}

	size_t find_avx(const std::string_view sv, const char ch)
	{
		auto end = sv.data() + sv.size();
		const __m128i tocmp = _mm_set1_epi8(ch);
		for (auto data = sv.data(); data < end; data += 16)
		{
			const __m128i cmp = _mm_cmpeq_epi8(*(__m128i*)data, tocmp);
			const int z = _mm_movemask_epi8(cmp);
			if (z)
			{
				const int first_1_pos = _tzcnt_u32(z);
				return (data + first_1_pos) - sv.data();
			}
		}
	}

	size_t find_avx_unaligned(const char* start, const char ch)
	{
		size_t align = (size_t)start % 16;
		const char* data = start - align;
		const __m128i tocmp = _mm_set1_epi8(ch);

		const __m128i cmp = _mm_cmpeq_epi8(*(__m128i*)data, tocmp);
		const int z = _mm_movemask_epi8(cmp) >> align;
		if (z)
			return _tzcnt_u32(z);

		data += 16;

		for (; true; data += 16)
		{
			const __m128i cmp = _mm_cmpeq_epi8(*(__m128i*)data, tocmp);
			const int z = _mm_movemask_epi8(cmp);
			if (z)
				return (data + _tzcnt_u32(z)) - start;
		}
	}

	size_t count_chars_8(std::string_view sv, const char ch)
	{
		size_t total = 0;
		for(const auto & c : sv) {
			if (c == ch)
				total += 1;
		}
		return total;
	}

	size_t count_chars_128(const std::string_view sv, const char ch)
	{
		size_t total = 0;
		//assert(size % 16 == 0);
		
		auto end = sv.data() + sv.size();
		const __m128i tocmp = _mm_set1_epi8(ch);
		for (auto data = sv.data(); data < end; data += 16) {
			__m128i chunk = _mm_load_si128((__m128i const*)data);
			__m128i results = _mm_cmpeq_epi8(chunk, tocmp);
			int mask = _mm_movemask_epi8(results);
			total += _mm_popcnt_u32(mask);
		}
		return total;
	}

	size_t count_chars_256(const std::string_view sv, const char ch)
	{
		size_t total = 0;
		//assert(size % 16 == 0);

		auto end = sv.data() + sv.size();
		const __m256i tocmp = _mm256_set1_epi8(ch);
		for (auto data = sv.data(); data < end; data += 32) {
			__m256i chunk = _mm256_load_si256((__m256i const*)data);
			__m256i results = _mm256_cmpeq_epi8(chunk, tocmp);
			int mask = _mm256_movemask_epi8(results);
			total += _mm_popcnt_u32(mask);
		}
		return total;
	}

	uint64_t prefix_xor(const uint64_t bits) {
		const __m128i all_ones = _mm_set1_epi8('\xFF');
		__m128i result = _mm_clmulepi64_si128(_mm_set_epi64x(0ULL, bits), all_ones, 0);
		return _mm_cvtsi128_si64(result); //result.m128i_i64[1];//
	}

	/*uint16_t prefix_xor16(const uint16_t bits) {
		const __m128i all_ones = _mm_set1_epi8('\xFF');
		__m128i result = _mm_clmulepi64_si128(_mm_set_epi64x(0ULL, bits), all_ones, 0);
		return result.m128i_u16[0];
	}*/

	inline uint64_t avx2_find64(const char * data, const __m256i & f)
	{
		const __m256i qm1 = _mm256_cmpeq_epi8(*(const __m256i*)data, f);
		const __m256i qm2 = _mm256_cmpeq_epi8(*(const __m256i*)(data + 32), f);

		uint64_t o = (uint32_t)_mm256_movemask_epi8(qm1);
		uint64_t t = (uint32_t)_mm256_movemask_epi8(qm2);
		t <<= 32;
		
		return t | o;
	}

	inline uint64_t avx2_find64_16(const char* data, const __m256i& f)
	{
		const __m256i qm1 = _mm256_cmpeq_epi8(*(const __m256i*)data, f);
		const __m256i qm2 = _mm256_cmpeq_epi8(*(const __m256i*)(data + 32), f);

		uint64_t o = (uint32_t)_mm256_movemask_epi8(qm1);
		uint64_t t = (uint32_t)_mm256_movemask_epi8(qm2);
		t <<= 32;

		return t | o;
	}

	size_t JsonAvx2ParseAlign64(const std::string_view sv, size_t depth = 0)
	{
		auto end = sv.data() + sv.size();

		const __m256i _q = _mm256_set1_epi8('"');
		const __m256i _o = _mm256_set1_epi8('{');
		const __m256i _c = _mm256_set1_epi8('}');

		for (auto data = sv.data(); data < end; data += 64)
		{
			auto str_nmask = ~prefix_xor(avx2_find64(data, _q));

			auto open_mask = avx2_find64(data, _o) & str_nmask;
			auto close_mask = avx2_find64(data, _c) & str_nmask;

			depth += _mm_popcnt_u64(open_mask); //_mm_popcnt_u64
			depth -= _mm_popcnt_u64(close_mask); //_mm_popcnt_u64
		}

		return 0;
	}

	/*size_t JsonAvxParseKVAlign16(const char* start, int depth = 0)
	{
		auto data = start;
		const __m128i _q = _mm_set1_epi8('"');
		const __m128i _o = _mm_set1_epi8('{');
		const __m128i _c = _mm_set1_epi8('}');

		int string_carry = 0;

		for (; true; data += 16)
		{
			const __m128i qm = _mm_cmpeq_epi8(*(const __m128i*)data, _q);

			auto m = _mm_movemask_epi8(qm) ^ string_carry;

			uint16_t str_mask = prefix_xor16(m);
			const uint16_t str_nmask = ~str_mask;
			string_carry = str_mask >> 15;

			const __m128i _open_mask = _mm_cmpeq_epi8(*(const __m128i*)data, _o);
			uint16_t open_mask = _mm_movemask_epi8(_open_mask) & str_nmask;

			const __m128i _close_mask = _mm_cmpeq_epi8(*(const __m128i*)data, _c);
			uint16_t close_mask = _mm_movemask_epi8(_close_mask) & str_nmask;

			auto open_count = __popcnt16(open_mask);
			auto close_count = __popcnt16(close_mask);

			depth += open_count - close_count;
			if (depth <= 0)
				return data - start + _lzcnt_u32(close_mask);
		}

		return -1;
	}*/

	size_t JsonAvx2ParseKVAlign64(const char* start, int depth = 0)
	{
		auto data = start;
		const __m256i _q = _mm256_set1_epi8('"');
		const __m256i _o = _mm256_set1_epi8('{');
		const __m256i _c = _mm256_set1_epi8('}');
		//const __m256i _v = _mm256_set1_epi8(':');
		//const __m256i _n = _mm256_set1_epi8(',');

		int string_carry = 0;

		for (; true; data += 64)
		{
			const uint64_t t = avx2_find64(data, _q) ^ string_carry;
			const auto str_mask = prefix_xor(t);
			const auto str_nmask = ~str_mask;
			string_carry = str_mask >> 63;

			auto open_mask = avx2_find64(data, _o) & str_nmask;
			auto close_mask = avx2_find64(data, _c) & str_nmask;

			auto open_count = _mm_popcnt_u64(open_mask);
			auto close_count = _mm_popcnt_u64(close_mask);


			depth += open_count - close_count;
			if (depth <= 0)
				return data - start + _lzcnt_u64(close_mask);
		}

		return -1;
	}

	size_t JsonMoveTrim(std::string_view json , char* out)
	{
		char* start = out;
		size_t depth = 0;
		const char* itr = json.data();
		const char* end = itr + json.size();

		while(itr < end)
		{
			switch (*itr)
			{
			case '\"':
				*out++ = *itr++;

				//start = itr;
				while (*itr != '"' && itr < end)
					*out++ = *itr++;

				*out++ = *itr++;

				break;
			case ' ':
			case '\n':
			case '\t':
				itr++;
				break;
			default:
				*out++ = *itr++;
				break;
			}
		}

		return out - start;
	}

	std::string_view JsonInplaceTrim(std::string_view json)
	{
		char* out=(char*)json.data();
		size_t depth = 0;
		const char* itr = json.data();
		const char* end = itr + json.size();

		while (itr < end)
		{
			switch (*itr)
			{
			case '\"':
				*out++ = *itr++;

				//start = itr;
				while (*itr != '"' && itr < end)
					*out++ = *itr++;

				*out++ = *itr++;

				break;
			case ' ':
			case '\n':
			case '\t':
				itr++;
				break;
			default:
				*out++ = *itr++;
				break;
			}
		}

		return std::string_view(json.data(),out - json.data());
	}

	std::string_view JsonAvx2InplaceTrim(std::string_view json)
	{
		/*
		for (int i = 0; i < count; i += 4) {
			__m128 val = _mm_load_ps(input + i);
			__m128 mask = _mm_cmpge_ps(val, _mm_set1_ps(limit));
			__m128 result = LeftPack(mask, val);
			_mm_storeu_ps(output, result);

			output += _popcnt(_mm_movemask_ps(mask));
		}



		__m128i LeftPack_SSSE3(__m128 mask, __m128 val)
		{
			// Move 4 sign bits of mask to 4-bit integer value.
			int mask = _mm_movemask_ps(mask);
			// Select shuffle control data
			__m128i shuf_ctrl = _mm_load_si128(&shufmasks[mask]);
			// Permute to move valid values to front of SIMD register
			__m128i packed = _mm_shuffle_epi8(_mm_castps_si128(val), shuf_ctrl);
			return packed;
		}



		*/

		char* out = (char*)json.data();
		size_t depth = 0;
		const char* itr = json.data();
		const char* end = itr + json.size();

		while (itr < end)
		{
			switch (*itr)
			{
			case '\"':
				*out++ = *itr++;

				//start = itr;
				while (*itr != '"' && itr < end)
					*out++ = *itr++;

				*out++ = *itr++;

				break;
			case ' ':
			case '\n':
			case '\t':
				itr++;
				break;
			default:
				*out++ = *itr++;
				break;
			}
		}

		return std::string_view(json.data(), out - json.data());
	}

	size_t JsonAvx2Parse(const char * start, int depth = 0)
	{
		const __m256i _q = _mm256_set1_epi8('"');
		const __m256i _o = _mm256_set1_epi8('{');
		const __m256i _c = _mm256_set1_epi8('}');

		const size_t align = (size_t)start % 32;
		const char* data = start - align;
		int string_carry = 0;

		{
			const uint64_t t = avx2_find64(data, _q);
			const auto str_mask = prefix_xor(t>>align);
			const auto str_nmask = ~str_mask;
			string_carry = str_mask >> 63;

			auto open_mask = (avx2_find64(data, _o) >> align) & str_nmask;
			auto close_mask = (avx2_find64(data, _c) >> align)  & str_nmask;

			auto close_count = _mm_popcnt_u64(close_mask);

			if (!close_count) [[likely]]
			{
				auto open_count = _mm_popcnt_u64(open_mask);
				depth += open_count - close_count;
			}
			else [[unlikely]]
			{
				/*auto scan = 0;
				while (scan < 64)
				{
					auto lim1 = _tzcnt_u64(open_mask >> scan);
					auto lim2 = _tzcnt_u64(close_mask >> scan);

					if (lim1 == lim2)
					{
						scan += lim2 + 1;
						continue;
					}
					else if (lim1 < lim2)
					{
						scan += lim1 + 1;
						depth++;
					}
					else
					{
						scan += lim2 + 1;
						depth--;

						if (depth <= 0)
							return scan;
					}
				}*/
				for (auto scan = 0; open_mask || close_mask;)
				{
					auto lim1 = _tzcnt_u64(open_mask);
					auto lim2 = _tzcnt_u64(close_mask);

					if (lim1 < lim2)
					{
						scan += lim1 + 1;
						open_mask >>= lim1 + 1;
						close_mask >>= lim1 + 1;
						depth++;
					}
					else
					{
						scan += lim2 + 1;
						open_mask >>= lim2 + 1;
						close_mask >>= lim2 + 1;
						depth--;

						if (!depth)
							return scan;
					}
				}
			}

			data += 64;
		}

		for (; true; data += 64)
		{
			const uint64_t t = avx2_find64(data, _q) ^ string_carry;
			const auto str_mask = prefix_xor(t);
			const auto str_nmask = ~str_mask;
			string_carry = str_mask >> 63;

			auto open_mask = avx2_find64(data, _o) & str_nmask;
			auto close_mask = avx2_find64(data, _c) & str_nmask;

			auto close_count = _mm_popcnt_u64(close_mask);

			if (close_count < depth) [[likely]]
			{
				auto open_count = _mm_popcnt_u64(open_mask);
				depth += open_count - close_count;
			}
			else [[unlikely]]
			{
				/*auto scan = 0;
				while (scan < 64)
				{
					auto lim1 = _tzcnt_u64(open_mask >> scan);
					auto lim2 = _tzcnt_u64(close_mask >> scan);

					if (lim1 == lim2)
					{
						scan += lim2 + 1;
						continue;
					}
					else if (lim1 < lim2)
					{
						scan += lim1 + 1;
						depth++;
					}
					else
					{
						scan += lim2 + 1;
						depth--;

						if (depth <= 0)
							return data - start + scan;
					}
				}*/
				for (auto scan = 0; open_mask || close_mask;)
				{
					auto lim1 = _tzcnt_u64(open_mask);
					auto lim2 = _tzcnt_u64(close_mask);

					if (lim1 < lim2)
					{
						scan += lim1 + 1;
						open_mask >>= lim1 + 1;
						close_mask >>= lim1 + 1;
						depth++;
					}
					else
					{
						scan += lim2 + 1;
						open_mask >>= lim2 + 1;
						close_mask >>= lim2 + 1;
						depth--;

						if (!depth)
							return data - start + scan;
					}
				}
			}
		}

		return -1;
	}

	inline uint64_t avx2_greater64(const char* data, const __m256i& f)
	{
		const __m256i qm1 = _mm256_cmpgt_epi8(*(const __m256i*)data, f);
		const __m256i qm2 = _mm256_cmpgt_epi8(*(const __m256i*)(data + 32), f);

		uint64_t o = (uint32_t)_mm256_movemask_epi8(qm1);
		uint64_t t = (uint32_t)_mm256_movemask_epi8(qm2);
		t <<= 32;

		return t | o;
	}

	size_t JsonAvx2ParseSafe(const char* start, int depth = 0)
	{
		const __m256i _q = _mm256_set1_epi8('"');
		const __m256i _o = _mm256_set1_epi8('{');
		const __m256i _c = _mm256_set1_epi8('}');
		const __m256i _lim = _mm256_set1_epi8(127);
		const __m256i _esc = _mm256_set1_epi8('\\');

		//TODO CHECK for any _lim or esc and then use iterative parsing for this block

		const size_t align = (size_t)start % 32;
		const char* data = start - align;
		int string_carry = 0;

		{
			const uint64_t t = avx2_find64(data, _q);
			const auto str_mask = prefix_xor(t >> align);
			const auto str_nmask = ~str_mask;
			string_carry = str_mask >> 63;

			auto open_mask = (avx2_find64(data, _o) >> align) & str_nmask;
			auto close_mask = (avx2_find64(data, _c) >> align) & str_nmask;

			auto close_count = _mm_popcnt_u64(close_mask);

			if (!close_count) [[likely]]
			{
				auto open_count = _mm_popcnt_u64(open_mask);
				depth += open_count - close_count;
			}
			else [[unlikely]]
			{
				/*auto scan = 0;
				while (scan < 64)
				{
					auto lim1 = _tzcnt_u64(open_mask >> scan);
					auto lim2 = _tzcnt_u64(close_mask >> scan);

					if (lim1 == lim2)
					{
						scan += lim2 + 1;
						continue;
					}
					else if (lim1 < lim2)
					{
						scan += lim1 + 1;
						depth++;
					}
					else
					{
						scan += lim2 + 1;
						depth--;

						if (depth <= 0)
							return scan;
					}
				}*/
				for (auto scan = 0; open_mask || close_mask;)
				{
					auto lim1 = _tzcnt_u64(open_mask);
					auto lim2 = _tzcnt_u64(close_mask);

					if (lim1 < lim2)
					{
						scan += lim1 + 1;
						open_mask >>= lim1 + 1;
						close_mask >>= lim1 + 1;
						depth++;
					}
					else
					{
						scan += lim2 + 1;
						open_mask >>= lim2 + 1;
						close_mask >>= lim2 + 1;
						depth--;

						if (!depth)
							return scan;
					}
				}
			}

			data += 64;
		}

		for (; true; data += 64)
		{
			const uint64_t t = avx2_find64(data, _q) ^ string_carry;
			const auto str_mask = prefix_xor(t);
			const auto str_nmask = ~str_mask;
			string_carry = str_mask >> 63;

			auto open_mask = avx2_find64(data, _o) & str_nmask;
			auto close_mask = avx2_find64(data, _c) & str_nmask;

			auto close_count = _mm_popcnt_u64(close_mask);

			if (close_count < depth) [[likely]]
			{
				auto open_count = _mm_popcnt_u64(open_mask);
				depth += open_count - close_count;
			}
			else [[unlikely]]
			{
				/*auto scan = 0;
				while (scan < 64)
				{
					auto lim1 = _tzcnt_u64(open_mask >> scan);
					auto lim2 = _tzcnt_u64(close_mask >> scan);

					if (lim1 == lim2)
					{
						scan += lim2 + 1;
						continue;
					}
					else if (lim1 < lim2)
					{
						scan += lim1 + 1;
						depth++;
					}
					else
					{
						scan += lim2 + 1;
						depth--;

						if (depth <= 0)
							return data - start + scan;
					}
				}*/
				for (auto scan = 0; open_mask || close_mask;)
				{
					auto lim1 = _tzcnt_u64(open_mask);
					auto lim2 = _tzcnt_u64(close_mask);

					if (lim1 < lim2)
					{
						scan += lim1 + 1;
						open_mask >>= lim1 + 1;
						close_mask >>= lim1 + 1;
						depth++;
					}
					else
					{
						scan += lim2 + 1;
						open_mask >>= lim2 + 1;
						close_mask >>= lim2 + 1;
						depth--;

						if (!depth)
							return data - start + scan;
					}
				}
			}
		}

		return -1;
	}

	size_t JsonAvx2ParseTest(const char* start, int depth = 0)
	{
		/*///////////////////////////
		const char* itr = start;
		bool is_string = false;
		size_t depth2 = 0;
		//////////////////////////////////////////*/

		const __m256i _q = _mm256_set1_epi8('"');
		const __m256i _o = _mm256_set1_epi8('{');
		const __m256i _c = _mm256_set1_epi8('}');

		size_t align = (size_t)start % 32;
		const char* data = start - align;
		int string_carry = 0;


		for (; true; data += 64,align=0)
		{
			/*if (data + align != itr)
				std::cout << "FDGA";

			if (depth != depth2)
				std::cout << "FDGA";

			////////////////////////////////////////////////////
			int delta1 = 0;
			int delta2 = 0;

			for (size_t i = 0; i < 64-align; itr++,i++)
			{
				switch (*itr)
				{
				case '\"':
					is_string = !is_string;

					break;
				case '{':
					if (!is_string)
					{
						delta1++;
						depth2++;
					}


					break;

				case '}':
					if (!is_string)
					{
						depth2--;
						delta1--;
					}

					if (!depth2)
					{
						i = 99;
						continue;
					}
				default:
					break;
				}
			}
			////////////////////////////////////////////////////*/

			const uint64_t t = avx2_find64(data, _q) ^ string_carry;
			const auto str_mask = prefix_xor(t);
			const auto str_nmask = ~str_mask;
			string_carry = str_mask >> 63;

			auto open_mask = avx2_find64(data, _o) & str_nmask;
			auto close_mask = avx2_find64(data, _c) & str_nmask;

			auto close_count = _mm_popcnt_u64(close_mask);

			if(close_count < depth) [[likely]]
			{
				auto open_count = _mm_popcnt_u64(open_mask);
				depth += open_count - close_count;
			}
			else [[unlikely]]
			{
				auto scan = align;
				while (scan < 64)
				{
					auto lim1 = _tzcnt_u64(open_mask >> scan);
					auto lim2 = _tzcnt_u64(close_mask >> scan);

					if (lim1 == lim2)
					{
						scan += lim2 + 1;
						continue;
					}
					else if (lim1 < lim2)
					{
						//std::cout << "S+ " << lim1 + 1 << std::endl;
						scan += lim1 + 1;
						depth++;

						//////////////////////////////////////////
						//delta2++;
					}
					else
					{
						////////////////////////////////////////////
						//delta2--;

						//std::cout << "S- " << lim2 + 1 << std::endl;

						scan += lim2 + 1;
						depth--;

						if (!depth)
							return data - start + scan;
					}
				}
			}

			/*////////////////////////////////////////////////////////
			if (is_string != string_carry)
				std::cout << "fsdagas";

			if (delta1 != delta2)
				std::cout << "HERE";*/
		}

		return -1;
	}



	template<bool simd_e = true> size_t PartialAvxObjectParse(std::string_view json, size_t depth = 0)
	{
		const char* itr = json.data();
		const char* end = itr + json.size();

		for (; itr < end; itr++)
		{
			switch (*itr)
			{
			case '\"':
				itr++;

				if constexpr (simd_e)
					itr += find_avx2_unaligned(itr, '"');
				else
				{
					while (*itr != '"')
						itr++;
				}

				break;
			case '{':
				depth++;

				break;

			case '}':
				if (!depth)
					return ++itr - json.data();

				depth--;

				break;
			}
		}

		return json.size();
	}
}