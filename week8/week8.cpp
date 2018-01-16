#include "../vecta/vecta.h"
#include "../vecta-util/vectautil.h"

template <typename N> N area(const vecta::vec2d<N> a, const vecta::vec2d<N> b, const vecta::vec2d<N> c) {
	vecta::polygon<N> triangle = { a, b, c };
	return triangle.directed_area();
}

template <typename N>
std::pair<vecta::vec2d<N>, vecta::vec2d<N>> build_point_pair(const vecta::polygon<N>& polygon, int i, int j) {
	return std::pair<vecta::vec2d<N>, vecta::vec2d<N>>(polygon[i], polygon[j]);
}

template <typename N> std::vector<std::pair<vecta::vec2d<N>, vecta::vec2d<N>>> get_antipodal_pairs(const vecta::polygon<N>& polygon) {
	std::vector<std::pair<vecta::vec2d<N>, vecta::vec2d<N>>> result;

	int last_point = polygon.size() - 1;
	int p = last_point;
	int q = 0;

	while (area(polygon[p], polygon[p + 1], polygon[q + 1]) > area(polygon[p], polygon[p + 1], polygon[q])) {
		q += 1;
	}

	int q0 = q;
	while (polygon[q] != polygon[0]) {
		p += 1;
		result.push_back(build_point_pair(polygon, p, q));
		while (area(polygon[p], polygon[p + 1], polygon[q + 1]) > area(polygon[p], polygon[p + 1], polygon[q])) {
			q += 1;
			if (polygon[p] != polygon[q0] || polygon[q] != polygon[0]) {
				result.push_back(build_point_pair(polygon, p, q));
			}
		}

		if (area(polygon[p], polygon[p + 1], polygon[q + 1]) == area(polygon[p], polygon[p + 1], polygon[q])) {
			if (polygon[p] != polygon[q0] || polygon[q] != polygon[last_point]) {
				result.push_back(build_point_pair(polygon, p, q + 1));
			}
		}
	}

	return result;
}


int main() {
	vecta::polygon<> polygon1 = {
		vecta::vec2d<>(-3, -3),
		vecta::vec2d<>(2, -2),
		vecta::vec2d<>(4, 4),
		vecta::vec2d<>(-2, 2),
	};

	vecta::polygon<> polygon2 = {
		vecta::vec2d<>(-2, 2),
		vecta::vec2d<>(4, 4),
		vecta::vec2d<>(2, -2),
		vecta::vec2d<>(-3, -3),
	};

	vecta::polygon<> polygon3 = {
		vecta::vec2d<>(-3, 0),
		vecta::vec2d<>(-2, -2),
		vecta::vec2d<>(0, -3),
		vecta::vec2d<>(2, -2),
		vecta::vec2d<>(3, 0),
		vecta::vec2d<>(2, 2),
		vecta::vec2d<>(0, 3),
		vecta::vec2d<>(-2, 2),
	};

	vecta::polygon<> polygon4 = {
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(1, 0),
		vecta::vec2d<>(1, 1),
		vecta::vec2d<>(0, 1),
	};

	auto pairs = get_antipodal_pairs(polygon4);

	for (auto x : pairs) {
		std::cout << x.first << ", " << x.second << std::endl;
	}
}