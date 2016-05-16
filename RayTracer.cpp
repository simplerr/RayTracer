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
	camera->position = vec3(3, 20, 7);
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

	InitObjects();
}

void RayTracer::InitObjects()
{
	// Add lights
	Light light1;
	light1.SetPosition(vec3(roomSize / 2, roomSize / 2, roomSize / 2));
	light1.SetDirection(vec3(0, 0, 1));
	light1.SetColor(vec3(1, 1, 1));
	light1.SetIntensity(vec3(0.2, 1, 1));
	light1.SetType(POINT_LIGHT);
	light1.SetSpot(1.0f);
	lights.push_back(light1);

	// Add spheres
	spheres.push_back(Sphere(roomSize / 2 + 5, 8, roomSize / 2 + 5, 4, Material(vec3(1, 0, 0), 1, false)));
	spheres.push_back(Sphere(roomSize / 2 - 10, 10, roomSize / 2 + 1, 6, Material(vec3(0, 0, 0), 0, false)));
	spheres.push_back(Sphere(roomSize / 2 + 5, 11, roomSize / 2 + 1, 0.5, Material(vec3(0, 0, 1), 1, false)));
	spheres.push_back(Sphere(roomSize / 2 + 15, 7, roomSize / 2 + 6, 3, Material(vec3(1, 1, 0), 1, true, 1)));

	// Add boxes
	boxes.push_back(Box(vec3(0, -0.1, 0), vec3(roomSize, 0.0, roomSize), Material(vec3(0.3, 0.3, 0.3), 1, true, 1)));
	boxes.push_back(Box(vec3(0, roomSize, 0), vec3(roomSize, roomSize - 0.1, roomSize), Material(vec3(0.5, 0, 0), 0, true)));
	boxes.push_back(Box(vec3(-0.1, 0, 0), vec3(0, roomSize, roomSize), Material(vec3(0, 0.5, 0), 1, true)));
	boxes.push_back(Box(vec3(roomSize, 0, 0), vec3(roomSize - 0.1, roomSize, roomSize), Material(vec3(0.0, 0.0, 0.5), 1, true)));
	boxes.push_back(Box(vec3(0, 0, -0.1), vec3(roomSize, roomSize, 0), Material(vec3(0.5, 0.5, 0), 1, true)));
	boxes.push_back(Box(vec3(0, 0, roomSize), vec3(roomSize, roomSize, roomSize - 0.1), Material(vec3(0, 0.5, 0.5), 1, true)));

	UpdateUniforms();
}

void RayTracer::Update(int value)
{
	camera->Update(value);

	CalculateRays();

	UpdateUniforms();

	// Update the selected object
	if (selectedObject != -1)
	{
		char modifier = 'q';
		if (keyIsDown(modifier) && keyIsDown('x'))
			spheres[selectedObject].center.x -= moveSpeed;
		else if (keyIsDown('x'))
			spheres[selectedObject].center.x += moveSpeed;
		if (keyIsDown(modifier) && keyIsDown('y'))
			spheres[selectedObject].center.y -= moveSpeed;
		else if (keyIsDown('y'))
			spheres[selectedObject].center.y += moveSpeed;
		if (keyIsDown(modifier) && keyIsDown('z'))
			spheres[selectedObject].center.z -= moveSpeed;
		else if (keyIsDown('z'))
			spheres[selectedObject].center.z += moveSpeed;
		
	}

	// Eye and frustum rays
	glUseProgram(computeProgram);
	glUniform3f(glGetUniformLocation(computeProgram, "eye"), camera->position.x, camera->position.y, camera->position.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray00"), frustumRays.ray00.x, frustumRays.ray00.y, frustumRays.ray00.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray01"), frustumRays.ray01.x, frustumRays.ray01.y, frustumRays.ray01.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray10"), frustumRays.ray10.x, frustumRays.ray10.y, frustumRays.ray10.z);
	glUniform3f(glGetUniformLocation(computeProgram, "ray11"), frustumRays.ray11.x, frustumRays.ray11.y, frustumRays.ray11.z);
}

void RayTracer::UpdateUniforms()
{
	// Upload to compute shader
	glUseProgram(computeProgram);

	// Ray trace depth
	glUniform1i(glGetUniformLocation(computeProgram, "maxTraceDepth"), maxTraceDepth);

	// Lights
	glUniform1i(glGetUniformLocation(computeProgram, "numLights"), lights.size());
	for (int i = 0; i < lights.size(); i++)
	{
		string pos = "lightList[" + to_string(i) + "].position";
		string dir = "lightList[" + to_string(i) + "].direction";
		string color = "lightList[" + to_string(i) + "].color";
		string intensity = "lightList[" + to_string(i) + "].intensity";
		string type = "lightList[" + to_string(i) + "].type";
		string spot = "lightList[" + to_string(i) + "].spot";
		
	
		glUniform3f(glGetUniformLocation(computeProgram, pos.c_str()), lights[i].position.x, lights[i].position.y, lights[i].position.z);
		glUniform3f(glGetUniformLocation(computeProgram, dir.c_str()), lights[i].direction.x, lights[i].direction.y, lights[i].direction.z);
		glUniform3f(glGetUniformLocation(computeProgram, color.c_str()), lights[i].color.x, lights[i].color.y, lights[i].color.z);
		glUniform3f(glGetUniformLocation(computeProgram, intensity.c_str()), lights[i].intensity.x, lights[i].intensity.y, lights[i].intensity.z);
		glUniform1i(glGetUniformLocation(computeProgram, type.c_str()), lights[i].type);
		glUniform1f(glGetUniformLocation(computeProgram, spot.c_str()), lights[i].spot);
	}

	// Spheres
	glUniform1i(glGetUniformLocation(computeProgram, "numSpheres"), spheres.size());
	for (int i = 0; i < spheres.size(); i++)
	{
		string center = "sphereList[" + to_string(i) + "].center";
		string radius = "sphereList[" + to_string(i) + "].radius";
		string matColor = "sphereList[" + to_string(i) + "].material.color";
		string matReflectivity = "sphereList[" + to_string(i) + "].material.reflectivity";
		string matIsDiffuse = "sphereList[" + to_string(i) + "].material.isDiffuse";
		string matSpecial = "sphereList[" + to_string(i) + "].material.special";

		glUniform3f(glGetUniformLocation(computeProgram, center.c_str()), spheres[i].center.x, spheres[i].center.y, spheres[i].center.z);	
		glUniform1f(glGetUniformLocation(computeProgram, radius.c_str()), spheres[i].radius);
		glUniform3f(glGetUniformLocation(computeProgram, matColor.c_str()), spheres[i].material.color.x, spheres[i].material.color.y, spheres[i].material.color.z);
		glUniform1f(glGetUniformLocation(computeProgram, matReflectivity.c_str()), spheres[i].material.reflectivity);
		glUniform1i(glGetUniformLocation(computeProgram, matIsDiffuse.c_str()), spheres[i].material.isDiffuse);
		glUniform1i(glGetUniformLocation(computeProgram, matSpecial.c_str()), spheres[i].material.special);
	}

	// Boxes
	glUniform1i(glGetUniformLocation(computeProgram, "numBoxes"), boxes.size());
	for (int i = 0; i < boxes.size(); i++)
	{
		string min = "boxList[" + to_string(i) + "].min";
		string max = "boxList[" + to_string(i) + "].max";
		string matColor = "boxList[" + to_string(i) + "].material.color";
		string matReflectivity = "boxList[" + to_string(i) + "].material.reflectivity";
		string matIsDiffuse = "boxList[" + to_string(i) + "].material.isDiffuse";
		string matSpecial = "boxList[" + to_string(i) + "].material.special";

		glUniform3f(glGetUniformLocation(computeProgram, min.c_str()), boxes[i].minPos.x, boxes[i].minPos.y, boxes[i].minPos.z);
		glUniform3f(glGetUniformLocation(computeProgram, max.c_str()), boxes[i].maxPos.x, boxes[i].maxPos.y, boxes[i].maxPos.z);
		glUniform3f(glGetUniformLocation(computeProgram, matColor.c_str()), boxes[i].material.color.x, boxes[i].material.color.y, boxes[i].material.color.z);
		glUniform1f(glGetUniformLocation(computeProgram, matReflectivity.c_str()), boxes[i].material.reflectivity);
		glUniform1i(glGetUniformLocation(computeProgram, matIsDiffuse.c_str()), boxes[i].material.isDiffuse);
		glUniform1i(glGetUniformLocation(computeProgram, matSpecial.c_str()), boxes[i].material.special);
	}

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

bool RayTracer::RaySphereIntersection(const Ray& ray, const Sphere& sphere, Hitinfo& hitinfo)
{
	float t0, t1;

	vec3 L = sphere.center - ray.origin;
	float tca = DotProduct(L, ray.direction);
	if (tca < 0)
		return false;

	float d2 = DotProduct(L, L) - tca * tca;

	if (d2 > pow(sphere.radius, 2))
		return false;

	float thc = sqrt(pow(sphere.radius, 2) - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	if (t0 < 0)
	{
		float tmp = t0;
		t0 = t1;
		t1 = tmp;

		if (t0 < 0)
			return false;
	}

	if (t0 > 0.0 && t0 < t1)
	{
		hitinfo.distance = t0;
		return true;
	}

	return false;
}

bool RayTracer::ClosestObjectIntersection(Ray ray, Hitinfo& hitinfo)
{
	float closest = 9999999;
	bool found = false;

	for (int i = 0; i < spheres.size(); i++)
	{
		Hitinfo tmpHitInfo;
		if (RaySphereIntersection(ray, spheres[i], tmpHitInfo))
		{
			if (tmpHitInfo.distance < closest)
			{
				// Hitinfo...
				hitinfo = tmpHitInfo;
				hitinfo.index = i;
				closest = tmpHitInfo.distance;
				found = true;
			}
		}
	}

	return found;
}

Ray RayTracer::GetWorldPickingRay(int x, int y)
{
	// Camera/view matrix
	mat4 viewMatrix = lookAt(camera->position.x, camera->position.y, camera->position.z, camera->target.x, camera->target.y, camera->target.z, 0, 1, 0);

	// Projection matrix
	mat4 projectionMatrix = perspective(90, WINDOW_WIDTH / WINDOW_HEIGHT, 1, 2);

	// Get inverse of view*proj
	mat4 inverseViewProjection = Mult(projectionMatrix, viewMatrix);
	inverseViewProjection = InvertMat4(inverseViewProjection);

	float vx = (+2.0f * x / WINDOW_WIDTH - 1);
	float vy = (-2.0f * y / WINDOW_HEIGHT + 1);

	vec4 rayDir = MultVec4(inverseViewProjection, vec4(vx, vy, 0, 1));
	rayDir = rayDir / rayDir.w;
	rayDir -= camera->position;
	rayDir = Normalize(vec3(rayDir.x, rayDir.y, rayDir.z));

	return Ray(camera->position, vec3(rayDir.x, rayDir.y, rayDir.z));
}

void RayTracer::MousePressed(int button, int state, int x, int y)
{
	// Find intersecting sphere
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		Ray ray = GetWorldPickingRay(x, y);
		Hitinfo hitinfo;
		if (ClosestObjectIntersection(ray, hitinfo))
		{
			selectedObject = hitinfo.index;
		}
	}
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