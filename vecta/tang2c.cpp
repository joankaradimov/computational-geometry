// Програмата служи за илюстрация и тестване на библиотеката vecta.
// Резултатът е текст, който може да се интерпретира чрез програмата
// sp.  Следното примерно пускане на двете програми създава файл
// test.svg (SVG), който съдържа рисунката (уголемена) и тя може да
// бъде видяна например в браузер:
//
//     tang2c | sp svg 3 >test.svg
// -----------------------------------------------------------------

#include <iostream>
#include "vecta.h"

using namespace std;
using namespace vecta;

template <typename N>
ostream& operator << (ostream& os, vec2d<N> p) {
	return  os << p.x << ' ' << p.y;
}

// Намиране на допирна точка при обща допирателна за две окръжности.
// Параметрите задават окръжностите и уточняват коя допирателна и коя
// именно нейна точка да бъде намерена, както следва:
//     c1, r1 = център и радиус на първата окръжност
//     c2, r2 = център и радиус на втората окръжност
//     s1 = 1 за „ляво-ляво“ и „дясно-дясно“ допиране, иначе -1
//     s2 = 1 за „дясно-ляво“ и „дясно-дясно“ допиране, иначе -1
//     k = върху коя окръжност е допирателната точка, 1 или 2
vec2d<> tang2c(vec2d<>& c1, double r1, vec2d<>& c2, double r2, int s1, int s2, int k) {
	vec2d<> u = c2 - c1,
		c = k == 1 ? c1 : c2;
	double d = norm(u),
		t = r1 + s1*r2,
		r = k == 1 ? r1 : r2,
		s = 2 * (k + s1 != 3) - 1;
	return c + s*r / d*(t*u + s2*sqrt(d - t*t)*~u);
}

int main() {
	// избираме непресичащи се окръжности, т. че да са налице всички допирателни:
	double r1 = 5, r2 = 3;
	vec2d<> c1(1, 1), c2(c1 + vec2d<>(10, 0));
	// намиране на всички допирни точки:
	vec2d<> pLL1 = tang2c(c1, r1, c2, r2, 1, -1, 1),
		pLL2 = tang2c(c1, r1, c2, r2, 1, -1, 2),
		pLR1 = tang2c(c1, r1, c2, r2, -1, -1, 1),
		pLR2 = tang2c(c1, r1, c2, r2, -1, -1, 2),
		pRL1 = tang2c(c1, r1, c2, r2, -1, 1, 1),
		pRL2 = tang2c(c1, r1, c2, r2, -1, 1, 2),
		pRR1 = tang2c(c1, r1, c2, r2, 1, 1, 1),
		pRR2 = tang2c(c1, r1, c2, r2, 1, 1, 2);
	// рисуване на окръжностите:
	cout << "width 1\ncolour gray\ncircles\n" << c1 << ' ' << r1 << endl
		<< c2 << ' ' << r2 << endl;
	// рисуване на точките
	cout << "colour blue\npoints\n" << pLL1 << endl << pLL2 << endl
		<< pLR1 << endl << pLR2 << endl
		<< pRL1 << endl << pRL2 << endl
		<< pRR1 << endl << pRR2 << endl;
	// рисуване на допирателните отсечки:
	cout << "width .3\ncolour purple\nlines\n" << pLL1 << ' ' << pLL2 << endl
		<< pLR1 << ' ' << pLR2 << endl
		<< pRL1 << ' ' << pRL2 << endl
		<< pRR1 << ' ' << pRR2 << endl;
	return 0;
}
