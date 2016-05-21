// Should work as is on Linux and Mac. MS Windows needs GLEW or glee.
// See separate Visual Studio version of my demos.
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include "MicroGlut.h"
#endif

#include <cstdio>
#include "RayTracer.h"

/*
	http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
	https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.2.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-II%29
	http://ray-tracing-conept.blogspot.se/2015/01/ray-box-intersection-and-normal.html

	https://www.ics.uci.edu/~gopi/CS211B/RayTracing%20tutorial.pdf
	http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_1_Introduction.shtml
*/

RayTracer rayTracer;

void dumpInfoWindows(void)
{
	MessageBoxA(NULL, (char*)glGetString(GL_VENDOR), "Error!", MB_OK);\
	MessageBoxA(NULL, (char*)glGetString(GL_RENDERER), "Error!", MB_OK);
	MessageBoxA(NULL, (char*)glGetString(GL_VERSION), "Error!", MB_OK);
	MessageBoxA(NULL, (char*)glGetString(GL_SHADING_LANGUAGE_VERSION), "Error!", MB_OK);
}

void OnTimer(int value)
{
	rayTracer.Update(value);

	glutPostRedisplay();
	glutTimerFunc(20, &OnTimer, value);
}

void MouseMove(int x, int y)
{
	rayTracer.MouseMove(x, y);
}

void MousePressed(int button, int state, int x, int y)
{
	rayTracer.MousePressed(button, state, x, y);
}

void Display(void)
{
	rayTracer.Render();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 5);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("RayTracer");
	glutDisplayFunc(Display);

	if (GLEW_OK != glewInit())
	{		
		printf("glewInit failed, aborting.\n");
		exit(1);
	}
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	const GLubyte* text = glGetString(GL_EXTENSIONS);
	
	glutTimerFunc(20, &OnTimer, 0);
	glutPassiveMotionFunc(&MouseMove);
	glutMouseFunc(&MousePressed);

	initKeymapManager();

	rayTracer.Init();

	glutMainLoop();
}
