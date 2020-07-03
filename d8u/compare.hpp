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

		bool files_bytes2(std::string_view o, std::string_view t)
		{
			auto file1 = mio::mmap_source(o);
			auto file2 = mio::mmap_source(t);

			if (file1.size() != file2.size())
				return false;

			for (size_t i = 0; i < file1.size(); i++)
			{
				if (file1[i] != file2[i])
					return false;
			}

			return true;
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

				if (F == 1)
				{
					if (!files(e.path().string(), string(t) + "\\" + e.path().string()))
						return false;
				}
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

		bool volumes(std::string_view o, std::string_view t, size_t F = 1)
		{
			std::atomic<size_t> threads = 0;

			bool result = true;
			for (auto& e : std::filesystem::recursive_directory_iterator(o, std::filesystem::directory_options::skip_permission_denied))
			{
				if (!result)
					break;

				if (e.is_directory())
					continue;

				if (F == 1)
				{
					auto s1 = e.path().string(), s2 = s1;
					s2[0] = t[0];
					if (!files(s1, s2))
						return false;
				}
				else
				{
					fast_wait(threads, F);

					threads++;

					std::thread([&](std::string s)
						{
							if (!files(s, string(t) + "\\" + s))
								result = false;

							threads--;
						}, e.path().string()).detach();

						slow_wait(threads);
				}
			}

			return result;
		}

		template<typename D1, typename D2> bool devices(const D1& d1, upair<uint64_t> seg1, const D2& d2, upair<uint64_t> seg2,std::string * report=nullptr)
		{
			if (seg1.second != seg2.second)
			{
				if (report)
					*report += "Device size mismatch";

				return false;
			}

			constexpr size_t B = 1024 * 1024;

			std::vector<uint8_t> b1(B), b2(B);

			d1.Seek(seg1.first);
			d2.Seek(seg2.first);

			uint64_t rem = seg1.second;

			size_t i = 0;
			size_t total_bad = 0;
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
					if (report)
					{
						size_t start = -1;
						size_t length = 0;

						for (size_t j = 0; j < b1.size() / 4096; j++)
						{
							if (!std::equal(b1.begin() + j * 4096, b1.begin() + (j+1) * 4096, b2.begin() + j * 4096))
							{
								if (start == -1)
									start = i*256 + j;
								length++;
							}
							else if (length)
							{
								*report += std::string("Cluster: ") + std::to_string(start) + "," + std::to_string(length) + "\n";
								start = -1;
								total_bad += length;
								length = 0;
							}
						}

						if (length)
						{
							*report += std::string("Cluster: ") + std::to_string(start) + "," + std::to_string(length) + "\n";
							start = -1;
							total_bad += length;
							length = 0;
						}
					}
					else
						return false;
				}

				rem -= b1.size();
				i++;
			}

			if(report && total_bad)
				*report += std::string("Total: ") + std::to_string(total_bad) + "\n";

			return true;
		}
	}
}