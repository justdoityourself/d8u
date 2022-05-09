/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <charconv>
#include <vector>

namespace d8u
{
	namespace allocators
	{
		enum class Alignment : size_t
		{
			Normal = sizeof(void*),
			SSE = 16,
			AVX = 32,
		};


		namespace detail {
			inline void* allocate_aligned_memory(size_t align, size_t size)
			{
				if (size == 0)
					return nullptr;

#ifdef _WIN32
				void* ptr = _aligned_malloc(size, align);
#else
				//void* ptr = aligned_alloc(size, align);
				void* ptr = malloc(size);
#endif

				return ptr;
			}


			inline void deallocate_aligned_memory(void* ptr) noexcept
			{
#ifdef _WIN32
				return _aligned_free(ptr);
#else
				return free(ptr);
#endif
				
			}
		}


		template <typename T, Alignment Align = Alignment::AVX>
		class AlignedAllocator;


		template <Alignment Align>
		class AlignedAllocator<void, Align>
		{
		public:
			typedef void* pointer;
			typedef const void* const_pointer;
			typedef void              value_type;

			template <class U> struct rebind { typedef AlignedAllocator<U, Align> other; };
		};


		template <typename T, Alignment Align>
		class AlignedAllocator
		{
		public:
			typedef T         value_type;
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef T& reference;
			typedef const T& const_reference;
			typedef size_t    size_type;
			typedef ptrdiff_t difference_type;

			typedef std::true_type propagate_on_container_move_assignment;

			template <class U>
			struct rebind { typedef AlignedAllocator<U, Align> other; };

		public:
			AlignedAllocator() noexcept
			{}

			template <class U>
			AlignedAllocator(const AlignedAllocator<U, Align>&) noexcept
			{}

			size_type
				max_size() const noexcept
			{
				return (size_type(~0) - size_type(Align)) / sizeof(T);
			}

			pointer
				address(reference x) const noexcept
			{
				return std::addressof(x);
			}

			const_pointer
				address(const_reference x) const noexcept
			{
				return std::addressof(x);
			}

			pointer
				allocate(size_type n, typename AlignedAllocator<void, Align>::const_pointer = 0)
			{
				const size_type alignment = static_cast<size_type>(Align);
				void* ptr = detail::allocate_aligned_memory(alignment, n * sizeof(T));
				if (ptr == nullptr) {
					throw std::bad_alloc();
				}

				return reinterpret_cast<pointer>(ptr);
			}

			void
				deallocate(pointer p, size_type) noexcept
			{
				return detail::deallocate_aligned_memory(p);
			}

			template <class U, class ...Args>
			void
				construct(U* p, Args&&... args)
			{
				::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
			}

			void
				destroy(pointer p)
			{
				p->~T();
			}
		};


		template <typename T, Alignment Align>
		class AlignedAllocator<const T, Align>
		{
		public:
			typedef T         value_type;
			typedef const T* pointer;
			typedef const T* const_pointer;
			typedef const T& reference;
			typedef const T& const_reference;
			typedef size_t    size_type;
			typedef ptrdiff_t difference_type;

			typedef std::true_type propagate_on_container_move_assignment;

			template <class U>
			struct rebind { typedef AlignedAllocator<U, Align> other; };

		public:
			AlignedAllocator() noexcept
			{}

			template <class U>
			AlignedAllocator(const AlignedAllocator<U, Align>&) noexcept
			{}

			size_type
				max_size() const noexcept
			{
				return (size_type(~0) - size_type(Align)) / sizeof(T);
			}

			const_pointer
				address(const_reference x) const noexcept
			{
				return std::addressof(x);
			}

			pointer
				allocate(size_type n, typename AlignedAllocator<void, Align>::const_pointer = 0)
			{
				const size_type alignment = static_cast<size_type>(Align);
				void* ptr = detail::allocate_aligned_memory(alignment, n * sizeof(T));
				if (ptr == nullptr) {
					throw std::bad_alloc();
				}

				return reinterpret_cast<pointer>(ptr);
			}

			void
				deallocate(pointer p, size_type) noexcept
			{
				return detail::deallocate_aligned_memory(p);
			}

			template <class U, class ...Args>
			void
				construct(U* p, Args&&... args)
			{
				::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
			}

			void
				destroy(pointer p)
			{
				p->~T();
			}
		};

		template <typename T, Alignment TAlign, typename U, Alignment UAlign>
		inline
			bool
			operator== (const AlignedAllocator<T, TAlign>&, const AlignedAllocator<U, UAlign>&) noexcept
		{
			return TAlign == UAlign;
		}

		template <typename T, Alignment TAlign, typename U, Alignment UAlign>
		inline
			bool
			operator!= (const AlignedAllocator<T, TAlign>&, const AlignedAllocator<U, UAlign>&) noexcept
		{
			return TAlign != UAlign;
		}
	}



	using sse_vector = std::vector<uint8_t,allocators::AlignedAllocator<uint8_t, d8u::allocators::Alignment::SSE> >;
	using aligned_vector = sse_vector;

	using namespace std;

	template < typename T, std::size_t N > constexpr size_t __array_elements(const T(&)[N]) { return (size_t)N; }
	template < typename T, std::size_t N > constexpr size_t __array_element_size(const T(&)[N]) { return (size_t)sizeof(T); }
	template < typename T > constexpr size_t __array_bytes(T& t) { return __array_elements(t) * __array_element_size(t); }

	inline vector<uint8_t> htob(std::string_view v)
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

		operator string_view() { return string_view((const char*)data(), size()); }
		operator int()
		{
			int result = 0;

			auto v = string_view((const char*)data(), size());
			std::from_chars(v.data(), v.data() + v.size(), result);

			return result;
		}

		operator bool()
		{
			int result = 0;

			if (!size())
				return false;

			auto v = string_view((const char*)data(), size());

			if (v[0] == 'T' || v[0] == 't')
				return true;

			if (v[0] == 'F' || v[0] == 'f')
				return false;

			std::from_chars(v.data(), v.data() + v.size(), result);

			return result != 0;
		}

		operator size_t()
		{
			size_t result = 0;

			auto v = string_view((const char*)data(), size());
			std::from_chars(v.data(), v.data() + v.size(), result);

			return result;
		}

		operator int64_t()
		{
			int64_t result = 0;

			auto v = string_view((const char*)data(), size());
			std::from_chars(v.data(), v.data() + v.size(), result);

			return result;
		}

		operator double()
		{
			return std::stod((const char*)data());
			/*double result = 0;

			auto v = string_view((const char*)data(), size());
			std::from_chars(v.data(), v.data() + v.size(), result);

			return result;*/
		}

		operator string() { return string((const char*)data(), size()); }

		operator vector<uint8_t>()
		{
			auto v = string_view((const char*)data(), size());
			return htob(v);
		}

		Memory Slice(size_t i, size_t l = -1) const
		{
			if (-1 == l)
				l = length - i;

			return Memory(buffer + i, l);
		}

		void TryQuoteWrapper()
		{
			if (*(buffer - 1) == '\"' && *(buffer + length) == '\"')
			{
				buffer--;
				length += 2;
			}
		}

		bool operator==(const Memory& m)
		{
			if (length != m.length)
				return false;

			return std::memcmp(buffer, m.buffer, length) == 0;
		}

	private:
		uint8_t* buffer;
		size_t length;
	};
}