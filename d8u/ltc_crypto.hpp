/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <vector>
#include <chrono>

#include "tomcrypt.h"


#include "hash.hpp"
#include "memory.hpp"
#include "encode.hpp"


namespace d8u
{
	namespace transform
	{
		template <typename T> void encrypt(T& buffer_to_encrypt, const Password& password)
		{
			//unsigned long taglen=16;
			//unsigned char tag[16] = { 0xB7,0x2D,0x46,0x6F,0xFC,0xFE,0x4E,0x75,0x98,0x92,0x85,0x96,0xA7,0x56,0xC7,0xEB };

			//if(gcm_memory(/*find_cipher("aes")*/0, (const unsigned char*)password.Key(), 32, (const unsigned char*)password.IV(), 16, NULL, NULL, buffer_to_encrypt.data(), buffer_to_encrypt.size(), buffer_to_encrypt.data(), tag, &taglen, GCM_ENCRYPT))
			//	throw "gcm error";
			symmetric_OFB cbc;

			auto status = ofb_start(0, (const unsigned char*)password.IV(), (const unsigned char*)password.Key(), 32, 0, &cbc);
			if (status) throw "aes error";
			
			status = ofb_encrypt((const unsigned char*)buffer_to_encrypt.data(), (unsigned char*)buffer_to_encrypt.data(), buffer_to_encrypt.size(), &cbc);
			if (status) throw "aes error";

		}

		template <typename T> d8u::sse_vector encrypt_copy(T& buffer_to_encrypt, const Password& password)
		{
			d8u::sse_vector output;
			output.resize(buffer_to_encrypt.size());

			//unsigned long taglen = 0;
			//unsigned char tag[16] = { 0xB7,0x2D,0x46,0x6F,0xFC,0xFE,0x4E,0x75,0x98,0x92,0x85,0x96,0xA7,0x56,0xC7,0xEB };

			//if (gcm_memory(/*find_cipher("aes")*/0, (const unsigned char*)password.Key(), 32, (const unsigned char*)password.IV(), 16, NULL, NULL, buffer_to_encrypt.data(), buffer_to_encrypt.size(), output.data(), tag, &taglen, GCM_ENCRYPT))
			//	throw "gcm error";

			return output;
		}

		template <typename T> void decrypt(T& buffer_to_decrypt, const Password& password)
		{
			//unsigned long taglen = 0;
			//unsigned char tag[16] = { 0xB7,0x2D,0x46,0x6F,0xFC,0xFE,0x4E,0x75,0x98,0x92,0x85,0x96,0xA7,0x56,0xC7,0xEB };

			//auto res = gcm_memory(/*find_cipher("aes")*/0, (const unsigned char*)password.Key(), 32, (const unsigned char*)password.IV(), 16, NULL, NULL, buffer_to_decrypt.data(), buffer_to_decrypt.size(), buffer_to_decrypt.data(), tag, &taglen, GCM_DECRYPT);

			//if(res)
			//	throw "gcm error";
			symmetric_OFB cbc;

			auto status = ofb_start(0, (const unsigned char*)password.IV(), (const unsigned char*)password.Key(), 32, 0, &cbc);
			if (status) throw "aes error";
			
			status = ofb_decrypt((const unsigned char*)buffer_to_decrypt.data(), (unsigned char*)buffer_to_decrypt.data(), buffer_to_decrypt.size(), &cbc);
			if (status) throw "aes error";
		}
	}

	template < typename D > void random_bytes_secure(D& d)
	{
		using namespace std::chrono;

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		prng_state prng;
		
		if (yarrow_start(&prng)) 
			throw "yarrow_error";
		
		if (yarrow_add_entropy((const unsigned char*) & t1, sizeof(t1), &prng))
			throw "yarrow_error";
		
		if (yarrow_ready(&prng))
			throw "yarrow_error";

		yarrow_read((uint8_t*)d.data(), d.size(), &prng);
	}

	namespace crypto
	{
		class PrivatePassword
		{
			rsa_key key;
			const int bitsize = 4096;
			const size_t bytesize = bitsize * 8 / 8; //Guess
		public:
			PrivatePassword() { }

			PrivatePassword(rsa_key& _key):key(_key) { }

			~PrivatePassword() {
				rsa_free(&key);
			}

			PrivatePassword(const std::string& _s)
			{
				Import(_s);
			}

			void Import(const std::string& _s)
			{
				auto s = d8u::decode::url(_s);
				if (rsa_import((const unsigned char*)s.data(), s.size(), &key))
					throw "rsa error";
			}

			std::string Export() const
			{
				std::vector<uint8_t> buffer;
				buffer.resize(bytesize);
				unsigned long pvlen = bytesize;

				if (rsa_export(buffer.data(), &pvlen, PK_PRIVATE, &key))
					throw "rsa error";

				buffer.resize(pvlen);
				
				return d8u::encode::url(buffer);
			}

			template <typename T> std::string Sign(const T& m) const
			{
				unsigned char hash[64];
				hash_state md;
				sha512_desc.init(&md);
				sha512_desc.process(&md, (const unsigned char*)m.data(), (unsigned long)m.size());
				sha512_desc.done(&md, hash);

				std::vector<uint8_t> buffer;
				buffer.resize(bytesize);
				unsigned long siglen = bytesize;

				if (rsa_sign_hash_ex(hash, sha512_desc.hashsize, buffer.data(), &siglen, LTC_PKCS_1_V1_5, NULL, 0/*prng_idx*/, 0/*sha512_desc*/, 0/*saltlen*/, &key))
					throw "rsa error";

				buffer.resize(siglen);

				return d8u::encode::url(buffer);
			}

			template < typename T > std::vector<uint8_t> Decrypt(const T& s) const
			{
				auto m = d8u::decode::url(s);

				std::vector<uint8_t> buffer;
				buffer.resize(bytesize);
				unsigned long declen = bytesize;

				int res = 0;

				if (rsa_decrypt_key(m.data(), m.size(), buffer.data(), &declen, nullptr, 0, 0/*sha512_desc*/,&res, &key)) 
					throw "rsa error";

				if (res != 1) throw "rsa_error";

				buffer.resize(declen);

				return buffer;
			}
		};

		class PublicPassword
		{
			rsa_key key;
			const int bitsize = 4096;
			const size_t bytesize = bitsize * 8 / 8; //Guess

		public:
			PublicPassword() {}
			PublicPassword(rsa_key& _key, bool dup=false) :key(_key) 
			{ 
				if (dup) {
					auto exp = Export();
					Import(exp);
				}
			}

			PublicPassword(const std::string& _s)
			{
				Import(_s);
			}

			~PublicPassword() {
				rsa_free(&key);
			}

			void Import(const std::string& _s)
			{
				auto s = d8u::decode::url(_s);
				if (rsa_import((const unsigned char*)s.data(), s.size(), &key))
					throw "rsa error";
			}

			void ImportSelfSigned(const std::string& _s)
			{
				throw "rsa error";
			}

			std::string ExportSelfSigned(const PrivatePassword& private_key)
			{
				throw "rsa error";
				return "";
			}

			std::string Export()
			{
				std::vector<uint8_t> buffer;
				buffer.resize(bytesize);
				unsigned long pvlen = bytesize;

				if (rsa_export(buffer.data(), &pvlen, PK_PUBLIC, &key))
					throw "rsa error";

				buffer.resize(pvlen);

				return d8u::encode::url(buffer);
			}

			template < typename T, typename V > bool Verify(const T& m, const V& _sig)
			{
				unsigned char hash[64];
				hash_state md;
				sha512_desc.init(&md);
				sha512_desc.process(&md, (const unsigned char*)m.data(), (unsigned long)m.size());
				sha512_desc.done(&md, hash);

				auto sig = d8u::decode::url(_sig);

				int stat = 0;
				if (rsa_verify_hash_ex(sig.data(), sig.size(), hash, sha512_desc.hashsize, LTC_PKCS_1_V1_5, 0/*sha512_desc*/, 0/*saltlen*/, &stat, &key))
					throw "rsa error";

				return stat ? true : false;
			}

			template < typename T >d8u::string Encrypt(const T& m)
			{
				std::vector<uint8_t> buffer;
				buffer.resize(bytesize);
				unsigned long enclen = bytesize;

				if (rsa_encrypt_key((const unsigned char*)m.data(), (unsigned long)m.size(),buffer.data(), &enclen, nullptr, 0, NULL, 0/*sprng_desc*/,0/*sha512_desc*/,&key))
					throw "rsa error";

				buffer.resize(enclen);

				return d8u::encode::url(buffer);
			}
		};

		class KeyPair
		{
		public:
			PublicPassword public_key;
			PrivatePassword private_key;

			void Import(const std::string& _s)
			{
				/*auto f = _s.find('|');
				public_key.Import(_s.substr(0, f));
				private_key.Import(_s.substr(f + 1, _s.size() - f - 2));*/
			}

			std::string Export()
			{
				/*std::stringstream s;
				s << public_key.Export() << "|" << private_key.Export() << "*";

				return s.str();*/

				return "";
			}

			static KeyPair Create(uint32_t bits = 4096)
			{
				rsa_key key;
				if (rsa_make_key(NULL, 0/*sprng_desc*/, bits / 8, 65537, &key))
					throw "rsa error";

				return { PublicPassword(key,true), PrivatePassword(key) };
			}
		};
	}
}