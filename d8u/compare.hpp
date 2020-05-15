/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "transform.hpp"
#include "util.hpp"

namespace d8u
{
	namespace compare
	{
		using namespace util;
		using namespace transform;

		DefaultHash hash_file(std::string_view name)
		{
			auto file = mio::mmap_source(name);
			DefaultHash result(file);
			return result;
		}

		bool files(std::string_view o, std::string_view t)
		{
			auto h1 = hash_file(o);
			auto h2 = hash_file(t);

			return std::equal(h1.begin(), h1.end(), h2.begin());
		}

		bool files_bytes(std::string_view o, std::string_view t)
		{
			auto file1 = mio::mmap_source(o);
			auto file2 = mio::mmap_source(t);

			if (file1.size() != file2.size())
				return false;

			return std::equal(file1.begin(), file1.end(), file2.begin());
		}

		bool folders(std::string_view o, std::string_view t,size_t F = 1)
		{
			std::atomic<size_t> threads = 0;

			bool result = true;
			for (auto& e : std::filesystem::recursive_directory_iterator(o))
			{
				if (!result)
					break;

				if (e.is_directory())
					continue;

				if(F==1)
				if (!files(e.path().string(), string(t) + "\\" + e.path().string()))
					return false;
				else
				{
					fast_wait(threads, F);

					threads++;

					std::thread([&](std::string s) 
					{
						if(!files(s, string(t) + "\\" + s))
							result = false;

						threads--;
					}, e.path().string()).detach();

					slow_wait(threads);
				}
			}

			return result;
		}

		template<typename D1, typename D2> bool devices(const D1& d1, upair<uint64_t> s1, const D2& d2, upair<uint64_t> s2)
		{
			if (s1.second != s2.second)
				return false;

			constexpr size_t B = 1024 * 1024;

			std::vector<uint8_t> b1(B), b2(B);

			d1.Seek(s1.first);
			d2.Seek(s2.first);

			uint64_t rem = s1.second;

			size_t i = 0;
			while (rem)
			{
				if (rem < B)
				{
					b1.resize(rem);
					b2.resize(rem);
				}

				d1.Read(b1);
				d2.Read(b2);

				if (!std::equal(b1.begin(), b1.end(), b2.begin()))
				{
					auto s1 = to_hex(b1);
					auto s2 = to_hex(b2);
					std::ofstream("sample1.txt").write(s1.data(),s1.size());
					std::ofstream("sample2.txt").write(s2.data(), s2.size());

					return false;
				}

				rem -= b1.size();
				i++;
			}

			return true;
		}
	}
}