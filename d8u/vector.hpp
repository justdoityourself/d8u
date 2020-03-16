/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <vector>
#include <random>

namespace d8u
{
	namespace vector
	{
		using namespace std;

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

				T* pres = (T*)(blocks.back().data() + current);
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

		template <size_t UNIT, typename T> class BlockVector
		{
		public:
			T& Allocate()
			{
				if (!blocks.size())
				{
					blocks.push_back(std::vector<T>());
					blocks.back().reserve(UNIT);
				}				

				if (current == UNIT)
				{
					blocks.push_back(std::vector<T>());
					blocks.back().reserve(UNIT);

					total += current;
					current = 0;
				}

				blocks.back().push_back(T());
				current++;

				return blocks.back().back();
			}

		private:

			size_t total = 0;
			size_t current = 0;
			std::vector<std::vector<T>> blocks;
		};

		/*template < typename T > T& vec_alloc(std::vector<uint8_t>& v)
		{
			T t;
			v.insert(v.end(), (uint8_t*)(&t), ((uint8_t*)(&t)) + sizeof(T));

			return *((T*)(v.data() + v.size() - sizeof(T)));
		}

		template < typename T > span<T> vec_span(std::vector<uint8_t>& v)
		{
			if (v.size() % sizeof(T) != 0)
				throw std::runtime_error("Buffer to typed span remainder");

			return span<T>((T*)v.data(), v.size() / sizeof(T));
		}

		template < typename T > T& vec_get(std::vector<uint8_t>& v, size_t& offset = 0)
		{
			if (v.size() < sizeof(T) + offset)
				throw std::runtime_error("Buffer request beyond end");

			auto res = ((T*)(v.data() + offset));
			offset += sizeof(T);

			return *res;
		}*/
	}
}