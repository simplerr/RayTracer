#include "Camera.h"


Camera::Camera(vec3 _pos, vec3 _target, vec3 _up, float _speed, float _pitch, float _yaw, int _windowWidth, int _windowHeight)
	: position(_pos),  target(_target), up(_up), speed(_speed), pitch(_pitch), yaw(_yaw), windowWidth(_windowWidth), windowHeight(_windowHeight)
{

}

Camera::Camera(int _windowWidth, int _windowHeight) : windowWidth(_windowWidth), windowHeight(_windowHeight)
{
	position = vec3(3, 2, 7);
	target = vec3(0, 0.5, 0);
	up = vec3(0, 1, 0);
	speed = 0.3f;
	pitch = 0.0f;
	yaw = -3.14f / 2.0f;
}

void Camera::Update(int value)
{
	// Move the camera using user input
	if (keyIsDown('a'))
	{
		vec3 dir = VectorSub(target, position);
		vec3 right_vec = CrossProduct(dir, up);
		right_vec = Normalize(right_vec) * speed;

		position = VectorSub(position, right_vec);
		target = VectorSub(target, right_vec);
	}
	else if (keyIsDown('d'))
	{
		vec3 dir = VectorSub(target, position);
		vec3 right_vec = CrossProduct(dir, up);
		right_vec = Normalize(right_vec) * speed;

		position = VectorAdd(position, right_vec);
		target = VectorAdd(target, right_vec);
	}

	if (keyIsDown('w'))
	{
		vec3 dir = VectorSub(target, position);
		dir = Normalize(dir) * speed;
		position = VectorAdd(position, dir);
		target = VectorAdd(target, dir);
	}
	if (keyIsDown('s'))
	{
		vec3 dir = VectorSub(target, position);
		dir = Normalize(dir) * speed;
		position = VectorSub(position, dir);
		target = VectorSub(target, dir);
	}
}

void Camera::MouseMove(int x, int y)
{
	static int last_x = windowWidth / 2;
	static int last_y = windowHeight / 2;

	int delta_x = last_x - x;
	int delta_y = last_y - y;

	float camera_sensitivity = 0.005f;
	yaw += (float)delta_x * camera_sensitivity;
	pitch += (float)delta_y * camera_sensitivity;

	vec3 dir = { cosf(pitch) * sinf(yaw),
		sinf(pitch),
		cosf(pitch) * cosf(yaw)
	};

	target = VectorAdd(position, dir);

	last_x = x;
	last_y = y;
}