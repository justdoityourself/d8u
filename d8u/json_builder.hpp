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
}

