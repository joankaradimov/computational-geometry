#include <algorithm>
#include <cassert>
#include <vector>
#include "../vecta/vecta.h"

template <typename ForwardIterator>
auto get_initial_point(ForwardIterator first, ForwardIterator last) {
	return *std::min_element(first, last, [](auto& a, auto& b) { return (a.y == b.y) ? a.x < b.x : a.y < b.y; });
}

template <typename RandomIterator, typename N>
std::vector<vecta::vec2d<N>> sorted_points(RandomIterator first, RandomIterator last, vecta::vec2d<N> initial_point) {
	std::vector<vecta::vec2d<N>> points(std::distance(first, last));

	std::partial_sort_copy(first, last, points.begin(), points.end(), [=](auto& a, auto& b) {
		vecta::vec2d<N> a_vector = a - initial_point;
		vecta::vec2d<N> b_vector = b - initial_point;
		return (a_vector || b_vector) ? vecta::len(a_vector) < vecta::len(b_vector) : a_vector < b_vector;
	});

	return points;
}

template <typename RandomIterator>
auto graham_scan_convex_hull(RandomIterator first, RandomIterator last) {
	auto inital_point = get_initial_point(first, last);
	auto points = sorted_points(first, last, inital_point);

	auto first_point = points.begin();
	auto it = std::unique(first_point, points.end());
	points.resize(std::distance(first_point, it));

	decltype(points) convex_hull;

	convex_hull.push_back(*(first_point++));
	convex_hull.push_back(*(first_point++));

	while (convex_hull[0].y == first_point->y) {
		convex_hull.back() = *(first_point++);
	}

	for (auto current_point = first_point; current_point != points.end(); ++current_point) {
		while (*current_point - convex_hull.back() <= convex_hull.back() - convex_hull.at(convex_hull.size() - 2)) {
			convex_hull.pop_back();
		}
		convex_hull.push_back(*current_point);
	}

	return convex_hull;
}

template <typename ForwardIterator, typename N>
vecta::vec2d<N> next_hull_point(ForwardIterator first, ForwardIterator last, vecta::vec2d<N> initial_point) {
	return *std::max_element(first, last, [=](auto& a, auto& b) {
		vecta::vec2d<N> a_vector = a - initial_point;
		vecta::vec2d<N> b_vector = b - initial_point;
		return (a_vector || b_vector) ? vecta::len(a_vector) < vecta::len(b_vector) : a_vector > b_vector;
	});
}

template <typename ForwardIterator> auto jarvis_march_convex_hull(ForwardIterator first, ForwardIterator last) {
	auto initial = get_initial_point(first, last);
	std::vector<decltype(initial)> convex_hull = { initial };

	for (auto current = next_hull_point(first, last, initial); current != initial; current = next_hull_point(first, last, current)) {
		convex_hull.push_back(current);
	}

	return convex_hull;
}

void test_initial_point() {
	std::vector<vecta::vec2d<>> points;
	vecta::vec2d<> initial_point;

	points = { vecta::vec2d<>(1, 2), vecta::vec2d<>(3, 0) };
	initial_point = get_initial_point(points.begin(), points.end());
	assert(initial_point == vecta::vec2d<>(3, 0)); // pick the bottommost point

	points = { vecta::vec2d<>(3, 0), vecta::vec2d<>(1, 0) };
	initial_point = get_initial_point(points.begin(), points.end());
	assert(initial_point == vecta::vec2d<>(1, 0)); // if there are two bottommost points - pick the leftmost one

	points = { vecta::vec2d<>(1, 0), vecta::vec2d<>(3, 0) };
	initial_point = get_initial_point(points.begin(), points.end());
	assert(initial_point == vecta::vec2d<>(1, 0)); // if there are two bottommost points - pick the leftmost one
}

void test_sort_points() {
	std::vector<vecta::vec2d<>> points;
	vecta::vec2d<> initial_point;

	// horizontal points - all above the initial point
	points = { vecta::vec2d<>(-1, 1), vecta::vec2d<>(1, 1), vecta::vec2d<>(0, 1) };
	points = sorted_points(points.begin(), points.end(), vecta::vec2d<>(0, 0));
	assert(points[0] == vecta::vec2d<>(1, 1));
	assert(points[1] == vecta::vec2d<>(0, 1));
	assert(points[2] == vecta::vec2d<>(-1, 1));

	// TODO: vertical - on the left and on the right

	// points around the initial and at equal distances
	points = { vecta::vec2d<>(0, 1), vecta::vec2d<>(1, 0), vecta::vec2d<>(-1, 0) };
	points = sorted_points(points.begin(), points.end(), vecta::vec2d<>(0, 0));
	assert(points[0] == vecta::vec2d<>(1, 0));
	assert(points[1] == vecta::vec2d<>(0, 1));
	assert(points[2] == vecta::vec2d<>(-1, 0));

	// varying distances with different precedence
	points = { vecta::vec2d<>(0, 5), vecta::vec2d<>(-6, 1), vecta::vec2d<>(30, 0), vecta::vec2d<>(2, 3) };
	points = sorted_points(points.begin(), points.end(), vecta::vec2d<>(0, 0));
	assert(points[0] == vecta::vec2d<>(30, 0));
	assert(points[1] == vecta::vec2d<>(2, 3));
	assert(points[2] == vecta::vec2d<>(0, 5));
	assert(points[3] == vecta::vec2d<>(-6, 1));

	// varying distances with the same precedence
	points = { vecta::vec2d<>(3, 0), vecta::vec2d<>(0, 0), vecta::vec2d<>(10, 0), vecta::vec2d<>(1, 0) };
	points = sorted_points(points.begin(), points.end(), vecta::vec2d<>(0, 0));
	assert(points[0] == vecta::vec2d<>(0, 0));
	assert(points[1] == vecta::vec2d<>(1, 0));
	assert(points[2] == vecta::vec2d<>(3, 0));
	assert(points[3] == vecta::vec2d<>(10, 0));

	// varying distances with the same precedence
	points = {
		vecta::vec2d<>(0, 2),
		vecta::vec2d<>(0, 1),
		vecta::vec2d<>(-3, 1),
		vecta::vec2d<>(2, 1),
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(1, 0),
	};
	points = sorted_points(points.begin(), points.end(), vecta::vec2d<>(0, 0));
	assert(points[0] == vecta::vec2d<>(0, 0));
	assert(points[1] == vecta::vec2d<>(1, 0));
	assert(points[2] == vecta::vec2d<>(2, 1));
	assert(points[3] == vecta::vec2d<>(0, 1));
	assert(points[4] == vecta::vec2d<>(0, 2));
	assert(points[5] == vecta::vec2d<>(-3, 1));
}

void test_graham_scan_convex_hull() {
	std::vector<vecta::vec2d<>> points;

	// convex hull for a list of points with duplication
	points = {
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(0, 1),
		vecta::vec2d<>(1, 1),
		vecta::vec2d<>(1, 0),
		vecta::vec2d<>(0, 1),
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(1, 0),
		vecta::vec2d<>(1, 1),
	};
	std::vector<vecta::vec2d<>> hull1 = graham_scan_convex_hull(points.begin(), points.end());
	assert(hull1[0] == vecta::vec2d<>(0, 0));
	assert(hull1[1] == vecta::vec2d<>(1, 0));
	assert(hull1[2] == vecta::vec2d<>(1, 1));
	assert(hull1[3] == vecta::vec2d<>(0, 1));

	// list of points where several are on the same line
	points = {
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(0, 10),
		vecta::vec2d<>(5, 5),
		vecta::vec2d<>(10, 0),
	};
	std::vector<vecta::vec2d<>> hull2 = graham_scan_convex_hull(points.begin(), points.end());
	assert(hull2[0] == vecta::vec2d<>(0, 0));
	assert(hull2[1] == vecta::vec2d<>(10, 0));
	assert(hull2[2] == vecta::vec2d<>(0, 10));

	// list of points where the first few ones are on the same line
	points = {
		vecta::vec2d<>(0, 0),
		vecta::vec2d<>(10, 0),
		vecta::vec2d<>(4, 0),
		vecta::vec2d<>(3, 5),
		vecta::vec2d<>(-1, 0),
	};
	std::vector<vecta::vec2d<>> hull3 = graham_scan_convex_hull(points.begin(), points.end());
	assert(hull3[0] == vecta::vec2d<>(-1, 0));
	assert(hull3[1] == vecta::vec2d<>(10, 0));
	assert(hull3[2] == vecta::vec2d<>(3, 5));

	// TODO: test with multiple points on the same line for the last side of the hull

	points = {
		vecta::vec2d<>(10, 5),
		vecta::vec2d<>(5, 0),
		vecta::vec2d<>(0, 5),
		vecta::vec2d<>(5, 10),
		vecta::vec2d<>(2, 2),
		vecta::vec2d<>(3, 3),
		vecta::vec2d<>(5, 5),
	};
	std::vector<vecta::vec2d<>> hull5 = graham_scan_convex_hull(points.begin(), points.end());
	assert(hull5[0] == vecta::vec2d<>(5, 0));
	assert(hull5[1] == vecta::vec2d<>(10, 5));
	assert(hull5[2] == vecta::vec2d<>(5, 10));
	assert(hull5[3] == vecta::vec2d<>(0, 5));
	assert(hull5[4] == vecta::vec2d<>(2, 2));
}

void test_next_hull_point() {
	// TODO: implement tests
}

void test_jarvis_march_convex_hull() {
	// TODO: implement tests
}

int main()
{
	test_initial_point();
	test_sort_points();
	test_graham_scan_convex_hull();
	test_next_hull_point();
	test_jarvis_march_convex_hull();

	return 0;
}
