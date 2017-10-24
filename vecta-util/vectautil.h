#pragma once

#include <assert.h>
#include <initializer_list>
#include <iostream>
#include <math.h>
#include <vector>

namespace vecta {

enum sign {
	negative = -1,
	zero = 0,
	positive = 1,
};

template<typename T> sign get_sign(const T& number) {
	if (number > 0) return sign::negative;
	if (number < 0) return sign::positive;
	return sign::zero;
}

template<typename T> bool have_same_sign(const T& a, const T& b) {
	return a != 0 && get_sign(a) == get_sign(b);
}

template <typename N>
std::ostream& operator<< (std::ostream& out, const vecta::vec2d<N>& v) {
	return out << '(' << v.x << ", " << v.y << ')';
}

template <typename N = vecta::Number>
class polygon : public std::vector<vecta::vec2d<N>> {
public:
	polygon(std::initializer_list<vecta::vec2d<N>> points) : std::vector<vecta::vec2d<N>>(points) {
	}

	const vecta::vec2d<N>& operator[] (int index) const {
		return this->std::vector<vecta::vec2d<N>>::operator[](index_modulo_size(index));
	}

	vecta::vec2d<N>& operator[] (int index) {
		return this->std::vector<vecta::vec2d<N>>::operator[](index_modulo_size(index));
	}

	bool is_ear(size_t point_index) const {
		sign polygon_sign = get_sign(directed_area());
		sign point_sign = get_point_sign(point_index);

		assert(point_sign != sign::zero);

		return polygon_sign == point_sign;
	}

	// S <sub>P1 ... Pn</sub>

	// Compute the area of polygon:
	// Sum the directed areas of triangles formed by each pair of consequent points
	// and one random point in the plain (we select the point (0, 0))
	N directed_area() const {
		N result = 0;

		for (int i = 0; i < size(); ++i) {
			auto& vector1 = (*this)[i];
			auto& vector2 = (*this)[i + 1];

			result += vector1 ^ vector2;
		}

		return result / 2;
	}
private:
	int index_modulo_size(int index) const {
		// TODO: use a faster method for calculation
		int n = size();
		return (index % n + n) % n;
	}

	sign get_point_sign(int point_index) const {
		auto& previous_point = (*this)[point_index - 1];
		auto& point = (*this)[point_index];
		auto& next_point = (*this)[point_index + 1];

		return get_sign((point - previous_point) ^ (next_point - point));
	}
};

}
