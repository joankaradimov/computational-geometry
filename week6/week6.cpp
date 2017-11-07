#include <memory>
#include <vector>

#include "../vecta/vecta.h"

// TODO: implement public methods for edge iteration and tests

namespace vecta {

class Edge;
class Point;
class Face;

class Edge {
	Point* start_point;
	Edge* twin;
	Edge* previous;
	Edge* next;
	Face* face;
};

class Point {
	Number x, y;
	Edge* first_edge;
};

class Face {
	Edge* first_edge;
	std::vector<Edge*> first_island_edges;
};

}

int main()
{
    return 0;
}
