#pragma once
#include <vector>
#include "common/VectorUtils3.h"
#include "Objects.h"

#define WINDOW_WIDTH 1280.0f
#define WINDOW_HEIGHT 1024.0f

using namespace std;

class Camera;

static float quadData[] = {
	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f
};

struct Ray
{
	Ray() {}
	Ray(vec3 o, vec3 d) : origin(o), direction(d) {}
	vec3 origin;
	vec3 direction;
};

struct Hitinfo
{
	Hitinfo() : ray(vec3(0, 0, 0), vec3(0,0, 0)) {}
	Ray ray;
	float distance;
	int index;		// Sphere index
	int type;		// 0 = sphere, 1 = box, 2 = light
	vec3 centerOffset;
};

struct FrustumRays
{
	vec4 ray00;
	vec4 ray01;
	vec4 ray10;
	vec4 ray11;
};

class RayTracer
{
public:
	RayTracer();
	~RayTracer();

	void Init();
	void InitObjects();
	void Update(int value);
	void Render();
	void UpdateUniforms();

	void MousePressed(int button, int state, int x, int y);
	void MouseMove(int x, int y);
	void CalculateRays();
	bool ClosestObjectIntersection(Ray ray, Hitinfo& hitinfo);
	bool RaySphereIntersection(const Ray& ray, const Sphere& sphere, Hitinfo& hitinfo);
	Ray GetWorldPickingRay(int x, int y);

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
	FrustumRays frustumRays;

	// Vertex array object
	unsigned int vertexArrayObjID;

	// Uniform data
	int maxTraceDepth = 4;
	int roomSize = 50;

	vector<Light> lights;
	vector<Sphere> spheres;
	vector<Box> boxes;

	Hitinfo selectedHitInfo;
	//int selectedObjectType = -1;	// 0 = sphere, 1 = box, 2 = light
	//int selectedObject = -1;
	float moveSpeed = 0.2f;

	bool isMovingObject = false;
	Ray previousRay;
};
