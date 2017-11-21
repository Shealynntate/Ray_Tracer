#include "stdafx.h"
#include "Lighting.h"

#pragma region Light Base Class

Light::Light(Color c, float ls)
	: m_color{ c }, m_ls{ ls }, m_shadows{ false }
{

}

#pragma endregion

#pragma region Ambient Light

Ambient::Ambient(float ls, Color color)
	: Light{ color, ls }
{

}

Vector<3> Ambient::direction(const Vector<3>& hitPoint) const
{
	return Vector<3>{};
}

Color Ambient::light(const Vector<3>& hitPoint) const
{
	return m_color * m_ls;
}

RayParameters Ambient::shadowRay(const Ray& sRay) const
{
	return RayParameters{};
}

#pragma endregion

#pragma region Directional Light

Directional::Directional(float ls, Color color, Vector<3> dir)
	: Light{ color, ls }, m_dir{ dir }
{

}

Vector<3> Directional::direction(const Vector<3>& hitPoint) const
{
	return m_dir;
}

Color Directional::light(const Vector<3>& hitPoint) const
{
	return m_color * m_ls;
}

RayParameters Directional::shadowRay(const Ray& sRay) const
{
	RayParameters result;
	result.d = MAX_T;
	result.t = MAX_T;

	return result;
}

#pragma endregion

#pragma region Point Light

Point::Point(float ls, Color color, Vector<3> position, Vector<3> atten)
	: Light{ color, ls }, m_position{ position }, m_attenuation{ atten }
{

}

Vector<3> Point::direction(const Vector<3>& hitPoint) const
{
	return (m_position - hitPoint).normal();
}

Color Point::light(const Vector<3>& hitPoint) const
{
	float dist = (m_position - hitPoint).magnitude();
	Vector<3> atten{ m_attenuation[0], m_attenuation[1] * dist, m_attenuation[2] * dist * dist };
	
	return m_color * m_ls / (atten[0] + atten[1] + atten[2]);
}

RayParameters Point::shadowRay(const Ray& sRay) const
{
	RayParameters result;
	result.d = (m_position - sRay.origin).magnitude();
	result.t = result.d;

	return result;
}

void Point::setAttenuation(Vector<3> a)
{
	m_attenuation = a;
}

#pragma endregion
