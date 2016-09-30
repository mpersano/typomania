#pragma once

#include "vec2.h"

struct mat3
{
	mat3()
	: m00 { 0 }, m01 { 0 }, m02 { 0 }
	, m10 { 0 }, m11 { 0 }, m12 { 0 }
	{ }

	mat3(float m00, float m01, float m02,
	      float m10, float m11, float m12)
	: m00 { m00 }, m01 { m01 }, m02 { m02 }
	, m10 { m10 }, m11 { m11 }, m12 { m12 }
	{ }

	static mat3 identity()
	{
		return { 1.f, 0.f, 0.f,
			 0.f, 1.f, 0.f };
	}

	static mat3 translation(float x, float y)
	{
		return { 1.f, 0.f, x,
			 0.f, 1.f, y };
	}

	static mat3 translation(const vec2f& pos)
	{
		return translation(pos.x, pos.y);
	}

	static mat3 scale(float s)
	{
		return scale(s, s);
	}

	static mat3 scale(float sx, float sy)
	{
		return {  sx, 0.f, 0.f,
			 0.f,  sy, 0.f };
	}

	static mat3 scale(const vec2f& s)
	{
		return scale(s.x, s.y);
	}

	static mat3 rotation(float a)
	{
		const float c = cosf(a);
		const float s = sinf(a);

		return { c, -s, 0.f,
			 s,  c, 0.f };
	}

	mat3& operator*=(const mat3& m)
	{
		return *this = *this*m;
	}

	mat3 operator*(const mat3& m) const
	{
		const float l00 = m00*m.m00 + m01*m.m10;
		const float l01 = m00*m.m01 + m01*m.m11;
		const float l02 = m00*m.m02 + m01*m.m12 + m02;

		const float l10 = m10*m.m00 + m11*m.m10;
		const float l11 = m10*m.m01 + m11*m.m11;
		const float l12 = m10*m.m02 + m11*m.m12 + m12;

		return { l00, l01, l02,
			 l10, l11, l12 };
	}

	vec2f operator*(const vec2f& v) const
	{
		const float x = m00*v.x + m01*v.y + m02;
		const float y = m10*v.x + m11*v.y + m12;

		return { x, y };
	}

	float m00, m01, m02;
	float m10, m11, m12;
};
