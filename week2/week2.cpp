#include <assert.h>
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

// S <sub>P1 ... Pn</sub> 

// Compute the area of polygon:
// Sum the directed areas of triangles formed by each pair of consequent points
// and one random point in the plain (we select the point (0, 0))
template<typename T> T directed_area(const std::vector<vecta::vec2d<T>>& polygon_points)
{
	int polygon_point_count = polygon_points.size();
	T result = 0;

	for (int i = 0; i < polygon_point_count; ++i) {
		auto& vector1 = polygon_points[i];
		auto& vector2 = polygon_points[(i + 1) % polygon_point_count];

		result += vector1 ^ vector2;
	}

	return result / 2;
}

void test1() {
	std::vector<vecta::vec2d<>> polygon_points = {
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(0, 0.1),
		vecta::vec2d<>(0.1, 0.1),
		vecta::vec2d<>(0.1, 0),
	};
	vecta::Number total_area = abs(directed_area(polygon_points));

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
	std::vector<vecta::vec2d<>> pab = { p, a, b };
	double directed_area_pac = directed_area(pab);

	if (directed_area_pac == 0) {
		return true;
	}

	std::vector<vecta::vec2d<>> pbc = { p, b, c };
	double directed_area_pbc = directed_area(pbc);
	if (directed_area_pac == 0) {
		return true;
	}
	if (!have_same_sign(directed_area_pac, directed_area_pbc)) {
		return false;
	}

	std::vector<vecta::vec2d<>> pca = { p, c, a };
	double directed_area_pca = directed_area(pca);
	if (directed_area_pca == 0) {
		return true;
	}
	if (!have_same_sign(directed_area_pac, directed_area_pca)) {
		return false;
	}

	return true;
}

template<typename T> bool is_ear(const std::vector<vecta::vec2d<T>>& polygon, size_t point_index) {
	int polygon_size = polygon.size();

	auto& previous_point = polygon[(point_index + polygon_size - 1) % polygon_size];
	auto& point = polygon[point_index];
	auto& next_point = polygon[(point_index + 1) % polygon_size];

	return is_ear(get_sign(directed_area(polygon)), previous_point, point, next_point);
}

template<typename T> bool is_ear(sign polygon_sign, const vecta::vec2d<T>& previous_point, const vecta::vec2d<T>& point, const vecta::vec2d<T>& next_point) {
	sign point_sign = get_sign((point - previous_point) ^ (next_point - point));

	assert(point_sign != sign::zero);

	return polygon_sign == point_sign;
}

void test2() {
	vecta::vec2d<> a(0, 0);
	vecta::vec2d<> b(0, 4);
	vecta::vec2d<> c(3, 0);
	vecta::vec2d<> p(1, 1);

	std::cout << is_inside_triangle(a, b, c, p) << std::endl;
}

template<typename T> size_t get_next_triangle_point(const std::vector<vecta::vec2d<T>>& polygon_points) {
	for (size_t potential_ear_index = 0; potential_ear_index < polygon_points.size(); ++potential_ear_index) {
		if (!is_ear(polygon_points, potential_ear_index)) {
			continue;
		}

		auto& ear_previous_point = polygon_points[(potential_ear_index + polygon_points.size() - 1) % polygon_points.size()];
		auto& ear_point = polygon_points[potential_ear_index];
		auto& ear_next_point = polygon_points[(potential_ear_index + 1) % polygon_points.size()];

		for (size_t point_index = 0; point_index < polygon_points.size(); ++point_index) {
			if (is_inside_triangle(ear_previous_point, ear_point, ear_next_point, polygon_points[point_index])) {
				continue;
			}

			return point_index;
		}
	}
	assert(false);
}

// Triangulation by ear clipping
int main() {
	std::vector<vecta::vec2d<>> polygon_points = {
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
