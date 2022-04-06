/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <vector>
#include <array>

#include "tomcrypt.h"

#include "../gsl-lite.hpp"

namespace d8u
{
	namespace transform
	{
		using gsl::span;

		using std::array;
		using std::vector;

		template <typename T, typename Y> void _default_hash(const T& input_buffer, Y& output_buffer)
		{
			hash_state md;
			sha256_desc.init(&md);
			sha256_desc.process(&md, (const unsigned char*)input_buffer.data(), (unsigned long)input_buffer.size());
			sha256_desc.done(&md, (unsigned char*)output_buffer.data());
		}

		template <typename T, typename Y> void _long_hash(const T& input_buffer, Y& output_buffer)
		{
			hash_state md;
			sha512_desc.init(&md);
			sha512_desc.process(&md, (const unsigned char*)input_buffer.data(), (unsigned long)input_buffer.size());
			sha512_desc.done(&md, (unsigned char*)output_buffer.data());
		}

#pragma pack ( push, 1 )

		class _DefaultHash : public array<uint8_t, 32>
		{
		public:

			_DefaultHash() {}

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

			template <typename T> _DefaultHash(const T& data)
			{
				_default_hash(data, *this);
			}

			template <typename D, typename T> _DefaultHash(const D& domain, const T& data)
			{
				State state;
				state.Update(domain);
				state.Update(data);
				*this = state.Finish();
			}

			template <typename T> void Hash(const T& data)
			{
				_default_hash(data, *this);
			}

			_DefaultHash GetNext() const
			{
				_DefaultHash result;

				_default_hash(*this, result);

				return result;
			}

			class State
			{
			public:

				State() {
					sha256_desc.init(&md);
				}

				template<typename T> void Update(T& r)
				{
					sha256_desc.process(&md, (const unsigned char*)r.data(), (unsigned long)r.size());
				}

				void Finish(_DefaultHash& finish)
				{
					sha256_desc.done(&md, (unsigned char*)finish.data());
				}

				_DefaultHash Finish()
				{
					_DefaultHash finish;

					sha256_desc.done(&md, (unsigned char*)finish.data());

					return finish;
				}

				template < typename T> T FinishT()
				{
					T finish;

					sha256_desc.done(&md, (unsigned char*)finish.data());

					return finish;
				}

				hash_state md;
				
				
			};
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
				_long_hash(r, *this);
			}

			void Iterate()
			{
				_long_hash(*this, *this);
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
			hash_state md;
			sha1_desc.init(&md);
			sha1_desc.process(&md, (const unsigned char*)input_buffer.data(), (unsigned long)input_buffer.size());
			sha1_desc.done(&md, (unsigned char*)output_buffer.data());
		}

		template <typename T, typename Y> void sha256(const T& input_buffer, Y& output_buffer)
		{
			hash_state md;
			sha256_desc.init(&md);
			sha256_desc.process(&md, (const unsigned char*)input_buffer.data(), (unsigned long)input_buffer.size());
			sha256_desc.done(&md, (unsigned char*)output_buffer.data());
		}

		template <typename T, typename Y> void sha512(const T& input_buffer, Y& output_buffer)
		{
			hash_state md;
			sha512_desc.init(&md);
			sha512_desc.process(&md, (const unsigned char*)input_buffer.data(), (unsigned long)input_buffer.size());
			sha512_desc.done(&md, (unsigned char*)output_buffer.data());
		}

		class Sha256State
		{
		public:

			Sha256State() {
				sha256_desc.init(&md);
			}

			template<typename T> void Update(T& r)
			{
				sha256_desc.process(&md, (const unsigned char*)r.data(), (unsigned long)r.size());
			}

			template < typename T >void Finish(T& finish)
			{
				sha256_desc.done(&md, (unsigned char*)finish.data());
			}

			template<typename T> T Finish()
			{
				T finish;

				sha256_desc.done(&md, (unsigned char*)finish.data());

				return finish;
			}

			hash_state md;	
		};
	}

	namespace custom_hash
	{
		using gsl::span;

		using std::array;
		using std::vector;

#pragma pack ( push, 1 )

		template < typename T > class DefaultHashT : public array<uint8_t, T::byte_size>
		{
		public:
			using State = T;

			DefaultHashT() {}

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

			void operator=(const DefaultHashT<T>& r)
			{
				std::copy(r.begin(), r.end(), array<uint8_t, T::byte_size>::begin());
			}

			/*void operator=(const array<uint8_t, T::byte_size>& r)
			{
				std::copy(r.begin(), r.end(), array<uint8_t, T::byte_size>::begin());
			}*/

			DefaultHashT(const DefaultHashT<T>& r)
			{
				std::copy(r.begin(), r.end(), array<uint8_t, T::byte_size>::begin());
			}

			/*explicit DefaultHashT(const array<uint8_t, T::byte_size>& r)
			{
				std::copy(r.begin(), r.end(), array<uint8_t, T::byte_size>::begin());
			}*/

			template <typename B> explicit DefaultHashT(const B& data)
			{
				T::hash(data, *this);
			}

			template <typename D, typename B> DefaultHashT(const D& domain, const B& data)
			{
				T state;
				state.Update(domain);
				state.Update(data);
				*this = state.template FinishT< DefaultHashT>();
			}

			template <typename B> void Hash(const B& data)
			{
				T::hash(data, *this);
			}

			DefaultHashT GetNext() const
			{
				alignas(16) DefaultHashT result;
				T::hash(*this, result);

				return result;
			}
		};

#pragma pack ( pop )
	}
}