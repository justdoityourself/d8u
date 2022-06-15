/* Copyright (C) 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "minilzo/minilzo.h"
#include <stdexcept>
#include <vector>

namespace d8u
{
	class MiniLZO
	{
	public:
		MiniLZO()
		{
			if (lzo_init() != LZO_E_OK)
				throw std::runtime_error("Failed to start MINILZO");
		}
		~MiniLZO() {}
	};

	auto decompress(const auto & m)
	{
		static MiniLZO Context;

		uint32_t fin_l = *(uint32_t*)(m.data() + m.size() - sizeof(uint32_t));
		fin_l &= 0x00ffffff;

		std::vector<uint8_t> result(fin_l);

		if (fin_l == m.size() - sizeof(uint32_t))
			copy((uint8_t*)m.data(), (uint8_t*)m.data() + fin_l, result.begin());
		else if(lzo_uint output_length = (lzo_uint)result.size(); LZO_E_OK != lzo1x_decompress((uint8_t*)m.data(), (lzo_uint)m.size() - sizeof(uint32_t), result.data(), &output_length, NULL) || output_length != result.size())
			throw std::runtime_error("LZMA Decompression Error");

		return result;
	}

	auto compress(const auto & m, int level = 5)
	{
		static MiniLZO Context;

		std::array<uint8_t, LZO1X_1_MEM_COMPRESS> workspace;

		std::vector<uint8_t> result(m.size() + 1024 * 65);

		lzo_uint output_length = (lzo_uint)m.size() + 1024;
		if (LZO_E_OK != lzo1x_1_compress((uint8_t*)m.data(), (lzo_uint)m.size(), result.data(), &output_length, workspace.data()))
			output_length = result.size();

		uint32_t l = (uint32_t)m.size() | 0x02000000;

		if (output_length >= m.size())
		{
			result.resize(m.size());
			std::copy(m.begin(), m.end(), result.begin());
		}
		else
			result.resize(output_length);

		result.insert(result.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));

		return result;
	}
}