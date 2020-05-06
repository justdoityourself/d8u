/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "cryptopp/sha.h"

namespace d8u
{
	namespace hash
	{
		template <typename T, typename Y> void sha1(const T& input_buffer, Y& output_buffer)
		{
			CryptoPP::SHA1().CalculateDigest(output_buffer.data(), (const CryptoPP::byte*) input_buffer.data(), input_buffer.size());
		}
	}
}