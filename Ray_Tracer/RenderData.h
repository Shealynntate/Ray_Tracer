/* -------------------------------------------------------------------------------------------------
   Copyright 2017 Shealyn Tate Hindenlang

   Permission is hereby granted, free of charge, to any person obtaining a copy of this software
   and associated documentation files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge, publish, distribute,
   sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies or
   substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
   BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   -------------------------------------------------------------------------------------------------
*/
#ifndef RENDERDATA_H
#define RENDERDATA_H

#include <vector>
#include <algorithm>

#include "Lighting.h"

#pragma region BRDF Abstract Base Class

/* -------------------------------------------------------------------------------------------------
   Base abstract class for all bidirectional reflectance distribution functions (BRDFs) that model
   how light is reflected off of opaque surfaces. The rho function returns the color property of 
   the particular BRDF and the function f computes the radiance reflected at a particular point of
   the material, given a specific light source, where radiance is simplified to a Color value.
   -------------------------------------------------------------------------------------------------
*/
class BRDF
{
public:
	virtual Color f(const ShaderData&, const Vector<3>&, const Vector<3>&) const = 0;
	virtual Color rho(const ShaderData&, const Vector<3>&) const = 0;
};

#pragma endregion

#pragma region Lambertian BRDF

/* -------------------------------------------------------------------------------------------------
   Lambertian BRDF models diffuse, plastic-like surfaces. The radiance function, f, depends on the 
   light direction and the surface normal.
   -------------------------------------------------------------------------------------------------
*/
class Lambertian : public BRDF
{
private:
	Color m_kd;

public:
	Lambertian(Color = Color{});

	Color f(const ShaderData&, const Vector<3>&, const Vector<3>&) const override;
	Color rho(const ShaderData&, const Vector<3>&) const override;

	friend class Material;
};

#pragma endregion

#pragma region Specular BRDF

/* -------------------------------------------------------------------------------------------------
   Specular BRDF models shiny metallic surfaces and highlights. The radiance function, f, depends on
   the angle between the viewer (camera) and the surface normal, as well as the light direction.
   In the Utilities section, you can choose between using the Blinn or Phong model.
   -------------------------------------------------------------------------------------------------
*/
class Specular : public BRDF
{
private:
	Color m_ks;
	float m_exp;

public:
	Specular(Color = Color{}, float = 1);
	
	Color f(const ShaderData&, const Vector<3>&, const Vector<3>&) const override;
	Color rho(const ShaderData&, const Vector<3>&) const override;

	friend class Material;
};

#pragma endregion

#pragma region Emissive BRDF

/* -------------------------------------------------------------------------------------------------
   Emissive BRDF models materials that produce their own light source. Currently the ray tracer
   doesn't compute this as a light source for illuminating other objects, but this could be done
   by implementing a bloom filter on the scene.
   -------------------------------------------------------------------------------------------------
*/
class Emissive : public BRDF
{
private:
	Color m_ke;

public:
	Emissive(Color = Color{});
	
	Color f(const ShaderData&, const Vector<3>&, const Vector<3>&) const override;
	Color rho(const ShaderData&, const Vector<3>&) const override;

	friend class EmissiveMaterial;
};

#pragma endregion

#pragma region Matrial

/* -------------------------------------------------------------------------------------------------
   Default material class for geometry. It can contain an ambient, diffuse, and specular component.
   It also has a reflective color that will tint any reflections that appear on its surface.
   -------------------------------------------------------------------------------------------------
*/
class Material
{
protected:
	Color m_kr;
	Lambertian m_ambientBRDF, m_diffuseBRDF;
	Specular m_specularBRDF;

public:
	Material(Color = Color{}, Color = Color{}, Color = Color{}, float = 0, Color = Color{});

	virtual Color ambient(const ShaderData&, const Light*) const;
	virtual Color diffuse(const ShaderData&, const Light*) const;
	virtual Color specular(const ShaderData&, const Light*) const;
	virtual Color emissive(const ShaderData&, const Light*) const;
	virtual Color reflective() const;

	bool isReflective() const;
	void setka(Color);
	void setkd(Color);
	void setks(Color);
	void setexp(float);
};

#pragma endregion

#pragma region Emissive Material

/* -------------------------------------------------------------------------------------------------
   Emissive Material is the same as a default material except it can also have an emissive BRDF.
   -------------------------------------------------------------------------------------------------
*/
class EmissiveMaterial : public Material
{
private:
	Emissive m_emissiveBRDF;

public:
	EmissiveMaterial(Color = Color{}, Color = Color{}, Color = Color{}, Color = Color{}, float = 0, Color = Color{});

	Color emissive(const ShaderData&, const Light*) const override;
	
	void setke(Color c);
};

#pragma endregion

#pragma region Shader Data

/* -------------------------------------------------------------------------------------------------
   Shader Data is the container class for all shading data. When tracing a ray, shader data keeps
   track of the point the ray hit, the surface normal at that point, the material of the object
   it hit, and the depth of recursion. The Scene class then uses this data to compute the final
   color for the pixel the ray was generated for.
   -------------------------------------------------------------------------------------------------
*/
class ShaderData
{
private:
	int m_depth;
	bool m_hitObject;
	Material m_material;
	Ray m_ray;
	Vector<3> m_normal, m_hitPoint;

public:
	ShaderData();
	ShaderData(const ShaderData&);
	ShaderData(ShaderData&&);

	ShaderData& operator =(ShaderData);
	ShaderData& operator =(ShaderData&&);

	void setDepth(int);
	void setMaterial(Material);
	void setRay(Ray);
	void setNormal(Vector<3>);
	void setHitPoint(Vector<3>);
	int getDepth() const;
	Material getMaterial() const;
	Ray getRay() const;
	Vector<3> getNormal() const;
	Vector<3> getHitPoint() const;
};

#pragma endregion

#endif