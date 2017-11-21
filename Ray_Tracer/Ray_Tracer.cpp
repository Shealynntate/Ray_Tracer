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
#include "stdafx.h"
#include <fstream>
#include <stack>

#include "Scene.h"

/* -------------------------------------------------------------------------------------------------
   The fileInputHandler reads in the source file line by line and constructs the materials, lights, 
   geometry, and camera for the scene based on the input.
   Input file format :
		- lines that begin with # are ignored as comments
		- size <int> <int> : size of display window as well as output file size.
		- output <filename> : the name of the file to save to, defaults to "defaultOutput.png"
		- camera <args> : the origin, look at, up vector, and fov of the scene's camera
		- point <args> : point light
		- directional <args> : directional light
		- ambient <args> : the ambient BRDF of material
		- diffuse <args> : the diffuse BRDF of material
		- specular <args> : the specular BRDF of material
		- shininess <float> : the exponent value for calculating highlight size of specular BRDF
		- emission <args> : the emissive BRDF of material
		- maxverts <int> : number of vertices created in scene
		- maxnorms <int> : number of normals created in scene
		- vertex <float, float float> : specifies a vertex 3d position
		- normal <float, float, float> : specifies a normal direction
		- pushTransform : saving the current transform state
		- popTransform : revert back to the previous transform state
		- scale <float> <float> <float> : apply scale transform to future geometries 
		- translate <float> <float> <float> : apply translation transform to future geometries
		- rotate <float> <float> <float> : apply rotation to future geometries
		- tri <int> <int> <int> : create triangle using indices of 3 vertices previously specified
		- sphere <float> <float> <float> <float> : create sphere with given position and radius
   -------------------------------------------------------------------------------------------------
*/
Scene fileInputHandler(std::string fileName)
{
	std::ifstream file(fileName, std::ifstream::in);
	std::string line;
	std::smatch m;
	EmissiveMaterial mat{};
	Scene scene{};

	Vector<3> atten{1, 0, 0}; 

	std::stack<Matrix<4, 4>> transStack{};
	std::stack<Matrix<4, 4>> invTransStack{};
	transStack.push(Matrix<4, 4>{});
	invTransStack.push(Matrix<4, 4>{});
	Matrix<4, 4> t{};
	Matrix<4,4> i{};

	std::vector<Vector<3>> vertices{};
	std::vector<Vector<3>> normals{};

	while (file.good())
	{
		getline(file, line);

		if (line.length() == 0 || line[0] == '#')
		{
			continue;
		}
		else if (regex_search(line, m, sphere))
		{
			auto offset = Matrix<4, 4>::Translation(Vector<3>{ p(m, 1), p(m, 2), p(m, 3) });
			auto scale = Matrix<4, 4>::Scale(Vector<3>{ p(m, 4), p(m, 4), p(m, 4) });
			Sphere *sphere = new Sphere(Vector<3>{}, 1.0f, mat, scale.inverse() * i * offset.inverse());
			sphere->generateBoundingBox(offset * t * scale);
			scene.addGeometry(sphere);
		}
		else if (regex_search(line, m, vertexnormal))
		{
			vertices.push_back(Vector<3>{ p(m, 1), p(m, 2), p(m, 3) });
			normals.push_back(Vector<3>{ p(m, 4), p(m, 5), p(m, 6) });
		}
		else if (regex_search(line, m, vertex))
		{
			vertices.push_back(Vector<3>{ p(m, 1), p(m, 2), p(m, 3) });
		}
		else if (regex_search(line, m, triangle))
		{
			auto a = higherDimension(vertices[stoi(m.str(1))], 1.0f);
			auto b = higherDimension(vertices[stoi(m.str(2))], 1.0f);
			auto c = higherDimension(vertices[stoi(m.str(3))], 1.0f);
			auto v0 = lowerDimension((t * a).homogenous());
			auto v1 = lowerDimension((t * b).homogenous());
			auto v2 = lowerDimension((t * c).homogenous());
			Triangle *tri = new Triangle(v0, v1, v2, mat, Matrix<4, 4>{});
			tri->generateBoundingBox(Matrix<4, 4>{});
			scene.addGeometry(tri);
		}
		else if (regex_search(line, m, ambient))
		{
			mat.setka(Color(p(m, 1), p(m, 2), p(m, 3)));
		}
		else if (regex_search(line, m, diffuse))
		{
			mat.setkd(Color(p(m, 1), p(m, 2), p(m, 3)));
		}
		else if (regex_search(line, m, specular))
		{
			mat.setks(Color(p(m, 1), p(m, 2), p(m, 3)));
		}
		else if (regex_search(line, m, emission))
		{
			mat.setke(Color(p(m, 1), p(m, 2), p(m, 3)));
		}
		else if (regex_search(line, m, shininess))
		{
			mat.setexp(p(m, 1));
		}
		else if (regex_search(line, m, directional))
		{
			Color c = Color(p(m, 4), p(m, 5), p(m, 6));
			Vector<3> dir{ p(m, 1), p(m, 2), p(m, 3) };
			dir = lowerDimension(t * higherDimension(dir, 0)).normal();
			scene.addLight(new Directional(1.0, c, dir));
		}
		else if (regex_search(line, m, point))
		{
			Color c = Color(p(m, 4), p(m, 5), p(m, 6));
			Vector<3> pos{ p(m, 1), p(m, 2), p(m, 3) };
			pos = lowerDimension((t * higherDimension(pos, 1)).homogenous());
			scene.addLight(new Point(1.0, c, pos, atten));
		}
		else if (regex_search(line, m, translate))
		{
			auto trans = Matrix<4, 4>::Translation(Vector<3>{ p(m, 1), p(m, 2), p(m, 3) });
			auto invTrans = trans.inverse();
			t = t * trans;
			i = invTrans * i;
		}
		else if (regex_search(line, m, rotate))
		{
			auto trans = Matrix<4, 4>::Rotation(Vector<3>{ p(m, 1), p(m, 2), p(m, 3) }, toRad(p(m, 4)));
			auto invTrans = trans.inverse();
			t = t * trans;
			i = invTrans * i;
		}
		else if (regex_search(line, m, scale))
		{
			auto trans = Matrix<4, 4>::Scale(Vector<3>{ p(m, 1), p(m, 2), p(m, 3) });
			auto invTrans = trans.inverse();
			t = t * trans;
			i = invTrans * i;
		}
		else if (regex_search(line, m, push))
		{
			transStack.push(Matrix<4, 4>{t});
			invTransStack.push(Matrix<4, 4>{i});
		}
		else if (regex_search(line, m, pop))
		{
			t = Matrix<4, 4>{ transStack.top() };
			transStack.pop();
			i = Matrix<4, 4>{ invTransStack.top() };
			invTransStack.pop();
		}
		else if (regex_search(line, m, camera))
		{
			Vector<3> eye{ p(m, 1), p(m, 2), p(m, 3) };
			Vector<3> center{ p(m, 4), p(m, 5), p(m, 6) };
			Vector<3> up{ p(m, 7), p(m, 8), p(m, 9) };
			float fov = toRad(p(m, 10));

			scene.buildMVP(eye, center, up, fov);
		}
		else if (regex_search(line, m, size))
		{
			int width = stoi(m.str(1));
			int height = stoi(m.str(2));
			scene.setScreenDimensions(width, height);
		}
		else if (regex_search(line, m, depth))
		{
			scene.setMaxDepth(stoi(m.str(1)));
		}
		else if (regex_search(line, m, maxverts))
		{
			// Total number of vertices
		}
		else if (regex_search(line, m, maxvertnorms))
		{
			// Total number of normals
		}
		else if (regex_search(line, m, output))
		{
			std::string outputFile = m.str(1);
			scene.setOutputFilename(outputFile);
		}
		else if (regex_search(line, m, attenuation))
		{
			atten = Vector<3>{ p(m, 1), p(m, 2), p(m, 3) };
		}
		else
		{
			// Print out unidentified lines
			std::cout << line << "\n";;
		}
	}
	file.close();
	return scene;
}

void keyboardHandler(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(int argc, char * argv[])
{
	// GLFW Initialization
	GLFWwindow *window;

	// Measure the time it takes to construct scene
	clock_t startTime = clock();
	double setupDuration;

	if (!glfwInit())
		return -1;

	auto scene = fileInputHandler(argv[1]);

	// Output scene construction time
	setupDuration = (clock() - startTime) / (double)CLOCKS_PER_SEC;
	std::cout << "Ray Tracer took " << setupDuration << " seconds to build scene" << std::endl;

	window = glfwCreateWindow(scene.screenWidth(), scene.screenHeight(), "Ray Tracer", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyboardHandler);

	startTime = clock();
	scene.generateScene();

	setupDuration = (clock() - startTime) / (double)CLOCKS_PER_SEC;
	std::cout << "Ray Tracer took " << setupDuration << " seconds to render" << std::endl;

	scene.outputToFile();

	// Main Loop
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();

		scene.display();

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}