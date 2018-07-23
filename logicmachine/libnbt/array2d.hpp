#ifndef LIBNBT_ARRAY2D_HPP
#define LIBNBT_ARRAY2D_HPP

#include <vector>
#include "vec2.hpp"

template <typename T>
class Array2D {

public:
	using value_type = T;

private:
	size_t n, m;
	std::vector<value_type> m_data;

public:
	Array2D() : n(0), m(0), m_data() { }
	Array2D(size_t n, size_t m, const value_type& v = value_type())
		: n(n), m(m), m_data(n * m, v)
	{ }

	size_t rows() const { return n; }
	size_t cols() const { return m; }

	const value_type& operator()(size_t y, size_t x) const {
		return m_data[y * m + x];
	}
	value_type& operator()(size_t y, size_t x){
		return m_data[y * m + x];
	}

	const value_type& operator()(const Vec2& v) const {
		return (*this)(v.y, v.x);
	}
	value_type& operator()(const Vec2& v){
		return (*this)(v.y, v.x);
	}

};

#endif
