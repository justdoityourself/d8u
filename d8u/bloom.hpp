#include <array>

namespace d8u
{
	
	template <size_t bytes_c, typename hash_t> class Bloom : public std::array<uint8_t, byte_c>
	{
		constexpr size_t lim_c = byte_c * 8;

		inline void set(size_t n) {
			(*this)[n / 8] |= 1 << n % 8;
		}

		inline bool test(size_t n) {
			return (bool) ((*this)[n / 8] & 1 << n % 8);
		}
	public:

		// Assumes that t is pre-distributed data.
		// t.size() controls hash count.
		// remainder % sizeof(hash_t) is discarded.
		template < typename T> void Insert(const T & t)
		{
			const auto l = t.size() / sizeof(hash_t);
			auto* p = (hash_t*)t.data();

			for (size_t i = 0; i < l; i++, p++)
				set(*p % lim_c);
		}

		template < typename T> bool Test(const T& t)
		{
			const auto l = t.size() / sizeof(hash_t);
			auto* p = (hash_t*)t.data();

			for (size_t i = 0; i < l; i++, p++) {
				if (!test(*p % lim_c))
					return false;
			}

			return true;
		}
	};
}