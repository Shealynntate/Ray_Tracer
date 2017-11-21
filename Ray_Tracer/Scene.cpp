#include "stdafx.h"
#include "Scene.h"

#pragma region Sampler

Sampler::Sampler(Vector<3> center, int horiz, int vert, float width, float height)
	: m_center{ center }, m_viewport{ width, height }, x{ -1 }, y{ 0 },
	  m_resolution{ new int[2]{horiz, vert} }
{
	
}

Vector<3> Sampler::getNext()
{
	++x;
	if (x == m_resolution[0])
	{
		x = 0;
		y++;
	}

	// TODO: Add DPI scalar factor to x, y (the size of a pixel)
	Vector<3> result{ ((x + 0.5f) * 2.0f / m_resolution[0] - 1.0f) * m_viewport[0],
					  ((y + 0.5f) * 2.0f / m_resolution[1] - 1.0f) * m_viewport[1] };

	return result + m_center;
}

bool Sampler::atEnd()
{
	return y >= m_resolution[1];
}

Sample Sampler::currentIndex()
{
	return Sample(x, y);
}

void Sampler::setCenter(Vector<3> c)
{
	m_center = c;
}

void Sampler::setResolution(int w, int h)
{
	m_resolution[0] = w;
	m_resolution[1] = h;
}

void Sampler::setViewPort(float w, float h)
{
	m_viewport[0] = w;
	m_viewport[1] = h;
}

#pragma endregion

#pragma region Camera

Camera::Camera(Vector<3> origin, PROJECTION projection)
	: m_origin{ origin }, m_projection{ projection }
{

}

Ray Camera::generateRay(Vector<3> coord, Sample sample)
{
	Vector<4> origin, direction;

	if (m_projection == ORTHO)
	{
		origin = m_transform * higherDimension(coord, 1.0f);
		direction = m_transform * Vector<4>{0, 0, -1, 0};
	}
	else
	{
		origin = m_transform * higherDimension(m_origin, 1.0f);
		direction = m_transform * higherDimension(coord, 0.0f);
	}

	Ray r = Ray{ sample };
	r.origin = lowerDimension(origin.homogenous());
	r.direction = lowerDimension(direction).normal();

	return r;
}

void Camera::setTransform(Matrix<4,4> t)
{
	m_transform = t;
}

void Camera::setOrigin(Vector<3> origin)
{
	m_origin = origin;
}

#pragma endregion

#pragma region Film

Film::Film(int width, int height)
	: m_width{ width }, m_height{ height },
	  m_filename{ "defaultOutput.png" },
	  m_pixels{ new Color[width * height] }
{
	std::fill_n(m_pixels, width * height, Color{});
}

Film::Film(const Film& f)
	: m_width{ f.m_width }, m_height{ f.m_height },
	  m_filename{f.m_filename}, 
	  m_pixels{ new Color[f.m_width * f.m_height] }
{
	for (int i = 0; i < m_width * m_height; ++i)
		m_pixels[i] = Color{ f.m_pixels[i] };
}

Film& Film::operator =(const Film& f)
{
	Film result{ f };

	m_width = result.width();
	m_height = result.height();
	std::swap(m_pixels, result.m_pixels);
	std::swap(m_filename, result.m_filename);

	return *this;
}

void Film::displayPixel(Sample sample, Color color)
{
	m_pixels[sample.y * m_width + sample.x] = Color::clamp(color);
}

void Film::display()
{
	glDrawPixels(m_width, m_height, GL_RGB, GL_FLOAT, m_pixels);
}

void Film::outputFile()
{
	FIBITMAP *bitmap = FreeImage_Allocate(m_width, m_height, 24);
	RGBQUAD color;

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			color.rgbRed = (BYTE)(m_pixels[y * m_width + x].r * 255);
			color.rgbGreen = (BYTE)(m_pixels[y * m_width + x].g * 255);
			color.rgbBlue = (BYTE)(m_pixels[y * m_width + x].b * 255);
			FreeImage_SetPixelColor(bitmap, x, y, &color);
		}
	}

	bool result = FreeImage_Save(FIF_PNG, bitmap, m_filename.c_str(), 0);
	if (result)
		std::cout << "Successfully saved file.\n";
	else
		std::cout << "Unable to save file.\n";
}

void Film::setOutputFilename(std::string file)
{
	m_filename = file;
}

void Film::setDimensions(int w, int h)
{
	m_width = w;
	m_height = h;

	// Resetting dimensions will wipe all stored output data
	delete[] m_pixels;

	m_pixels = new Color[m_width * m_height];
	std::fill_n(m_pixels, m_width * m_height, Color{});
}

int Film::width()
{
	return m_width;
}

int Film::height()
{
	return m_height;
}

Film::~Film()
{
	delete[] m_pixels;
}

#pragma endregion

#pragma region Scene

Color Scene::traceRay(const Ray& ray, const std::vector<Geometry*>& geometries, const int depth)
{
	// Recusion base case, return if we've exceeded bounce depth
	if (depth > m_maxDepth)
		return Color{};

	float tMin = MAX_T;
	bool hitObject = false;
	Vector<3> point, normal;
	Material material;
	ShaderData shaderData;
	shaderData.setDepth(depth);
	shaderData.setRay(ray);

	// Loop over all geometries to check for valid ray intersections
	for (auto geo = m_geometries.begin(); geo != m_geometries.end(); ++geo)
	{
		float t = tMin;
		// If ray intersects object at a new minimum t value, update shader data
		if ((*geo)->hit(ray, t, shaderData) && t < tMin)
		{
			material = shaderData.getMaterial();
			point = shaderData.getHitPoint();
			normal = shaderData.getNormal();
			tMin = t;
			hitObject = true;
		}
	}

	if (hitObject)
	{
		// Update shader data to that of intersected object and perform shading calculations
		shaderData.setMaterial(material);
		shaderData.setHitPoint(point);
		shaderData.setNormal(normal);

		// Ambient Shading
		Color color = material.ambient(shaderData, m_ambient);

		// Rest of Shading : loop over each light
		for (auto light = m_lights.begin(); light != m_lights.end(); ++light)
		{
			Vector<3> l = (*light)->direction(shaderData.getHitPoint());
			Color lIntensity = (*light)->light(shaderData.getHitPoint());

			// Get shadow ray from the light source
			bool inShadow = false;
			Ray shadowRay{ shaderData.getHitPoint(), l };
			RayParameters params = (*light)->shadowRay(shadowRay);

			//	Loop over each object in scene to see if it casts shadow
			for (auto geo = m_geometries.begin(); geo != m_geometries.end(); ++geo)
			{
				// As soon as we find one object in the path, we can stop checking
				if ((*geo)->shadowHit(shadowRay, params.t) && params.t < params.d)
				{
					inShadow = true;
					break;
				}
			}

			// Lambertian and Phong Specular Shading
			if (!inShadow)
				color += material.diffuse(shaderData, *light) + material.specular(shaderData, *light);
		}

		// Reflection Shading
		if (shaderData.getMaterial().isReflective())
		{
			auto direction = shaderData.getRay().direction;
			auto r = (direction - normal * 2.0f * direction.dotProduct(normal)).normal();
			Ray reflectedRay{ shaderData.getHitPoint(), r };

			color += material.reflective() * traceRay(reflectedRay, geometries, depth + 1);
		}

		return color;
	}

	return Color{};
}

Scene::Scene(int horizRes, int vertRes, PROJECTION projection, bool gridOn)
    : m_gridOn{ gridOn },
	  m_maxDepth{ 5 },
	  m_projection{ projection },
	  m_grid{ new Grid },
	  m_sampler{ Vector<3>{}, horizRes, vertRes },
	  m_camera{ Vector<3>{}, projection },
	  m_film{ Film(horizRes, vertRes) },
	  m_ambient{ new Ambient() },
	  m_geometries{ std::vector<Geometry*>() },
	  m_lights{ std::vector<Light*>() }
{
	
}

Scene::Scene(Scene&& scene)
	: m_gridOn{ scene.m_gridOn },
	  m_maxDepth{ scene.m_maxDepth },
	  m_projection{ scene.m_projection },
	  m_grid{ scene.m_grid },
	  m_sampler{ scene.m_sampler },
	  m_camera{ scene.m_camera },
	  m_film{ scene.m_film },
	  m_ambient{ scene.m_ambient },
	  m_geometries{ scene.m_geometries },
	  m_lights{ scene.m_lights }
{
	scene.m_geometries = std::vector<Geometry*>{};
	scene.m_lights = std::vector<Light*>{};
}

Scene& Scene::operator =(Scene&& scene)
{
	m_gridOn = scene.m_gridOn;
	m_maxDepth = scene.m_maxDepth;
	m_projection = scene.m_projection;
	m_grid = scene.m_grid;
	m_sampler = scene.m_sampler;
	m_camera = scene.m_camera;
	m_film = scene.m_film;
	m_ambient = scene.m_ambient;
	m_geometries = scene.m_geometries;
	m_lights = scene.m_lights;

	return *this;
}

void Scene::buildMVP(Vector<3> eye, Vector<3> center, Vector<3> up, float fov)
{
	auto dist = (center - eye).magnitude();
	auto top = tanf(fov / 2.0f) * dist;
	auto right = top * m_film.width() / m_film.height();

	//	Spin up an orthonormal basis using given vectors
	auto w = (eye - center).normal();
	auto u = (up.crossProduct(w)).normal();
	auto v = w.crossProduct(u);

	Matrix<4, 4> basis;
	auto translate = Matrix<4,4>::Translation(eye * -1);
	basis[0][0] = u[0]; basis[1][0] = u[1]; basis[2][0] = u[2];
	basis[0][1] = v[0]; basis[1][1] = v[1]; basis[2][1] = v[2];
	basis[0][2] = w[0]; basis[1][2] = w[1]; basis[2][2] = w[2];
	auto transform = basis * translate;

	if (m_projection == ORTHO)
	{
		m_sampler.setCenter(eye);
		m_camera.setOrigin(eye);
		m_camera = Camera(eye, m_projection);
	}
	else
	{
		auto vpCenter = lowerDimension((transform * higherDimension(center, 1.0f)).homogenous());
		m_sampler.setCenter(vpCenter);
		m_camera = Camera(Vector<3>{}, m_projection);
	}

	m_sampler.setResolution(m_film.width(), m_film.height());
	m_sampler.setViewPort(right, top);
	m_camera.setTransform(transform.inverse());
}

void Scene::generateScene()
{
	if (m_gridOn)
	{
		m_grid->generateCells();
		m_geometries.push_back(m_grid);
	}

	auto nextPoint = m_sampler.getNext();
	while (!m_sampler.atEnd())
	{
		Ray r = m_camera.generateRay(nextPoint, m_sampler.currentIndex());
		Color c = traceRay(r, m_geometries, 0);
		m_film.displayPixel(r.sample, c);

		nextPoint = m_sampler.getNext();
	}
}

void Scene::display()
{
	m_film.display();
}

void Scene::outputToFile()
{
	m_film.outputFile();
}

void Scene::addLight(Light *light)
{
	m_lights.push_back(light);
}

void Scene::addGeometry(Geometry *geo)
{
	if (m_gridOn)
		m_grid->addGeometry(geo);
	else
		m_geometries.push_back(geo);
}

void Scene::setScreenDimensions(int width, int height)
{
	m_sampler.setResolution(width, height);
	m_film.setDimensions(width, height);
}

void Scene::setOutputFilename(std::string filename)
{
	m_film.setOutputFilename(filename);
}

void Scene::setMaxDepth(int d)
{
	m_maxDepth = d;
}

int Scene::numGeometries()
{
	return (int)m_geometries.size();
}

int Scene::numLights()
{
	return (int)m_lights.size();
}

int Scene::screenWidth()
{
	return m_film.width();
}

int Scene::screenHeight()
{
	return m_film.height();
}

#pragma endregion
