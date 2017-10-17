#pragma once

#include <cmath>

typedef double Number;

class vec2d {
private:
	void cp(Number xx, Number yy) { x = xx;  y = yy; }
public:
	Number x, y;
	vec2d(const Number x = 0, const Number y = 0) { cp(x, y); }
	vec2d(const vec2d& q) { cp(q.x, q.y); }
	vec2d& operator= (const vec2d& q) { cp(q.x, q.y);  return *this; }
	vec2d& operator+= (const vec2d& q) { x += q.x;  y += q.y;  return *this; }
	vec2d& operator-= (const vec2d& q) { x -= q.x;  y -= q.y;  return *this; }
	vec2d& operator*= (const Number c) { x *= c;  y *= c;  return *this; }
	vec2d& operator/= (const Number c) { x /= c;  y /= c;  return *this; }
};

vec2d operator- (const vec2d& p) { return vec2d(-p.x, -p.y); }
vec2d operator+ (const vec2d& p, const vec2d& q) { return vec2d(p.x + q.x, p.y + q.y); }
vec2d operator- (const vec2d& p, const vec2d& q) { return p + -q; }
vec2d operator* (const Number c, const vec2d& q) { return vec2d(c*q.x, c*q.y); }
vec2d operator/ (const vec2d& p, const Number c) { return vec2d(p.x / c, p.y / c); }
Number operator* (const vec2d& p, const vec2d& q) { return p.x*q.x + p.y*q.y; }
vec2d operator~ (const vec2d& p) { return vec2d(-p.y, p.x); }
Number operator^ (const vec2d& p, const vec2d& q) { return ~p*q; }
bool operator< (const vec2d& p, const vec2d& q) { return (p^q)>0; }
bool operator> (const vec2d& p, const vec2d& q) { return (p^q)<0; }
bool operator|| (const vec2d& p, const vec2d& q) { return (p^q) == 0; }
bool operator<= (const vec2d& p, const vec2d& q) { return (p^q) >= 0; }
bool operator>= (const vec2d& p, const vec2d& q) { return (p^q) <= 0; }
Number len(const vec2d& p) { return hypot(p.x, p.y); }
Number dir(const vec2d& p) { return atan2(p.y, p.x); }
Number angle(const vec2d& u, const vec2d& v) { return atan2(u^v, u*v); }
