#ifndef LIBNBT_ARRAY3D_HPP
#define LIBNBT_ARRAY3D_HPP

#include <vector>

template <typename T>
class Array3D {

public:
	using value_type = T;

private:
	size_t n, m, k;
	std::vector<value_type> m_data;

public:
	Array3D() : n(0), m(0), k(0), m_data() { }
	Array3D(size_t n, size_t m, size_t k, const value_type& v = value_type())
		: n(n), m(m), k(k), m_data(n * m * k, v)
	{ }

	size_t depth() const { return n; }
	size_t rows() const { return m; }
	size_t cols() const { return k; }

	const value_type& operator()(size_t z, size_t y, size_t x) const {
		return m_data[z * m * k + y * k + x];
	}
	value_type& operator()(size_t z, size_t y, size_t x){
		return m_data[z * m * k + y * k + x];
	}

	const value_type& operator()(const Vec3& v) const {
		return (*this)(v.z, v.y, v.x);
	}
	value_type& operator()(const Vec3& v){
		return (*this)(v.z, v.y, v.x);
	}

};

#endif
