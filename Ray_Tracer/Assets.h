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
#ifndef ASSETS_H
#define ASSETS_H

#include "RenderData.h"

#pragma region Grid Data

/* -------------------------------------------------------------------------------------------------
   Struct to store grid cell data used in linear grid acceleration.
   -------------------------------------------------------------------------------------------------
*/
struct GridData
{
	int ix, iy, iz;
	float dtx, dty, dtz;
	float 	txNext, tyNext, tzNext;
	int 	ixStep, iyStep, izStep;
	int 	ixStop, iyStop, izStop;
};

#pragma endregion

#pragma region Bounding Box

/* -------------------------------------------------------------------------------------------------
   Bounding Box for a Geometry object. A simple Axis-aligned box that contains the entire geometric
   shape, used for linear grid acceleration. Rays check to see if they intersect the box in world
   space before doing more costly transformations and hit calculations.
   -------------------------------------------------------------------------------------------------
*/
class BoundingBox
{
public:
	Vector<3> min, max;
	
	BoundingBox();
	bool hit(const Ray&) const;
	bool inside(const Vector<3>&) const;
	
	void updateMin(const Vector<3>&);
	void updateMax(const Vector<3>&);
};

#pragma endregion

/* -------------------------------------------------------------------------------------------------
   Abstract base class for Geometric object in the scene. Requires an object have a material for
   shading, a matrix to transform to its local space and the corresponding inverse, as well as a 
   bounding box for linear grid acceleration.
   -------------------------------------------------------------------------------------------------
*/
#pragma region Geometry

class Geometry
{
protected:
	Material material;
	BoundingBox boundingBox;
	Matrix<4, 4> invTransform;
	Matrix<4, 4> invTranspose;

public:
	Geometry(Material, Matrix<4, 4>);

	virtual bool hit(const Ray&, float&, ShaderData&) const = 0;
	virtual bool shadowHit(const Ray&, float&) const = 0;
	virtual void generateBoundingBox(Matrix<4,4>) = 0;
	virtual void setBoundingBox();
	virtual BoundingBox getBoundingBox();
	virtual void addGeometry(Geometry*);

	virtual Material getMaterial();
	virtual void setMaterial(Material);
	
	virtual ~Geometry();
};

#pragma endregion

#pragma region Sphere Geometry

/* -------------------------------------------------------------------------------------------------
   Sphere geometry class. Spheres are defined by a 3d vector center point and a radius value.
   -------------------------------------------------------------------------------------------------
*/
class Sphere : public Geometry
{
private:
	Vector<3> center;
	float radius;

	bool hitCalculations(Ray&, float&, float&, float&, Vector<3>&) const;

public:
	Sphere(Vector<3>, float, Material, Matrix<4,4>);

	bool hit(const Ray&, float&, ShaderData&) const override;
	bool shadowHit(const Ray&, float&) const override;
	void generateBoundingBox(Matrix<4,4>) override;
};

#pragma endregion

#pragma region Triangle Geometry

/* -------------------------------------------------------------------------------------------------
   Triangle geometry class. Triangles are defined by three 3d vectors, 3 computed normals using
   those vectors and by default are converted into world space to make ray intersections more 
   efficient.
   -------------------------------------------------------------------------------------------------
*/
class Triangle : public Geometry
{
private:
	Vector<3> v0, v1, v2;
	Vector<3> m_normal;
	bool useTransform;

	bool hitCalculations(Ray&, float&) const;

public:
	Triangle(Vector<3>, Vector<3>, Vector<3>, Material, Matrix<4,4>);

	bool hit(const Ray&, float&, ShaderData&) const override;
	bool shadowHit(const Ray&, float&) const override;
	void generateBoundingBox(Matrix<4,4>) override;
	
	void setUseTransform(bool);
};

#pragma endregion

#pragma region Compound Geometry

/* -------------------------------------------------------------------------------------------------
   Compound geometry class used in linear grid acceleration. When multiple geometries occupy a grid
   cell, they are put into a compound instance. When calculating a ray intersection, the compound
   loops over all contained geometries to find the correct intersection point.
   -------------------------------------------------------------------------------------------------
*/
class Compound : public Geometry
{
protected:
	std::vector<Geometry*> geometries;

public:
	Compound();

	Compound(const Compound&);
	Compound& operator =(const Compound&);
	
	bool hit(const Ray&, float&, ShaderData&) const override;
	bool shadowHit(const Ray&, float&) const override;
	void generateBoundingBox(Matrix<4, 4>) override;

	BoundingBox getBoundingBox() override;
	void addGeometry(Geometry*) override;
};

#pragma endregion

#pragma region Grid Compound Geometry

/* -------------------------------------------------------------------------------------------------
   Grid geometry class. A scene using linear grid acceleration has only one grid. It uses three 
   values in the x, y, and z planes to generate a 3 dimentional grid with that many cells.
   It then adds the rest of the scene's object into these cells based on the intersection of the
   cell volume and the object's bounding box.
   -------------------------------------------------------------------------------------------------
*/
class Grid : public Compound
{
private:
	std::vector<Geometry*> cells;
	int nx, ny, nz;

	Vector<3> minCoordinate();
	Vector<3> maxCoordinate();
	bool hitCalculations(const Ray&, float&, GridData&) const;

public:
	Grid();

	virtual BoundingBox getBoundingBox();
	bool hit(const Ray&, float&, ShaderData&) const override;
	bool shadowHit(const Ray&, float&) const override;
	
	void generateCells();
};

#pragma endregion

#endif