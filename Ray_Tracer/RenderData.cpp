
#include "stdafx.h"
#include "RenderData.h"

#pragma region Lambertian BRDF

Lambertian::Lambertian(Color kd)
	: m_kd{ kd }
{

}

Color Lambertian::f(const ShaderData& sd, const Vector<3>& out, const Vector<3>& light) const
{
	float ndotl = light.dotProduct(sd.getNormal());

	return m_kd * std::max(ndotl, 0.0f);
}

Color Lambertian::rho(const ShaderData& sd, const Vector<3>& out) const
{
	return m_kd;
}

#pragma endregion

#pragma region Specular BRDF

Specular::Specular(Color ks, float exp)
	: m_ks{ ks }, m_exp{ exp }
{

}

Color Specular::f(const ShaderData& sd, const Vector<3>& out, const Vector<3>& light) const
{
	Color result;

	Vector<3> view, half;
	float ndotl, ndoth;

	switch (SPECULAR_MODEL)
	{
		case BLINN:
			view = (sd.getRay().origin - sd.getHitPoint()).normal();
			half = (view + light).normal();
			ndoth = sd.getNormal().dotProduct(half);

			if (ndoth > 0.0f)
				result += m_ks * powf(ndoth, m_exp);
			break;
		case PHONG:
			ndotl = light.dotProduct(sd.getNormal());
			view = light * -1.0f + sd.getNormal() * 2.0f * ndotl;
			float viewdotdir = view.dotProduct(sd.getRay().direction) * -1.0f;

			if (viewdotdir > 0)
				result += m_ks * powf(viewdotdir, m_exp);
			break;
	}

	return result;
}

Color Specular::rho(const ShaderData& sd, const Vector<3>& out) const
{
	return m_ks;
}

#pragma endregion

#pragma region Emissive BRDF

Emissive::Emissive(Color ke)
	: m_ke{ ke }
{

}

Color Emissive::f(const ShaderData& sd, const Vector<3>& out, const Vector<3>& in) const
{
	return m_ke;
}

Color Emissive::rho(const ShaderData& sd, const Vector<3>& out) const
{
	return m_ke;
}

#pragma endregion

#pragma region Material

Material::Material(Color ka, Color kd, Color ks, float spec, Color kr)
	: m_ambientBRDF{ ka }, m_diffuseBRDF{ kd }, m_specularBRDF{ ks, spec }, m_kr{ kr }
{

}

Color Material::ambient(const ShaderData& sd, const Light* light) const
{
	auto negDirection = sd.getRay().direction * -1.0f;
	
	return m_ambientBRDF.rho(sd, negDirection) * light->light(sd.getHitPoint());
}

Color Material::diffuse(const ShaderData& sd, const Light* light) const
{
	auto negDirection = sd.getRay().direction * -1.0f;
	auto lightDirection = light->direction(sd.getHitPoint());
	Color lIntensity = light->light(sd.getHitPoint());

	return m_diffuseBRDF.f(sd, negDirection, lightDirection) * lIntensity;
}

Color Material::specular(const ShaderData& sd, const Light* light) const
{
	auto negDirection = sd.getRay().direction * -1.0f;
	auto lightDirection = light->direction(sd.getHitPoint());
	Color lIntensity = light->light(sd.getHitPoint());

	return m_specularBRDF.f(sd, negDirection, lightDirection) * lIntensity;
}

Color Material::emissive(const ShaderData& sd, const Light* light) const
{
	return Color{};
}

Color Material::reflective() const
{
	return m_specularBRDF.m_ks;
}

bool Material::isReflective() const
{
	auto ks = m_specularBRDF.m_ks;
	return ks.r > 0 || ks.g > 0 || ks.b > 0;
}

void Material::setka(Color c)
{
	m_ambientBRDF.m_kd = c;
}

void Material::setkd(Color c)
{
	m_diffuseBRDF.m_kd = c;
}

void Material::setks(Color c)
{
	m_specularBRDF.m_ks = c;
}

void Material::setexp(float exp)
{
	m_specularBRDF.m_exp = exp;
}

#pragma endregion

#pragma region Emissive Material

EmissiveMaterial::EmissiveMaterial(Color ka, Color kd, Color ke, Color ks, float spec, Color kr)
	: Material{ ka, kd, ks, spec, kr }, m_emissiveBRDF{ ke }
{

}

void EmissiveMaterial::setke(Color c)
{
	m_emissiveBRDF.m_ke = c;
}

Color EmissiveMaterial::emissive(const ShaderData& sd, const Light* light) const
{
	return m_emissiveBRDF.rho(sd, sd.getRay().direction * -1);
}

#pragma endregion

#pragma region Shader Data

ShaderData::ShaderData()
	: m_depth{ 0 }, 
	  m_hitObject{ false }, 
	  m_material{},
	  m_ray{},
	  m_normal{},
	  m_hitPoint{}
{

}

ShaderData::ShaderData(const ShaderData &sd)
	: m_depth{ sd.m_depth },
	  m_hitObject{ false },
	  m_material{ sd.m_material },
	  m_ray{ sd.m_ray },
	  m_normal{ sd.m_normal },
	  m_hitPoint{ sd.m_hitPoint }
{

}

ShaderData::ShaderData(ShaderData&& sd)
	: m_depth{ sd.m_depth },
	  m_hitObject{ sd.m_hitObject },
	  m_material{ sd.m_material },
	  m_ray{ sd.m_ray },
	  m_normal{ sd.m_normal },
	  m_hitPoint{ sd.m_hitPoint }
{
	
}

ShaderData& ShaderData::operator =(ShaderData sd)
{
	std::swap(m_depth, sd.m_depth);
	std::swap(m_hitObject, sd.m_hitObject);
	std::swap(m_material, sd.m_material);
	std::swap(m_ray, sd.m_ray);
	std::swap(m_normal, sd.m_normal);
	std::swap(m_hitPoint, m_hitPoint);

	return *this;
}

ShaderData& ShaderData::operator =(ShaderData&& sd)
{
	std::swap(m_depth, sd.m_depth);
	std::swap(m_hitObject, sd.m_hitObject);
	std::swap(m_material, sd.m_material);
	std::swap(m_ray, sd.m_ray);
	std::swap(m_normal, sd.m_normal);
	std::swap(m_hitPoint, m_hitPoint);

	return *this;
}

void ShaderData::setDepth(int d)
{
	m_depth = d;
}

void ShaderData::setMaterial(Material m)
{
	m_material = m;
}

void ShaderData::setRay(Ray r)
{
	m_ray = r;
}

void ShaderData::setNormal(Vector<3> n)
{
	m_normal = n;
}

void ShaderData::setHitPoint(Vector<3> hp)
{
	m_hitPoint = hp;
}

int ShaderData::getDepth() const
{
	return m_depth;
}

Material ShaderData::getMaterial() const
{
	return m_material;
}

Ray ShaderData::getRay() const
{
	return m_ray;
}

Vector<3> ShaderData::getNormal() const
{
	return m_normal;
}

Vector<3> ShaderData::getHitPoint() const
{
	return m_hitPoint;
}

#pragma endregion
