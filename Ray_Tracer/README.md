# Ray_Tracer
A Ray Tracer built from scratch in C++. Includes a grid acceleration scheme that renders large scenes in a fraction of the time.

## Basic Structure
The program works by creating a scene filled with primitive objects, point and directional lights, and a camera. All this
data is read in from a file designated by the user. A camera is given an origin, field of view, and near and far clipping
planes that dictate the view frustrum.

![Ray Tracer Setup](/Images/Ray_Tracer_05.png?raw=true "Ray Tracer Setup Diagram")

## Ray Tracing
For every pixel in the camera, the program sends out a ray into the scene. This means it has to transform the ray from
screen coordinates into world space - the space all the objects in the scene use. It then checks to see if that ray collides
with any objects. If it does, the ray tracer perfoms a Blinn-Phong lighting calculation using the angle of collision,
surface normal, and its relation to all lights in the scene.

![Multiple Sphere Test](/Images/Ray_Tracer_01.png?raw=true "Multiple Sphere Test Scene")

## Shading
As stated above, the ray tracer using the Blinn-Phong shading model. Every object has a material. That material has an 
ambient, diffuse, and specular component. Ambient refers to a background color it has even in the absence of light. The
diffuse component represents a flat matte color and the specular component gives it a highlight or metallic shine. 
It sends this data to a film class that renders it out into a rgb color value and sets the corresponding pixel to
that value.

The image below illustrates the difference between entirely diffuse materials and entirely specular ones.

![Diffuse vs Specular Comparison](/Images/Ray_Tracer_02.png?raw=true "Diffuse vs Specular Comparison Scene")

## Reflections
Using recursion, the program can create reflective surfaces. Each time a camera ray hits an object in a scene, it can then
generate a collection of new rays, "bouncing" them out from the initial contact point. When these bounce rays strike other 
objects, they do a shading computation and pass that data back to the original ray. It can then combine these calculations
into one final result value.

The end result is a mirror-like surface. The program can bounce rays to an arbitrary depth, but at a huge performance cost.
For each of N initial rays, it's generating k additional rays every bounce, which is an exponential process.

![Reflective Cornell Box](/Images/Ray_Tracer_03.png?raw=true "Reflective Cornell Box Scene")

## Linear Grid Acceleration
To speed up the process, the ray tracer uses a technique called linear grid acceleration.  The scene is divide up into a 
3 dimensional regular grid. Every object is enclosed in an axis-aligned bounding box. The bounding boxes then register with
the grid, which marks every grid square that falls within the object's bounding box.

When rays are sent out from the camera, the ray tracer simply checks every grid square that the ray path intersects. If 
there are any objects registered with that square, it does the normal computations. Since most rays end up traveling
through empty space or only striking one surface, this saves quite a bit of time.

The program rendered the Stanford Dragon, shown below, in 150 seconds using this acceleration, whereas the default 
implementation can take several hours.

![Stanford Dragon](/Images/Ray_Tracer_04.png?raw=true "Stanford Dragon Scene")


