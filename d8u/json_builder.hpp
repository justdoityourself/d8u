/* Copyright (C) 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string_view>

namespace d8u
{
	class JsonBuilder
	{
		struct ValueHelper
		{
			JsonBuilder& parent;
			std::string_view key;

			void operator= (const auto value)
			{
				constexpr bool is_num = std::is_arithmetic_v<decltype(value)>;

				if (parent.c++)
					parent.data += ",";

				parent.data += "\"";
				parent.data += key;
				parent.data += "\"";
				parent.data += ":";

				if constexpr (is_num) parent.data += std::to_string(value);
				else {
					if (value[0] != '{') parent.data += "\"";
					parent.data += value;
					if (value[0] != '{') parent.data += "\"";
				}
			}

			void operator() (auto f) {
				JsonBuilder obj;
				f(obj);
				this->operator=(obj.Finished());
			}
		};
	public:

		JsonBuilder()
		{
			data = "{";
		}

		ValueHelper operator[] (std::string_view key)
		{
			return { *this,key };
		}

		std::string Finished()
		{
			data += "}";

			return std::move(data);
		}

		std::string data;
		int c = 0;
	};

	std::string BuildJson(auto f)
	{
		JsonBuilder obj;
		f(obj);
		return obj.Finished();
	}

	class JsonPiecemeal
	{
		struct ValueHelper
		{
			JsonPiecemeal& parent;
			std::string_view key;

			void operator= (const auto value)
			{
				parent.KV(key, value);
			}

			void operator() (auto f) {
				JsonBuilder obj;
				f(obj);
				this->operator=(obj.Finished());
			}
		};
	public:
		JsonPiecemeal(size_t target_size)
		{
			data.reserve(target_size);
		}

		void Open() {
			data += "{";
			depth++;

			if (depth >= 12)
				throw "Exceeded Max Json Depth";

			started[depth] = false;
		}

		void Close() {
			data += "}";
			depth--;
		}

		void Key(auto key)
		{
			if (started[depth])
				data += ",";
			else
				started[depth] = true;

			data += "\"";
			data += std::string_view(key);
			data += "\":";
		}

		void Value(auto value)
		{
			constexpr bool has_qr = requires(const decltype(value) & t) { t.TryQuoteWrapper(); };
			constexpr bool is_num = std::is_arithmetic_v<decltype(value)>;

			if constexpr (has_qr) value.TryQuoteWrapper();

			if constexpr (is_num) data += std::to_string(value);
			else data += std::string_view(value);
		}

		void KV(auto key, auto value)
		{
			Key(key);
			Value(value);
		}

		ValueHelper operator[] (std::string_view key)
		{
			return { *this,key };
		}

		void Splice(auto main, auto key, auto value) {
			constexpr bool is_num = std::is_arithmetic_v<decltype(value)>;

			data += std::string_view(main);
			data[data.size() - 1] = ',';
			data += "\"";
			data += std::string_view(key);
			data += "\":";
			if constexpr (is_num) data += std::to_string(value);
			else data += std::string_view(value);
			data += "}";
		}

		void Purge(auto _main, auto _key, auto value)
		{
			//Purge Expects [key] to be order in such a way that it is the trigger to cut data from the end of the merging value

			constexpr bool is_num = std::is_arithmetic_v<decltype(value)>;

			auto main = std::string_view(_main);
			auto key = std::string_view(_key);

			//auto max = main.rfind("}",main.size()-1); // main ends with } skip that one
			//auto cut = main.rfind(key);

			// Sometimes a loop is better. Above commented code works though.
			int cut = -1;
			for (int i = (int)main.size() - 2, j = 0; i >= 0; i--, j++)
			{
				if (main[i] == '}') break;
				if (j >= key.size() && key == std::string_view(main.data() + i, key.size())) {
					cut = i;
					break;
				}
			}

			if (cut == -1)
				return Splice(_main, key, value);

			main = std::string_view(main.data(), cut);

			data += std::string_view(main);
			data += std::string_view(key);
			data += "\":";
			if constexpr (is_num) data += std::to_string(value);
			else data += std::string_view(value);
			data += "}";
		}

		void Sub(auto key, auto value) {
			data += "{\"";
			data += std::string_view(key);
			data += "\":";
			data += std::string_view(value);
			data += "}";
		}

		auto size() { return data.size(); }
		auto Claim() { return std::move(data); }
	private:
		size_t depth = -1;
		bool started[12];

		std::string data;
	};
}
