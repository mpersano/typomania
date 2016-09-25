#ifndef RGBA_H_
#define RGBA_H_

struct rgba {
	rgba(float r, float g, float b, float a)
	: r(r), g(g), b(b), a(a)
	{ }

	rgba operator*(const float s) const
	{ return rgba(s*r, s*g, s*b, s*a); }

	rgba operator+(const rgba& other) const
	{ return rgba(r + other.r, g + other.g, b + other.b, a + other.a); }

	rgba operator-(const rgba& other) const
	{ return rgba(r - other.r, g - other.g, b - other.b, a - other.a); }

	float r, g, b, a;
};

static inline rgba
operator*(float s, const rgba& color)
{ return color*s; }

#endif // RGBA_H_
