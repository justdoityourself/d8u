/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <charconv>
#include <string_view>
#include <vector>

#include "../gsl-lite.hpp"

namespace d8u
{
	template < typename C> gsl::span<uint8_t> byte_buffer(C& buffer)
	{
		return gsl::span<uint8_t>((uint8_t*)buffer.data(), buffer.size() * sizeof(C::value_type));
	}

	template < typename T, typename C> gsl::span<T> t_buffer(C& buffer)
	{
		return gsl::span<T>((T*)buffer.data(), buffer.size() * sizeof(C::value_type) / sizeof(T));
	}

	template < typename T, typename C> auto t_buffer_copy(C& buffer)
	{
		auto _buffer = t_buffer<T>(buffer);
		std::vector<T> result(_buffer.size());

		std::copy(_buffer.begin(), _buffer.end(), result.begin());

		return result;
	}

	namespace buffer
	{
		class Helper
		{
			gsl::span<uint8_t> buffer;
		public:
			Helper() {}

			Helper(gsl::span<uint8_t> _b)
				: buffer(_b) {}

			Helper(uint8_t * data, size_t size)
				: buffer(data,size) {}

			template <typename T> Helper(const T & t)
				: buffer((uint8_t*)t.data(), t.size()) {}

			Helper substr(uint32_t s, size_t e = 0)
			{
				if (!e)
					e = size() - s;

				if (e < 0)
					e = size() + e - s;

				if (s + e > size())
					return Helper();

				return Helper(data() + s, e);
			}

			void Zero() const
			{
				std::memset(buffer.data(), 0, buffer.size());
			}

			bool StartsWith(Helper m)
			{
				if (m.size() > size()) 
					return false;

				return 0 == std::memcmp(data(), m.data(), m.size());
			}

			const char* c_str() { return (const char*)data(); }

			uint8_t* begin() { return buffer.begin(); }
			uint8_t* end() { return buffer.end(); }

			const uint8_t* begin() const { return buffer.begin(); }
			const uint8_t* end() const { return buffer.end(); }

			uint8_t* data() { return buffer.data(); }
			const uint8_t* data() const { return buffer.data(); }
			size_t size() const { return buffer.size(); }


			template < typename T > T* NextT(size_t& o, const T& del)
			{
				T* itr = pt<T>(o);
				T* start = itr;
				T* end = plt<T>();

				while (itr < end)
				{
					o++;
					if (del == *itr)
						break;
					itr++;
				}

				if (!(itr < end))
					o++;

				return start;
			}

			template < typename T, typename D > T* NextUnalignedT(size_t& o, const D& del)
			{
				uint8_t* itr = pt<uint8_t>(o);
				uint8_t* start = itr;
				uint8_t* end = plt<uint8_t>();

				while (itr < end)
				{
					o++;
					if (del == *((D*)itr))
						break;
					itr++;
				}

				if (!(itr < end))
					o++;

				return (T*)start;
			}

			uint8_t* Next(size_t& o, uint8_t del)
			{
				return NextT<uint8_t>(o, del);
			}

			Helper QuoteSegment(uint8_t del = '\"')
			{
				size_t itr = 0;
				uint8_t* f = nullptr, * l = nullptr;
				for (; itr < size(); itr++) { if (del == *(data() + itr)) { f = data() + itr; break; } }
				for (; itr < size(); itr++) { if (del == *(data() + itr)) { l = data() + itr; break; } }

				if (!(l && f))
					return Helper();

				return Helper(f + 1, (size_t)(l - f - 2));
			}

			Helper NextString(size_t& o, uint8_t del)
			{
				size_t start = o;

				auto str = NextT<uint8_t>(o, del);

				if (!str)
					return Helper();

				return Helper(str, o - start - 1);
			}

			std::pair<Helper, Helper> Divide(uint8_t c)
			{
				for (size_t i = 0; i < size(); i++)
				{
					if (buffer[i] == c)
						return std::make_pair(Helper(data(), i), Helper(data() + i + 1, size() - (i + 1)));
				}

				return std::make_pair(Helper(data(), size()), Helper());
			}

			Helper NextWord(size_t & o)
			{
				return NextString(o, ' ');
			}

			Helper NextLine(size_t& o)
			{
				size_t start = o;
				uint16_t nl = '\n'; nl <<= 8; nl ^= '\r';
				auto str = NextUnalignedT<uint8_t, uint16_t>(o, nl);

				return Helper(str, o - start - 1);
			}

			Helper GetLine()
			{
				size_t o = 0;
				Helper result = NextLine(o);

				Seek(o + 1);

				return result;
			}

			Helper GetWord()
			{
				size_t o = 0;
				Helper result = NextWord(o);

				Seek(o);

				return result;
			}

			Helper GetWord(uint8_t d)
			{
				size_t o = 0;
				Helper result = NextString(o, d);

				Seek(o);

				return result;
			}

			Helper SkipWord(uint8_t d)
			{
				size_t o = 0;
				NextString(o, d);

				Seek(o);

				return (*this);
			}

			Helper GetString(uint8_t d)
			{
				size_t o = 0;
				Helper result = NextString(o, d);

				Seek(o);

				return result;
			}

			Helper LowerCase()
			{
				std::transform(begin(), end(), begin(), ::tolower);

				return (*this);
			}

			Helper FindI(Helper m)
			{
				if (m.size() > size())
					return Helper();

				for (uint32_t i = 0; i < size() - m.size() + 1; i++)
				{
					bool match = true;
					for (uint32_t j = 0; j < m.size(); j++)
					{
						if (::tolower(*(m.data() + j)) != ::tolower(*(data() + i + j)))
						{
							match = false;
							break;
						}
					}
					if (match)
						return Helper(data() + i, m.size());
				}

				return Helper();
			}

			Helper Find(Helper m)
			{
				if (m.size() > size())
					return Helper();

				for (uint32_t i = 0; i < size() - m.size() + 1; i++)
				{
					if (0 == std::memcmp(m.data(), data() + i, m.size()))
						return Helper(data() + i, m.size());
				}

				return Helper();
			}

			void Seek(size_t n)
			{
				if (n > size())
					buffer = gsl::span<uint8_t> ( data() + size(), (size_t)0 );
				else
					buffer = gsl::span<uint8_t> (data() + n, size() - n);
			}

			operator std::string_view() const
			{
				return { (char*)data(),size() };
			}

			operator int() const
			{
				int result = 0;

				auto v = std::string_view((const char*)data(), size());
				std::from_chars(v.data(), v.data() + v.size(), result);

				return result;
			}

			template < typename Y > Y* pt(size_t index = 0, size_t g_index = 0) const
			{
				return ((Y*)(data() + g_index)) + index;
			}

			template < typename Y > Y& rt(size_t index = 0, size_t g_index = 0) const
			{
				return *pt<Y>(index, g_index);
			}

			template < typename T > T* plt(size_t index = 0)
			{
				return (((T*)(data() + size())) - (index));
			}

			template < typename Y > Y& rlt(size_t index = 0)
			{
				return *plt<Y>(index);
			}
		};

		template <typename T> size_t _memory_size(const T& t)
		{
			return t.size();
		}

		template <typename T, typename ... t_args> size_t _memory_size(const T& t, t_args ... args)
		{
			return t.size() + _memory_size(args...);
		}

		template <typename D, typename T> void _join_memory(D& m, size_t offset, const T& t)
		{
			if (t.data()) 
				std::memcpy(m.data() + offset, t.data(), t.size());
		}

		template <typename D, typename T, typename ... t_args> void _join_memory(D& m, size_t offset, const T& t, t_args ... args)
		{
			if (t.data()) 
				std::memcpy(m.data() + offset, t.data(), t.size());

			_join_memory(m, offset + t.size(), args...);
		}

		template <typename T, typename ... t_args> std::vector<uint8_t> join_memory(const T& t, t_args ... args)
		{
			std::vector<uint8_t> result( _memory_size(t, args...) );

			_join_memory(result,0,t, args...);

			return result;
		}
	}
}