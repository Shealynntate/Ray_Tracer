#include "stdafx.h"
#include "Assets.h"

#include <typeinfo>

#pragma region Bounding Box

BoundingBox::BoundingBox()
	: min{ -1, -1, -1 }, max{ 1, 1, 1 }
{

}

bool BoundingBox::hit(const Ray &ray) const
{
	float ox = ray.origin[0]; float oy = ray.origin[1]; float oz = ray.origin[2];
	float dx = ray.direction[0]; float dy = ray.direction[1]; float dz = ray.direction[2];

	float txMin, tyMin, tzMin;
	float txMax, tyMax, tzMax;

	float a = 1.0f / dx;
	float b = 1.0f / dy;
	float c = 1.0f / dz;

	txMin = (a >= 0.0f) ? (min[0] - ox) * a : (max[0] - ox) * a;
	txMax = (a >= 0.0f) ? (max[0] - ox) * a : (min[0] - ox) * a;

	tyMin = (b >= 0.0f) ? (min[1] - oy) * b : (max[1] - oy) * b;
	tyMax = (b >= 0.0f) ? (max[1] - oy) * b : (min[1] - oy) * b;

	tzMin = (c >= 0.0f) ? (min[2] - oz) * c : (max[2] - oz) * c;
	tzMax = (c >= 0.0f) ? (max[2] - oz) * c : (min[2] - oz) * c;

	float t0, t1;

	t0 = (txMin > tyMin) ? txMin : tyMin;
	if (tzMin > t0) t0 = tzMin;

	t1 = (txMax < tyMin) ? txMax : tyMax;
	if (tzMax < t1) t1 = tzMax;

	return (t0 < t1 && t1 > MIN_T);
}

bool BoundingBox::inside(const Vector<3>& p) const
{
	return p > min && p < max;
}

void BoundingBox::updateMin(const Vector<3>& v)
{
	min[0] = fminf(min[0], v[0]);
	min[1] = fminf(min[1], v[1]);
	min[2] = fminf(min[2], v[2]);
}

void BoundingBox::updateMax(const Vector<3>& v)
{
	max[0] = fmaxf(max[0], v[0]);
	max[1] = fmaxf(max[1], v[1]);
	max[2] = fmaxf(max[2], v[2]);
}

#pragma endregion

#pragma region Geometry

Geometry::Geometry(Material mat, Matrix<4, 4> inv)
	: material{ mat }, invTransform{ inv }, invTranspose{ inv.transposition() }, boundingBox {}
{

}

BoundingBox Geometry::getBoundingBox()
{
	return boundingBox;
}

void Geometry::setBoundingBox()
{
	//TODO
}

void Geometry::addGeometry(Geometry *geo)
{

}

Material Geometry::getMaterial()
{
	return material;
}

void Geometry::setMaterial(Material m)
{
	material = m;
}

Geometry::~Geometry()
{

}

#pragma endregion

#pragma region Sphere Geometry

Sphere::Sphere(Vector<3> center, float radius, Material mat, Matrix<4,4> inv)
	: Geometry{ mat, inv }, center{ center }, radius{ radius }
{

}

void Sphere::generateBoundingBox(Matrix<4,4> transform)
{
	Vector<3> v[8];

	v[0][0] = boundingBox.min[0]; v[0][1] = boundingBox.min[1]; v[0][2] = boundingBox.min[2];
	v[1][0] = boundingBox.max[0]; v[1][1] = boundingBox.min[1]; v[1][2] = boundingBox.min[2];
	v[2][0] = boundingBox.max[0]; v[2][1] = boundingBox.max[1]; v[2][2] = boundingBox.min[2];
	v[3][0] = boundingBox.min[0]; v[3][1] = boundingBox.max[1]; v[3][2] = boundingBox.min[2];
	v[4][0] = boundingBox.min[0]; v[4][1] = boundingBox.min[1]; v[4][2] = boundingBox.max[2];
	v[5][0] = boundingBox.max[0]; v[5][1] = boundingBox.min[1]; v[5][2] = boundingBox.max[2];
	v[6][0] = boundingBox.max[0]; v[6][1] = boundingBox.max[1]; v[6][2] = boundingBox.max[2];
	v[7][0] = boundingBox.min[0]; v[7][1] = boundingBox.max[1]; v[7][2] = boundingBox.max[2];

	v[0] = lowerDimension((transform * higherDimension(v[0], 1.0f)).homogenous());
	v[1] = lowerDimension((transform * higherDimension(v[1], 1.0f)).homogenous());
	v[2] = lowerDimension((transform * higherDimension(v[2], 1.0f)).homogenous());
	v[3] = lowerDimension((transform * higherDimension(v[3], 1.0f)).homogenous());
	v[4] = lowerDimension((transform * higherDimension(v[4], 1.0f)).homogenous());
	v[5] = lowerDimension((transform * higherDimension(v[5], 1.0f)).homogenous());
	v[6] = lowerDimension((transform * higherDimension(v[6], 1.0f)).homogenous());
	v[7] = lowerDimension((transform * higherDimension(v[7], 1.0f)).homogenous());

	float x0 = MAX_T;
	float y0 = MAX_T;
	float z0 = MAX_T;

	for (int j = 0; j <= 7; j++)
	{
		if (v[j][0] < x0) x0 = v[j][0];
		if (v[j][1] < y0) y0 = v[j][1];
		if (v[j][2] < z0) z0 = v[j][2];
	}

	float x1 = -MAX_T;
	float y1 = -MAX_T;
	float z1 = -MAX_T;

	for (int j = 0; j <= 7; j++)
	{
		if (v[j][0] > x1) x1 = v[j][0];
		if (v[j][1] > y1) y1 = v[j][1];
		if (v[j][2] > z1) z1 = v[j][2];
	}

	boundingBox.min[0] = x0;
	boundingBox.min[1] = y0;
	boundingBox.min[2] = z0;
	boundingBox.max[0] = x1;
	boundingBox.max[1] = y1;
	boundingBox.max[2] = z1;
}

bool Sphere::hitCalculations(Ray& localRay, float& a, float& b, float& e, Vector<3>& diff) const
{ 
	auto origin = invTransform * higherDimension(localRay.origin, 1.0f);
	auto direction = invTransform * higherDimension(localRay.direction, 0);
	origin.homogenize();

	localRay.origin = lowerDimension(origin);
	localRay.direction = lowerDimension(direction);

	diff = localRay.origin - center;
	a = localRay.direction.dotProduct(localRay.direction);
	b = diff.dotProduct(localRay.direction) * 2.0f;
	float c = diff.dotProduct(diff) - radius * radius;
	float disc = b * b - 4.0f * a * c;

	if (disc < 0)
		return false;

	e = sqrt(disc);

	return true;
}

bool Sphere::hit(const Ray &ray, float &tMin, ShaderData &shaderData) const
{
	float a, b, e;
	Vector<3> diff;
	Ray localRay{ ray };
	bool result = hitCalculations(localRay, a, b, e, diff);

	if (!result)
		return false;

	float denom = 2.0f * a;

	float hit0 = (-b - e) / denom;
	if (hit0 > MIN_T)
	{
		tMin = hit0;
		auto localNormal = (diff + localRay.direction * hit0) / radius;
		auto transNormal = invTranspose * higherDimension(localNormal, 0);
		shaderData.setNormal(lowerDimension(transNormal).normal());
		shaderData.setHitPoint(ray.origin + ray.direction * hit0);
		shaderData.setMaterial(material);

		return true;
	}

	float hit1 = (-b + e) / denom;
	if (hit1 > MIN_T)
	{
		tMin = hit1;
		auto localNormal = (diff + localRay.direction * hit1) / radius;
		auto transNormal = invTranspose * higherDimension(localNormal, 0);
		shaderData.setNormal(lowerDimension(transNormal).normal());
		shaderData.setHitPoint(ray.origin + ray.direction * hit1);
		shaderData.setMaterial(material);

		return true;
	}
	return false;
}

bool Sphere::shadowHit(const Ray& ray, float& tMin) const
{
	float a, b, e;
	Vector<3> diff;
	Ray localRay = Ray(ray);
	bool result = hitCalculations(localRay, a, b, e, diff);

	if (!result)
		return false;

	float denom = 2.0f * a;

	float hit0 = (-b - e) / denom;
	if (hit0 > MIN_T)
	{
		tMin = hit0;
		return true;
	}
	float hit1 = (-b + e) / denom;
	if (hit1 > MIN_T)
	{
		tMin = hit1;
		return true;
	}
	return false;
}

#pragma endregion

#pragma region Triangle Geometry

Triangle::Triangle(Vector<3> v0, Vector<3> v1, Vector<3> v2, Material mat, Matrix<4,4> inv)
	: Geometry(mat, inv), v0(v0), v1(v1), v2(v2), useTransform(false)
{
	auto cross = higherDimension((v1 - v0).crossProduct(v2 - v0), 0);
	m_normal = lowerDimension(invTranspose * cross).normal();
}

bool Triangle::hitCalculations(Ray& loc, float& tMin) const
{
	if (useTransform)
	{
		auto origin = invTransform * higherDimension(loc.origin, 1.0f);
		auto direction = invTransform * higherDimension(loc.direction, 0);
		origin.homogenize();

		loc.origin = lowerDimension(origin);
		loc.direction = lowerDimension(direction);
	}

	float a = v0[0] - v1[0], b = v0[0] - v2[0], c = loc.direction[0], d = v0[0] - loc.origin[0];
	float e = v0[1] - v1[1], f = v0[1] - v2[1], g = loc.direction[1], h = v0[1] - loc.origin[1];
	float i = v0[2] - v1[2], j = v0[2] - v2[2], k = loc.direction[2], l = v0[2] - loc.origin[2];

	float m = f * k - g * j, n = h * k - g * l, p = f * l - h * j;
	float q = g * i - e * k, s = e * j - f * i;

	float inv_denom = 1.0f / (a * m + b * q + c * s);

	float e1 = d * m - b* n - c * p;
	float beta = e1 * inv_denom;

	if (beta < 0.0f)
		return false;

	float r = e * l - h * i;
	float e2 = a * n + d * q + c * r;
	float gamma = e2 * inv_denom;

	if (gamma < 0.0f)
		return false;

	if (beta + gamma > 1.0f)
		return false;

	float e3 = a * p - b * r + d * s;
	float t = e3 * inv_denom;

	if (t < MIN_T)
		return false;

	tMin = t;
	return true;
}

bool Triangle::hit(const Ray& ray, float& tMin, ShaderData& shaderData) const
{
	Ray loc = Ray(ray);
	bool result = hitCalculations(loc, tMin);

	if (result)
	{
		shaderData.setHitPoint(ray.origin + ray.direction * tMin);
		shaderData.setNormal(m_normal);
		shaderData.setMaterial(material);
	}

	return result;
}

bool Triangle::shadowHit(const Ray& ray, float& tMin) const
{
	Ray local = Ray(ray);
	return hitCalculations(local, tMin);
}

void Triangle::generateBoundingBox(Matrix<4,4> inv)
{
	float epsilon = 0.5f;

	boundingBox.min = Vector<3>{ MAX_T, MAX_T, MAX_T };
	boundingBox.max = Vector<3>{ -MIN_T, -MIN_T, -MIN_T };

	boundingBox.updateMin(v0);
	boundingBox.updateMin(v1);
	boundingBox.updateMin(v2);

	boundingBox.updateMax(v0);
	boundingBox.updateMax(v1);
	boundingBox.updateMax(v2);

	boundingBox.min -= epsilon;
	boundingBox.max += epsilon;
}

void Triangle::setUseTransform(bool f)
{
	useTransform = f;
}

#pragma endregion

#pragma region Compound Geometry

Compound::Compound()
	: Geometry{ Material{}, Matrix<4, 4>{} }, geometries{}
{

}

Compound::Compound(const Compound &c)
	: Geometry(c.material, c.invTransform), geometries{geometries.size()}
{
	for (unsigned i = 0; i < c.geometries.size(); ++i)
		geometries[i] = c.geometries[i];
}

Compound& Compound::operator =(const Compound& c)
{
	Compound result{ c };
	geometries = result.geometries;

	return *this;
}

void Compound::addGeometry(Geometry *geo)
{
	geometries.push_back(geo);
}

bool Compound::hit(const Ray& ray, float& tMin, ShaderData& sd) const
{
	Vector<3> normal, hitPoint;
	Material m{};
	bool hit = false;

	for (unsigned i = 0; i < geometries.size(); ++i)
	{
		float t = tMin;
		if (geometries[i]->hit(ray, t, sd) && t < tMin)
		{
			hit = true;
			tMin = t;
			m = sd.getMaterial();
			normal = sd.getNormal();
			hitPoint = sd.getHitPoint();
			
		}
	}

	if (hit)
	{
		sd.setNormal(normal);
		sd.setHitPoint(hitPoint);
		sd.setMaterial(m);
	}

	return hit;
}

bool Compound::shadowHit(const Ray& ray, float& tMin) const
{
	bool hit = false;

	for (unsigned i = 0; i < geometries.size(); ++i)
	{
		float t = tMin;
		if (geometries[i]->shadowHit(ray, t) && t < tMin)
		{
			hit = true;
			tMin = t;
			break;
		}
	}

	return hit;
}

void Compound::generateBoundingBox(Matrix<4, 4> m)
{
	// TODO
}

BoundingBox Compound::getBoundingBox()
{
	boundingBox.min = Vector<3>{ MAX_T, MAX_T, MAX_T };
	boundingBox.max = Vector<3>{ -MAX_T, -MAX_T, -MAX_T };
	
	for (unsigned i = 0; i < geometries.size(); ++i)
	{
		BoundingBox box = geometries[i]->getBoundingBox();

		boundingBox.updateMin(box.min);
		boundingBox.updateMax(box.max);
	}

	return boundingBox;
}

#pragma endregion

#pragma region Grid Compound Geometry

Grid::Grid()
{

}

Vector<3> Grid::minCoordinate()
{
	float epsilon = 0;
	Vector<3> point{ MAX_T, MAX_T, MAX_T };
	
	for (unsigned i = 0; i < geometries.size(); ++i)
	{
		BoundingBox box = geometries[i]->getBoundingBox();

		point[0] = fminf(point[0], box.min[0]);
		point[1] = fminf(point[1], box.min[1]);
		point[2] = fminf(point[2], box.min[2]);
	}

	point -= epsilon;
	
	return point;
}

Vector<3> Grid::maxCoordinate()
{
	float epsilon = 0;
	Vector<3> point{ -MAX_T, -MAX_T, -MAX_T };
	
	for (unsigned i = 0; i < geometries.size(); ++i)
	{
		BoundingBox box = geometries[i]->getBoundingBox();

		point[0] = fmaxf(point[0], box.max[0]);
		point[1] = fmaxf(point[1], box.max[1]);
		point[2] = fmaxf(point[2], box.max[2]);
	}

	point += epsilon;
	
	return point;
}

BoundingBox Grid::getBoundingBox()
{
	return boundingBox;
}

void Grid::generateCells()
{
	auto min = minCoordinate();
	auto max = maxCoordinate();

	boundingBox.min = minCoordinate();
	boundingBox.max = maxCoordinate();

	int numObjects = geometries.size();
	float dimx = max[0] - min[0];
	float dimy = max[1] - min[1];
	float dimz = max[2] - min[2];
	float multiplier = 2.0f;
	float side = pow(dimx * dimy * dimz / numObjects, 0.333333f);
	nx = (int)round(multiplier * dimx / side) + 1;
	ny = (int)round(multiplier * dimy / side) + 1;
	nz = (int)round(multiplier * dimz / side) + 1;

	int numCells = nx * ny * nz;
	cells.reserve(numObjects);

	for (int i = 0; i < numCells; ++i)
		cells.push_back(NULL);

	std::vector<int> counts;
	counts.reserve(numCells);

	for (int i = 0; i < numCells; i++)
		counts.push_back(0);

	BoundingBox objectBox;
	int index;

	for (int i = 0; i < numObjects; i++)
	{
		objectBox = geometries[i]->getBoundingBox();
		int ixmin = clamp((objectBox.min[0] - min[0]) * nx / dimx, 0, nx - 1);
		int iymin = clamp((objectBox.min[1] - min[1]) * ny / dimy, 0, ny - 1);
		int izmin = clamp((objectBox.min[2] - min[2]) * nz / dimz, 0, nz - 1);
		int ixmax = clamp(round(objectBox.max[0] - min[0]) * nx / dimx, 0, nx - 1);
		int iymax = clamp(round(objectBox.max[1] - min[1]) * ny / dimy, 0, ny - 1);
		int izmax = clamp(round(objectBox.max[2] - min[2]) * nz / dimz, 0, nz - 1);

		for (int iz = izmin; iz <= izmax; iz++)
		{
			for (int iy = iymin; iy <= iymax; iy++)
			{
				for (int ix = ixmin; ix <= ixmax; ix++)
				{
					index = nx * ny * iz + nx * iy + ix;

					if (counts[index] == 0)
					{
						cells[index] = geometries[i];
						counts[index]++;
					}
					else
					{
						// If more than one object in a cell, create a compound geometry
						if (counts[index] == 1)
						{
							Compound *compound = new Compound;
							compound->addGeometry(cells[index]);
							compound->addGeometry(geometries[i]);
							cells[index] = compound;
							counts[index]++;
						}
						else
						{
							cells[index]->addGeometry(geometries[i]);
							counts[index]++;
						}
					}
				}
			}
		}
	}
	geometries.erase(geometries.begin(), geometries.end());
	counts.erase(counts.begin(), counts.end());
}

bool Grid::hitCalculations(const Ray& ray, float& tMin, GridData& gd) const
{
	float ox = ray.origin[0];
	float oy = ray.origin[1];
	float oz = ray.origin[2];
	float dx = ray.direction[0];
	float dy = ray.direction[1];
	float dz = ray.direction[2];

	float x0 = boundingBox.min[0];
	float y0 = boundingBox.min[1];
	float z0 = boundingBox.min[2];
	float x1 = boundingBox.max[0];
	float y1 = boundingBox.max[1];
	float z1 = boundingBox.max[2];

	float txMin, tyMin, tzMin;
	float txMax, tyMax, tzMax;

	float a = 1.0f / dx;
	txMin = (a >= 0) ? (x0 - ox) * a : (x1 - ox) * a;
	txMax = (a >= 0) ? (x1 - ox) * a : (x0 - ox) * a;

	float b = 1.0f / dy;
	tyMin = (b >= 0) ? (y0 - oy) * b : (y1 - oy) * b;
	tyMax = (b >= 0) ? (y1 - oy) * b : (y0 - oy) * b;

	float c = 1.0f / dz;
	tzMin = (c >= 0) ? (z0 - oz) * c : (z1 - oz) * c;
	tzMax = (c >= 0) ? (z1 - oz) * c : (z0 - oz) * c;

	float t0, t1;

	t0 = (txMin > tyMin) ? txMin : tyMin;
	if (tzMin > t0)
		t0 = tzMin;

	t1 = (txMax < tyMax) ? txMax : tyMax;
	if (tzMax < t1)
		t1 = tzMax;

	if (t0 > t1)
		return false;

	float dimx = x1 - x0;
	float dimy = y1 - y0;
	float dimz = z1 - z0;
	if (boundingBox.inside(ray.origin))
	{
		gd.ix = clamp((ox - x0) * nx / dimx, 0, nx - 1);
		gd.iy = clamp((oy - y0) * ny / dimy, 0, ny - 1);
		gd.iz = clamp((oz - z0) * nz / dimz, 0, nz - 1);
	}
	else
	{
		Vector<3> p = ray.origin + ray.direction * t0;
		gd.ix = clamp((p[0] - x0) * nx / (dimx - 0), 0, nx - 1);
		gd.iy = clamp((p[1] - y0) * ny / (dimy - 0), 0, ny - 1);
		gd.iz = clamp((p[2] - z0) * nz / (dimz - 0), 0, nz - 1);
	}

	gd.dtx = (txMax - txMin) / nx;
	gd.dty = (tyMax - tyMin) / ny;
	gd.dtz = (tzMax - tzMin) / nz;

	if (dx > 0)
	{
		gd.txNext = txMin + (gd.ix + 1) * gd.dtx;
		gd.ixStep = 1;
		gd.ixStop = nx;
	}
	else if (dx == 0.0)
	{
		gd.txNext = MAX_T;
		gd.ixStep = -1;
		gd.ixStop = -1;
	}
	else
	{
		gd.txNext = txMin + (nx - gd.ix) * gd.dtx;
		gd.ixStep = -1;
		gd.ixStop = -1;
	}

	if (dy > 0)
	{
		gd.tyNext = tyMin + (gd.iy + 1) * gd.dty;
		gd.iyStep = 1;
		gd.iyStop = ny;
	}
	else if (dy == 0.0)
	{
		gd.tyNext = MAX_T;
		gd.iyStep = -1;
		gd.iyStop = -1;
	}
	else
	{
		gd.tyNext = tyMin + (ny - gd.iy) * gd.dty;
		gd.iyStep = -1;
		gd.iyStop = -1;
	}

	if (dz > 0)
	{
		gd.tzNext = tzMin + (gd.iz + 1) * gd.dtz;
		gd.izStep = 1;
		gd.izStop = nz;
	}
	else if (dz == 0.0)
	{
		gd.tzNext = MAX_T;
		gd.izStep = -1;
		gd.izStop = -1;
	}
	else
	{
		gd.tzNext = tzMin + (nz - gd.iz) * gd.dtz;
		gd.izStep = -1;
		gd.izStop = -1;
	}

	return true;
}

bool Grid::hit(const Ray& ray, float& t, ShaderData& sd) const
{
	GridData gd;
	bool result = hitCalculations(ray, t, gd);

	if (!result)
		return false;

	while (true)
	{
		float tNew = t;
		Geometry *geo = cells[gd.ix + nx * gd.iy + nx * ny * gd.iz];

		if (gd.txNext < gd.tyNext && gd.txNext < gd.tzNext)
		{
			if (geo && geo->hit(ray, tNew, sd) && tNew < gd.txNext)
			{
				t = tNew;
				return true;
			}

			gd.txNext += gd.dtx;
			gd.ix += gd.ixStep;

			if (gd.ix == gd.ixStop)
				return false;
		}
		else
		{
			if (gd.tyNext < gd.tzNext)
			{
				if (geo && geo->hit(ray, tNew, sd) && tNew < gd.tyNext)
				{
					t = tNew;
					return true;
				}

				gd.tyNext += gd.dty;
				gd.iy += gd.iyStep;

				if (gd.iy == gd.iyStop)
					return false;
			}
			else
			{
				if (geo && geo->hit(ray, tNew, sd) && tNew < gd.tzNext)
				{
					t = tNew;
					return true;
				}

				gd.tzNext += gd.dtz;
				gd.iz += gd.izStep;

				if (gd.iz == gd.izStop)
					return false;
			}
		}
	}
}

bool Grid::shadowHit(const Ray& ray, float& tMin) const
{
	GridData gd;
	bool result = hitCalculations(ray, tMin, gd);

	if (!result)
		return false;

	while (true)
	{
		Geometry *geo = cells[gd.ix + nx * gd.iy + nx * ny * gd.iz];
		float tNew = tMin;

		if (gd.txNext < gd.tyNext && gd.txNext < gd.tzNext)
		{
			if (geo && geo->shadowHit(ray, tNew) && tNew < gd.txNext)
			{
				tMin = tNew;
				return true;
			}
			gd.txNext += gd.dtx;
			gd.ix += gd.ixStep;

			if (gd.ix == gd.ixStop)
				return false;
		}
		else
		{
			if (gd.tyNext < gd.tzNext)
			{
				if (geo && geo->shadowHit(ray, tNew) && tNew < gd.tyNext)
				{
					tMin = tNew;
					return true;
				}
				gd.tyNext += gd.dty;
				gd.iy += gd.iyStep;

				if (gd.iy == gd.iyStop)
					return false;
			}
			else
			{
				if (geo && geo->shadowHit(ray, tNew) && tNew < gd.tzNext)
				{
					tMin = tNew;
					return true;
				}

				gd.tzNext += gd.dtz;
				gd.iz += gd.izStep;

				if (gd.iz == gd.izStop)
					return false;
			}
		}
	}

}

#pragma endregion