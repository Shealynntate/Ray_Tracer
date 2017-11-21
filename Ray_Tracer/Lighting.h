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
#ifndef LIGHTING_H
#define LIGHTING_H

#include "Utilities.h"

#pragma region Light Base Class

/* -------------------------------------------------------------------------------------------------
   Abstract base class for the different types of lights in a scene. All lights must return a 
   direction vector towards themselves, their light color, and compute a ray towards themselves
   so the scene can check for shadows.
-------------------------------------------------------------------------------------------------
*/
class Light
{
protected:
	Color m_color;
	bool m_shadows;
	float m_ls;

	Light(Color, float);

public:
	virtual Vector<3> direction(const Vector<3>&) const = 0;
	virtual Color light(const Vector<3>&) const = 0;
	virtual RayParameters shadowRay(const Ray&) const = 0;
};

#pragma endregion

#pragma region Ambient Light

/* -------------------------------------------------------------------------------------------------
   Ambient light is a global illumination for all objects to quickly simulate all bound, incident,
   reflected light in a scene. Consequently there is no meaningful direction for it and it can't
   be in shadow. There's only one per scene.
   -------------------------------------------------------------------------------------------------
*/
class Ambient : public Light
{
public:
	Ambient(float = 1.0f, Color = Color{ 1.0f, 1.0f, 1.0f });
	
	Vector<3> direction(const Vector<3>&) const override;
	Color light(const Vector<3>&) const override;
	RayParameters shadowRay(const Ray&) const override;
};

#pragma endregion

#pragma region Directional Light

/* -------------------------------------------------------------------------------------------------
   Directional lights simulate light from a far away source, e.g. the sun. Consequently they have
   a uniform direction vector for all objects in the scene, and their origin point can be thought
   of as inifinitely far away. A scene can have multiple directional lights.
   -------------------------------------------------------------------------------------------------
*/
class Directional : public Light
{
private:
	Vector<3> m_dir;

public:
	Directional(float = 0, Color = Color{}, Vector<3> = Vector<3>{});
	
	Vector<3> direction(const Vector<3>&) const override;
	Color light(const Vector<3>&) const override;
	RayParameters shadowRay(const Ray&) const override;
};

#pragma endregion

#pragma region Point Light

/* -------------------------------------------------------------------------------------------------
   Point lights simulate a light coming from a specific point in 3d space. Consequently the
   direction of the light changes for each surface point of the geometry and the light is subject
   to falloff, simulated by the attenuation parameter and distance from the object. A scene can have
   multiple point lights.
   -------------------------------------------------------------------------------------------------
*/
class Point : public Light
{
	Vector<3> m_position, m_attenuation;

public:
	Point(float = 0, Color = Color{}, Vector<3> = Vector<3>{}, Vector<3> = Vector<3>{ 1, 0, 0 });

	Vector<3> direction(const Vector<3>&) const override;
	Color light(const Vector<3>&) const override;
	RayParameters shadowRay(const Ray&) const override;

	void setAttenuation(Vector<3>);
};

#pragma endregion

#endif