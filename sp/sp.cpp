#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <ctime>

using namespace std;

#define ASSERT(c) {if (!(c)) throw "";}

const double PI = 3.1415926536;

const double deg2rad = PI / 180.;

#define uint unsigned

typedef const char * const CString;

enum { EPS, SVG } outputmode;

ostringstream datastr;

// constant prelude for PostScript output
CString initstrEPS =
"%%Creator: sp v2.0 (March 2015)\n"
"\n"
"%%Page: 1 1\n"
// mm to PostScript point (1/72") conversion factor
"/1mm 72 25.4 div def\n"
"\n"
// square root of the determinant of the CTM: this is how much lengths change under CT
"/det-sqrt {\n"
"  aload pop pop pop\n"
"  4 1 roll mul 3 1 roll mul sub abs sqrt\n"
"} bind def\n"
"\n"
// mm to current unit conversion
"/mm2ct {\n"
"  matrix defaultmatrix det-sqrt\n"
"  matrix currentmatrix det-sqrt\n"
"  div\n"
"  1mm mul mul\n"
"} bind def\n"
"\n"
// draw in current width and linetype, taken as mm
"/stroke-mm {\n"
"  currentlinewidth dup mm2ct setlinewidth\n"
"  currentdash 2 copy mm2ct [ 3 -1 roll {mm2ct} forall ] exch setdash\n"
"  stroke\n"
"  setdash setlinewidth\n"
"} bind def\n"
"\n"
// call pattern: x y a s setxfrm -
// sets scaling s x outer.scale mm, rotation a degrees and moving by
// (x,y) + (outer.shift.x,outer.shift.y), also in mm
"/setxfrm {\n"
"  initmatrix\n"
"  1mm dup scale\n"
"  [ 1 0 0 1 outer.shift.x outer.shift.y ] concat\n"
"  outer.scale dup scale\n"
"  [ 1 0 0 1 9 -2 roll ] concat\n"
"  dup scale\n"
"  rotate\n"
"} bind def\n"
"\n"
"/dcircle {3 copy 3 -1 roll add exch moveto 0 360 arc closepath} bind def\n"
"\n"
"/dpnt {\n"
"  gsave\n"
"  .4 setlinewidth\n"
"  newpath\n"
"  r mm2ct dcircle\n"
"  gsave 1 setgray fill grestore stroke-mm\n"
"  grestore\n"
"} def\n"
"\n"
"/darrows {\n"   // dx dy scaling 1|2
"  4 -2 roll 2 copy exch atan\n"
"  {\n"
"   gsave\n"
"   /dx /dy translate /phi rotate /scl dup scale\n"
"   -4.5 0 moveto -3 1.5 0 .5 .5 0 0 -.5 -3 -1.5 5 {lineto} repeat closepath fill\n"
"   grestore\n"
"   180 rotate\n"
"  }\n"
"  dup 4 4 -1 roll put\n"
"  dup 6 7 -1 roll put\n"
"  dup 1 5 -1 roll put\n"
"  dup 2 4 -1 roll put\n"
"  cvx repeat\n"
"} def\n"
"\n"
"/r .7 def\n"
;

// numeric type for coordinates and dimensional values
typedef double Number;

// coordinate pair type
typedef struct { Number x, y; } Pt;

// colour type
struct RGBcolour { double r, g, b; };

// graphical context type
struct Gcontext {
	Pt move;
	double rotate;
	double scale;
	RGBcolour colour;
	double width;
	vector<double> linetype;
	char cap, join;
};

// 16 colour values by names as known in HTML

const double C00 = 0.;
const double C12 = 0.5;
const double C34 = 0.75;
const double C11 = 1.;

typedef struct { CString n; RGBcolour c; } HTMLcolour;

HTMLcolour htmlcolours[] = {
	{ "black",{ C00,C00,C00 } },
	{ "silver",{ C34,C34,C34 } },
	{ "gray",{ C12,C12,C12 } },
	{ "white",{ C11,C11,C11 } },
	{ "maroon",{ C12,C00,C00 } },
	{ "red",{ C11,C00,C00 } },
	{ "purple",{ C12,C00,C12 } },
	{ "fuchsia",{ C11,C00,C11 } },
	{ "green",{ C00,C12,C00 } },
	{ "lime",{ C00,C11,C00 } },
	{ "olive",{ C12,C12,C00 } },
	{ "yellow",{ C11,C11,C00 } },
	{ "navy",{ C00,C00,C12 } },
	{ "blue",{ C00,C00,C11 } },
	{ "teal",{ C00,C12,C12 } },
	{ "aqua",{ C00,C11,C11 } }
};

// type values for commands (NEUT being non-construction)
enum { NEUT, POINT, POLYL, POLYG, FPOLYG, LINE, ARROW, BIARROW, CIRCLE, FCIRCLE };

// global (outer) graphical context
Pt outermove;
double outerscale;

// graphical context;
// default initialization: move 0,0, rotate 0, scale 1, black, width .2, solid, exact, cut
Gcontext gcontext = { { 0.,0. },0.,1.,{ 0.,0.,0. },.2,{},'|','|' };

typedef struct { Number x, y, rad; } Fig;

// point coordinates, in the order of appearance in the input
vector<Pt> pts;

// constructive data for the non-point objects, in the order of appearance
vector<Fig> figs;

// point set representation with an index to a range end in pts[] and a shared graphical context
typedef struct { uint idx; Gcontext gcontext; } Ptends;

// non-point object set representation with an index to a range end in figs[] and a shared graphical context;
// type is an enum value representing the object type
typedef struct { uint type; uint idx; Gcontext gcontext; } Figends;

// points sets
vector<Ptends> ptends;

// non-point object sets
vector<Figends> figends;

void startgroupSVG(Gcontext & gc) {
	datastr << "\n<g transform=\"translate(" << outermove.x << ',' << outermove.y << ") scale(" << outerscale << ',' << outerscale
		<< ") translate(" << gc.move.x << ',' << gc.move.y << ") rotate(" << gc.rotate << ") scale(" << gc.scale << ',' << gc.scale << ")\""
		" stroke=\"rgb(" << (uint)(.5 + 255 * gc.colour.r) << ',' << (uint)(.5 + 255 * gc.colour.g) << ',' << (uint)(.5 + 255 * gc.colour.b) << ")\""
		" fill=\"rgb(" << (uint)(.5 + 255 * gc.colour.r) << ',' << (uint)(.5 + 255 * gc.colour.g) << ',' << (uint)(.5 + 255 * gc.colour.b) << ")\">\n";
}

// drawing non-point objects from the range [p,q) in figs[]
void dfigsEPS(uint p, uint q) {
	uint type = figends[q].type;
	Gcontext gc = figends[q].gcontext;
	bool filled = type == FCIRCLE || type == FPOLYG;
	datastr << '\n' << gc.move.x << ' ' << gc.move.y << ' ' << gc.rotate << ' ' << gc.scale << " setxfrm\n";
	datastr << gc.colour.r << ' ' << gc.colour.g << ' ' << gc.colour.b << " setrgbcolor\n";
	if (!filled) {
		datastr << gc.width << " setlinewidth\n";
		if (gc.linetype.empty())
			datastr << "[] 0";
		else {
			datastr << "[ ";
			for (uint i = 1; i<gc.linetype.size(); ++i)
				datastr << gc.linetype[i] << ' ';
			datastr << "] " << gc.linetype[0];
		}
		datastr << " setdash\n";
		datastr << (gc.cap == ']' ? 2 : gc.cap == ')' ? 1 : 0) << " setlinecap\n";
		datastr << (gc.join == '>' ? 0 : gc.join == ')' ? 1 : 2) << " setlinejoin\n";
	}
	q = figends[q].idx;
	datastr << "newpath\n";
	if (type == LINE || type == ARROW || type == BIARROW || type == CIRCLE || type == FCIRCLE) {
		for (uint i = p; i<q; ++i)
			if (type == LINE || type == ARROW || type == BIARROW) {
				datastr << figs[i].x << ' ' << figs[i].y << " moveto ";
				++i;
				datastr << figs[i].x << ' ' << figs[i].y << " lineto\n";
				if (type == ARROW || type == BIARROW) {
					Number x1 = figs[i - 1].x / 2, y1 = figs[i - 1].y / 2, x2 = figs[i].x / 2, y2 = figs[i].y / 2,
						s = gc.width / outerscale / gc.scale;
					datastr << "gsave\n" << (x1 + x2) << ' ' << (y1 + y2) << " translate\n"
						<< (x2 - x1) << ' ' << (y2 - y1) << ' ' << s << ' ' << (type == ARROW ? 1 : 2) << " darrows\ngrestore\n";
				}
			}
			else
				datastr << figs[i].x << ' ' << figs[i].y << ' ' << figs[i].rad << " dcircle\n";
	}
	else {
		datastr << figs[p].x << ' ' << figs[p].y << " moveto\n";
		for (uint i = p + 1; i<q; ++i)
			datastr << figs[i].x << ' ' << figs[i].y << " lineto\n";
		if (type == POLYG) datastr << "closepath\n";
	}
	datastr << (filled ? "eofill" : "stroke-mm\n");
}

// drawing non-point objects from the range [p,q) in figs[]
void dfigsSVG(uint p, uint q) {
	uint type = figends[q].type;
	Gcontext gc = figends[q].gcontext;
	bool filled = type == FCIRCLE || type == FPOLYG;
	startgroupSVG(gc);
	if (filled)
		datastr << "<g stroke=\"none\" fill-rule=\"evenodd\">\n";
	else {
		datastr << "<g fill=\"none\" stroke-width=\"" << (gc.width / outerscale / gc.scale) << "\" stroke-dasharray=\"";
		if (gc.linetype.empty())
			datastr << "none\"";
		else {
			for (uint i = 1; i<gc.linetype.size(); ++i)
				datastr << (gc.linetype[i] / outerscale / gc.scale) << ' ';
			datastr << '"';
			datastr << " stroke-dashoffset=\"" << (gc.linetype[0] / outerscale / gc.scale) << "\"";
		}
		datastr << " stroke-linecap=\"" << (gc.cap == ']' ? "square" : gc.cap == ')' ? "round" : "butt") << "\"";
		datastr << " stroke-linejoin=\"" << (gc.join == '>' ? "miter" : gc.join == ')' ? "round" : "bevel") << "\"";
		datastr << ">\n";
	}
	q = figends[q].idx;
	if (type == LINE || type == ARROW || type == BIARROW || type == CIRCLE || type == FCIRCLE) {
		for (uint i = p; i<q; ++i)
			if (type == LINE || type == ARROW || type == BIARROW) {
				datastr << "<line x1=\"" << figs[i].x << "\" y1=\"" << figs[i].y << "\" x2=\"" << figs[i + 1].x << "\" y2=\"" << figs[i + 1].y << "\"/>\n";
				if (type == ARROW || type == BIARROW) {
					Number x1 = figs[i].x, y1 = figs[i].y, x2 = figs[i + 1].x, y2 = figs[i + 1].y,
						a = atan2(y2 - y1, x2 - x1) / deg2rad, s = gc.width / outerscale / gc.scale;
					datastr << "<g stroke=\"none\" fill=\"rgb(" << (uint)(.5 + 255 * gc.colour.r) << ',' << (uint)(.5 + 255 * gc.colour.g) << ',' << (uint)(.5 + 255 * gc.colour.b) << ")\">\n";
					datastr << "<g transform=\"translate(" << x2 << ',' << y2 << ") rotate(" << a << ") scale(" << s << ',' << s << ")\">\n";
					datastr << "<path d=\"M-4.5,0 L-3,-1.5 0,-0.5 0.5,0 0,0.5 -3,1.5 z\"/>\n</g>\n";
					if (type == BIARROW) {
						datastr << "<g transform=\"translate(" << x1 << ',' << y1 << ") rotate(" << (180 + a) << ") scale(" << s << ',' << s << ")\">\n";
						datastr << "<path d=\"M-4.5,0 L-3,-1.5 0,-0.5 0.5,0 0,0.5 -3,1.5 z\"/>\n</g>\n";
					}
					datastr << "</g>\n";
				}
				++i;
			}
			else {
				datastr << "<circle cx=\"" << figs[i].x << "\" cy=\"" << figs[i].y << "\" r=\"" << figs[i].rad << "\"/>\n";
			}
	}
	else {
		datastr << '<' << (type == POLYL ? "polyline" : "polygon") << " points=\"";
		for (uint i = p; i<q; ++i)
			datastr << figs[i].x << ',' << figs[i].y << ' ';
		datastr << "\"/>\n";
	}
	datastr << "</g>\n";
	datastr << "</g>\n";
}

// drawing points from the range [p,q) in pts[]
void dpointsEPS(uint p, uint q) {
	Gcontext gc = ptends[q].gcontext;
	datastr << '\n' << gc.move.x << ' ' << gc.move.y << ' ' << gc.rotate << ' ' << gc.scale << " setxfrm\n";
	datastr << gc.colour.r << ' ' << gc.colour.g << ' ' << gc.colour.b << " setrgbcolor\n";
	for (uint i = p; i<ptends[q].idx; ++i)
		datastr << pts[i].x << ' ' << pts[i].y << " dpnt\n";
}

// drawing points from the range [p,q) in pts[]
void dpointsSVG(uint p, uint q) {
	Gcontext gc = ptends[q].gcontext;
	startgroupSVG(gc);
	for (uint i = p; i<ptends[q].idx; ++i)
		datastr << "<circle cx=\"" << pts[i].x << "\" cy=\"" << pts[i].y
		<< "\" r=\"" << (.7 / outerscale / gc.scale)
		<< "\" stroke-width=\"" << (.4 / outerscale / gc.scale)
		<< "\" fill=\"white\"/>\n";
	datastr << "</g>\n";
}

// bounding box in mm
Number bbx1, bby1, bbx2, bby2;

// fitting a point in the bounding box (expanding the latter as needed)
void addtobbox(Number x, Number y) {
	static bool boxempty = true;
	double cx, cy, t;
	t = deg2rad*gcontext.rotate;
	cx = cos(t); cy = sin(t);
	t = x*cx - y*cy;
	y = x*cy + y*cx;
	x = t;
	x *= gcontext.scale;
	y *= gcontext.scale;
	x += gcontext.move.x;
	y += gcontext.move.y;
	Number d = gcontext.width / 2. + .5;
	if (boxempty) {
		bbx1 = x - d;
		bbx2 = x + d;
		bby1 = y - d;
		bby2 = y + d;
		boxempty = false;
	}
	else {
#define setm(m,r,t) r=m(r,t)
		setm(min, bbx1, x - d); setm(min, bby1, y - d);
		setm(max, bbx2, x + d); setm(max, bby2, y + d);
#undef setm
	}
}

string emsg;
uint mode = NEUT;
Gcontext newgcontext;
vector<string> tks;

//Number getNumber(uint i) {long uint j; auto x = (Number)stod(tks[i],&j); ASSERT(tks[i][j]==0); return x;}
Number getNumber(uint i) { char *p; auto x = (Number)strtod(tks[i].c_str(), &p); ASSERT(*p == 0); return x; }

// reading a command title or an entire command
// returns (cmd==)true iff command name found on the line
// sets mode according to the command вЂ“ specific for construction, NEUT for the rest
bool readcommandline() {
	bool cmd = true;
	if (tks[0] == "move") {
		mode = NEUT;
		newgcontext.move.x = getNumber(1);
		newgcontext.move.y = getNumber(2);
	}
	else if (tks[0] == "unit") {
		mode = NEUT;
		newgcontext.scale = getNumber(1);
	}
	else if (tks[0] == "rotate") {
		mode = NEUT;
		newgcontext.rotate = getNumber(1);
	}
	else if (tks[0] == "colour") {
		mode = NEUT;
		bool b = true;
		for (auto cnv : htmlcolours)
			if (tks[1] == cnv.n) {
				newgcontext.colour = cnv.c;
				b = false;
				break;
			}
		if (b) {       //  colour is given by value(s)
			newgcontext.colour.r =
				newgcontext.colour.g =
				newgcontext.colour.b = getNumber(1);
			if (tks.size() >= 4) {
				newgcontext.colour.g = getNumber(2);
				newgcontext.colour.b = getNumber(3);
			}
			if (!(0. <= newgcontext.colour.r && newgcontext.colour.r <= 1. &&
				0. <= newgcontext.colour.g && newgcontext.colour.g <= 1. &&
				0. <= newgcontext.colour.b && newgcontext.colour.b <= 1.)
				) throw emsg + "incorrect colour value";
		}
	}
	else if (tks[0] == "width") {
		mode = NEUT;
		newgcontext.width = fabs(getNumber(1));
	}
	else if (tks[0] == "linetype") {
		mode = NEUT;
		newgcontext.linetype.clear();
		for (uint i = 1; i<tks.size(); ++i)       // storing into xв‚Ђ,xв‚Ѓ,вЂ¦, meaning вЂ[xв‚Ѓ вЂ¦] xв‚Ђ setdashвЂ™, or {} for solid
			newgcontext.linetype.push_back(fabs(getNumber(i)));
		if (tks.size() == 2) {                    // if only one argument x, {x/2,x} is stored, meaning вЂ[x] x/2 setdashвЂ™
			newgcontext.linetype.push_back(newgcontext.linetype[0]);
			newgcontext.linetype[0] /= 2;
		}
	}
	else if (tks[0] == "cap") {
		mode = NEUT;
		newgcontext.cap = tks.size() >= 2 ? tks[1][0] : ' ';
	}
	else if (tks[0] == "join") {
		mode = NEUT;
		newgcontext.join = tks.size() >= 2 ? tks[1][0] : ' ';
	}
	else if (tks[0] == "points") {
		mode = POINT;
	}
	else if (tks[0] == "polyline") {
		mode = POLYL;
	}
	else if (tks[0] == "polygon") {
		mode = POLYG;
	}
	else if (tks[0] == "polygon*") {
		mode = FPOLYG;
	}
	else if (tks[0] == "lines") {
		mode = LINE;
	}
	else if (tks[0] == "lines>") {
		mode = ARROW;
	}
	else if (tks[0] == "lines<>") {
		mode = BIARROW;
	}
	else if (tks[0] == "circles") {
		mode = CIRCLE;
	}
	else if (tks[0] == "circles*") {
		mode = FCIRCLE;
	}
	else {
		cmd = false;
	}
	return cmd;
}

// reading a data line for a construction command
void readdataline() {
	Number x, y, r;
	if (mode == POINT) {
		x = getNumber(0);
		y = getNumber(1);
		pts.push_back({ x,y });
		addtobbox(x, y);
	}
	else if (mode == POLYL || mode == POLYG || mode == FPOLYG) {
		x = getNumber(0);
		y = getNumber(1);
		figs.push_back({ x,y });
		addtobbox(x, y);
	}
	else if (mode == LINE || mode == ARROW || mode == BIARROW) {
		x = getNumber(0);
		y = getNumber(1);
		figs.push_back({ x,y });
		addtobbox(x, y);
		x = getNumber(2);
		y = getNumber(3);
		figs.push_back({ x,y });
		addtobbox(x, y);
	}
	else if (mode == CIRCLE || mode == FCIRCLE) {
		x = getNumber(0);
		y = getNumber(1);
		r = getNumber(2);
		if (r <= 0.) throw emsg + "non-positive radius";
		figs.push_back({ x,y,r });
		addtobbox(x - r, y - r);
		addtobbox(x - r, y + r);
		addtobbox(x + r, y - r);
		addtobbox(x + r, y + r);
	}
}

void storecmd(uint type) {
	if (type == POINT) ptends.push_back({ (uint)pts.size(),gcontext });
	else             figends.push_back({ type,(uint)figs.size(),gcontext });
}

bool readallinput() {
	try {
		bool iskeyword;                  // true iff the input line holds a command name
		uint prevmode = mode;
		uint lineno = 0;
		string s;
		// storing newly read object(s), points or others
		while (getline(cin, s)) {         // read and process an input line
			++lineno;
			string snum;
			for (uint n = lineno; n; n /= 10) snum += '0' + n % 10;
			reverse(snum.begin(), snum.end());
			emsg = "Error on line " + snum + ": ";
			for (auto & c : s) c = tolower(c);
			istringstream sstr(s);
			string tk;
			for (tks.clear(); sstr >> tk && tk != "-";)
				tks.emplace_back(tk);
			if (tks.empty()) continue;     // process non-empty lines only
			prevmode = mode;               // because readcommandline changes mode
			iskeyword = true;
			if (!readcommandline()) {      // the input line does not hold a command name
				iskeyword = false;
				if (mode == NEUT) throw emsg + "keyword expected, none found";
				else            readdataline();
			}
			// effectuate a command: store a construction, set a mode
			if (iskeyword && prevmode != NEUT)
				storecmd(prevmode);
			if (tks[0] == "move")
				gcontext.move = newgcontext.move;
			else if (tks[0] == "unit")
				gcontext.scale = newgcontext.scale;
			else if (tks[0] == "rotate")
				gcontext.rotate = newgcontext.rotate;
			else if (tks[0] == "width")
				gcontext.width = newgcontext.width;
			else if (tks[0] == "linetype")
				gcontext.linetype = newgcontext.linetype;
			else if (tks[0] == "cap")
				gcontext.cap = newgcontext.cap;
			else if (tks[0] == "join")
				gcontext.join = newgcontext.join;
			else if (tks[0] == "colour")
				gcontext.colour = newgcontext.colour;
		}
		if (!iskeyword) storecmd(prevmode);
		return true;
	}
	catch (string e) {
		if (e.substr(0, 14) != emsg.substr(0, 14))
			e = emsg + "missing or incorrect data";
		cerr << e << endl;
		return false;
	}
}

int main(int argc, CString argv[]) {
	outputmode = EPS;
	// reading possible command-line arguments
	int na = 1;
	if (argc>1 && isalpha(argv[1][0])) {
		++na;
		string s(argv[1]);
		for (auto & c : s) c = tolower(c);
		if (s == "eps")      outputmode = EPS;
		else if (s == "svg") outputmode = SVG;
		else {
			cerr << "Wrong output mode argument: " << argv[1] << endl;
			return 0;
		}
	}
	if (argc - na>3 || (outputmode == SVG && argc - na>1)) {
		cerr << "Wrong number of command-line arguments\n";
		return 0;
	}
	vector<string> args;
	for (; na<argc; ++na) args.push_back(argv[na]);
	na = args.size();
	//#define RDI(x,i) {long uint j; x = stod(args[i],&j); ASSERT(args[i][j]==0);}
#define RDI(x,i) {char *p; x = strtod(args[i].c_str(),&p); ASSERT(*p==0);}
	if (na<2)
		outermove.x = outermove.y = 0;
	else
	{
		RDI(outermove.x, 0); RDI(outermove.y, 1);
	}
	if (na == 0 || na == 2)
		outerscale = 1;
	else try {
		RDI(outerscale, na == 1 ? 0 : 2);
		ASSERT(outerscale>0);
	}
	catch (...) {
		cerr << "Wrong value of a command-line argument\n";
		return 1;
	}
#undef RDI
	if (!readallinput()) return 1;
	// calculating the outermove translation
	if (outputmode == EPS && na>0) {
		outermove.x -= (bbx1 + bbx2) / 2.;
		outermove.y -= (bby1 + bby2) / 2.;
	}
	// time string formation in strtm
	time_t t = time(nullptr);
	tm time_components;
	localtime_s(&time_components, &t);
	char strtm[21];
	strftime(strtm, sizeof(strtm), "%Y-%m-%d %H:%M:%S", &time_components); // ISO 8601 date & time, also "%F %T"
																		   // writing down the global transformation
	switch (outputmode) {
	case EPS: datastr << "/outer.scale " << outerscale << " def\n";
		datastr << "/outer.shift.x " << outermove.x << " def\n";
		datastr << "/outer.shift.y " << outermove.y << " def\n";
		break;
	case SVG: break;
	}
	uint i, j;
	// writing down drawings of non-point objects
	for (i = j = 0; j<figends.size(); i = figends[j++].idx)
		switch (outputmode) {
		case EPS: dfigsEPS(i, j); break;
		case SVG: dfigsSVG(i, j); break;
		}
	// writing down drawings of points
	for (i = j = 0; j<ptends.size(); i = ptends[j++].idx)
		switch (outputmode) {
		case EPS: dpointsEPS(i, j); break;
		case SVG: dpointsSVG(i, j); break;
		}
	// form the actual output
	switch (outputmode) {
	case EPS: {cout << "%!PS-Adobe-2.0 EPSF-3.0\n";
		auto bbstring = [&](Number t, char c, int pm)->int {
			(((t *= outerscale) += c == 'x' ? outermove.x : outermove.y) *= 72 / 25.4) += pm * 5;
			t += .5*((t>0) - (t<0));
			return (int)t;
		};
		cout << "%%BoundingBox: " << bbstring(bbx1, 'x', -1)
			<< ' ' << bbstring(bby1, 'y', -1)
			<< ' ' << bbstring(bbx2, 'x', 1)
			<< ' ' << bbstring(bby2, 'y', 1) << endl;
		cout << "%%CreationDate: " << strtm << endl;
		cout << initstrEPS << endl;
		cout << datastr.str();
		cout << "\n%%Trailer\n";
		//cout << "showpage\n";
		// writing down a comment about the global transformation if any
		if (outerscale != 1 || outermove.x != 0 || outermove.y != 0)
			cout << "% outer move = (" << outermove.x << "," << outermove.y
			<< ") + outer scale = " << outerscale << endl;
		cout << "%%EOF\n";
		break;}
	case SVG: {cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
		cout << "<!-- Created by sp v2.0, " << strtm << " -->\n";
		auto bbstring = [&](Number t, int pm)->int {
			(t *= outerscale) += pm * 5;
			t += .5*((t>0) - (t<0));
			return (int)t;
		};
		cout << "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"";
		int x1 = bbstring(bbx1, -1), y1 = bbstring(bby1, -1), y2 = bbstring(bby2, 1);
		int w = bbstring(bbx2, 1) - x1, h = y2 - y1;
		cout << " width=\"" << w << "mm\" height=\"" << h << "mm\" viewBox=\"" << x1 << ' ' << y1 << ' ' << w << ' ' << h << "\">\n";
		cout << "<g transform=\"matrix(1 0 0 -1 0 " << (y1 + y2) << ")\">\n";
		cout << datastr.str();
		cout << "</g>\n</svg>\n";
		break;}
	}
	return 0;
}
// http://open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3797.pdf
