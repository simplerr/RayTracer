#pragma once

#include "common/VectorUtils3.h"

#define WINDOW_WIDTH 1280.0f
#define WINDOW_HEIGHT 1024.0f

class Camera;

static float quadData[] = {
	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f
};

class RayTracer
{
public:
	RayTracer();
	~RayTracer();

	void Init();
	void Update(int value);
	void Render();

	void MouseMove(int x, int y);
	void CalculateRays();

	GLuint LoadComputeShader(const char* filename);
	GLuint GenerateTexture();
private:
	// These need to be assigned during the different stages with 
	// glAttachShader, glLinkProgram, glUseProgram
	// First assign compute shader to draw pixels to texture
	// then assign render shader to draw the texture on the screen
	GLuint renderProgram;
	GLuint computeProgram;

	// The texture the compute shader renders to
	GLuint computeTexture;

	// Camera stuff
	Camera* camera;

	// Vertex array object
	unsigned int vertexArrayObjID;
};
