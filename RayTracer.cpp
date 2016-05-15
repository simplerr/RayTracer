#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <string>
#include "RayTracer.h"
#include  "Camera.h"

RayTracer::RayTracer()
{

}

RayTracer::~RayTracer()
{
	delete camera;
}

void RayTracer::Init()
{
	// Init camera
	camera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);
	camera->position = vec3(3, 2, 7);
	camera->target = vec3(0, 0.5, 0);
	camera->up = vec3(0, 1, 0);
	camera->speed = 0.3f;
	camera->pitch = 0.0f;
	camera->yaw = -3.14f / 2.0f;

	glutWarpPointer(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

	unsigned int vertexBufferObjID;

	// GL inits
	glClearColor(0.5, 0.5, 0.5, 0);
	glDisable(GL_DEPTH_TEST);

	// Generate the compute texture
	computeTexture = GenerateTexture();

	// Load and compile render program
	renderProgram = loadShaders("vertex.vert", "pixel.frag");

	// Load and compile compute program
	computeProgram = LoadComputeShader("raytracer.comp");

	CalculateRays();

	// Allocate and activate Vertex Array Object
	glGenVertexArrays(1, &vertexArrayObjID);
	glBindVertexArray(vertexArrayObjID);
	// Allocate Vertex Buffer Objects
	glGenBuffers(1, &vertexBufferObjID);

	// VBO for vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), quadData, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(renderProgram, "in_Position"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(renderProgram, "in_Position"));
}

void RayTracer::Update(int value)
{
	camera->Update(value);

	CalculateRays();

	UpdateUniforms();
}

void RayTracer::UpdateUniforms()
{
	// Upload to compute shader
	glUseProgram(computeProgram);

	// Ray trace depth
	glUniform1i(glGetUniformLocation(computeProgram, "maxTraceDepth"), maxTraceDepth);

	// Lights
	glUniform1i(glGetUniformLocation(computeProgram, "numLights"), lights.size());

	// Spheres
	glUniform1i(glGetUniformLocation(computeProgram, "numSpheres"), spheres.size());

	// Boxes
	glUniform1i(glGetUniformLocation(computeProgram, "numBoxes"), boxes.size());

	// Eye and frustum rays
	glUniform3f(glGetUniformLocation(computeProgram, "eye"), camera->position.x, camera->position.y, camera->position.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray00"), frustumRays.ray00.x, frustumRays.ray00.y, frustumRays.ray00.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray01"), frustumRays.ray01.x, frustumRays.ray01.y, frustumRays.ray01.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray10"), frustumRays.ray10.x, frustumRays.ray10.y, frustumRays.ray10.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray11"), frustumRays.ray11.x, frustumRays.ray11.y, frustumRays.ray11.z);
}

void RayTracer::Render()
{
	GLfloat t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

	// Compute shader
	// Note: is this needed all the time???
	glUseProgram(computeProgram);
	glUniform1f(glGetUniformLocation(computeProgram, "time"), t);
	//CalculateRays();
	glDispatchCompute(WINDOW_WIDTH / 16, WINDOW_HEIGHT / 16, 1);

	// Vertex and fragment shader
	glUseProgram(renderProgram);

	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vertexArrayObjID);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glutSwapBuffers();
}

void RayTracer::MouseMove(int x, int y)
{
	camera->MouseMove(x, y);
}

void RayTracer::CalculateRays()
{
	// Camera/view matrix
	mat4 viewMatrix = lookAt(camera->position.x, camera->position.y, camera->position.z, camera->target.x, camera->target.y, camera->target.z, 0, 1, 0);

	// Projection matrix
	mat4 projectionMatrix = perspective(90, WINDOW_WIDTH / WINDOW_HEIGHT, 1, 2);

	// Get inverse of view*proj
	mat4 inverseViewProjection = Mult(projectionMatrix, viewMatrix);
	inverseViewProjection = InvertMat4(inverseViewProjection);

	// Calculate rays
	vec4 ray00 = MultVec4(inverseViewProjection, vec4(-1, -1, 0, 1));
	ray00 = ray00 / ray00.w;
	ray00 -= camera->position;

	vec4 ray10 = MultVec4(inverseViewProjection, vec4(+1, -1, 0, 1));
	ray10 = ray10 / ray10.w;
	ray10 -= camera->position;

	vec4 ray01 = MultVec4(inverseViewProjection, vec4(-1, +1, 0, 1));
	ray01 = ray01 / ray01.w;
	ray01 -= camera->position;

	vec4 ray11 = MultVec4(inverseViewProjection, vec4(+1, +1, 0, 1));
	ray11 = ray11 / ray11.w;
	ray11 -= camera->position;

	frustumRays.ray00 = ray00;
	frustumRays.ray10 = ray10;
	frustumRays.ray01 = ray01;
	frustumRays.ray11 = ray11;
}

GLuint RayTracer::LoadComputeShader(const char* filename)
{
	computeProgram = glCreateProgram();
	GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

	// Load the compute shader from file
	std::ifstream fin(filename);
	std::stringstream ss;
	ss << fin.rdbuf();
	std::string shaderText = ss.str();

	const char* shader = shaderText.c_str();
	const GLint length = shaderText.length();

	glShaderSource(cs, 1, &shader, &length);
	glCompileShader(cs);

	// Error checking
	int rvalue;
	glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling the compute shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(cs, 10239, &length, log);
		//fprintf(stderr, "Compiler log:\n%s\n", log);
		MessageBoxA(NULL, log, "Error!", MB_OK);

		exit(40);
	}

	glAttachShader(computeProgram, cs);
	glLinkProgram(computeProgram);

	// Error checking
	glGetProgramiv(computeProgram, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking compute shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(computeProgram, 10239, &length, log);
		//sprintf(stderr, "Linker log:\n%s\n", log);
		MessageBoxA(NULL, log, "Error!", MB_OK);
		exit(41);
	}

	glUseProgram(computeProgram);

	// Set the default value of destTex = 0
	glUniform1i(glGetUniformLocation(computeProgram, "destTex"), 0);

	return computeProgram;
}

GLuint RayTracer::GenerateTexture()
{
	// We create a single float channel 512^2 texture
	GLuint texHandle;
	glGenTextures(1, &texHandle);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// Because we're also using this tex as an image (in order to write to it),
	// we bind it to an image unit as well
	glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	return texHandle;
}