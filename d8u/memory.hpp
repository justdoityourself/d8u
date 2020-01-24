/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <cstdint>
#include <stdexcept>

using namespace std;

namespace d8u
{
	template < typename T, std::size_t N > constexpr size_t __array_elements(const T(&)[N]) { return (size_t)N; }
	template < typename T, std::size_t N > constexpr size_t __array_element_size(const T(&)[N]) { return (size_t)sizeof(T); }
	template < typename T > constexpr size_t __array_bytes(T& t) { return __array_elements(t) * __array_element_size(t); }

	class Memory
	{
	public:
		Memory() :buffer(nullptr), length(0) {}
		Memory(const void* data_, size_t length_) : buffer((uint8_t*)data_), length(length_) {};

		template <typename T> Memory(T&t) : buffer((uint8_t*)t.data()), length(t.size()) {};
		template < typename T, typename std::enable_if<std::is_array<T>::value && std::is_same<char, typename std::remove_all_extents<T>::type>::value, void* >::type = nullptr > Memory(const T& t) : buffer((uint8_t*)&t[0]), length(__array_bytes(t) - 1) {}
		template < typename T, typename std::enable_if<std::is_array<T>::value && !std::is_same<char, typename std::remove_all_extents<T>::type>::value, void* >::type = nullptr > Memory(const T& t) : buffer((uint8_t*)&t[0]), length(__array_bytes(t)) {}
		template < typename T, typename std::enable_if<std::is_same<char*, T>::value || std::is_same<const char*, T>::value, void* >::type = nullptr> Memory(const T& t) : buffer((uint8_t*)t), length((size_t)strlen(t)) {}
		template < typename T, typename std::enable_if<std::is_integral<T>::value, void* >::type = nullptr> Memory(const T& t) : buffer((uint8_t*)&t), length((size_t)sizeof(T)) {}

		size_t size() const { return length; }
		uint8_t* data() const { return buffer; }

		void operator++(int) { buffer++; length--; }
		uint8_t& operator[](size_t index) const { return *(buffer + index); }

		template < typename T> bool ScanTo(T t) { while (length--) if (*buffer++ == t) return true; return false; }
		template < typename T > size_t SegmentSize(T* t) { return (size_t)(buffer - ((uint8_t*)t)); }

		void Insert(void* data, size_t length, size_t* offset = nullptr) { memcpy(Allocate(length, offset), data, length); }
		void Insert(const Memory& m, size_t* offset = nullptr) { memcpy(Allocate(m.size(), offset), m.data(), m.size()); }

		template <typename T, typename ...t_args> T* AllocateT(t_args&&...args) { return new(Allocate(sizeof(T))) T(std::forward<t_args>(args)...); }
		template <typename T> T* FetchT() { return (T*)Allocate(sizeof(T)); }
		uint8_t* Allocate(size_t count, size_t* offset = nullptr)
		{
			if (count > length)
				throw std::runtime_error("Memory Object out of Space");

			if (offset)
				*offset += count;

			uint8_t* result = buffer;
			buffer += count;
			length -= count;

			return result;
		}

		Memory Slice(size_t i, size_t l = -1) const
		{
			if (-1 == l)
				l = length - i;

			return Memory(buffer + i, l);
		}

	private:
		uint8_t* buffer;
		size_t length;
	};
}