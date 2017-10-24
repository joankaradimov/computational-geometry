#include <iostream>
#include <math.h>
#include <string>

#include "../vecta/vecta.h"

int main()
{
	vecta::vec2d<> a(2, 0);
	vecta::vec2d<> b(9, 0);
	vecta::vec2d<> l(0, 8);
	vecta::vec2d<> r(10, 6);
	vecta::vec2d<> q(3, 10);

	auto length_ab = vecta::len(-a + b);
	auto length_ab_side = abs((-l + r) * (-a + b)) / length_ab;
	auto length_ab_normal_side = abs((-a + b) ^ (-a + q)) / length_ab;

	//std::stringstream sp_input;
	//sp_intput << "echo -e sp svg " <<
	system("printf 'points\\n1 1\\n2 2\\n' | C:\\dev\\work\\computational-graphics\\x64\\Debug\\sp svg > asd.svg");
	//system("\"\"C:\\Program Files(x86)\\Google\\Chrome\\Application\\chrome.exe\" asd.svg\"");
	//system("\"\"c:\\program files\\internet explorer\\iexplore.exe\" \"asd.svg\"\"");
	std::cout << length_ab_side * length_ab_normal_side << std::endl;

    return 0;
}
