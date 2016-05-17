#pragma once

#include "common/VectorUtils3.h"

class Camera
{
public:
	Camera(vec3 _pos, vec3 _target, float _speed, int _windowWidth, int _windowHeight);

	void Update(int value);
	void MouseMove(int x, int y);
	void MousePressed(int button, int state, int x, int y);

	vec3 position;
	vec3 target;
	vec3 up;
	float speed;
	float pitch;
	float yaw;

	int windowWidth;
	int windowHeight;

	int last_x;
	int last_y;
};
