#ifndef VECTOR2_H_
#define VECTOR2_H_

#include <cstdio>
#include <cmath>

struct vector2 {
	float x, y;

	vector2(float x = 0, float y = 0) : x(x), y(y) { }

	vector2 operator*(float s) const
	{ return vector2(x*s, y*s); }

	vector2& operator*=(float s)
	{ x *= s; y *= s; return *this; }

	vector2 operator+(const vector2& v) const
	{ return vector2(x + v.x, y + v.y); }

	vector2& operator+=(const vector2& v)
	{ x += v.x; y += v.y; return *this; }

	vector2 operator-(const vector2& v) const
	{ return vector2(x - v.x, y - v.y); }

	vector2& operator-=(const vector2& v)
	{ x -= v.x; y -= v.y; return *this; }

	float dot(const vector2& v) const
	{ return x*v.x + y*v.y; }

	float length_squared() const
	{ return x*x + y*y; }

	float length() const
	{ return sqrt(length_squared()); }

	vector2& normalize()
	{ *this *= 1./length(); return *this; }

	vector2& set_length(float l)
	{ *this *= l/length(); return *this; }

	vector2 rotate(float ang) const
	{
		const float c = cos(ang);
		const float s = sin(ang);
		return vector2(x*c - y*s, y*c + x*s);
	}

	float distance(const vector2& v) const
	{ 
		vector2 d = v - *this;
		return d.length();
	}
};

inline vector2
operator*(float s, const vector2& v)
{
	return vector2(s*v.x, s*v.y);
}

#endif // VECTOR2_H_
