// Should work as is on Linux and Mac. MS Windows needs GLEW or glee.
// See separate Visual Studio version of my demos.
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include "MicroGlut.h"
#endif

#include "common/GL_utilities.h"
#include "common/VectorUtils3.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <string>
/*
	http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
	https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.2.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-II%29
	http://ray-tracing-conept.blogspot.se/2015/01/ray-box-intersection-and-normal.html

	https://www.ics.uci.edu/~gopi/CS211B/RayTracing%20tutorial.pdf
	http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_1_Introduction.shtml
*/

#define WINDOW_WIDTH 1280.0f
#define WINDOW_HEIGHT 1024.0f

void calculateRays();

// Globals
// Data would normally be read from files
GLfloat vertices[] =
{
	-0.5f,-0.5f,0.0f,
	-0.5f,0.5f,0.0f,
	0.5f,-0.5f,0.0f
};

float data[] = {
	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f
};

// vertex array object
unsigned int vertexArrayObjID;

/* Camera stuff */
vec3 cameraPos = { 3, 2, 7 };
vec3 cameraTarget = { 0, 0.5, 0 };
vec3 upVector = { 0, 1, 0 };
float cameraSpeed = 0.3f;
float pitch = 0.0f;
float yaw = -3.14 / 2.0;

// These need to be assigned during the different stages with 
// glAttachShader, glLinkProgram, glUseProgram
// First assign compute shader to draw pixels to texture
// then assign render shader to draw the texture on the screen
GLuint renderProgram;
GLuint computeProgram;

// The texture the compute shader renders to
GLuint computeTexture;

GLuint loadCompute(const char* filename)
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

GLuint generateTexture() {
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

void dumpInfoWindows(void)
{
	MessageBoxA(NULL, (char*)glGetString(GL_VENDOR), "Error!", MB_OK);\
	MessageBoxA(NULL, (char*)glGetString(GL_RENDERER), "Error!", MB_OK);
	MessageBoxA(NULL, (char*)glGetString(GL_VERSION), "Error!", MB_OK);
	MessageBoxA(NULL, (char*)glGetString(GL_SHADING_LANGUAGE_VERSION), "Error!", MB_OK);
}

void OnTimer(int value)
{
	// Move the camera using user input
	if (keyIsDown('a'))
	{
		vec3 dir = VectorSub(cameraTarget, cameraPos);
		vec3 right_vec = CrossProduct(dir, upVector);
		right_vec = Normalize(right_vec) * cameraSpeed;

		cameraPos = VectorSub(cameraPos, right_vec);
		cameraTarget = VectorSub(cameraTarget, right_vec);
	}
	else if (keyIsDown('d'))
	{
		vec3 dir = VectorSub(cameraTarget, cameraPos);
		vec3 right_vec = CrossProduct(dir, upVector);
		right_vec = Normalize(right_vec) * cameraSpeed;

		cameraPos = VectorAdd(cameraPos, right_vec);
		cameraTarget = VectorAdd(cameraTarget, right_vec);
	}

	if (keyIsDown('w'))
	{
		vec3 dir = VectorSub(cameraTarget, cameraPos);
		dir = Normalize(dir) * cameraSpeed;
		cameraPos = VectorAdd(cameraPos, dir);
		cameraTarget = VectorAdd(cameraTarget, dir);
	}
	if (keyIsDown('s'))
	{
		vec3 dir = VectorSub(cameraTarget, cameraPos);
		dir = Normalize(dir) * cameraSpeed;
		cameraPos = VectorSub(cameraPos, dir);
		cameraTarget = VectorSub(cameraTarget, dir);
	}

	glutPostRedisplay();
	glutTimerFunc(20, &OnTimer, value);
}

void mouseMove(int x, int y)
{
	static int last_x = WINDOW_WIDTH / 2;
	static int last_y = WINDOW_HEIGHT / 2;

	int delta_x = last_x - x;
	int delta_y = last_y - y;

	float camera_sensitivity = 0.005f;
	yaw += (float)delta_x * camera_sensitivity;
	pitch += (float)delta_y * camera_sensitivity;

	vec3 dir = { cosf(pitch) * sinf(yaw),
		sinf(pitch),
		cosf(pitch) * cosf(yaw)
	};

	cameraTarget = VectorAdd(cameraPos, dir);

	last_x = x;
	last_y = y;
}

void init(void)
{
	initKeymapManager();
	glutTimerFunc(20, &OnTimer, 0);

	glutPassiveMotionFunc(&mouseMove);
	glutWarpPointer(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

	unsigned int vertexBufferObjID;

	//dumpInfoWindows();

	// GL inits
	glClearColor(0.5, 0.5, 0.5, 0);
	glDisable(GL_DEPTH_TEST);

	// Generate the compute texture
	computeTexture = generateTexture();

	// Load and compile render program
	renderProgram = loadShaders("vertex.vert", "pixel.frag");

	// Load and compile compute program
	computeProgram = loadCompute("raytracer.comp");

	calculateRays();

	// Allocate and activate Vertex Array Object
	glGenVertexArrays(1, &vertexArrayObjID);
	glBindVertexArray(vertexArrayObjID);
	// Allocate Vertex Buffer Objects
	glGenBuffers(1, &vertexBufferObjID);

	// VBO for vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(renderProgram, "in_Position"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(renderProgram, "in_Position"));
}

void calculateRays()
{
	// Camera/view matrix
	//vec3 cameraPos = vec3(3, 2, 7);
	//vec3 cameraTarget = vec3(0, 0.5, 0);
	mat4 viewMatrix = lookAt(cameraPos.x, cameraPos.y, cameraPos.z, cameraTarget.x, cameraTarget.y, cameraTarget.z, 0, 1, 0);

	// Projection matrix
	mat4 projectionMatrix = perspective(90, WINDOW_WIDTH / WINDOW_HEIGHT, 1, 2);

	// Get inverse of view*proj
	mat4 inverseViewProjection = Mult(projectionMatrix, viewMatrix);
	inverseViewProjection = InvertMat4(inverseViewProjection);

	// Calculate rays
	vec4 ray00 = MultVec4(inverseViewProjection, vec4(-1, -1, 0, 1));
	ray00 = ray00 / ray00.w;
	ray00 -= cameraPos;

	vec4 ray10 = MultVec4(inverseViewProjection, vec4(+1, -1, 0, 1));
	ray10 = ray10 / ray10.w;
	ray10 -= cameraPos;

	vec4 ray01 = MultVec4(inverseViewProjection, vec4(-1, +1, 0, 1));
	ray01 = ray01 / ray01.w;
	ray01 -= cameraPos;

	vec4 ray11 = MultVec4(inverseViewProjection, vec4(+1, +1, 0, 1));
	ray11 = ray11 / ray11.w;
	ray11 -= cameraPos;

	// Upload to compute shader
	glUseProgram(computeProgram);
	glUniform3f(glGetUniformLocation(computeProgram, "eye"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray00"), ray00.x, ray00.y, ray00.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray01"), ray01.x, ray01.y, ray01.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray10"), ray10.x, ray10.y, ray10.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray11"), ray11.x, ray11.y, ray11.z);
}

void display(void)
{
	GLfloat t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

	// Compute shader
	// Note: is this needed all the time???
	glUseProgram(computeProgram);
	glUniform1f(glGetUniformLocation(computeProgram, "time"), t);
	calculateRays();
	glDispatchCompute(WINDOW_WIDTH / 16, WINDOW_HEIGHT / 16, 1);

	// Vertex and fragment shader
	glUseProgram(renderProgram);

	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vertexArrayObjID);	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glutSwapBuffers();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 5);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("RayTracer");
	glutDisplayFunc(display);

	if (GLEW_OK != glewInit())
	{		
		printf("glewInit failed, aborting.\n");
		exit(1);
	}
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	init();
	glutMainLoop();
}
