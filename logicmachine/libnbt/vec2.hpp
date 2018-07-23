#ifndef LIBNBT_VEC2_HPP
#define LIBNBT_VEC2_HPP

#include <utility>

struct Vec2 {
	int x, y;

	Vec2() : x(0), y(0) { }
	Vec2(int x, int y) : x(x), y(y) { }

	bool operator==(const Vec2& v) const noexcept { return x == v.x && y == v.y; }
	bool operator!=(const Vec2& v) const noexcept { return !(*this == v); }

	bool operator< (const Vec2& v) const noexcept { return std::make_pair(x, y) <  std::make_pair(v.x, v.y); }
	bool operator<=(const Vec2& v) const noexcept { return std::make_pair(x, y) <= std::make_pair(v.x, v.y); }
	bool operator> (const Vec2& v) const noexcept { return std::make_pair(x, y) >  std::make_pair(v.x, v.y); }
	bool operator>=(const Vec2& v) const noexcept { return std::make_pair(x, y) >= std::make_pair(v.x, v.y); }

	Vec2 operator-() const noexcept { return Vec2{ -x, -y }; }
	Vec2 operator+(const Vec2& v) const noexcept { return Vec2{ x + v.x, y + v.y }; }
	Vec2 operator-(const Vec2& v) const noexcept { return Vec2{ x - v.x, y - v.y }; }
	Vec2& operator+=(const Vec2& v) noexcept { x += v.x; y += v.y; return *this; }
	Vec2& operator-=(const Vec2& v) noexcept { x -= v.x; y -= v.y; return *this; }

	Vec2 operator*(int s) const noexcept { return Vec2{ x * s, y * s }; }
	Vec2& operator*=(int s) noexcept { x *= s; y *= s; return *this; }

	bool region_check(const Vec2& size) const noexcept {
		return 0 <= x && x < size.x && 0 <= y && y < size.y;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Vec2& v){
	return os << "(" << v.x << ", " << v.y << ")";
}

namespace std {
	template <> struct hash<Vec2> {
		using argument_type = Vec2;
		using result_type = std::size_t;
		result_type operator()(const argument_type& v) const noexcept {
			result_type h = 0;
			h = h * 1234567891u + static_cast<result_type>(v.x);
			h = h * 1234567891u + static_cast<result_type>(v.y);
			return h;
		}
	};
}

#endif
