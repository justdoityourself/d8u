/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <vector>

#include "cryptopp/sha.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/gzip.h"


#include "lzma/lzmalib.h"

#include "../gsl-lite.hpp"

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

		class DefaultHash : public array<uint8_t,32>
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

			template <typename T> DefaultHash(const T & data)
			{
				default_hash(data, *this);
			}

			template <typename D, typename T> DefaultHash(const D & domain, const T& data)
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

			void Finish(DefaultHash & finish)
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


		class Password : public array<uint8_t,64>
		{
		public:
			~Password()
			{
				for (auto& e : (*this))
					e = 0;
			}

			Password(){}

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

			const uint8_t * IV() const
			{
				return data() + 32;
			}
		};

#pragma pack ( pop )

		template <typename T> void encrypt(T& buffer_to_encrypt, const Password& password)
		{
			CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(password.Key(), 32, password.IV());
			cfbEncryption.ProcessData(buffer_to_encrypt.data(), buffer_to_encrypt.data(), buffer_to_encrypt.size());
		}

		template <typename T> vector<uint8_t> encrypt_copy(T& buffer_to_encrypt, const Password& password)
		{
			vector<uint8_t> output;
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

		/*void gzip(vector<uint8_t>& m, int level = 5)
		{
			CryptoPP::Gzip zipper(nullptr,level);    // 1 is fast, 9 is slow

			zipper.Put(m.data(), m.size());
			zipper.MessageEnd();

			m.resize(zipper.MaxRetrieveable());
			zipper.Get(m.data(), m.size());
		}*/
		
		void compress(vector<uint8_t> & m, int level = 5)
		{
			std::array<uint8_t, LZMA_PROPS_SIZE> props;
			size_t prop = LZMA_PROPS_SIZE;
			size_t fin_l = m.size() + 1024;
			vector<uint8_t> result(fin_l);

			int res = LzmaCompress(result.data(), &fin_l, m.data(), m.size(), props.data(), &prop, level, 4, -1, -1, -1, -1, 1);

			if(prop == LZMA_PROPS_SIZE && res == SZ_OK && fin_l < m.size())
			{
				result.resize(fin_l + sizeof(uint32_t));
				*(uint32_t*)(result.data() + fin_l) = (uint32_t)m.size();
				m = std::move(result);
			}
			else
			{
				uint32_t l = (uint32_t)m.size();
				m.insert(m.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
			}
		}

		template < typename T > vector<uint8_t> compress2(const T& m, int level = 5)
		{
			std::array<uint8_t, LZMA_PROPS_SIZE> props;
			size_t prop = LZMA_PROPS_SIZE;
			size_t fin_l = m.size() + 1024;
			vector<uint8_t> result(fin_l);

			int res = LzmaCompress(result.data(), &fin_l, (const unsigned char*)m.data(), m.size(), props.data(), &prop, level, 4, -1, -1, -1, -1, 1);

			if (prop == LZMA_PROPS_SIZE && res == SZ_OK && fin_l < m.size())
			{
				result.resize(fin_l + sizeof(uint32_t));
				*(uint32_t*)(result.data() + fin_l) = (uint32_t)m.size();
			}
			else
			{
				result.resize(m.size());
				std::copy(m.begin(), m.end(), result.begin());
				uint32_t l = (uint32_t)m.size();
				result.insert(result.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
			}

			return result;
		}

		void decompress(vector<uint8_t> & m)
		{
			std::array<uint8_t, LZMA_PROPS_SIZE> props = { 93,0,16,0,0 };

			uint32_t fin_l = *(uint32_t*)(m.data() + m.size() - sizeof(uint32_t));

			if (fin_l == m.size() - sizeof(uint32_t))
			{
				m.resize(fin_l);
				return;
			}

			vector<uint8_t> result(fin_l);
			size_t dest = (size_t)fin_l;
			size_t src = m.size() - sizeof(uint32_t);
			SRes res = LzmaUncompress( result.data(), &dest, m.data(), &src, props.data(), LZMA_PROPS_SIZE);

			if (res != SZ_OK)
				throw std::runtime_error("Decryption Error");
			
			m = std::move(result);
		}

		template <typename D, typename T> std::pair<DefaultHash, DefaultHash> identify(const D& domain, const T& buffer)
		{
			HashState state;
			state.Update(domain);
			state.Update(buffer);

			auto key = state.Finish();
			auto id = key;
			id.Iterate();

			return std::make_pair(key, id);
		}

		template <typename IN_OUT> DefaultHash encode2(IN_OUT& buffer,const DefaultHash & key, const DefaultHash & id, int level = 5)
		{
			compress(buffer,level);

			Password pw(key);
			encrypt(buffer, pw);

			DefaultHash check(buffer);

			buffer.insert(buffer.end(), id.begin(), id.end());
			buffer.insert(buffer.end(), check.begin(), check.end());

			return key;
		}

		template <typename D, typename IN_OUT> DefaultHash encode(const D& domain, IN_OUT& buffer, int level = 5)
		{
			DefaultHash key, id;

			std::tie(key, id) = identify(domain, buffer);

			return encode2(buffer, key, id, level);
		}

		template <typename IN_OUT> void quick_decode(IN_OUT& buffer, const DefaultHash & key)
		{
			buffer.resize( buffer.size() - sizeof(DefaultHash) * 2);

			Password pw(key);
			decrypt(buffer, pw);
			decompress(buffer);
		}

		template <typename D, typename IN_OUT> void decode(const D& domain, IN_OUT& buffer, const DefaultHash& key)
		{
			buffer.resize(buffer.size() - sizeof(DefaultHash) * 2);

			Password pw(key);
			decrypt(buffer, pw);
			decompress(buffer);

			HashState state;
			state.Update(domain);
			state.Update(buffer);

			auto check = state.Finish();

			if (!std::equal(key.begin(), key.end(), check.begin()))
				throw std::runtime_error("Decode Error");
		}

		template <typename T> DefaultHash& id_block(const T& buffer)
		{
			auto phash = (DefaultHash*) (buffer.end() - sizeof(DefaultHash)*2);
			return *phash;
		}

		template <typename T> bool validate_block(const T & buffer)
		{
			auto data = span<const uint8_t>(buffer.data(), buffer.size() - sizeof(DefaultHash) * 2);
			DefaultHash check(data);

			return std::equal(check.begin(), check.end(), buffer.end() - sizeof(DefaultHash));
		}
	}
}