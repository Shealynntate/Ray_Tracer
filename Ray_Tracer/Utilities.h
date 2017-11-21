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
#ifndef UTILITIES_H
#define UTILITIES_H

#include <GraphicsMathLib\Matrix.h>
#include <regex>

using namespace GraphicsMath;

#pragma region Global Constants 

static const float MIN_T = 0.001f;
static const float MAX_T = 999999999.0f;
static const float M_PI = 3.141592f;

static const int SCREEN_WIDTH = 256;
static const int SCREEN_HEIGHT = 256;


enum PROJECTION { ORTHO, PERSPECTIVE };
enum SPECULAR { BLINN, PHONG };

static const SPECULAR SPECULAR_MODEL = BLINN;

class ShaderData;

#pragma endregion

#pragma region Regex Patterns

//	Regex patterns for parsing input files to construct scene
static const std::string start = "\\s*";
static const std::string num = "\\s+([\\+-]?\\d+\\.\\d*|[\\+-]?\\.\\d+|[\\+-]?\\d+)";
static const std::string end = "\\s*";

static const std::regex camera(start + "camera" + num + num + num + num + num + num + num + num + num + num + end);
static const std::regex size(start + "size" + num + num + end);
static const std::regex depth(start + "maxdepth" + num + end);

static const std::regex output(start + "output" + "\\s+([A-Za-z0-9_-]+\\.png)" + end);

static const std::regex sphere(start + "sphere" + num + num + num + num + end);
static const std::regex maxverts(start + "maxverts" + num + end);
static const std::regex maxvertnorms(start + "maxvertnorms" + num + end);
static const std::regex vertex(start + "vertex" + num + num + num + end);
static const std::regex vertexnormal(start + "vertexnormal" + num + num + num + num + num + num + end);
static const std::regex triangle(start + "tri" + num + num + num + end);

static const std::regex ambient(start + "ambient" + num + num + num + end);
static const std::regex diffuse(start + "diffuse" + num + num + num + end);
static const std::regex specular(start + "specular" + num + num + num + end);
static const std::regex emission(start + "emission" + num + num + num + end);
static const std::regex shininess(start + "shininess" + num + end);

static const std::regex directional(start + "directional" + num + num + num + num + num + num + end);
static const std::regex point(start + "point" + num + num + num + num + num + num + end);
static const std::regex attenuation(start + "attenuation" + num + num + num + end);

static const std::regex translate(start + "translate" + num + num + num + end);
static const std::regex rotate(start + "rotate" + num + num + num + num + end);
static const std::regex scale(start + "scale" + num + num + num + end);
static const std::regex push(start + "pushTransform" + end);
static const std::regex pop(start + "popTransform" + end);

#pragma endregion

#pragma region Utility Functions

inline float clamp(float x, int min, int max)
{
	return (x < min ? min : (x > max ? max : x));
}

inline float clamp(float x, float min, float max)
{
	return (x < min ? min : (x > max ? max : x));
}

inline float roundBound(float d)
{
	return (round(d * 1000) / 1000.0f);
}

inline float p(std::smatch m, int i) 
{ 
	return stof(m.str(i)); 
}

inline float toRad(float deg) 
{ 
	return deg * M_PI / 180.0f; 
}

#pragma endregion

#pragma region Color Data

struct Color
{
	float r, g, b;

	Color(float = 0, float = 0, float = 0);
	Color operator +(const Color& c) const;
	Color operator *(const Color& c) const;
	Color operator *(float s) const;
	Color operator /(float s) const;
	void operator +=(Color c);
	void operator *=(float s);
	void operator /=(float s);

	static Color clamp(Color c);
};

#pragma endregion

#pragma region Pixel Sample

struct Sample
{
	int x, y;

	Sample(int x = 0, int y = 0);
};

#pragma endregion

#pragma region Ray

struct RayParameters
{
	float t;
	float d;
};

struct Ray
{
	Vector<3> origin, direction;
	Sample sample;

	Ray(Vector<3> = Vector<3>{}, Vector<3> = Vector<3>{}, Sample = Sample{});
	Ray(Sample);
};

#pragma endregion

#endif