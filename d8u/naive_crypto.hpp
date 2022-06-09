/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <vector>

#include "hash.hpp"
#include "memory.hpp"
//#include "util.hpp"

//
// Purpose of this file is to allow easy building and as much function as possible for evaluation
// without the hassle of dependencies until POC is complete.
//


namespace d8u
{
	namespace transform
	{
		template <typename T> void encrypt(T& buffer_to_encrypt, const Password& password)
		{
			//Do nothing
			return;
		}

		template <typename T> d8u::sse_vector encrypt_copy(T& buffer_to_encrypt, const Password& password)
		{
			d8u::sse_vector output;
			output.resize(buffer_to_encrypt.size());

			std::memcpy(output.data(), buffer_to_encrypt.data(), buffer_to_encrypt.size());

			return output;
		}

		template <typename T> void decrypt(T& buffer_to_decrypt, const Password& password)
		{
			//Do nothing
			return;
		}
	}

	template < typename D > void random_bytes_secure(D& d)
	{
		for (size_t i = 0; i < d.size(); i++)
			*((uint8_t*)(d.data() + i)) = (uint8_t)i;
	}

	namespace crypto
	{
		class PrivatePassword
		{
		public:
			PrivatePassword() {}

			PrivatePassword(const std::string& _s)
			{
				Import(_s);
			}

			void Import(const std::string& _s)
			{
			}

			std::string Export() const
			{
				return "";
			}

			template <typename T> d8u::sse_vector Sign(const T& m) const
			{
				return d8u::sse_vector();
			}

			template < typename T > d8u::sse_vector Decrypt(const T& m) const
			{
				return d8u::sse_vector();
			}
		};

		class PublicPassword
		{
		public:
			PublicPassword() {}

			PublicPassword(const std::string& _s)
			{
				Import(_s);
			}

			void Import(const std::string& _s)
			{
			}

			void ImportSelfSigned(const std::string& _s)
			{
			}

			std::string ExportSelfSigned(const PrivatePassword& private_key)
			{
				return "";
			}

			std::string Export()
			{
				return "";
			}

			template < typename T, typename V > bool Verify(const T& m, const V& h)
			{
				return false;
			}

			template < typename T >d8u::sse_vector Encrypt(const T& m)
			{
				return d8u::sse_vector();
			}
		};

		class KeyPair
		{
		public:
			PublicPassword public_key;
			PrivatePassword private_key;

			void Import(const std::string& _s)
			{
			}

			std::string Export()
			{
				return "";
			}

			static KeyPair Create(uint32_t bits = 2048)
			{
				return { PublicPassword(), PrivatePassword() };
			}
		};
	}
}