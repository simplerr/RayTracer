#pragma once
#include "common/VectorUtils3.h"

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1 
#define SPOT_LIGHT 2

class Material
{
public:
	Material(vec3 _color, float _reflectivity, int _isDiffuse);
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
	void SetColor(vec3 color);
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
	Sphere(float x, float y, float z, float _radius, Material material);
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
	Box(vec3 _min, vec3 _max, Material _material);
	void SetMin(vec3 min);
	void SetMax(vec3 max);
	void SetMaterial(Material material);
//private:

	Material material;
	vec3 minPos;
	vec3 maxPos;
};
