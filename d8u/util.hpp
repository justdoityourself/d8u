/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <locale>
#include <codecvt>

#include <string>
#include <string_view>
#include <array>

#include <random>
#include <time.h>
#include <chrono>

//Picosha is slow slow slow, but you can uncomment it if you don't have cryptopp set up.
//#include "../picosha2.hpp"
#include "cryptopp/sha.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <thread>

#include "../mio.hpp"
#include "../gsl-lite.hpp"

#ifdef _WIN32
#include "Objbase.h"
#endif

namespace d8u
{
	#define self_t (*this)

	namespace util
	{
		using namespace gsl;
		using namespace std;


		constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
									'8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

		template <typename T> string to_hex(const T& d)
		{
			string s(d.size() * 2, ' ');
			for (size_t i = 0; i < d.size(); ++i)
			{
				s[2 * i] = hexmap[(d[i] & 0xF0) >> 4];
				s[2 * i + 1] = hexmap[d[i] & 0x0F];
			}
			return s;
		}

		vector<uint8_t> to_bin(std::string_view v)
		{
			std::vector<uint8_t> result; result.reserve(v.size() / 2 + 1);
			auto ctoi = [](char c)
			{
				if (c >= '0' && c <= '9')
					return c - '0';
				if (c >= 'A' && c <= 'F')
					return c - 'A' + 10;
				if (c >= 'a' && c <= 'f')
					return c - 'a' + 10;

				return 0;
			};

			for (auto c = v.begin(); c < v.end(); c += 2)
				result.push_back((ctoi(*c) << 4) + ctoi(*(c + 1)));

			return result;
		}

		template<typename T> class dec_scope
		{
			T& t;

		public:
			dec_scope(T& _t) :t(_t) {}

			~dec_scope() { t--; }
		};

		void fast_wait(std::atomic<size_t> & A, size_t M=1)
		{
			while (A.load() >= M)
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		void slow_wait(std::atomic<size_t>& A, size_t M=1)
		{
			while (A.load() >= M)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

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
			uint64_t queries;
			size_t threads;
			size_t files;
			size_t items;
		};

		struct Atomic
		{
			std::atomic<uint64_t> target;

			std::atomic<uint64_t> read;
			std::atomic<uint64_t> write;
			std::atomic<uint64_t> duplicate;
			std::atomic<uint64_t> blocks;
			std::atomic<uint64_t> dblocks;
			std::atomic<uint64_t> queries;
			std::atomic<size_t> threads;
			std::atomic<size_t> files;
			std::atomic<size_t> items;
		};

		struct Statistics
		{
			Statistics()
			{
				Initialize();
			}

			void Print()
			{
				if (direct.target)
				{
					auto p = (double)direct.read / (double)direct.target * 100;
					if (p > 100)
						p = 100;

					std::cout << "" << p << "% , ";
				}
				if(direct.threads) 
					std::cout << "Threads " << direct.threads << ", ";
				if(direct.files) 
					std::cout << "Handles " << direct.files << ", ";
				if (direct.items) 
					std::cout << "Files " << direct.items << ", ";
				if (direct.blocks) 
					std::cout << "Blocks " << direct.blocks << ", ";
				if (direct.queries) 
					std::cout << "Queries " << direct.queries << ", ";
				if (direct.read)
					std::cout << "Read " << direct.read << ", ";
				if (direct.write) 
					std::cout << "Write " << direct.write << ", ";
				if (direct.duplicate) 
					std::cout << "Duplicate " << direct.duplicate << " ";
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
				direct.queries = 0;
				direct.files = 0;
				direct.items = 0;
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
				direct.items += r.direct.items;
				direct.queries += r.direct.queries;
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

		string now()
		{
			chrono::system_clock::time_point p = chrono::system_clock::now();
			time_t t = chrono::system_clock::to_time_t(p);

			return to_string((uint64_t)t);
		}

		typedef array<uint8_t, 32> file_id;
		void id_file(string_view name, file_id& hash)
		{
			mio::mmap_source file(name);
			CryptoPP::SHA256().CalculateDigest(hash.data(), (const CryptoPP::byte*) file.data(), file.size());
			//picosha2::hash256(file.begin(), file.end(), hash.begin(), hash.end());
		}

		size_t Random(size_t max = -1)
		{
			static default_random_engine e(std::random_device{}());

			size_t result = e();
			result <<= 32; //e() doesn't fill top dword of qword
			result += e();

			return result % max;
		}

		bool Flip() { return (Random() % 2) == 0; }

		template < typename T > vector<T> RandomVector(size_t size)
		{
			vector<T> result;
			result.resize(size);

			static default_random_engine e(std::random_device{}());

			for (size_t i = 0; i < size; i++)
				result[i] = (T)Random();

			return result;
		}

		//Original from d88::factor
		/*template < typename T > void FactorExpand(const span<T>& poly, span<T> output)
		{
			auto const u = poly.size();
			vector<T> l(u + output.size());

			for (size_t i = 0; i < u - 2; i++)
				l[i] = 0;

			l[u - 2] = 1;

			for (size_t i = 0; i < output.size(); i++)
			{
				size_t sum = 0;

				for (size_t j = 0, k = u - 1; j < u - 1; j++, k--)
					sum += poly[k] * l[i + j];

				l[u + i - 1] = sum;
				output[i] = sum;
			}
		}*/

		//Performance consideration, limit length to 16 iterations
		template < typename T > void FactorExpand(const span<T>& poly, span<T> output)
		{
			auto const u = poly.size();
			vector<T> l(u + output.size());

			for (size_t i = 0; i < u - 2; i++)
				l[i] = 0;

			l[u - 2] = 1;

			for (size_t i = 0; i < output.size(); i++)
			{
				if (i >= 16)
				{
					output[i] = 0;
					continue;
				}

				size_t sum = 0;

				for (size_t j = 0, k = u - 1; j < u - 1; j++, k--)
					sum += poly[k] * l[i + j];

				l[u + i - 1] = sum;
				output[i] = sum;
			}
		}

		constexpr size_t _mb(size_t s) { return s * 1024 * 1024; }
		constexpr size_t _kb(size_t s) { return s * 1024; }
		constexpr size_t _gb(size_t s) { return s * 1024 * 1024 * 1024; }

		wstring to_wide(string_view s)
		{
			//This Method, is so slow and uses thread locks
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//return converter.from_bytes(string(s));

			//Back to the basics, no good deed unpunished.
			return std::wstring(s.begin(), s.end());
		}

		string to_narrow(wstring_view s)
		{
			//This Method, is so slow and uses thread locks
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//return converter.to_bytes(wstring(s));

			//Back to the basics, no good deed unpunished.
			return std::string(s.begin(), s.end());
		}

		void string_as_file(string_view file, string_view data)
		{
			filesystem::remove(file);

			ofstream f(file);
			f << data;
		}

		array<uint8_t, 16> unique_id()
		{
#ifdef _WIN32
			array<uint8_t, 16> ret;
			HRESULT hresult = CoCreateGuid((GUID*)ret.data());

			if (S_OK != hresult)
				throw std::runtime_error("Failed to create GUID");

			return ret;
#else
			throw "TODO";

			return array<uint8_t, 16>();
#endif
		}


	}
}