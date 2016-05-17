#include "Camera.h"


Camera::Camera(vec3 _pos, vec3 _target, float _speed, int _windowWidth, int _windowHeight)
	: position(_pos),  target(_target), speed(_speed), windowWidth(_windowWidth), windowHeight(_windowHeight)
{
	last_x = windowWidth / 2;
	last_y = windowHeight / 2;

	up = vec3(0, 1, 0);

	vec3 ds = target - position;
	yaw = atan2(ds.x, ds.z);
	pitch = atan2(ds.y, sqrt((ds.x*ds.x) + (ds.z*ds.z)));
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

	// Look 
	if (GetAsyncKeyState(VK_RBUTTON))
	{
		HWND hwnd = GetForegroundWindow();
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hwnd, &point);

		MouseMove(point.x, point.y);

		//int delta_x = last_x - point.x;
		//int delta_y = last_y - point.y;

		//float camera_sensitivity = 0.005f;
		//yaw += (float)delta_x * camera_sensitivity;
		//pitch += (float)delta_y * camera_sensitivity;

		//vec3 dir = { cosf(pitch) * sinf(yaw),
		//	sinf(pitch),
		//	cosf(pitch) * cosf(yaw)
		//};

		//target = VectorAdd(position, dir);

		//last_x = point.x;
		//last_y = point.y;
	}
}

void Camera::MousePressed(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		HWND hwnd = GetForegroundWindow();
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hwnd, &point);

		last_x = point.x;
		last_y = point.y;
	}
}

void Camera::MouseMove(int x, int y)
{
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