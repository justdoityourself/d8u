/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <vector>

#include "cryptopp/aes.h"
#include "cryptopp/modes.h"

#include "hash.hpp"



namespace d8u
{
	namespace transform
	{
		template <typename T> void encrypt(T& buffer_to_encrypt, const Password& password)
		{
			CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(password.Key(), 32, password.IV());
			cfbEncryption.ProcessData(buffer_to_encrypt.data(), buffer_to_encrypt.data(), buffer_to_encrypt.size());
		}

		template <typename T> d8u::sse_vector encrypt_copy(T& buffer_to_encrypt, const Password& password)
		{
			d8u::sse_vector output;
			output.resize(buffer_to_encrypt.size());

			CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(password.Key(), 32, password.IV());
			cfbEncryption.ProcessData(output.data(), buffer_to_encrypt.data(), buffer_to_encrypt.size());

			return output;
		}

		template <typename T> void decrypt(T& butter_to_decrypt, const Password& password)
		{
			CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(password.Key(), 32, password.IV());
			cfbDecryption.ProcessData(butter_to_decrypt.data(), butter_to_decrypt.data(), butter_to_decrypt.size());
		}
	}
}