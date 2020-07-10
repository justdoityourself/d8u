/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "cryptopp/sha.h"

namespace d8u
{
	namespace transform
	{
		using gsl::span;

		using std::array;
		using std::vector;

		template <typename T, typename Y> void default_hash(const T& input_buffer, Y& output_buffer)
		{
			CryptoPP::SHA256().CalculateDigest(output_buffer.data(), (const CryptoPP::byte*) input_buffer.data(), input_buffer.size());
		}

		template <typename T, typename Y> void long_hash(const T& input_buffer, Y& output_buffer)
		{
			CryptoPP::SHA512().CalculateDigest(output_buffer.data(), (const CryptoPP::byte*)input_buffer.data(), input_buffer.size());
		}

#pragma pack ( push, 1 )

		class HashState;

		class DefaultHash : public array<uint8_t, 32>
		{
		public:

			DefaultHash() {}

			void Zero()
			{
				for (auto& e : (*this))
					e = 0;
			}

			bool IsZero() const
			{
				for (auto& e : (*this))
				{
					if (e != 0)
						return false;
				}

				return true;
			}

			template <typename T> DefaultHash(const T& data)
			{
				default_hash(data, *this);
			}

			template <typename D, typename T> DefaultHash(const D& domain, const T& data)
			{
				HashState state;
				state.Update(domain);
				state.Update(data);
				*this = state.Finish();
			}

			template <typename T> void Hash(const T& data)
			{
				default_hash(data, *this);
			}

			void Iterate()
			{
				default_hash(*this, *this);
			}

			DefaultHash Next() const
			{
				DefaultHash res;
				res.Hash(*this);
				return res;
			}
		};

		class HashState
		{
		public:

			template<typename T> void Update(T& r)
			{
				state.Update(r.data(), r.size());
			}

			void Finish(DefaultHash& finish)
			{
				state.Final(finish.data());
			}

			DefaultHash Finish()
			{
				DefaultHash finish;

				state.Final(finish.data());

				return finish;
			}

			CryptoPP::SHA256 state;
		};


		class Password : public array<uint8_t, 64>
		{
		public:
			~Password()
			{
				for (auto& e : (*this))
					e = 0;
			}

			Password() {}

			template<typename T> Password(const T& r)
			{
				long_hash(r, *this);
			}

			void Iterate()
			{
				long_hash(*this, *this);
			}

			const uint8_t* Key() const
			{
				return data();
			}

			const uint8_t* IV() const
			{
				return data() + 32;
			}
		};

#pragma pack ( pop )
	}

	namespace hash
	{
		template <typename T, typename Y> void sha1(const T& input_buffer, Y& output_buffer)
		{
			CryptoPP::SHA1().CalculateDigest(output_buffer.data(), (const CryptoPP::byte*) input_buffer.data(), input_buffer.size());
		}
	}
}