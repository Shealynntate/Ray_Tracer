#include "stdafx.h"
#include "Utilities.h"

#pragma region Color Data

Color::Color(float r, float g, float b)
	: r(r), g(g), b(b)
{

}

Color Color::operator +(const Color& c) const
{
	return Color(r + c.r, g + c.g, b + c.b);
}

Color Color::operator *(const Color& c) const
{
	return Color(r * c.r, g * c.g, b * c.b);
}

Color Color::operator *(float s) const
{
	return Color(r * s, g * s, b * s);
}

Color Color::operator /(float s) const
{
	if (s == 0.0f)
		return Color();
	else
		return Color(r / s, g / s, b / s);
}

void Color::operator +=(Color c)
{
	r += c.r;
	g += c.g;
	b += c.b;
}

void Color::operator *=(float s)
{
	r *= s;
	g *= s;
	b *= s;
}

void Color::operator /=(float s)
{
	r = (s == 0.0f) ? 0 : r / s;
	g = (s == 0.0f) ? 0 : g / s;
	b = (s == 0.0f) ? 0 : b / s;
}

Color Color::clamp(Color c)
{
	return Color(fminf(c.r, 1.0f), fminf(c.g, 1.0f), fminf(c.b, 1.0f));
}

#pragma endregion

#pragma region Pixel Sample

Sample::Sample(int x, int y)
	: x(x), y(y)
{

}

#pragma endregion

#pragma region Ray

Ray::Ray(Vector<3> origin, Vector<3> direction, Sample sample)
	: origin(origin), direction(direction), sample(sample)
{

}

Ray::Ray(Sample sample)
	: origin(), direction(), sample(sample)
{

}

#pragma endregion