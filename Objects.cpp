#include "Objects.h"

Material::Material(vec3 _color, float _reflectivity, int _isDiffuse, int _special)
	: color(_color), reflectivity(_reflectivity), isDiffuse(_isDiffuse), special(_special)
{
	
}

Material::Material()
{
	color = vec3(1, 1, 1);
	reflectivity = 1;
	isDiffuse = true;
	special = 0;
}

void Light::SetPosition(vec3 pos)
{
	position = pos;
}

void Light::SetDirection(vec3 dir)
{
	direction = dir;
}

void Light::SetColor(vec3 color)
{
	this->color = color;
}

void Light::SetIntensity(vec3 intensity)
{
	this->intensity = intensity;
}

void Light::SetType(int type)
{
	this->type = type;
}

void Light::SetSpot(float spot)
{
	this->spot = spot;
}

void Light::SetSpecularFactor(float specularFactor)
{
	this->specularFactor = specularFactor;
}

Sphere::Sphere(float x, float y, float z, float _radius, Material _material) 
	: center(vec3(x, y, z)), material(_material), radius(_radius)
{

}

void Sphere::SetCenter(vec3 center)
{
	this->center = center;
}

void Sphere::SetRadius(float radius)
{
	this->radius = radius;
}

void Sphere::SetMaterial(Material material)
{
	this->material = material;
}

Box::Box(vec3 _min, vec3 _max, Material _material)
	: minPos(_min), maxPos(_max), material(_material)
{

}

void Box::SetMin(vec3 min)
{
	this->minPos = min;
}

void Box::SetMax(vec3 max)
{
	this->maxPos = max;
}

void Box::SetMaterial(Material material)
{
	this->material = material;
}
