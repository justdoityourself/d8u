/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>

#include "../gsl-lite.hpp"

using namespace gsl;
using namespace std;

namespace d8u
{
#define self_t (*this)

	namespace util
	{
		void empty_file(const string_view file)
		{
			ofstream fs(file, ios::out);
		}

		template <typename T> uint64_t append_file(const string_view file, const T& m)
		{
			ofstream fs(file, ios::out | fstream::app | ios::binary);
			uint64_t result = fs.tellp();
			fs.write((const char*)&m.length, sizeof(uint32_t));
			fs.write((const char*)m.data(), m.size());

			return result;
		}

		void empty_file1(const string_view file)
		{
			ofstream fout;
			fout.open(file, ios::out);
			fout << 'c';
		}

#if defined ( _WIN32 )
#include <sys/stat.h>
#endif

		std::time_t GetFileWriteTime(const std::filesystem::path& filename)
		{
#if defined ( _WIN32 )
			{
				struct _stat64 fileInfo;
				if (_wstati64(filename.wstring().c_str(), &fileInfo) != 0)
				{
					throw std::runtime_error("Failed to get last write time.");
				}
				return fileInfo.st_mtime;
			}
#else
			{
				auto fsTime = std::filesystem::last_write_time(filename);
				return decltype (fsTime)::clock::to_time_t(fsTime);
			}
#endif
		}

		template <typename T> T& singleton()
		{
			static T t;

			return t;
		}

		template <typename T> using upair = std::pair<T, T>;
		template <typename T> using utri = std::tuple<T, T, T>;

		struct Direct
		{
			uint64_t target;

			uint64_t read;
			uint64_t write;
			uint64_t duplicate;
			uint64_t blocks;
			uint64_t dblocks;
			uint32_t threads;
			uint32_t files;
		};

		struct Atomic
		{
			std::atomic<uint64_t> target;

			std::atomic<uint64_t> read;
			std::atomic<uint64_t> write;
			std::atomic<uint64_t> duplicate;
			std::atomic<uint64_t> blocks;
			std::atomic<uint64_t> dblocks;
			std::atomic<uint32_t> threads;
			std::atomic<uint32_t> files;
		};

		struct Statistics
		{
			Statistics()
			{
				Initialize();
			}

			static_assert(sizeof(uint64_t) == sizeof(std::atomic<uint64_t>));

			union
			{
				Direct direct;
				Atomic atomic;
			};

			void Initialize()
			{
				direct.target = 0;
				direct.read = 0;
				direct.write = 0;
				direct.duplicate = 0;
				direct.blocks = 0;
				direct.dblocks = 0;
				direct.threads = 0;
				direct.files = 0;
			}

			void operator += (const Statistics& r)
			{
				direct.read += r.direct.read;
				direct.write += r.direct.write;
				direct.duplicate += r.direct.duplicate;
				direct.blocks += r.direct.blocks;
				direct.dblocks += r.direct.dblocks;
				direct.threads += r.direct.threads;
				direct.files += r.direct.files;
			}
		};

		constexpr std::array<uint8_t, 16> default_domain = { 0xC5,0x22,0xAC,0xAD,0x91,0xDD,0x42,0xC6,0x90,0xC8,0xEE,0x60,0x82,0x0B,0x18,0x79 };


		template <size_t UNIT> class BlockBuffer
		{
		public:
			template <typename T> T& Allocate()
			{
				if (!blocks.size())
					blocks.push_back(std::vector<uint8_t>(UNIT));

				if (sizeof(T) > current + UNIT)
				{
					blocks.back().resize(current);
					blocks.push_back(std::vector<uint8_t>(UNIT));

					total += current;
					current = 0;
				}

				T* pres = (T*) (blocks.back().data() + current);
				current += sizeof(T);

				return *pres;
			}

			const auto& data() const { return blocks; }
			std::vector<std::vector<uint8_t>>& finish() { blocks.back().resize(current);  return blocks; }
			size_t size() const { return total + current; }

		private:

			size_t total = 0;
			size_t current = 0;
			std::vector<std::vector<uint8_t>> blocks;
		};

		template < typename T > T& vec_alloc(std::vector<uint8_t>& v)
		{
			T t;
			v.insert(v.end(), (uint8_t*)(&t), ( (uint8_t*)(&t)) + sizeof(T));

			return *( (T*) (v.data() + v.size() - sizeof(T)) );
		}

		template < typename T > span<T> vec_span(std::vector<uint8_t>& v)
		{
			if (v.size() % sizeof(T) != 0)
				throw std::runtime_error("Buffer to typed span remainder");

			return span<T>((T*)v.data(), v.size() / sizeof(T));
		}

		template < typename T > T& vec_get(std::vector<uint8_t>& v, size_t & offset = 0)
		{
			if (v.size() < sizeof(T) + offset)
				throw std::runtime_error("Buffer request beyond end");

			auto res = ((T*)(v.data() + offset));
			offset += sizeof(T);

			return *res;
		}


	}
}