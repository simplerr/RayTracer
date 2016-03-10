// Should work as is on Linux and Mac. MS Windows needs GLEW or glee.
// See separate Visual Studio version of my demos.
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include "MicroGlut.h"
#endif

#include "common/GL_utilities.h"
#include <cstdio>
#include <cstdlib>

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
	char* cs_source = readFile((char *)filename);
	if (cs_source == NULL)
		fprintf(stderr, "Failed to read %s from disk.\n", filename);

	glShaderSource(cs, 1, &cs_source, NULL);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

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

void init(void)
{
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


void display(void)
{
	// Compute shader
	//glUseProgram(computeProgram);
	glUniform1f(glGetUniformLocation(computeProgram, "roll"), 1);
	glDispatchCompute(512 / 16, 512 / 16, 1);

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
	glutCreateWindow("GL3 white triangle example");
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
