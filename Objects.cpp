#include "Objects.h"

void Light::SetPosition(vec3 pos)
{
	position = pos;
}

void Light::SetDirection(vec3 dir)
{
	direction = dir;
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

void Box::SetMin(vec3 min)
{
	this->min = min;
}

void Box::SetMax(vec3 max)
{
	this->max = max;
}

void Box::SetMaterial(Material material)
{
	this->material = material;
}
