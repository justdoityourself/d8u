/* Copyright (C) 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

namespace d8u
{
	size_t JsonObjectParse(std::string_view json,size_t depth=0)
	{
		const char* itr = json.data();
		const char* end = itr+json.size();

		for (; itr < end; itr++)
		{
			switch (*itr)
			{
			case '\"':
				itr++;

				while (*itr != '"' && itr < end)
					itr++;

				break;
			case '{':
				depth++;

				break;

			case '}':
				depth--;

				if (!depth)
					return ++itr-json.data();

				break;
			}
		}

		return json.size();
	}

	enum StackTypes : uint8_t
	{
		TypeObject,
		TypeArray,

		TypeString,
		TypeUndetermined,
		TypeClose,
		TypeUnused,
		TypeRoot
	};

	struct JsonStreamRecord
	{
		StackTypes current;
		StackTypes transition;
		std::string_view key;
		std::string_view value;
	};

	template <size_t depth_c=64> bool StreamJsonNoRecursion(std::string_view json, auto cb)
	{
		std::string_view key;
		const char* start = nullptr;
		bool inside_string = false;
		StackTypes current_type = TypeRoot;

		size_t depth = -1;
		StackTypes stack[depth_c];

		for (const auto& c : json)
		{
			if (inside_string)
			{
				if (c != '"')
					continue;
				else if (key.size())
					cb(current_type, TypeString, key, std::string_view(start, &c - start)), key = std::string_view(), start=nullptr;
				else if (current_type == TypeArray)
					cb(current_type, TypeString, key, std::string_view(start, &c - start)), start = nullptr;
				else
					key = std::string_view(start, &c - start), start = nullptr;
				
				inside_string = false;
				continue;
			}	

			if (current_type == TypeUndetermined && c != '{')
				continue;

			switch (c) 
			{
			case ':': case ' ': case '\t': case '\n': case '\r': break;
			case '\"':
				start = (const char*)&c + 1;
				inside_string = true;
				break;
			case '{':
				if (depth == depth_c)
					return false;

				cb(current_type, TypeObject, key, std::string_view()), key = std::string_view(), start = nullptr;
				stack[++depth] = current_type = TypeObject;

				break;

			case ']':
			case '}':

				if (start)
					cb(current_type, TypeUndetermined, key, std::string_view(start, &c - start)), key = std::string_view(), start = nullptr;

				if (depth==-1)
					return false;

				cb(current_type, TypeClose, std::string_view(), std::string_view()), key = std::string_view(), start = nullptr;
				current_type = stack[--depth];

				break;

			case '[':
				if (depth == depth_c)
					return false;

				cb(current_type, TypeArray, key, std::string_view()), key = std::string_view(), start = nullptr;
				stack[++depth] = current_type = TypeArray;

				break;

			case ',':
				if (start)
					cb(current_type, TypeUndetermined, key, std::string_view(start, &c - start)), key = std::string_view(), start = nullptr;
				break;
			default:
				if(!start) 
					start = (const char*)&c;
				break;
			}
		}

		return true;
	}

	template <typename int_t> struct JsonStreamRecord2
	{
		StackTypes current;
		StackTypes type;
		int_t key;
		int_t key_length;
		int_t value;
		int_t value_length;

		auto Key(const auto& json) const
		{
			return std::string_view(json.data() + key, key_length);
		}

		auto Value(const auto& json) const
		{
			return std::string_view(json.data() + value, value_length);
		}
	};

	enum class StreamActions2
	{
		Continue,
		Break,
		SkipObject
	};

#define ActionHandler2() \
if constexpr (action_e) {switch(action){\
case StreamActions2::Continue: break;\
case StreamActions2::Break: return false; break;\
case StreamActions2::SkipObject: if constexpr (simd_e) itr += JsonAvx2Parse(itr,1)-2; else itr += JsonObjectParse(std::string_view(itr,end),1)-2; break;\
}}

	template <typename int_t = int32_t, size_t depth_c = 64, bool action_e = false, bool simd_e = true> bool StreamJsonNoRecursion2(std::string_view json, auto cb)
	{
		int_t key_dx = 0;
		int_t key_len = 0;
		StreamActions2 action;

		const char* start = nullptr;
		StackTypes current_type = TypeRoot;

		size_t depth = -1;
		StackTypes stack[depth_c];

		const char* itr = json.data();
		const char* end = itr + json.size();

		const char* root = itr;
		for (; itr < end; itr++)
		{
			if (current_type == TypeUndetermined && *itr != '{')
				continue;

			switch (*itr)
			{
			case '\"':
				start = itr + 1;

				if constexpr (simd_e)
					itr += find_avx2_unaligned(start, '"') + 1;
				else
					while (*++itr != '"');

				if (key_dx)
				{
					action = cb(JsonStreamRecord2< int_t>{ current_type, TypeString, key_dx, key_len, (int_t)(start - root), (int_t)(itr - start) }, depth);

					ActionHandler2();

					key_dx = 0, key_len = 0, start = nullptr;
				}
				else if (current_type == TypeArray)
				{
					action = cb(JsonStreamRecord2< int_t>{ current_type, TypeString, 0, 0, (int_t)(start - root), (int_t)(itr - start) }, depth);

					ActionHandler2();

					start = nullptr;
				}
				else
					key_dx = (int_t)(start - root), key_len = (int_t)(itr - start), start = nullptr;

				break;
			case '{':
				if (depth == depth_c)
					return false;

				action = cb(JsonStreamRecord2< int_t>{ current_type, TypeObject, key_dx, key_len, (int_t)(itr - root), 0 }, depth);

				ActionHandler2();

				key_dx = 0, key_len = 0, start = nullptr;
				stack[++depth] = current_type = TypeObject;

				break;

			case ']':
			case '}':

				if (start)
				{
					action = cb(JsonStreamRecord2< int_t>{ current_type, TypeUndetermined, key_dx, key_len, (int_t)(start - root), (int_t)(itr - start) }, depth);

					ActionHandler2();

					key_dx = 0, key_len = 0, start = nullptr;
				}

				if (depth == (size_t)-1)
					return false;

				action = cb(JsonStreamRecord2< int_t>{ current_type, TypeClose, 0, 0, (int_t)((itr + 1) - root), 0 }, depth);

				ActionHandler2();

				key_dx = 0, key_len = 0, start = nullptr;
				current_type = stack[--depth];

				break;

			case '[':
				if (depth == depth_c)
					return false;

				action = cb(JsonStreamRecord2< int_t>{ current_type, TypeArray, key_dx, key_len, 0, 0 }, depth);

				ActionHandler2();

				key_dx = 0, key_len = 0, start = nullptr;
				stack[++depth] = current_type = TypeArray;

				break;

			case ':': case ' ': case '\t': case '\n': case '\r':
			case ',':
				if (start)
				{
					action = cb(JsonStreamRecord2< int_t>{ current_type, TypeUndetermined, key_dx, key_len, (int_t)(start - root), (int_t)(itr - start) }, depth);

					ActionHandler2();

					key_dx = 0, key_len = 0, start = nullptr;
				}
				break;
			default:
				if (!start)
					start = itr;
				break;
			}
		}

		return true;
	}

	template<size_t max_c> class JsonFixedSeq
	{
	public:
		JsonFixedSeq() {}
		JsonFixedSeq(const auto& j) { Init(j); }

		bool Init(const auto& j) {
			size_t index = 0;
			return StreamJsonNoRecursion(j, [&](auto cur, auto type, auto k, auto v) {
				if (index >= max_c)
					throw "Overflow";

				sequence[index++] = { cur,type,k,v };
			});
		}

		bool Stream(auto f)
		{
			for (const auto& s : sequence)
				f(s.current, s.transition, s.key, s.value);

			return true;
		}

		bool Stream2(const auto& j, auto f) {
			size_t index = 0;
			return StreamJsonNoRecursion(j, [&](auto cur, auto type, auto k, auto v) {
				if (index >= max_c)
					throw "Overflow";

				sequence[index++] = { cur,type,k,v };
				f(cur, type, k, v);
			});
		}

	private:
		std::array< JsonStreamRecord, max_c> sequence;
	};

	template<typename int_t, size_t max_c> class JsonFixedSeq2
	{
	public:
		JsonFixedSeq2() {}
		JsonFixedSeq2(const auto& j) { Init(j); }

		bool Init(const auto& j) {
			index = 0;
			return StreamJsonNoRecursion2<int_t>(j, [&](const auto & v, auto depth) {
				if (index >= max_c)
					throw "Overflow";

				sequence[index++] = v;

				return StreamActions2::Continue;
			});
		}

		bool Stream(auto f)
		{
			for (const auto& s : sequence)
				f(s.current, s.transition, s.key, s.value);

			return true;
		}

		bool Stream2(const auto& j, auto f) {
			index = 0;
			return StreamJsonNoRecursion2<int_t>(j, [&](const auto & v, auto depth) {
				if (index >= max_c)
					throw "Overflow";

				sequence[index++] = v;
				f(v);

				return StreamActions2::Continue;
			});
		}

	private:
		size_t index;
		std::array< JsonStreamRecord2<int_t>, max_c> sequence;
	};

	class JsonCountControl
	{
	public:
		JsonCountControl() {}
		JsonCountControl(const auto& j) { Init(j); }

		bool Init(const auto& j) {
			return StreamJsonNoRecursion(j, [&](auto cur, auto type, auto k, auto v) {
				count++;
			});
		}

		size_t count = 0;
	};

	class JsonSeq
	{
	public:
		JsonSeq() {}
		JsonSeq(const auto& j) { Init(j); }

		bool Init(const auto& j) {
			return StreamJsonNoRecursion(j, [&](auto cur, auto type, auto k, auto v) {
				sequence.push_back({ cur,type,k,v });
			});
		}

		bool Stream(auto f)
		{
			for (const auto& s : sequence)
				f(s.current, s.transition, s.key, s.value);

			return true;
		}

		bool Stream2(const auto& j, auto f) {
			return StreamJsonNoRecursion(j, [&](auto cur, auto type, auto k, auto v) {
				sequence.push_back({ cur,type,k,v });
				f(cur, type, k, v);
			});
		}

	private:
		std::vector< JsonStreamRecord > sequence;
	};

	class FastHash 
	{
		uint32_t hash = 0;
	public:
		FastHash() {}

		FastHash(uint32_t _hash): hash(_hash) {}

		FastHash(std::string_view v)
		{
			Run(v);
		}

		void Run(std::string_view v)
		{
			for (const auto& c : v)
				hash ^= 882319 * c;
		}

		size_t operator % (size_t lim)
		{
			return hash % lim;
		}

		FastHash operator ^ (const FastHash& r)
		{
			return FastHash(hash ^ r.hash);
		}
	};

	class DoubleHash
	{
		uint64_t hash = 0;
	public:
		DoubleHash() {}

		DoubleHash(uint64_t _hash) : hash(_hash) {}

		DoubleHash(std::string_view v)
		{
			Run(v);
		}

		void Run(std::string_view v)
		{
			for (const auto& c : v)
				hash ^= 436327324831 * c;
		}

		size_t operator % (size_t lim)
		{
			return hash % lim;
		}

		DoubleHash operator ^ (const DoubleHash& r)
		{
			return DoubleHash(hash ^ r.hash);
		}
	};

	//MSVC fails to compile a perfectly valid fold expression so this is the work around
	size_t ___match_all(size_t i, const auto & key, const auto & test, const auto &... rest) {
		if (test == key)
			return i;

		if constexpr (sizeof...(rest) > 0) 
			return ___match_all(i+1,key,rest...);
		
		return -1;
	}

	auto ArraySelect(const auto& j, auto ... args_t)
	{
		constexpr auto total = sizeof...(args_t);
		std::array<std::string_view, total> result;

		size_t current = 0;

		StreamJsonNoRecursion2<int16_t,64,true>(j, [&](const auto& v, auto depth)
		{
			if (current == total)
				return StreamActions2::Break; // Todo short circuit iteration

			switch (v.type)
			{
			default: return StreamActions2::Continue;
			case TypeString:
			case TypeUndetermined:
				if (size_t i = ___match_all(0, v.Key(j), args_t...); i != -1)
					current++, result[i] = v.Value(j);

				return StreamActions2::Continue;
			}
		});

		return result;
	}

	template < typename int_t = int32_t, size_t depth_c = 64, bool action_e=false> auto StreamPath(const auto& j, auto f)
	{
		std::string path;
		int_t index[depth_c] = { 0 };

		return StreamJsonNoRecursion2<int_t, depth_c,action_e>(j, [&](const auto& v, auto depth)
		{
			switch (v.type)
			{
			case TypeClose:
			{
				auto find = path.rfind('/', path.size() - 2);
				if (find == -1)
					path = "";
				else
				{
					path.erase(find + 1);
					index[depth] = 0;
				}
			}
				return StreamActions2::Continue;
			case TypeObject:
			case TypeArray:
				if (v.current == TypeRoot)
					return StreamActions2::Continue;

				if (v.current == TypeObject)
				{
					auto key = v.Key(j);
					path.append(key.data(), key.size());
					path += '/';
				}
				else
					path += std::to_string(index[depth]) + "/";

				index[depth]++;

				return StreamActions2::Continue;
			case TypeString:
			case TypeUndetermined:
			{
				auto result = f(path + ( v.current == TypeObject ? std::string(v.Key(j)) : std::to_string(index[depth])), v);

				index[depth]++;
				return result;
			}
			}
		});
	}

	template < typename int_t=int32_t, size_t depth_c=64> auto ArrayPath(const auto& j, auto ... args_t)
	{
		constexpr auto total = sizeof...(args_t);
		std::array<std::string_view, total> result;

		size_t current = 0;

		StreamPath<int_t, depth_c,true>(j, [&](const auto& path, const auto& v) 
		{
			if (size_t i = ___match_all(0, path, args_t...); i != -1)
				current++, result[i] = v.Value(j);

			return current != total ? StreamActions2::Continue : StreamActions2::Break;
		});

		return result;
	}

	auto JsonSelectOne(const auto& j, auto one)
	{
		auto arr = ArraySelect(j, one);
		return arr[0];
	}

	auto JsonSelect(const auto& j, auto ... args_t)
	{
		return std::tuple_cat(ArraySelect(j,args_t...));
	}

	auto JsonPathOne(const auto& j, auto one)
	{
		auto arr = ArrayPath(j, one);
		return arr[0];
	}

	auto JsonPath(const auto& j, auto ... args_t)
	{
		return std::tuple_cat(ArrayPath(j, args_t...));
	}

	auto JsonSelectCopy(const auto& j, auto ... args_t)
	{
		auto r = d8u::JsonSelect(j, args_t...);

		auto copy = [&]<std::size_t... I>(std::index_sequence<I...>) {
			return std::make_tuple(std::string(std::get<I>(r))...);
		};

		return copy(std::make_index_sequence<sizeof...(args_t)>()); 
	}

//TODO this doesn't work because all #__VA_ARGS__ turns into one string instead of a list of strings
//#define JsonMove(x,...) std::tie(__VA_ARGS__) = d8u::JsonSelectCopy(x,#__VA_ARGS__)

	template<typename int_t> int_t fast_atot(std::string_view str)
	{
		int_t val = 0;
		for (auto c : str)
			val = val * 10 + (c - '0');

		return val;
	}

	template <typename T> T _Convert(const auto& a)
	{
		if (!a.size())
			return T();

		if constexpr (std::is_same_v<T, bool>)
			return (a == "true" || a == "1") ? true : false;
		else if constexpr (std::is_same_v<T, std::string_view>)
			return a;
		else if constexpr (std::is_same_v<T, std::string>)
			return std::string(a);
		else if constexpr (std::is_same_v<T, int64_t>)
			return fast_atot<T>(a);
		else if constexpr (std::is_same_v<T, uint64_t>)
			return fast_atot<T>(a);
		else if constexpr (std::is_same_v<T, int>)
			return fast_atot<T>(a);
		else if constexpr (std::is_same_v<T, float>)
			return std::stof(a.data());
		else if constexpr (std::is_same_v<T, double>)
			return std::stod(a.data());
	}

	template<typename ...types_t> auto TypeBase(const auto& arr)
	{
#ifdef _MSC_VER
		size_t i = arr.size();

		auto convert = [&]<typename T>(T t) { return _Convert<T>(arr[--i]); };
#else
		size_t i = 0;

		auto convert = [&]<typename T>(T t) { return _Convert<T>(arr[i++]); };
#endif

		return std::tuple{ convert(types_t()) ... };
	}

	template<typename ...types_t> auto TypeSelect(const auto& j, auto ... args_t)
	{
		return TypeBase<types_t...>(ArraySelect(j, args_t...));
	}

	template<typename ...types_t> auto TypePath(const auto& j, auto ... args_t)
	{
		return TypeBase<types_t...>(ArrayPath(j, args_t...));
	}

	template<typename T> auto TypeSelectOne(const auto& j, auto one)
	{
		return _Convert<T>(JsonSelectOne(j, one));
	}

	template<typename T> auto TypePathOne(const auto& j, auto one)
	{
		return _Convert<T>(JsonPathOne(j, one));
	}

	template<size_t depth_c=64, size_t map_c=256, typename int_t=uint32_t, typename hash_t = FastHash, bool simple = true> class JsonFixedRng
	{
	public:
		JsonFixedRng() {}
		JsonFixedRng(const auto& j) 
		{ 
			if constexpr (simple)
				InitFlat(j);
			else 
				Init(j); 
		}

		auto Find(std::string_view path)
		{
			return FindN<0>(path);
		}

		template <size_t N> auto FindN(std::string_view path)
		{
			size_t bucket = hash_t(path) % map_c;

			auto ts = path.rfind("/");
			auto tail = (ts == -1) ? path : path.substr(ts+1);

			size_t extension = 0;
			size_t dx = 0;

			while (extension <= overflow)
			{
				const auto& v = map[bucket + extension % map_c];
				
				if (auto key = v.Key(json); (!key.size() || tail == key) && dx++ == N)
					return v.Value(json);

				extension++;
			}

			return std::string_view();
		}

		auto operator[] (const auto & path)
		{
			return Find(path);
		}

		auto FindEach(auto ... args_t)
		{
			return std::make_tuple(Find(args_t)...);
		}

		template <typename type> auto FindT(std::string_view path)
		{
			return FindEachT<type>(path);
		}

		template <typename ... types_t> auto FindEachT(auto ... args_t)
		{
			std::array<std::string_view, sizeof...(types_t)> arr;
			size_t i = arr.size();

			((arr[--i] = Find(args_t)), ...);

			return TypeBase<types_t...>(arr);
		}

		template <typename ... types_t> auto operator() (auto ... args_t)
		{
			return FindEachT<types_t...>(args_t...);
		}

		template <size_t N> auto FindEachN(auto ... args_t)
		{
			return std::tuple{ FindN<N>(args_t)... };
		}

		void FindAll(std::string_view path, auto f)
		{
			size_t bucket = hash_t(path) % map_c;

			size_t extension = 0;

			while (extension <= overflow)
			{
				const auto& v = map[bucket + extension % map_c];
				if (path == std::string_view(json.data() + v.key, v.key_length))
					f(std::string_view(json.data() + v.value, v.value_length));

				extension++;
			}
		}

	protected:
		void Clear()
		{
			overflow = total = 0;

			for (auto& m : map)
				m.type = TypeUnused;
		}

		auto* Insert(const auto& key, const auto& v, hash_t parent = 0)
		{
			if (total >= map_c)
				throw "Overflow";

			size_t bucket = (parent ^ hash_t(key)) % map_c;

			size_t extension = 0;

			while (map[(bucket + extension) % map_c].type != TypeUnused)
				extension++;

			if (extension > overflow)
				overflow = extension;

			total++;

			auto dx = (bucket + extension) % map_c;
			map[dx] = v;

			return &map[dx];
		}

		bool InitFlat(const auto& j) 
		{
			//
			// All key accessible in a single tier
			// Collisions must be discovered with FindAll or FindN
			// Pros: is faster, less work for simple schema's that don't have recursion or collitions.
			//

			Clear();
			json = j;

			return StreamJsonNoRecursion2<int_t,depth_c>(j, [&](const auto& v, auto depth)
			{
				switch (v.type)
				{
				default: return StreamActions2::Continue;
				case TypeString:
				case TypeUndetermined: Insert(v.Key(json),v); return StreamActions2::Continue;
				}
			});
		}

		bool Init(const auto& j)
		{
			//
			// All keys accessible in a nested path structure
			// "object/key"
			// "object/array/2"
			// When interating collisions, arrays can't be resolved correctly right now.
			//

			Clear();
			json = j;

			return StreamPath<int_t, depth_c>(j, [&](const auto& path, const auto& v) {
				Insert(path, v);
				return StreamActions2::Continue;
			});
		}

	private:
		size_t total;
		size_t overflow;

		std::string_view json;
		std::array< JsonStreamRecord2<int_t>, map_c> map;
	};

	using JsonSelectS = JsonFixedRng<16,256,uint16_t,FastHash,true>;
	using JsonPathS = JsonFixedRng<16, 256, uint16_t,FastHash, false>;

	template <typename H = uint64_t, size_t max_depth = 16> class JsonStackHash
	{
	public:
		JsonStackHash() { }

		template <typename J> auto Add(const J& j)
		{
			size_t depth = 1;
			H stack[max_depth]; stack[0] = 0;

			H result = 0;

			StreamJsonNoRecursion( j, [&](auto cur, auto type, auto k, auto v) {
				switch (type)
				{
				case TypeClose:
					depth--;
					break;
				case TypeObject:
				case TypeArray:
					if (depth == max_depth)
						throw "Json depth exceeds limit.";

					stack[depth++] = HashK(k) ^ (depth ? stack[depth++] : 0);
					break;

				case TypeString:
				case TypeUndetermined:
					result ^= stack[depth++] = AddKV(k, v, stack[depth]);
					break;
				}
			});

			return hash ^= result;
		}

		H AddKV(std::string_view key, std::string_view value, H parent = 0)
		{
			return hash ^= Rotate(parent % (sizeof(H) * 8), HashKV(key, value));
		}

		template <typename J> H Remove(const J& j) { return Add(j); }
		H RemoveKV(std::string_view key, std::string_view value) { return AddKV(key, value); }

	private:

		H Rotate(size_t rotation, H target) {
			if (!rotation) return target;

			return std::rotl(target, rotation);
		}

		H HashM(std::string_view m, H result = 0x41feda847185c128)
		{
			H* p = (H*)m.data();

			size_t c = m.size() / sizeof(H), r = m.size() % sizeof(H);

			for (size_t i = 0; i < c; i++, p++)
				result ^= *p;

			return result ^= *p >> (r * 8);
		}

		H HashV(std::string_view value) {
			return HashM(value);
		}

		H HashK(std::string_view key) {
			if (!key.size())
				return 0;

			return HashM(key, 0xef7128eab3018429);
		}

		H HashKV(std::string_view key, std::string_view value)
		{
			return HashK(key) ^ HashV(value);
		}

		H hash = 0xfe48ca1209bcd713;
	};


	template <size_t depth_c = 64> std::string StreamJsonMirror(std::string_view json)
	{
		std::string result;
		result.reserve(json.size());

		size_t depth = -1;
		int16_t stack[depth_c];

		auto success = StreamJsonNoRecursion<depth_c>(json, [&](auto current_type, auto type, auto key, auto value) 
		{
			switch (type)
			{
			case TypeObject:
				if (current_type!= TypeUndetermined  && stack[depth]++)
					result += ",";

				if (key.size())
					result += "\"" + std::string(key) + "\":";

				result += "{";

				stack[++depth] = 0;

				break;
			case TypeArray:
				if (current_type != TypeUndetermined && stack[depth]++)
					result += ",";

				if (key.size())
					result += "\"" + std::string(key) + "\":";

				result += "[";


				stack[++depth] = 0;

				break;
			
			case TypeString:
				value = std::string_view(value.data() - 1, value.size() + 2);
			case TypeUndetermined:
				if (stack[depth]++)
					result += ",";

				if (current_type == TypeObject)
					result += "\"" + std::string(key) + "\":" + std::string(value);
				else
					result += value;
				break;
			case TypeClose:
				depth--;
				if(current_type == TypeObject)
					result += "}";
				else
					result += "]";
				break;
			}
		});

		return success ? result : "";
	}
}