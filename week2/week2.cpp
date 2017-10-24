#include "../vecta/vecta.h"
#include "../vecta-util/vectautil.h"

void test1() {
	vecta::polygon<> polygon = {
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(0, 0.1),
		vecta::vec2d<>(0.1, 0.1),
		vecta::vec2d<>(0.1, 0),
	};
	vecta::Number total_area = abs(polygon.directed_area());

	std::cout << total_area << std::endl;
}

template<typename T> bool is_inside_triangle(vecta::vec2d<T> a, vecta::vec2d<T> b, vecta::vec2d<T> c, vecta::vec2d<T> p) {
	vecta::polygon<> pab = { p, a, b };
	double directed_area_pac = pab.directed_area();

	if (directed_area_pac == 0) {
		return true;
	}

	vecta::polygon<> pbc = { p, b, c };
	double directed_area_pbc = pbc.directed_area();
	if (directed_area_pac == 0) {
		return true;
	}
	if (!vecta::have_same_sign(directed_area_pac, directed_area_pbc)) {
		return false;
	}

	vecta::polygon<> pca = { p, c, a };
	double directed_area_pca = pca.directed_area();
	if (directed_area_pca == 0) {
		return true;
	}
	if (!vecta::have_same_sign(directed_area_pac, directed_area_pca)) {
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

template<typename T> size_t get_next_triangle_point(const vecta::polygon<T>& polygon) {
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
	vecta::polygon<> polygon_points = {
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
