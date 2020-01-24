/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <type_traits>
#include "memory.hpp"

namespace d8u
{

	template < typename T > void ExpandT ( T* root, int count, int at_index )
	{
		for (int i = count - 1; i >= at_index; i--)
			*(root + i + 1) = *(root + i);
	}

	template <typename T, typename P, typename std::enable_if<!std::is_pointer<T>::value>::type* = nullptr > T* InsertTSM(T* root, size_t count, const T & element, bool multimap, bool*pexpand, P param )
	{
		if (!count)
		{
			root[0] = element;
			if(pexpand)*pexpand = true;
			return root;
		}

		auto lt = [] ( Memory&& l, Memory && r ) -> bool 
		{ 
			auto cmp = strncmp ( (const char*)l.data() , (const char *) r.data(), (l.size() > r.size()) ? r.size() : l.size() );

			if(0 == cmp)
				return l.size() < r.size();

			auto re = cmp < 0;
			return re; 
		};
		auto cmp = [&] ( auto & l, auto & r ) -> bool { return lt ( l.Key(param), r.Key(param) ); };

		bool e = false;
		int low = 0;
		int high = (int)count-1;

		while (low <= high)
		{
			int middle = (low + high) >> 1;

			if (cmp(*(root+middle), element))
				low = middle + 1;
			else if (cmp(element, *(root+middle)))
				high = middle - 1;
			else { low = middle;  e = true; break; }
		}

		if( e && multimap || !e) ExpandT(root, (int)count, low);
		else if ( e&& !multimap )  { root[low].~T(); *pexpand = false; }
		else *pexpand = false;

		root[low] = element;

		return root + low;
	}

	template <typename T, typename P, typename std::enable_if<!std::is_pointer<T>::value>::type* = nullptr > T* FindTSM(T* root, size_t count, const Memory& s, P param)
	{
		if (!count)
			return nullptr;

		auto lt = [] ( const Memory& l, const Memory & r ) -> bool 
		{ 
			auto cmp = strncmp ( (const char*)l.data() , (const char *) r.data(), (l.size() > r.size()) ? r.size() : l.size() );

			if(0 == cmp)
				return l.size() < r.size();

			auto re = cmp < 0;
			return re; 
		};
		auto cmpl = [&](auto & o) -> bool { return lt(o.Key(param), s); };
		auto cmpr = [&](auto & o) -> bool { return lt(s, o.Key(param)); };

		int low = 0;
		int high = (int)count-1;

		while (low <= high)
		{
			int middle = (low + high) >> 1;

			if (cmpl(*(root+middle)))
				low = middle + 1;
			else if (cmpr(*(root+middle)))
				high = middle - 1;
			else { return root + middle; }
		}

		return nullptr;
	}
}