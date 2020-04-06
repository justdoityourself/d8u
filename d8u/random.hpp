/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <vector>
#include <random>

namespace d8u
{
	namespace random
	{
		using namespace std;

		size_t Integer(size_t max = -1)
		{
			static default_random_engine e(random_device{}());

			size_t result = e();
			result <<= 32; //e() doesn't fill top dword of qword
			result += e();

			return result % max;
		}

		char Char()
		{
			return "abcdefghijklmnopqrstuvwxyzABDREFGHIJKLMNOPQRSTUVWXYZ"[Integer(52)];
		}

		std::string Word(size_t max = 16)
		{
			auto length = Integer(max);

			if (length == 0) length = 1;

			std::string result;

			for (size_t i = 0; i < length; i++)
				result += Char();

			return result;
		}

		bool Flip() { return (Integer() % 2) == 0; }

		template < typename T > vector<T> Vector(size_t size)
		{
			vector<T> result;
			result.resize(size);

			for (size_t i = 0; i < size; i++)
			{
				if constexpr (std::is_class<T>())
					result[i].Random();
				else
					result[i] = (T)Integer();
			}

			return result;
		}
	}
}