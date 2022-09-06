#include <fstream>
#include <filesystem>
#include <sstream>

namespace file
{
	inline void string_as(std::string_view file, std::string_view data)
	{
		std::filesystem::remove(file);

		std::ofstream f(file.data());
		f << data;
	}

	inline std::string as_string(std::string_view file)
	{
		std::ifstream f(file.data());

		if (std::ostringstream ss; f)
		{
			ss << f.rdbuf();

			return ss.str();
		}

		return "";
	}

	inline void empty(std::string_view file)
	{
		std::ofstream fs(file.data(), std::ios::out);
	}

	template <typename T> uint64_t append(std::string_view file, const T& m)
	{
		std::ofstream fs(file.data(), std::ios::out | std::fstream::app | std::ios::binary);
		uint64_t result = fs.tellp();
		fs.write((const char*)&m.length, sizeof(uint32_t));
		fs.write((const char*)m.data(), m.size());

		return result;
	}

	inline void of_size(std::string_view file, size_t size)
	{
		std::ofstream ofs(file.data(), std::ios::binary | std::ios::out);
		ofs.seekp(size);
		ofs.write("", 1);
	}
}