/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <vector>

#include "hash.hpp"
#include "crypto.hpp"

#include "cryptopp/gzip.h"

#include "lzma/lzmalib.h"
//#include "zopfli/zopfli.h"
#include "minilzo/minilzo.h"

#include "../gsl-lite.hpp"



namespace d8u
{
	namespace transform
	{
		using gsl::span;

		using std::array;
		using std::vector;

		void gzip_compress(d8u::sse_vector& m, int level = 5)
		{
			CryptoPP::Gzip zipper(nullptr,level);
			zipper.Put((CryptoPP::byte*)m.data(), m.size());
			zipper.MessageEnd();

			auto avail = zipper.MaxRetrievable();
			if (avail && avail < m.size())
			{
				uint32_t dsz = (uint32_t)m.size();
				m.resize(avail+sizeof(uint32_t));
				auto pdesc = (uint32_t*)(m.data() + m.size() - sizeof(uint32_t));
				*pdesc = dsz;
				*pdesc |= 0x01000000; //GZIP

				zipper.Get((CryptoPP::byte*)m.data(), m.size()-sizeof(uint32_t));
			}
			else
			{
				uint32_t l = (uint32_t)m.size() | 0x01000000;
				m.insert(m.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
			}
		}

		template < typename T > d8u::sse_vector gzip_compress2(const T & m, int level = 5)
		{
			d8u::sse_vector r;

			CryptoPP::Gzip zipper(nullptr, level);
			zipper.Put((CryptoPP::byte*)m.data(), m.size());
			zipper.MessageEnd();

			auto avail = zipper.MaxRetrievable();
			if (avail && avail < m.size())
			{
				uint32_t dsz = (uint32_t)m.size();
				r.resize(avail + sizeof(uint32_t));
				auto pdesc = (uint32_t*)(r.data() + r.size() - sizeof(uint32_t));
				*pdesc = dsz;
				*pdesc |= 0x01000000; //GZIP

				zipper.Get((CryptoPP::byte*)r.data(), r.size() - sizeof(uint32_t));
			}
			else
			{
				r = m;
				uint32_t l = (uint32_t)r.size() | 0x01000000;
				r.insert(r.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
			}

			return r;
		}

		void gzip_decompress(d8u::sse_vector& m)
		{
			uint32_t fin_l = *(uint32_t*)(m.data() + m.size() - sizeof(uint32_t));
			fin_l &= 0x00ffffff;

			if (fin_l == m.size() - sizeof(uint32_t))
			{
				m.resize(fin_l);
				return;
			}

			d8u::sse_vector result(fin_l);

			CryptoPP::ArraySource as(m.data(), m.size()-sizeof(uint32_t),true, new CryptoPP::Gunzip(new CryptoPP::ArraySink(result.data(),result.size())));

			m = std::move(result);
		}

		/*void zopfli_compress(d8u::sse_vector& m, int level = 5)
		{
			ZopfliOptions o;
			ZopfliInitOptions(&o);
			size_t r = 0;
			unsigned char* buffer = nullptr;
			ZopfliCompress(&o, ZOPFLI_FORMAT_GZIP, m.data(), m.size(), &buffer, &r);
			m.resize(r);
			std::memcpy(m.data(), buffer, r);
			free(buffer);
		}

		void zopfli_decompress(d8u::sse_vector& m)
		{
			throw "todo";
		}*/

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

		void minilzo_decompress(d8u::sse_vector& m)
		{
			static MiniLZO Context;

			uint32_t fin_l = *(uint32_t*)(m.data() + m.size() - sizeof(uint32_t));
			fin_l &= 0x00ffffff;

			if (fin_l == m.size() - sizeof(uint32_t))
			{
				m.resize(fin_l);
				return;
			}

			d8u::sse_vector result(fin_l);

			lzo_uint output_length = (lzo_uint)result.size();
			if (LZO_E_OK != lzo1x_decompress((uint8_t*)m.data(), (lzo_uint)m.size() - sizeof(uint32_t), result.data(), &output_length, NULL) || output_length != result.size()) 
				throw std::runtime_error("LZMA Decompression Error");

			m = std::move(result);
		}

		void minilzo_compress(d8u::sse_vector& m, int level = 5)
		{
			static MiniLZO Context;

			std::array<uint8_t, LZO1X_1_MEM_COMPRESS> workspace;

			d8u::sse_vector result(m.size() + 1024 * 65);

			lzo_uint output_length = (lzo_uint)m.size() + 1024;
			if (LZO_E_OK != lzo1x_1_compress((uint8_t*)m.data(), (lzo_uint)m.size(), result.data(), &output_length, workspace.data()))
				output_length = result.size();

			uint32_t l = (uint32_t)m.size() | 0x02000000;

			if (output_length >= m.size())
				m.insert(m.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
			else
			{
				result.resize(output_length);
				result.insert(result.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
				m = std::move(result);
			}
		}

		template < typename T > auto minilzo_compress2(const T & m, int level = 5)
		{
			static MiniLZO Context;

			std::array<uint8_t, LZO1X_1_MEM_COMPRESS> workspace;

			d8u::sse_vector result(m.size() + 1024 * 65);

			lzo_uint output_length = (lzo_uint)m.size() + 1024;
			if (LZO_E_OK != lzo1x_1_compress((uint8_t*)m.data(), (lzo_uint)m.size(), result.data(), &output_length, workspace.data()))
				output_length = result.size();

			uint32_t l = (uint32_t)m.size() | 0x02000000;

			if (output_length >= m.size())
			{
				result = m;
				result.insert(result.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
			}
			else
			{
				result.resize(output_length);
				result.insert(result.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
			}

			return result;
		}
		
		void lzma_compress(d8u::sse_vector & m, int level = 5)
		{
			std::array<uint8_t, LZMA_PROPS_SIZE> props;
			size_t prop = LZMA_PROPS_SIZE;
			size_t fin_l = m.size() + 1024;
			d8u::sse_vector result(fin_l);

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

		template < typename T > d8u::sse_vector lzma_compress2(const T& m, int level = 5)
		{
			std::array<uint8_t, LZMA_PROPS_SIZE> props;
			size_t prop = LZMA_PROPS_SIZE;
			size_t fin_l = m.size() + 1024;
			d8u::sse_vector result(fin_l);

			int res = LzmaCompress(result.data(), &fin_l, (const unsigned char*)m.data(), m.size(), props.data(), &prop, level, 4, -1, -1, -1, -1, 1);

			if (prop == LZMA_PROPS_SIZE && res == SZ_OK && fin_l < m.size())
			{
				result.resize(fin_l + sizeof(uint32_t));
				*(uint32_t*)(result.data() + fin_l) = (uint32_t)m.size();
			}
			else
			{
				result.resize(m.size());
				std::copy(m.data(), m.data() + m.size(), result.begin());
				uint32_t l = (uint32_t)m.size();
				result.insert(result.end(), (uint8_t*)&l, ((uint8_t*)&l) + sizeof(uint32_t));
			}

			return result;
		}

		void lzma_decompress(d8u::sse_vector& m)
		{
			std::array<uint8_t, LZMA_PROPS_SIZE> props = { 93,0,16,0,0 };

			uint32_t fin_l = *(uint32_t*)(m.data() + m.size() - sizeof(uint32_t));

			if (fin_l == m.size() - sizeof(uint32_t))
			{
				m.resize(fin_l);
				return;
			}

			d8u::sse_vector result(fin_l);
			size_t dest = (size_t)fin_l;
			size_t src = m.size() - sizeof(uint32_t);
			SRes res = LzmaUncompress( result.data(), &dest, m.data(), &src, props.data(), LZMA_PROPS_SIZE);

			if (res != SZ_OK)
				throw std::runtime_error("LZMA Decompression Error");
			
			m = std::move(result);
		}

		void compress(d8u::sse_vector& m, int _level = 5)
		{
			auto algorithm = _level / 10;
			auto level = _level % 10;

			switch (algorithm)
			{
			default:
			case 0:
				return lzma_compress(m, level);
			case 1:
				return gzip_compress(m, level);
			case 2:
				return minilzo_compress(m, level);
			}
		}

		void decompress(d8u::sse_vector& m)
		{
			uint32_t fin_l = *(uint32_t*)(m.data() + m.size() - sizeof(uint32_t));

			auto algorithm = fin_l >> 24;

			switch (algorithm)
			{
			default:
				throw std::runtime_error("Invalid block");
			case 0:
				return lzma_decompress(m);
			case 1:
				return gzip_decompress(m);
			case 2:
				return minilzo_decompress(m);
			}
		}

		template <typename T, typename B> T& id_block(const B& buffer)
		{
			auto phash = (T*)(buffer.end() - sizeof(T) * 2);
			return *phash;
		}

		template <typename T, typename B> bool validate_block(const B& buffer)
		{
			if (buffer.size() < sizeof(T) * 2)
				return false;

			auto data = span<uint8_t>((uint8_t*)buffer.data(), buffer.size() - sizeof(T) * 2);
			T check(data);

			return std::equal(check.begin(), check.end(), buffer.end() - sizeof(T));
		}

		template <typename T , typename D, typename B> std::pair<T, T> identify(const D& domain, const B& buffer)
		{
			typename T::State state;
			state.Update(domain);
			state.Update(buffer);

			alignas(16) T key = state.FinishT<T>();
			T id = key.GetNext();

			return std::make_pair(key, id);
		}

		template <typename T , typename IN_OUT> T encode2(IN_OUT& buffer,const T & key, const T & id, int level = 5)
		{
			compress(buffer,level);

			Password pw(key);
			encrypt(buffer, pw);

			T check(buffer);

			buffer.insert(buffer.end(), id.begin(), id.end());
			buffer.insert(buffer.end(), check.begin(), check.end());

			return key;
		}

		//Pending compress2 implementation for all algorithms.
		/*template <typename INB> auto encode2_span(const INB& span, const DefaultHash& key, const DefaultHash& id, int level = 5)
		{
			auto buffer = compress2(span, level);

			Password pw(key);
			encrypt(buffer, pw);

			DefaultHash check(buffer);

			buffer.insert(buffer.end(), id.begin(), id.end());
			buffer.insert(buffer.end(), check.begin(), check.end());

			return std::make_pair(key,buffer);
		}*/

		template <typename T , typename D, typename IN_OUT> T encode(const D& domain, IN_OUT& buffer, int level = 5)
		{
			T key, id;

			std::tie(key, id) = identify<T>(domain, buffer);

			return encode2<T>(buffer, key, id, level);
		}

		template <typename T , typename IN_OUT> void quick_decode(IN_OUT& buffer, const T & key)
		{
			buffer.resize( buffer.size() - sizeof(T) * 2);

			Password pw(key);
			decrypt(buffer, pw);
			decompress(buffer);
		}

		template <typename T , typename D, typename IN_OUT> void decode(const D& domain, IN_OUT& buffer, const T& key)
		{
			buffer.resize(buffer.size() - sizeof(T) * 2);

			Password pw(key);
			decrypt(buffer, pw);
			decompress(buffer);

			typename T::State state;
			state.Update(domain);
			state.Update(buffer);

			auto check = state.Finish();

			if (!std::equal(key.begin(), key.end(), check.begin()))
				throw std::runtime_error("Decode Error");
		}

		template <typename T > class Audit
		{
		public:

			void IO(uint32_t cluster, const T & block)
			{
				for (size_t i = 0; i < sum.size(); i++)
					sum[(i + cluster) % sum.size()] ^= block[i];
			}

			void operator += (const Audit& r)
			{
				for (size_t i = 0; i < sum.size(); i++)
					sum[i] ^= r.sum[i];
			}

			bool operator ==(const Audit& r) const
			{
				return std::equal(sum.begin(),sum.end(),r.sum.begin());
			}

		private:
			std::array<uint8_t, 32> sum = { 0 };
		};
	}
}