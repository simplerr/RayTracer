#pragma once
#include "common/VectorUtils3.h"

class Material
{
public:
	vec3 color;
	float reflectivity;
	int isDiffuse;
	// [TODO] ...
};

class Light
{
public:
	void SetPosition(vec3 pos);
	void SetDirection(vec3 dir);
	void SetIntensity(vec3 intensity);
	void SetType(int type);
	void SetSpot(float spot);
//private:

	vec3 position;
	vec3 direction;
	vec3 color;
	vec3 intensity;			// x = ambient, y = diffuse, z = specular
	int type;				// 0 = directional, 1 = point light, 2 = spot light
	float spot;
};

class Sphere
{
public:
	void SetCenter(vec3 center);
	void SetRadius(float radius);
	void SetMaterial(Material material);
//private:

	Material material;
	vec3 center;
	float radius;
};

class Box
{
public:
	void SetMin(vec3 min);
	void SetMax(vec3 max);
	void SetMaterial(Material material);
//private:

	Material material;
	vec3 min;
	vec3 max;
};
