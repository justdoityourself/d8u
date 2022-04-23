/* Copyright (C) 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string>
#include "json.hpp"
#include "json_builder.hpp"
#include "console.hpp"

namespace d8u
{
	class GraphJson 
	{
		std::string latest;
		std::string instant;

		void _Update(const auto & jl, const auto & jp, size_t m = 1)
		{
			JsonPiecemeal js(latest.size());

			js.Open();

			jl.ForEachValue([&](auto k, auto v)
			{
				js[k] = (int64_t)v - (int64_t)jp[k];
			});

			js.Close();

			instant = js.Claim();
		}

	public:
		GraphJson() { }

		void Update(std::string _latest, size_t m = 1)
		{
			auto prev = std::move(latest);
			latest = std::move(_latest);

			json::JsonReader jl(latest);
			json::JsonReader jp(prev);

			_Update(jl, jp, m);
		}

		std::string Instant()
		{
			return instant;
		}

		std::string Total()
		{
			return latest;
		}

		std::string Dashboard()
		{
			std::string result;
			result.reserve((latest.size() + instant.size()) * 2);

			json::JsonReader ji(instant);
			json::JsonReader jt(latest);

			auto match_width = [](auto v1, auto v2, bool bright=false) {
				std::string result = v1;

				int delta = v2.size() - v1.size();

				while (delta-- > 0) result += " ";

				return bright ? bwhite(result) : result;
			};

			ji.ForEachValue([&](auto k, auto v)
			{
				result += std::string_view(k);
				result += ":";
				result += match_width(v, jt[k], (int)v != 0);
				result += '\t';
			});

			result += "\n";

			jt.ForEachValue([&](auto k, auto v)
			{
				auto v2 = ji[k];
				result += std::string_view(k);
				result += ":";
				result += match_width(v,v2, (int)v2 != 0);
				result += '\t';
			});

			result += "\n";

			return result;
		}
	};
}