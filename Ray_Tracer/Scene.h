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
#ifndef SCENE_H
#define SCENE_H

#include <iostream>
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <GLFW\glfw3.h>
#include <FreeImage\FreeImage.h>

#include "Assets.h"

#pragma region Sampler

/* -------------------------------------------------------------------------------------------------
   Sampler is used as an iterator by the Scene to grab each pixel one by one to be converted into rays.
   Its resolution and viewport should be set for starting raytracing to avoid errors in rendering
   -------------------------------------------------------------------------------------------------
*/
class Sampler
{
private:
	Vector<3> m_center;
	Vector<2> m_viewport;
	int x, y, *m_resolution;

public:
	Sampler(Vector<3> = Vector<3>{}, int = 0, int = 0, float = 0, float = 0);
	
	Vector<3> getNext();
	bool atEnd();
	Sample currentIndex();
	void setCenter(Vector<3>);
	void setResolution(int, int);
	void setViewPort(float, float);
};

#pragma endregion

#pragma region Camera

/* -------------------------------------------------------------------------------------------------
   Camera creates each ray using the pixel information from Sampler, the projection mode 
   (perspective or orthographic), and the transform matrix created from the camera's origin, look at
   vector, and field of view  amount provided in the input file.
   -------------------------------------------------------------------------------------------------
*/

class Camera
{
private:
	PROJECTION m_projection;
	Vector<3> m_origin;
	Matrix<4,4> m_transform;

public:
	Camera(Vector<3> = Vector<3>(), PROJECTION = ORTHO);

	Ray generateRay(Vector<3>, Sample);
	void setTransform(Matrix<4,4>);
	void setOrigin(Vector<3>);
};

#pragma endregion

#pragma region Film

/* -------------------------------------------------------------------------------------------------
   Film stores the array of rgb values that the ray tracer computes for each pixel. It can then 
   store the result into a file and/or blit it to the display window using glDrawPixels.
   -------------------------------------------------------------------------------------------------
*/

class Film
{
private:
	Color *m_pixels;
	std::string m_filename;
	int m_width, m_height;

public:
	Film(int, int);

	Film(const Film&);
	Film& operator =(const Film&);

	void displayPixel(Sample, Color);
	void display();
	void outputFile();
	void setOutputFilename(std::string);
	void setDimensions(int, int);
	int width();
	int height();

	~Film();
};

#pragma endregion

#pragma region Scene

/* -------------------------------------------------------------------------------------------------
   Scene is the main ray tracing class. It hold lists of the geometries and lights in the scene and
   loops over them in traceRay to compute the shader data for each ray. traceRay is a recursive 
   function, and can compute reflective surfaces up to maxDepth levels of bounced rays.
   Tracing a Ray:
       - A ray is created by the Camera class
	   - traceRay then loops over all geometry in the scene to find the closest intersection of an
	     object and the ray (if any)
	   - If it does hit an object, ambient color is computed
	   - Then traceRay loops over all lights in the scene, if there is an unobstructed path from the
	     hitPoint to the light, then diffuse and specular shading are added to the color.
	   - If the material is reflective and we haven't exceeded max depth, then traceRay recurses and
	     we add future computed shading data to the color
	   - The resulting color is stored in Film's pixel array.
   Notes:
       - Currently the ambient light is set to a default (1, 1, 1) color value. Can change to give 
	     scenes a colored tint.
-------------------------------------------------------------------------------------------------
*/

class Scene
{
private:
	bool m_gridOn;
	int m_maxDepth;
	PROJECTION m_projection;
	Grid *m_grid;
	Sampler m_sampler;
	Camera m_camera;
	Film m_film;
	Ambient* m_ambient;
	std::vector<Geometry*> m_geometries;
	std::vector<Light*> m_lights;

	Color traceRay(const Ray&, const std::vector<Geometry*>&, const int);

public:
	Scene(int = SCREEN_WIDTH, int = SCREEN_HEIGHT, PROJECTION = PERSPECTIVE, bool = true);
	
	// Only one scene created in program
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	Scene(Scene&&);
	Scene& operator =(Scene&&);

	void buildMVP(Vector<3>, Vector<3>, Vector<3>, float);
	void generateScene();
	void display();
	void outputToFile();
	void addLight(Light*);
	void addGeometry(Geometry*);
	void setScreenDimensions(int, int);
	void setOutputFilename(std::string);
	void setMaxDepth(int);
	int numGeometries();
	int numLights();
	int screenWidth();
	int screenHeight();
};

#pragma endregion

#endif