/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <locale>
#include <codecvt>

#include <string>
#include <sstream>
#include <string_view>
#include <array>

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

#include "string.hpp"

namespace d8u
{
	#define self_t (*this)

	template < typename T > class PlainOldData : public T
	{
	public:

		const uint8_t* data() const { return (uint8_t*)this; }
		size_t size() const { return sizeof(PlainOldData<T>); }

		const uint8_t* begin() const { return data(); }
		const uint8_t* end() const { return data() + size(); }
	};

	namespace util
	{
		void ClearScreen()
		{
#ifdef WIN32

			static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			COORD topLeft = { 0, 0 };

			std::cout.flush();

			if (!GetConsoleScreenBufferInfo(hOut, &csbi))
				abort();

			DWORD length = csbi.dwSize.X * csbi.dwSize.Y;

			DWORD written;

			FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

			FillConsoleOutputAttribute(hOut, csbi.wAttributes, length, topLeft, &written);

			SetConsoleCursorPosition(hOut, topLeft);

#else
			::systel("cls");
#endif
		}

		using namespace gsl;
		using namespace std;

		template<typename T> class dec_scope
		{
			T& t;

		public:
			dec_scope(T& _t) :t(_t) {}

			~dec_scope() { t--; }
		};

		void fast_until(std::atomic<size_t>& A, size_t M = 1)
		{
			while (A.load() != M)
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		void fast_wait(std::atomic<size_t> & A, size_t M=1)
		{
			while (A.load() >= M)
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		void fast_wait_inc(std::atomic<size_t>& A, size_t M = 1)
		{
			while (A.load() >= M)
				std::this_thread::sleep_for(std::chrono::milliseconds(10));

			A++; // yes there is a race condition here, this function is not intended to prevent a race.
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

		uint64_t GetFileSize(std::string_view name)
		{
			uint64_t size = 0;
			
			try
			{
				size = std::filesystem::file_size(name);
			}
			catch (...)
			{
#if defined ( _WIN32 )
				{
					struct _stat64 fileInfo;
					auto result = _wstati64((std::wstring(L"\\\\?\\") + to_wide(name)).c_str(), &fileInfo);
					if (result != 0)
					{
						std::cout << "Failed to get size for file " << name << " with errno " << result << std::endl;
						return 0;
					}
					size = fileInfo.st_size;
				}
#else
				throw;
#endif
			}

			return size;
		}

		std::time_t GetFileWriteTime(const std::filesystem::path& filename)
		{
#if defined ( _WIN32 )
			{
				struct _stat64 fileInfo;
				try
				{
					auto result = _stati64(filename.string().c_str(), &fileInfo);
					if (result != 0)
					{
						throw "unicode exception or file not found";
					}
				}
				catch (...)
				{
					auto result = _wstati64((std::wstring(L"\\\\?\\") + filename.wstring()).c_str(), &fileInfo);
					if (result != 0)
					{
						std::cout << "Failed to get last write time for file " << filename.string() << " with errno " << result << std::endl;
						return 0;
						//throw std::runtime_error("Failed to get last write time.");
					}
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

		std::time_t GetFileWriteTime2(const std::string & filename)
		{
#if defined ( _WIN32 )
			{
				struct _stat64 fileInfo;
				try
				{
					auto result = _stati64(filename.c_str(), &fileInfo);
					if (result != 0)
					{
						throw "unicode exception or file not found";
					}
				}
				catch (...)
				{
					auto result = _wstati64((std::wstring(L"\\\\?\\") + to_wide(filename)).c_str(), &fileInfo);
					if (result != 0)
					{
						std::cout << "Failed to get last write time for file " << filename << " with errno " << result << std::endl;
						return 0;
						//throw std::runtime_error("Failed to get last write time.");
					}
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
			size_t memory;
			size_t connections;
			size_t sequence;
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
			std::atomic<size_t> memory;
			std::atomic<size_t> connections;
			std::atomic<size_t> sequence;
		};

		struct Statistics
		{
			Statistics()
			{
				Initialize();
			}

			void Print()
			{
				if (direct.blocks) 
					std::cout << "BLK " << direct.blocks << " ";
				if (direct.queries) 
					std::cout << "Q " << direct.queries << " ";
				if (direct.items)
					std::cout << "FT " << direct.items << " ";
				if (direct.read)
					std::cout << "RIO " << direct.read / (1024*1024) << "MB ";
				if (direct.write)
					std::cout << "WIO " << direct.write / (1024 * 1024) << "MB ";
				if (direct.duplicate)
					std::cout << "DIO " << direct.duplicate / (1024 * 1024) << "MB ";

				if (direct.threads)
					std::cout << "T " << direct.threads << " ";
				if (direct.files)
					std::cout << "H " << direct.files << " ";
				if (direct.connections)
					std::cout << "Conns " << direct.connections << " ";
				if (direct.memory)
					std::cout << "MMIO " << direct.memory / (1024 * 1024) << "MB ";

				if (direct.target)
				{
					auto p = (double)direct.read / (double)direct.target * 100;
					if (p > 100)
						p = 100;

					std::cout << "" << p << "% ";
				}
			}

			std::string String()
			{
				std::stringstream ss;

				if (direct.blocks)
					ss << "BLK " << direct.blocks << " ";
				if (direct.queries)
					ss << "Q " << direct.queries << " ";
				if (direct.items)
					ss << "FT " << direct.items << " ";
				if (direct.read)
					ss << "RIO " << direct.read / (1024 * 1024) << "MB ";
				if (direct.write)
					ss << "WIO " << direct.write / (1024 * 1024) << "MB ";
				if (direct.duplicate)
					ss << "DIO " << direct.duplicate / (1024 * 1024) << "MB ";

				return ss.str();
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
				direct.memory = 0;
				direct.connections = 0;
				direct.sequence = 0;
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

		constexpr std::array<uint8_t, 32> default_domain = { 0xC5,0x22,0xAC,0xAD,0x91,0xDD,0x42,0xC6,0x90,0xC8,0xEE,0x60,0x82,0x0B,0x18,0x79,0x8F,0xB4,0x39,0xE9,0x24,0x00,0x48,0xC4,0xB1,0xC9,0x7B,0xC3,0xB0,0xA6,0x66,0xA3 };

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