/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <stdexcept>
#include <cstdint>
#include <array>
#include <string_view>
#include <charconv>
#include <bit>

#include "memory.hpp"
#include "util.hpp"
#include "tsm.hpp"

#ifndef D8ULEAN
#include "../mio.hpp"
#endif

namespace d8u
{
	namespace json
	{
		using namespace std;

		class JsonStream
		{
		public:
			enum Types : uint8_t
			{
				TypeObject,
				TypeArray,

				TypeString,
				TypeParseFinished,
				TypeUndetermined,
			};

			struct StreamContext
			{
				StreamContext(bool bm = false) :current_key(-1), inside_string(false), binary_mode(bm) { Clear(false); }

				void Clear(bool comma = false)
				{
					value = nullptr;
					key = nullptr;

					current_key++;
					type = TypeString;

					binary = false;
					decimal = false;
					number = false;
					negative = false;
					expected_value = comma;
				}

				char* value;
				char* key;

				size_t current_key;

				size_t value_length;
				size_t key_length;

				bool binary;
				bool inside_string;
				bool decimal;
				bool number;
				bool negative;
				bool expected_value;
				bool binary_mode;
				Types type;
			};

			template < typename F > static void StreamObject(Memory& input, F function)
			{
				if (!input.size()) return;

				StreamContext context;
				size_t trim_chars = 0;

				if (!input.ScanTo('{'))
					throw std::runtime_error("Not a valid JSON Stream");

				while (input.size()) {
					switch (*input.data()) {

					case ' ': case '\t': case '\n': case '\r': trim_chars++; input++; continue;
					case '\\': if (context.inside_string) { input++; } break;
					case '\"':
						if (context.key == nullptr)context.key = (char*)input.data() + 1;
						else if (context.inside_string && !context.value)context.key_length = input.SegmentSize(context.key);
						else if (context.inside_string && context.value) { context.value_length = input.SegmentSize(context.value); function(TypeString, context); context.Clear(false); }
						else context.value = (char*)input.data() + 1;
						context.inside_string = !context.inside_string; break;
					case '{':
						if (!context.inside_string && context.key)
						{
							function(TypeObject, context);

							context.Clear(false);
							continue;
						} break;
					case '}':
						if (!context.inside_string)
						{
							if (context.key && context.value) 
							{ 
								context.value_length = input.SegmentSize(context.value) - trim_chars; 
								function(TypeUndetermined, context); 
								context.Clear(false); 
							}

							input++;

							function(TypeParseFinished, context);
							return;
						} break;
					case '[':
						if (!context.inside_string && context.key)
						{
							function(TypeArray, context);

							context.Clear(false);
							continue;
						} break;
					case ']': break;
					case ',':
						if (!context.inside_string)
						{
							if (context.key && context.value) { context.value_length = input.SegmentSize(context.value); function(TypeUndetermined, context); context.Clear(true); }
							else context.expected_value = true;
						}break;
					case ':': break;
					case '.': if (!context.inside_string) context.decimal = true; break;
					case '0': case '1': case '2': case '3': case '4': case'5': case '6': case '7': case '8': case '9': if (!context.inside_string) context.number = true;
					default:
						if (context.key && !context.value && !context.inside_string)
						{
							context.value = (char*)input.data();
						} break;
					} input++; trim_chars = 0;
				}

				throw std::runtime_error("Unexpected JSON Object EOF");
			}

			template < typename F > static void StreamArray(Memory& input, F function)
			{
				StreamContext context;
				uint32_t trim_chars = 0;

				if (!input.ScanTo('['))
					throw std::runtime_error("Not a valid JSON Array");

				while (input.size()) {
					switch (*input.data()) {

					case ' ': case '\t': case '\n': case '\r': trim_chars++; input++; continue;
					case '\\': if (context.inside_string) { input++; } break;
					case '\"':
						if (!context.value) context.value = (char*)input.data() + 1;
						else { context.value_length = input.SegmentSize(context.value); function(TypeString, context); context.Clear(false); }
						context.inside_string = !context.inside_string; break;
					case '{':
						if (!context.inside_string)
						{
							function(TypeObject, context);

							context.Clear(false);
							continue;
						} break;
					case '}': break;
					case '[':
						if (!context.inside_string)
						{
							function(TypeArray, context);

							context.Clear(false);

							continue;
						} break;
					case ']':
						if (!context.inside_string)
						{
							if (context.value) { context.value_length = input.SegmentSize(context.value) - trim_chars; function(Types::TypeUndetermined, context); context.Clear(false); }

							input++;

							function(Types::TypeParseFinished, context);

							return;
						} break;
					case ',':
						if (!context.inside_string)
						{
							if (context.value) { context.value_length = input.SegmentSize(context.value); function(Types::TypeUndetermined, context); context.Clear(true); }
							else context.expected_value = true;
						}break;
					case ':': break;
					case '.': if (!context.inside_string) context.decimal = true; break;
					case '0': case '1': case '2': case '3': case '4': case'5': case '6': case '7': case '8': case '9': if (!context.inside_string) context.number = true;
					default:
						if (!context.value && !context.inside_string)
						{
							context.value = (char*)input.data();
						}
						break;
					} input++; trim_chars = 0;
				}

				throw std::runtime_error("Unexpected JSON Array EOF");
			}
		};

		template < typename T > class JsonIndexDef : public JsonStream
		{
		public:
			struct _Index
			{
				_Index() {}
				_Index(T type_, T count_, T offset_, T length_) :
					type(type_),
					count(count_),
					offset(offset_),
					length(length_) {}

				Memory String(void* base) { return Memory(((uint8_t*)base) + offset, length); }
				std::string_view StringView(void* base) { return std::string_view(((const char*)base) + offset, length); }

				T type;
				T count;
				T offset;
				T length;

				void* Root() { return this; }
			};

		};

		template < typename T> class JsonIndexBackend : public JsonIndexDef<T>
		{
		public:
			using typename JsonIndexDef<T>::_Index;
			struct Header
			{
				Header() {}
				Header(T s, T t, T m, T a) :size(s), start(t) {}

				T size;
				T start;
			};

			struct Array
			{
				Array(const typename JsonStream::StreamContext& context, uint8_t* input_root)
					: Array
					((T)((context.value == (char*)-2) ? -2 : ((context.value == (char*)-3) ? -3 : (context.value - (char*)input_root)))
						, (T)context.value_length
						, context.binary
					) {}
				Array() {}
				Array(T o, T l, bool binary = false) 
					: offset(o)
					, length(l) 
				{ }

				Memory Value(void* base)
				{
					return Memory(((uint8_t*)base) + offset, length);
				}

				bool IsNode() { return offset == (T)-1; }
				typename JsonIndexDef<T>::_Index& Node(void* index_root) { return *((typename JsonIndexDef<T>::_Index*)(((uint8_t*)index_root) + length)); }

			private:
				T offset;
				T length;
			};

			struct Object
			{
				Object(const typename JsonStream::StreamContext& context, uint8_t* input_root)
					: Object ((T)((context.key == (char*)-2) ? -2 : (context.key - (char*)input_root))
						, (T)context.key_length
						, (T)((context.value == (char*)-2) ? -2 : ((context.value == (char*)-3) ? -3 : (context.value - (char*)input_root)))
						, (T)context.value_length
						, context.binary) {}
				Object() {}
				Object(T ko, T kl, T vo, T vl, bool binary = false) 
					: key_offset(ko)
					, key_length(kl)
					, value_offset(vo)
					, value_length(vl) 
				{ }

				void* Array()
				{
					return &value_offset;
				}

				Memory Key(void* base) const
				{
					return Memory(((uint8_t*)base) + key_offset, key_length);
				}

				Memory Value(void* base)
				{
					return Memory(((uint8_t*)base) + value_offset, value_length);
				}

				bool IsNode() { return value_offset == (T)-1; }

				typename JsonIndexDef<T>::_Index& Node(void* index_root)
				{
					return *((typename JsonIndexDef<T>::_Index*)(((uint8_t*)index_root) + value_length));
				}

			private:
				T key_offset;
				T key_length;
				T value_offset;
				T value_length;
			};
		};

		template <typename T, size_t I, size_t O> class JsonIndexStream : public JsonIndexBackend<T>
		{
			JsonIndexStream() = delete;
			~JsonIndexStream() = delete;
		public:

			using typename JsonIndexBackend<T>::Array;
			using typename JsonIndexBackend<T>::Object;
			using typename JsonIndexBackend<T>::Header;
			using typename JsonIndexDef<T>::_Index;

			static const size_t index_size_t = sizeof(typename JsonIndexStream<T, I, O> ::_Index);

			struct ObjectIndex : public JsonIndexBackend<T>::_Index
			{
				using JsonIndexBackend<T>::_Index::count;

				typename JsonIndexBackend<T>::Object* data() { return Root(); }
				size_t size() { return (size_t)count; }

				typename JsonIndexBackend<T>::Object* begin() { return Root(); }
				typename JsonIndexBackend<T>::Object* end() { return Root() + size(); }

				typename JsonIndexBackend<T>::Object* Find(size_t index)
				{
					if (index >= size()) return nullptr;

					return Root() + index;
				}

				typename JsonIndexBackend<T>::Object* Find(const Memory& name, void* root)
				{
					return FindTSM(Root(), count, name, root);
				}

				typename JsonIndexBackend<T>::Object* Root() { return ((typename JsonIndexBackend<T>::Object*)this) - count; }
			};

			struct ArrayIndex : public JsonIndexBackend<T>::_Index
			{
				using JsonIndexBackend<T>::_Index::count;

				typename JsonIndexBackend<T>::Array* data() { return Root(); }
				size_t size() { return (size_t)count; }

				typename JsonIndexBackend<T>::Array* Find(size_t index) { if (index >= size()) return nullptr; return Root() + index; }

				typename JsonIndexBackend<T>::Array* begin() { return Root(); }
				typename JsonIndexBackend<T>::Array* end() { return Root() + size(); }

			private:
				typename JsonIndexBackend<T>::Array* Root() { return ((typename JsonIndexBackend<T>::Array*)this) - count; }
			};

			typename JsonIndexBackend<T>::_Index& RootAmbiguous() { return *(typename JsonIndexBackend<T>::_Index*)(((uint8_t*)this) + header.start); }
			ObjectIndex& Root() { return *(ObjectIndex*)(((uint8_t*)this) + header.start); }

			typename JsonIndexBackend<T>::Object* Find(const Memory& name, void* root) { return Root().Find(name, root); }

			typename JsonIndexBackend<T>::Object& operator()(const Memory& name, void* root) { return Root()(name, root); }

			template < typename J, typename U>  static JsonIndexStream* Stream(J& input, U& output)
			{
				uint8_t* input_root = (uint8_t*)input.data();
				uint8_t* output_root = (uint8_t*)output.data();
				size_t output_size = (size_t)output.size();

				auto* header = output. template AllocateT< typename JsonIndexBackend<T>::Header>();

				typename JsonIndexBackend<T>::Object stack[I];

				size_t scan = 0;

				while (scan < input.size() && (input[scan] == '\n' || input[scan] == '\r' || input[scan] == '\t' || input[scan] == ' ')) scan++;

				if (scan && input[scan] == '{')
					input = input.Slice(scan);


				size_t root_object_offset = IndexObject(input_root, output_root, input, output, stack, I);
				size_t size = output_size - output.size();

				if (size > O) throw std::runtime_error("Json Stream Overflowed Static Memory");

				header->size = (T)size;
				header->start = (T)root_object_offset;

				return (JsonIndexStream*)header;
			}

			template < typename J, typename U> static JsonIndexStream* Index(const J& input, U& o) { Memory i(input); return Stream(i, o); }

			uint8_t* data() { return (uint8_t*)this; }
			size_t size() { return header.size; }

			typename JsonIndexBackend<T>::Header& GetHeader() { return header; }

		private:

			static size_t IndexObject(uint8_t* input_root, uint8_t* output_root, Memory& input, Memory& output, typename JsonIndexBackend<T>::Object* temp_headers, size_t headers_available)
			{
				size_t result = -1;
				auto start_offset = input.data();
				JsonStream::StreamObject(input, [&](JsonStream::Types type, JsonStream::StreamContext& context) {

					if (context.current_key == headers_available)
						throw "Json Parser, Object Key Limit.";

					switch (type)
					{
					case JsonStream::Types::TypeUndetermined:
					case JsonStream::Types::TypeString:
						temp_headers[context.current_key] = typename JsonIndexBackend<T>::Object(context, input_root);
						break;
					case JsonStream::Types::TypeArray:
						IndexArray(input_root, output_root, input, output, (typename JsonIndexBackend<T>::Array*)(temp_headers + context.current_key + 1), headers_available - (context.current_key + 1));
						temp_headers[context.current_key] = typename JsonIndexBackend<T>::Object((T)((context.key == (char*)-2) ? -2 : (context.key - (char*)input_root)), (T)context.key_length, (T)-1, (T)((output.data() - output_root) - sizeof(typename JsonIndexBackend<T>::_Index)));
						break;
					case JsonStream::Types::TypeObject:
						IndexObject(input_root, output_root, input, output, temp_headers + context.current_key + 1, headers_available - (context.current_key + 1));
						temp_headers[context.current_key] = typename JsonIndexBackend<T>::Object((T)((context.key == (char*)-2) ? -2 : (context.key - (char*)input_root)), (T)context.key_length, (T)-1, (T)((output.data() - output_root) - sizeof(typename JsonIndexBackend<T>::_Index)));
						break;
					case JsonStream::Types::TypeParseFinished:
						auto binary_search = output.Allocate(context.current_key * sizeof(typename JsonIndexBackend<T>::Object));

						for (size_t i = 0; i < context.current_key; i++)
							InsertTSM((typename JsonIndexBackend<T>::Object*)binary_search, i, temp_headers[i], true, nullptr, input_root);

						auto index_start = (uint8_t*)output.template AllocateT<typename JsonIndexBackend<T>::_Index>(JsonStream::Types::TypeObject, (T)context.current_key, (T)(start_offset - input_root), (T)(input.data() - start_offset));

						result = (size_t)(index_start - output_root);
					}
				});

				return result;
			}

			static void IndexArray(uint8_t* input_root, uint8_t* output_root, Memory& input, Memory& output, typename JsonIndexBackend<T>::Array* temp_headers, size_t headers_available)
			{
				auto start_offset = input.data();
				JsonStream::StreamArray(input, [&](JsonStream::Types type, JsonStream::StreamContext& context) {

					if (context.current_key == headers_available)
						throw "Json Parser, Array Key Limit.";

					switch (type)
					{
					case JsonStream::Types::TypeUndetermined:
					case JsonStream::Types::TypeString:
						temp_headers[context.current_key] = typename JsonIndexBackend<T>::Array(context, input_root);
						break;
					case JsonStream::Types::TypeArray:
						IndexArray(input_root, output_root, input, output, temp_headers + (context.current_key + 1), headers_available - (context.current_key + 1));

						temp_headers[context.current_key] = typename JsonIndexBackend<T>::Array((T)-1, (T)((output.data() - output_root) - index_size_t));

						break;
					case JsonStream::Types::TypeObject:
						IndexObject(input_root, output_root, input, output, ((typename JsonIndexBackend<T>::Object*)temp_headers) + (context.current_key + 1), headers_available - (context.current_key + 1));

						temp_headers[context.current_key] = typename JsonIndexBackend<T>::Array((T)-1, (T)((output.data() - output_root) - sizeof(typename JsonIndexBackend<T>::_Index)));

						break;
					case JsonStream::Types::TypeParseFinished:
						output.Insert(temp_headers, context.current_key * sizeof(typename JsonIndexBackend<T>::Array));

						output.template AllocateT<typename JsonIndexBackend<T>::_Index>(JsonStream::Types::TypeArray, (T)context.current_key, (T)(start_offset - input_root), (T)(input.data() - start_offset));

						break;
					}
				});
			}

			typename JsonIndexBackend<T>::Header header;
		};

		template < typename T >class JsonIndexBase
		{
		public:

			~JsonIndexBase()
			{
			}

			class Getter : public Memory
			{
			public:

				bool Valid() const { return value != nullptr; }

				Getter() : allocator(nullptr), value(nullptr) {}
				Getter(Memory v_, void* u, void* _root, Memory* p, bool read_only)
					: Memory(v_)
					, ro(read_only)
					, root(_root)
					, value((typename T::Array*)u)
					, allocator(p) {}

				~Getter(){}

				void* root;
				bool ro;
				Memory* allocator;
				typename T::Array* value;
			};

			JsonIndexBase() : _json(Memory()), index(nullptr), root(nullptr), allocator(nullptr) {}

			JsonIndexBase(Memory json_, T* index_, Memory* allocator_)
				: _json(json_)
				, index(index_)
				, root(&index_->RootAmbiguous())
				, allocator(allocator_) {}

			JsonIndexBase(Memory json_, T* index_, typename T::_Index* root_, Memory* allocator_)
				: _json(json_)
				, index(index_)
				, root(root_)
				, allocator(allocator_) {}

			Getter operator[](uint32_t i) const
			{
				if (!Valid())return Getter();

				if (isObject())
				{
					typename T::Object* o = Object().Find(i);

					if (!o) return Getter();

					if (o->IsNode())
						return Getter(o->Node(index).String(_json.data()), o->Array(), index, allocator, true);
					else
						return Getter(o->Value(_json.data()), o->Array(), index, allocator, false);
				}
				else
				{
					typename T::Array* pa = Array().Find(i);

					if (!pa)
						return Getter();

					Memory m;
					bool ro = false;
					if (pa->IsNode()) {
						m = pa->Node(index).String(_json.data()); ro = true;
					}
					else
						m = pa->Value(_json.data(), index);

					return Getter(m, pa, index, allocator, ro);
				}
			}

			Getter operator[](const Memory& e) const
			{
				if (!Valid())return Getter();

				typename T::Object* o = Object().Find(e, _json.data());

				if (!o) return Getter();

				Memory m;
				bool ro = false;
				if (o->IsNode()) {
					m = o->Node(index).String(_json.data()); ro = true;
				}
				else
					m = o->Value(_json.data());

				return Getter(m, o->Array(), index, allocator, ro);
			}

			JsonIndexBase operator()(const Memory& e) const
			{
				if (!Valid())return JsonIndexBase();

				typename T::Object* o = Object().Find(e, _json.data());
				if (!o)return JsonIndexBase();

				if (o->IsNode()) return JsonIndexBase(_json, index, &o->Node(index), allocator);

				return JsonIndexBase();
			}

			JsonIndexBase operator()(const Memory& e, bool & isValue) const
			{
				isValue = false;

				if (!Valid())return JsonIndexBase();

				typename T::Object* o = Object().Find(e, _json.data());
				if (!o)return JsonIndexBase();

				isValue = !o->IsNode();
				if (!isValue) return JsonIndexBase(_json, index, &o->Node(index), allocator);

				return JsonIndexBase();
			}

			JsonIndexBase operator()(uint32_t i) const
			{
				if (!Valid())return JsonIndexBase();

				if (isObject())
				{
					typename T::Object* pa = Object().Find(i);

					if (!pa)
						return JsonIndexBase();

					if (pa->IsNode()) return JsonIndexBase(_json, index, &pa->Node(index), allocator);

					return JsonIndexBase();
				}
				else
				{
					typename T::Array* pa = Array().Find(i);

					if (!pa)
						return JsonIndexBase();

					if (pa->IsNode()) return JsonIndexBase(_json, index, &pa->Node(index), allocator);

					return JsonIndexBase();
				}
			}

			Memory Find(const Memory& e) const
			{
				if (!Valid())return Memory();

				typename T::Object* o = Object().Find(e, _json.data());

				if (!o) return Memory();

				if (o->IsNode())
					return o->Node(index).String(_json.data());
				else
					return o->Value(_json.data());
			}

			template < typename ... M > Memory Find(const Memory& e, M ... m) const
			{
				if (!Valid())return Memory();

				typename T::Object* o = Object().Find(e, _json.data());

				if (!o)
					return Find(m...);

				if (o->IsNode())
					return o->Node(index).String(_json.data());
				else
					return o->Value(_json.data(), index);
			}

			JsonIndexBase FindObject(const Memory& e)
			{
				if (!Valid())return JsonIndexBase();

				typename T::Object* o = Object().Find(e, _json.data());

				if (!o) return JsonIndexBase();

				if (o->IsNode()) return JsonIndexBase(_json, index, &o->Node(index), allocator);
				else return JsonIndexBase();
			}

			template < typename ... M > JsonIndexBase FindObject(const Memory& e, M ... m)
			{
				if (!Valid())return JsonIndexBase();

				typename T::Object* o = Object().Find(e, _json.data());

				if (!o)
					return FindObject(m...);

				if (!o) return JsonIndexBase();

				if (o->IsNode()) return JsonIndexBase(_json, index, &o->Node(index), allocator);
				else return JsonIndexBase();
			}

			uint32_t ElementCount() const
			{
				if (!root) return 0;
				return root->count;
			}

			bool isValidIndex(uint32_t i)
			{
				if (!root) return false;
				return i < root->count;
			}

			template < typename F > void ForEach(F f) const
			{
				if (!Valid()) return;

				if (isObject())
				{
					auto& object = Object();

					for (uint32_t i = 0; i < object.count; i++)
					{
						auto* e = object.Find(i);
						if (!e) break;

						if (e->IsNode())
							f(e->Key(_json.data()), e->Value(_json.data()), i, JsonIndexBase(_json, index, &e->Node(index), allocator));
						else
							f(e->Key(_json.data()), e->Value(_json.data()), i, JsonIndexBase());
					}
				}
				else
				{
					auto& arr = Array();

					for (uint32_t i = 0; i < arr.count; i++)
					{
						auto* e = arr.Find(i);
						if (!e) break;

						if (e->IsNode())
							f(Memory(""), e->Value(_json.data()), i, JsonIndexBase(_json, index, &e->Node(index), allocator));
						else
							f(Memory(""), e->Value(_json.data()), i, JsonIndexBase());
					}
				}
			}

			template < typename F1, typename F2 > void ForEach(F1 f1, F2 f2) const
			{
				if (!Valid()) return;

				if (isObject())
				{
					auto& object = Object();

					for (uint32_t i = 0; i < object.count; i++)
					{
						auto* e = object.Find(i);
						if (!e) break;

						if (e->IsNode())
							f2(e->Key(_json.data()), JsonIndexBase(_json, index, &e->Node(index), allocator), i);
						else
							f1(e->Key(_json.data()), e->Value(_json.data()), i);
					}
				}
				else
				{
					auto& arr = Array();

					for (uint32_t i = 0; i < arr.count; i++)
					{
						auto* e = arr.Find(i);
						if (!e) break;

						if (e->IsNode())
							f2(Memory(""), JsonIndexBase(_json, index, &e->Node(index), allocator), i);
						else
							f1(Memory(""), e->Value(_json.data()), i);
					}
				}
			}

			template < typename F> void ForEachValue(F f) const
			{
				if (!Valid()) return;

				if (isObject())
				{
					auto& object = Object();

					for (uint32_t i = 0; i < object.count; i++)
					{
						auto* e = object.Find(i);
						if (!e) break;

						if (!e->IsNode())
							f(e->Key(_json.data()), e->Value(_json.data()));
					}
				}
				else
				{
					auto& arr = Array();

					for (uint32_t i = 0; i < arr.count; i++)
					{
						auto* e = arr.Find(i);
						if (!e) break;

						if (!e->IsNode())
							f(Memory(""), e->Value(_json.data()));
					}
				}
			}

			template < typename F> void ForEachObject(F f) const
			{
				if (!Valid()) return;

				if (isObject())
				{
					auto& object = Object();

					for (uint32_t i = 0; i < object.count; i++)
					{
						auto* e = object.Find(i);
						if (!e) break;

						if (e->IsNode())
							f(e->Key(_json.data()), JsonIndexBase(_json, index, &e->Node(index), allocator));
					}
				}
				else
				{
					auto& arr = Array();

					for (uint32_t i = 0; i < arr.count; i++)
					{
						auto* e = arr.Find(i);
						if (!e) break;

						if (e->IsNode())
							f(Memory(""), JsonIndexBase(_json, index, &e->Node(index), allocator));
					}
				}
			}

			explicit operator bool() const { return Valid(); }

			explicit operator Memory() { if (!Valid()) return Memory(); return Memory(data(), size()); } const

			uint8_t* data() const { if (!Valid()) return nullptr; return root->String(_json.data()).data(); }
			uint32_t size() const { if (!Valid()) return 0; return (uint32_t)root->String(_json.data()).size(); }
			uint32_t count() const { if (!Valid()) return 0; return (uint32_t)Object().count; }
			std::string key(const Memory& e) const
			{
				if (!Valid())return "";

				auto* o = Object().Find(e);
				if (!o)return "";

				return o->Key(_json.data());
			}

			std::string key(uint32_t e) const
			{
				if (!Valid())return "";

				auto* o = Object().Find(e);
				if (!o)return "";

				return o->Key(_json.data());
			}

			bool isObject() const { return root->type == JsonStream::Types::TypeObject; }
			bool isArray() const { return root->type == JsonStream::Types::TypeArray; }

			bool Valid() const { return _json.size() > 0; }

			std::string_view Json() const { if (!Valid()) return std::string_view(); return root->StringView(_json.data()); }

			auto SplitAt(const Memory& e) const
			{ 
				auto seg = Find(e);

				auto full = root->String(_json.data());

				auto start_offset = seg.data() - full.data();
				auto end_offset = start_offset + seg.size();
				
				return std::make_pair(std::string_view((char*)full.data(),start_offset), std::string_view((char*)(full.data()+end_offset),full.size()-end_offset));
			}

			template< typename R > std::string Merge(const R& right)
			{
				std::string result;
				result.reserve(size() + right.size());

				_Merge(right, result);

				return result;
			}

			/*template< typename R, typename T > std::string Pool(const R& right, const T& base, std::vector<T>& pool)
			{
				std::string result;
				result.reserve(size() + right.size());

				_Pool(right, result, base, pool);

				return result;
			}

			template< typename R, typename T > std::string Depool(const R& right, const T& base, std::vector<T>& pool)
			{
				std::string result;
				result.reserve(size() + right.size());

				_Depool(right, result, base, pool);

				return result;
			}*/

			template< typename R > std::string Demerge(const R& right)
			{
				std::string result;
				result.reserve(size() + right.size());

				_Demerge(right, result);

				return result;
			}

			template< typename R > std::string Mask(const R& right)
			{
				std::string result;
				result.reserve(size() + right.size());

				_Mask(right, result);

				return result;
			}

			template< typename R > auto Compare(const R& right) const
			{
				auto result = std::make_pair(unsigned(0), unsigned(0));

				_Compare(right, result);

				return result;
			}

			template< typename R > std::string Accumulate(const R& right) const
			{
				if (!Valid() && !right.Valid())
					return "{}";
				else if (!Valid())
					return std::string(right.Json());

				std::string result;
				result.reserve(size() + right.size());

				_Accumulate(right, result);

				return result;
			}

			template< typename R1, typename R2 > std::string Accumulate2(const R1& r1, const R2& r2) const
			{
				if (!Valid() && !r1.Valid())
					return "{}";
				else if (!Valid())
					return r1.Accumulate(r2);

				std::string result;
				result.reserve(size() + r1.size() + r2.size());

				_Accumulate2(r1,r2, result);

				return result;
			}

		protected:

			template< typename R > void _Merge(const R& right, std::string& result)
			{
				result += (isObject()) ? "{" : "[";

				unsigned values = 0;

				auto add_key = [&](auto key) { if (values++) result += ","; result += "\""; result += std::string_view(key); result += "\":"; };
				auto add_value = [&](auto value) { value.TryQuoteWrapper();  result += std::string_view(value); };

				ForEach([&](auto key, auto value, auto index) {
					if (key == "_vmt" || key == "_bmt" || key == "_cmt")
						return;

					add_key(key);
					auto rvalue = right.Find(key);
					add_value((rvalue.size()) ? rvalue : value);
				}, [&](auto key, auto object, auto index) {
					add_key(key);

					auto robject = right(key);

					if (robject.Valid())
						object._Merge(robject, result);
					else
						result += object.Json();
				});

				right.ForEach([&](auto key, auto value, auto index) {
					auto lvalue = Find(key);

					if (!lvalue.size())
					{
						add_key(key);
						add_value(value);
					}
				}, [&](auto key, auto object, auto index) {
					auto lobject = (*this)(key);

					if (!lobject.Valid())
					{
						add_key(key);
						result += object.Json();
					}
				});

				result += (isObject()) ? "}" : "]";
			}

			/*template< typename R, typename T > void _Pool(const R& right, std::string& result, const T& base, std::vector<T>& pool)
			{
				result += (isObject()) ? "{" : "[";

				unsigned values = 0;

				auto add_key = [&](auto key) { if (values++) result += ","; result += "\""; result += key; result += "\":"; };
				auto add_value = [&](auto value) { value.TryQuoteWrapper();  result += value; };

				while (pool.size() < ElementCount())
					pool.push_back(base);

				ForEach([&](auto key, auto value, auto index) {
					add_key(key);
					auto rvalue = right.Find(key);

					if (rvalue.size()) pool[index] = base;
					
					add_value((rvalue.size()) ? rvalue : value);
					}, [&](auto key, auto object, auto index) {
						add_key(key);

						auto robject = right(key);

						if (robject.Valid()) {
							pool[index] = base;
							object._Merge(robject, result);
						}
						else
							result += object.Json();
					});

				right.ForEach([&](auto key, auto value, auto index) {
					auto lvalue = Find(key);

					if (!lvalue.size())
					{
						pool.push_back(base);
						add_key(key);
						add_value(value);
					}
					}, [&](auto key, auto object, auto index) {
						auto lobject = (*this)(key);

						if (!lobject.Valid())
						{
							pool.push_back(base);
							add_key(key);
							result += object.Json();
						}
					});

				result += (isObject()) ? "}" : "]";
			}*/

			template< typename R > void _Demerge(const R& right, std::string& result)
			{
				result += (isObject()) ? "{" : "[";

				unsigned values = 0;

				auto add_key = [&](auto key) { if (values++) result += ","; result += "\""; result += key; result += "\":"; };
				auto add_value = [&](auto value) { value.TryQuoteWrapper();  result += value; };

				ForEach([&](auto key, auto value, auto index) {
					
					if (!right.Find(key).size())
					{
						add_key(key);
						add_value(value);
					}
					
				}, [&](auto key, auto object, auto index) {
					auto robject = right(key);

					if (robject.Valid() && robject.isObject())
					{
						add_key(key);
						object._Demerge(robject, result);
					}
					else if(!robject.Valid())
					{
						add_key(key);
						result += object.Json();
					}				
				});

				result += (isObject()) ? "}" : "]";
			}

			/*template< typename R, typename T  > void _Depool(const R& right, std::string& result, const T& base, std::vector<T>& pool)
			{
				result += (isObject()) ? "{" : "[";

				unsigned values = 0;

				auto add_key = [&](auto key) { if (values++) result += ","; result += "\""; result += key; result += "\":"; };
				auto add_value = [&](auto value) { value.TryQuoteWrapper();  result += value; };

				while (pool.size() < ElementCount())
					pool.push_back(base);

				ForEach([&](auto key, auto value, auto index) {

					if (!right.Find(key).size())
					{
						add_key(key);
						add_value(value);
					}
					else {
						pool[index] = base;
						add_key(key);
						add_value(Memory(""));
					}

					}, [&](auto key, auto object, auto index) {
						auto robject = right(key);

						if (robject.Valid() && robject.isObject())
						{
							pool[index] = base;
							add_key(key);
							object._Demerge(robject, result);
						}
						else if (!robject.Valid())
						{
							add_key(key);
							result += object.Json();
						}
					});

				result += (isObject()) ? "}" : "]";
			}*/

			template< typename R > void _Mask(const R& right, std::string& result)
			{
				result += (isObject()) ? "{" : "[";

				unsigned values = 0;

				auto add_key = [&](auto key) { if (values++) result += ","; result += "\""; result += key; result += "\":"; };
				auto add_value = [&](auto value) { value.TryQuoteWrapper();  result += value; };

				right.ForEach([&](auto key, auto value, auto index) {

					if (Find(key).size())
					{
						add_key(key);
						add_value((*this)[key]);
					}

					}, [&](auto key, auto robject, auto index) {
						auto object = (*this)(key);

						if (object.Valid() && object.isObject())
						{
							add_key(key);
							object._Mask(robject, result);
						}
						else if (object.Valid())
						{
							add_key(key);
							add_value((*this)[key]);
						}
					});

				result += (isObject()) ? "}" : "]";
			}

			template< typename R > void _Compare(const R& right, std::pair<unsigned,unsigned> & result) const
			{
				ForEach([&](auto key, auto value, auto index) 
				{
					auto rvalue = right.Find(key);
					if (rvalue.size() && std::string_view(rvalue) == std::string_view(value))
						result.first++;
					else
						result.second++;
				}, [&](auto key, auto object, auto index) {
					auto robject = right(key);

					if (robject.Valid())
						object._Compare(robject, result);
					else
						result.second++;
				});

				right.ForEach([&](auto key, auto value, auto index) 
				{
					auto lvalue = Find(key);

					if (!lvalue.size())
						result.second++;
				}, [&](auto key, auto object, auto index) {
					auto lobject = (*this)(key);

					if (!lobject.Valid())
						result.second++;
				});
			}

			template< typename R > void _Accumulate(const R& right, std::string & result) const
			{
				result += (isObject()) ? "{" : "[";

				unsigned values = 0;

				auto add_key = [&](auto key) { if (values++) result += ","; result += "\""; result += std::string_view(key); result += "\":"; };
				auto add_value = [&](auto value) { value.TryQuoteWrapper();  result += std::string_view(value); };

				ForEach([&](auto key, auto value, auto index) {
					if (key == "_vmt" || key == "_bmt" || key == "_cmt")
						return;

					add_key(key);
					auto rvalue = right.Find(key);
					if (rvalue.size())
					{
						int64_t r = rvalue, l = value;
						result += std::to_string(l + r);
					}
					else
						add_value(value);
					}, [&](auto key, auto object, auto index) {
						add_key(key);

						auto robject = right(key);

						if (robject.Valid())
							object._Accumulate(robject, result);
						else
							result += object.Json();
					});

				right.ForEach([&](auto key, auto value, auto index) {
					if (key == "_vmt" || key == "_bmt" || key == "_cmt")
						return;

					auto lvalue = Find(key);

					if (!lvalue.size())
					{
						add_key(key);
						add_value(value);
					}
					}, [&](auto key, auto object, auto index) {
						auto lobject = (*this)(key);

						if (!lobject.Valid())
						{
							add_key(key);
							result += object.Json();
						}
					});

				result += (isObject()) ? "}" : "]";
			}

			template< typename R1, typename R2 > void _Accumulate2(const R1& r1, const R2& r2, std::string& result) const
			{
				result += (isObject()) ? "{" : "[";

				unsigned values = 0;

				auto add_key = [&](auto key) { if (values++) result += ","; result += "\""; result += std::string_view(key); result += "\":"; };
				auto add_value = [&](auto value) { value.TryQuoteWrapper();  result += std::string_view(value); };

				ForEach([&](auto key, auto value, auto index) {
					if (key == "_vmt" || key == "_bmt" || key == "_cmt")
						return;

					add_key(key);

					int64_t rv1 = r1[key], rv2 = r2[key], lv = value;
					result += std::to_string(lv + rv1 + rv2);

					}, [&](auto key, auto object, auto index) {
						add_key(key);

						auto ro1 = r1(key);
						auto ro2 = r2(key);

						if (ro1.Valid() || ro2.Valid())
							object._Accumulate2(r1,r2, result);
						else
							result += object.Json();
					});

				r1.ForEach([&](auto key, auto value, auto index) {
					if (key == "_vmt" || key == "_bmt" || key == "_cmt")
						return;

					auto lvalue = Find(key);

					if (!lvalue.size())
					{
						add_key(key);
						add_value(value);
					}
					}, [&](auto key, auto object, auto index) {
						auto lobject = (*this)(key);

						if (!lobject.Valid())
						{
							add_key(key);
							result += object.Json();
						}
					});

				r2.ForEach([&](auto key, auto value, auto index) {
					if (key == "_vmt" || key == "_bmt" || key == "_cmt")
						return;

					auto lvalue = Find(key);

					if (!lvalue.size())
					{
						add_key(key);
						add_value(value);
					}
					}, [&](auto key, auto object, auto index) {
						auto lobject = (*this)(key);

						if (!lobject.Valid())
						{
							add_key(key);
							result += object.Json();
						}
					});

				result += (isObject()) ? "}" : "]";
			}

			typename T::ObjectIndex& Object() const
			{
				return *((typename T::ObjectIndex*)root);
			}

			typename T::ArrayIndex& Array() const
			{
				return *((typename T::ArrayIndex*)root);
			}

			Memory _json;
			T* index;
			typename T::_Index* root;
			Memory* allocator;
		};

		template <typename T, size_t M> class JsonIndexT : public JsonIndexBase<T>
		{
		public:
			using JsonIndexBase<T>::operator();

			~JsonIndexT() {}

			JsonIndexT() : m(buffer) {}
			template < typename J > JsonIndexT(const J& input) : m(buffer) { Index(input); }

			template < typename U > void Index(const U& _input)
			{
				Memory input(_input);

				m = buffer;
				j = input;
				JsonIndexBase<T>::index = T::Index(input, m);

				if (m.size() == buffer.size() - 4)
				{
					JsonIndexBase<T>::_json = Memory();
					return;
				}

				JsonIndexBase<T>::_json = j;
				JsonIndexBase<T>::root = &JsonIndexBase<T>::index->RootAmbiguous();
				JsonIndexBase<T>::allocator = &m;
			}

		private:
			Memory j;
			Memory m;
			std::array<uint8_t, M * sizeof(typename T::_Index) /*object junction size*/ * 2> buffer;
		};

		constexpr uint32_t SmallJsonObjectMax = 32;
		constexpr uint32_t MediumJsonObjectMax = 256;
		constexpr uint32_t JsonObjectMax = 4096;

		typedef JsonIndexStream<uint8_t, 32, 0xfe> IndexStreamS;
		typedef JsonIndexStream<uint16_t, 256, 0xfffe> IndexStreamM;
		typedef JsonIndexStream<uint32_t, 4096, 0xfffffffe> IndexStream;
		typedef JsonIndexStream<uint64_t, 4096, 0xfffffffe> IndexStreamH;

		typedef JsonIndexT<IndexStreamS, SmallJsonObjectMax> JsonReaderS;
		typedef JsonIndexT<IndexStreamM, MediumJsonObjectMax> JsonReader;
		typedef JsonIndexT<IndexStream, JsonObjectMax> JsonReaderL;
		typedef JsonIndexT<IndexStreamH, JsonObjectMax> JsonReaderH;

		class JsonReaderAny
		{
		public:
			JsonReaderAny() {
				//TODO ANY READER
			}

			~JsonReaderAny() {

			}
		private:
		};

		//typedef JsonReaderAny JsonReader;

		auto WithJson(const auto &json, auto f)
		{
			JsonReader index(json);

			return f(index);
		}

		template < typename F, typename S > auto JsonCompare(const F& first, const S& second)
		{
			return JsonReader(first).Compare(JsonReader(second));
		}

		class JsonScanner
		{
		public:
			template < typename J, typename K, typename A, typename V, typename P> static void Scan(const J& input, K k, A a, V v, P p) { Memory i(input); return Stream(i, k,a,v, p); }

		private:

			template < typename J, typename K, typename A, typename V, typename P>  static void Stream(J& input, K k, A a, V v, P p)
			{
				size_t scan = 0;

				while (scan < input.size() && (input[scan] == '\n' || input[scan] == '\r' || input[scan] == '\t' || input[scan] == ' ')) scan++;

				if (scan && input[scan] == '{')
					input = input.Slice(scan);

				IndexObject(input, k, a, v, p);
			}

			template < typename K, typename A, typename V, typename P> static void IndexObject(Memory& input, K k, A a, V v, P p)
			{
				JsonStream::StreamObject(input, [&](JsonStream::Types type, JsonStream::StreamContext& context)
					{
						switch (type)
						{
						case JsonStream::Types::TypeUndetermined:
						case JsonStream::Types::TypeString:
							v(Memory(context.key, context.key_length), Memory(context.value, context.value_length),false);
							break;
						case JsonStream::Types::TypeArray:
							a(Memory(context.key, context.key_length), false);
							IndexArray(input, k, v, a);
							break;
						case JsonStream::Types::TypeObject:
							k(Memory(context.key, context.key_length), false);
							IndexObject(input, k, v, a);
							break;
						default:
						case JsonStream::Types::TypeParseFinished:
							p(false);
							break;
						}
					});
			}

			template < typename K, typename A, typename V, typename P> static void IndexArray(Memory& input, K k, A a, V v, P p)
			{
				auto start_offset = input.data();
				JsonStream::StreamArray(input, [&](JsonStream::Types type, JsonStream::StreamContext& context) {

					switch (type)
					{
					case JsonStream::Types::TypeUndetermined:
					case JsonStream::Types::TypeString:
						v(Memory(&context.current_key, sizeof(context.current_key)), Memory(context.value, context.value_length),true);
						break;
					case JsonStream::Types::TypeArray:
						a(Memory(&context.current_key, sizeof(context.current_key)), true);
						IndexArray(input, k, v, a);
						break;
					case JsonStream::Types::TypeObject:
						k(Memory(&context.current_key, sizeof(context.current_key)), true);
						IndexObject(input, k, v, a);
						break;
					default:
					case JsonStream::Types::TypeParseFinished:
						p(true);
						break;
					}
					});
			}
		};

		template <typename H = uint64_t, size_t max_depth = 16> class JsonHash
		{
		public:
			JsonHash() { }

			template <typename J> void Add(const J& j)
			{
				size_t depth = 1;
				H stack[max_depth]; stack[0] = 0;

				H result = 0;

				auto push = [&](auto& o, bool _a) {
					if (depth == max_depth)
						throw "Json depth exceeds limit.";

					stack[depth++] = HashK(_a ? std::to_string(*((size_t*)o.data())) : o) ^ (depth ? stack[depth++] : 0);
				};

				JsonScanner::Scan(j, push, push, [&](auto& k, auto & v, bool _a){
					result ^= stack[depth++] = AddKV(_a ? std::to_string(*((size_t*)k.data())) : k,v,stack[depth]);
				}, [&](bool _a) { depth--; });

				return hash ^= result;
			}

			H AddKV(Memory key, Memory value, H parent = 0)
			{
				return hash ^= Rotate(parent % (sizeof(H)*8), HashKV(key, value));
			}

			template <typename J> H Remove(const J& j) { return Add(j); }
			H RemoveKV(Memory key, Memory value) { return AddKV(key, value); }

		private:

			H Rotate(size_t rotation, H target) {
				if (!rotation) return target;

				return std::rotl(target, rotation);
			}

			H HashM(Memory m, H result = 0x41feda847185c128)
			{
				H* p = (H*)m.data();

				size_t c = m.size() / sizeof(H), r = m.size() % sizeof(H);

				for (size_t i = 0; i < c; i++, p++)
					result ^= *p;

				return result ^= *p >> (r * 8);
			}

			H HashV(Memory value) {
				return HashM(value);
			}

			H HashK(Memory key) {
				return HashM(key, 0xef7128eab3018429);
			}

			H HashKV(Memory key, Memory value)
			{
				return HashK(key) ^ HashV(value);
			}

			H hash = 0xfe48ca1209bcd713;
		};


#ifndef D8ULEAN
		template <typename T> class JsonMapT : public T
		{
		public:
			JsonMapT() {}
			JsonMapT(string_view file)
			{
				if (std::filesystem::exists(file))
				{
					Map(file);
					T::Index(mapped_data);
				}
				else
					T::Index(file);
			}

			void Map(string_view file)
			{
				std::error_code error;
				mapped_data.map(file, error);

				if (error) 
					throw std::system_error(error);

				T::Index(mapped_data);
			}

		private:
			mio::mmap_source mapped_data;
		};

		typedef JsonMapT<JsonReaderS> JsonMapS;
		typedef JsonMapT<JsonReader> JsonMap;
		typedef JsonMapT<JsonReaderL> JsonMapL;
#endif
	}


}
