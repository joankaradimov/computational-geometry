#include <assert.h>
#include <initializer_list>
#include <iostream>
#include <math.h>
#include <vector>

#include "../vecta/vecta.h"

enum sign {
	negative = -1,
	zero = 0,
	positive = 1,
};

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
		auto& previous_point = (*this)[point_index - 1];
		auto& point = (*this)[point_index];
		auto& next_point = (*this)[point_index + 1];

		return is_ear(previous_point, point, next_point);
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

	bool is_ear(const vecta::vec2d<N>& previous_point, const vecta::vec2d<N>& point, const vecta::vec2d<N>& next_point) const {
		sign polygon_sign = get_sign(directed_area());

		sign point_sign = get_sign((point - previous_point) ^ (next_point - point));
		assert(point_sign != sign::zero);

		return polygon_sign == point_sign;
	}
};

void test1() {
	polygon<> polygon = {
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(0, 0.1),
		vecta::vec2d<>(0.1, 0.1),
		vecta::vec2d<>(0.1, 0),
	};
	vecta::Number total_area = abs(polygon.directed_area());

	std::cout << total_area << std::endl;
}

template<typename T> sign get_sign(const T& number) {
	if (number > 0) return sign::negative;
	if (number < 0) return sign::positive;
	return sign::zero;
}

template<typename T> bool have_same_sign(const T& a, const T& b) {
	return a != 0 && get_sign(a) == get_sign(b);
}

template<typename T> bool is_inside_triangle(vecta::vec2d<T> a, vecta::vec2d<T> b, vecta::vec2d<T> c, vecta::vec2d<T> p) {
	polygon<> pab = { p, a, b };
	double directed_area_pac = pab.directed_area();

	if (directed_area_pac == 0) {
		return true;
	}

	polygon<> pbc = { p, b, c };
	double directed_area_pbc = pbc.directed_area();
	if (directed_area_pac == 0) {
		return true;
	}
	if (!have_same_sign(directed_area_pac, directed_area_pbc)) {
		return false;
	}

	polygon<> pca = { p, c, a };
	double directed_area_pca = pca.directed_area();
	if (directed_area_pca == 0) {
		return true;
	}
	if (!have_same_sign(directed_area_pac, directed_area_pca)) {
		return false;
	}

	return true;
}

void test2() {
	vecta::vec2d<> a(0, 0);
	vecta::vec2d<> b(0, 4);
	vecta::vec2d<> c(3, 0);
	vecta::vec2d<> p(1, 1);

	std::cout << is_inside_triangle(a, b, c, p) << std::endl;
}

template<typename T> size_t get_next_triangle_point(const polygon<T>& polygon) {
	for (size_t potential_ear_index = 0; potential_ear_index < polygon.size(); ++potential_ear_index) {
		if (!polygon.is_ear(potential_ear_index)) {
			continue;
		}

		auto& ear_previous_point = polygon[potential_ear_index- 1];
		auto& ear_point = polygon[potential_ear_index];
		auto& ear_next_point = polygon[potential_ear_index + 1];

		for (size_t point_index = 0; point_index < polygon.size(); ++point_index) {
			if (is_inside_triangle(ear_previous_point, ear_point, ear_next_point, polygon[point_index])) {
				continue;
			}

			return point_index;
		}
	}
	assert(false);
}

// Triangulation by ear clipping
int main() {
	polygon<> polygon_points = {
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(0, 4),
		vecta::vec2d<>(2, 2),
		vecta::vec2d<>(20, 0),
	};

	// TODO: this implementation is incorrect
	// The algorithm is quadratic (not cubic)
	while (polygon_points.size() > 2) {
		size_t index = get_next_triangle_point(polygon_points);
		std::cout << polygon_points[index] << std::endl;
		polygon_points.erase(polygon_points.begin() + index);
	}
}
