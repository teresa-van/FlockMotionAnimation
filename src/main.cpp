#include "givr.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "io.h"
#include "turntable_controls.h"

#include <iostream>
#include <fstream>

//if we want to add a file, eg: #include "circle.h"

using namespace givr;
using namespace givr::camera;
using namespace givr::style;
using namespace givr::geometry;
using namespace std;

PhongStyle::InstancedRenderContext spheres;
PhongStyle::InstancedRenderContext oSpheres;
auto view = View(TurnTable(), Perspective());
vec3f mousePosition;

// auto phongStyle = Phong(
// 	LightPosition(0.0, 50.0, 75.0),
// 	Colour(1.0, 0.2, 0.3)
// );

float u = 0.;
mat4f m{1.f};

////////////////////////////////////////////////////////////////////

int scene = 1;
int iterations = 1;

float separationRadius = 3.0f;		
float alignmentRadius = 8.0f;	
float cohesionRadius = 8.0f;	

float maxSpeed = 10.0;

float width = 40;
float height = 30;
float depth = 10;

float deltaTime = 1.f/60.0;
bool tend = false;

int numBoids = 300;
int numObstacles = 8;

////////////////////////////////////////////////////////////////////

struct Boid
{
    vec3f prevPosition;
    vec3f position;
    vec3f velocity;
};

vector<Boid*> boids;
vector<vec3f> obstacles;

////////////////////////////////////////////////////////////////////

float random(float low, float high)
{
	return (low + rand() / (RAND_MAX / (high - low)));
}

Boid * CreateBoid(vec3f position, vec3f velocity)
{
    Boid * boid = new Boid();
    boid->position = position;
    boid->velocity = velocity;
	boid->prevPosition = boid->position - boid->velocity * deltaTime;

    return boid;
}

void InitBoids()
{
    int n = numBoids;
	for (int i = 0; i < n; i++)
    {
		boids.push_back(
            CreateBoid(vec3f(random(-width + 15, width - 15), random(-height + 20, height - 20), random(-depth, depth)), 
                       vec3f(random(-maxSpeed, maxSpeed), random(-maxSpeed, maxSpeed), random(-maxSpeed, maxSpeed))));
    }

	spheres = createInstancedRenderable(
        Mesh(Filename("./fishred.obj")),
        Phong(
            Colour(1.0, 1.5, 3.0),
            LightPosition(30.0, 30.0, 50.0)
        )
    );
}

void InitObstacles()
{
	int n = numObstacles;
	for (int i = 0; i < n; i++)
    {
		obstacles.push_back(vec3f(random(-width + 15, width - 15), random(-height + 15, height - 15), random(-depth, depth)));
    }

	oSpheres = createInstancedRenderable(
        Sphere(
				Centroid(0,0,0),
				Radius(2.0),
				AzimuthPoints(10),
				AltitudePoints(10)),
        Phong(
            Colour(0.5, 0.5, 0.5),
            LightPosition(30.0, 30.0, 50.0)
        )
    );

}

void BoundPosition(Boid * boid)
{
	float radius = 45;
	vec3f velocity(0,0,0);

	if (length(boid->position) - radius >= 0)
		boid->velocity *= -3.25;
}

vec3f Separation(Boid * boid)
{
	vec3f displacement(0,0,0);
	int count = 0;
	for (Boid * b : boids)
	{
		if (b == boid) continue;

		float distance = length(b->position - boid->position);
		if (distance > 0 && distance < separationRadius)
		{
			displacement -= (b->position - boid->position);
			count++;
		}
	}

	for (vec3f o : obstacles)
	{
		float distance = length(o - boid->position);
		if (distance > 0 && distance < separationRadius * 2.0)
		{
			displacement -= (o - boid->position);
			count++;
		}
	}

	if (count > 0)
	{
		displacement /= count;
		displacement /= 5;
	}

	return displacement;
}

vec3f Alignment(Boid * boid)
{
	vec3f sum(0,0,0);
	int count = 0;
	for (Boid * b : boids)
	{
		if (b == boid) continue;

		float distance = length(b->position - boid->position);
		if (distance > 0 && distance < alignmentRadius)
		{
			sum += b->velocity;
			count++;
		}
	}

	if (count > 0)
	{
		sum /= count;
		sum -= boid->velocity;
		sum /= 50;
	}

	return sum;
}

vec3f Cohesion(Boid * boid)
{
	vec3f center(0,0,0);
	int count = 0;
	for (Boid * b : boids)
	{
		if (b == boid) continue;

		float distance = length(b->position - boid->position);
		if (distance > 0 && distance < cohesionRadius)		
		{
			center += b->position;
			count++;
		}
	}

	if (count > 0)
	{
		center /= count;
		center -= boid->position;
		center /= 50;
	}

	return center;
}

vec3f TendToMouse(Boid * boid)
{
	vec3f displacement = mousePosition - boid->position;
	displacement /= 175;
	return displacement;
}

void MoveBoids()
{
	vec3f v1, v2, v3, v4;
	for (Boid * b : boids)
	{
		v1 = Cohesion(b);
		v2 = Separation(b);
		v3 = Alignment(b);

		b->velocity += (v1 + v2 + v3);

		if (tend) b->velocity += TendToMouse(b);

		if (length(b->velocity) > maxSpeed)
			b->velocity = (b->velocity / length(b->velocity)) * maxSpeed;

		BoundPosition(b);

		b->prevPosition = b->position;
		b->position += b->velocity * deltaTime;
	}
}

void InitialStateFromFile()
{
	ifstream file;
	string line;
	file.open ("initialstate.txt");
	if (file.is_open())
	{
		while (getline(file, line))
    	{
			if (line.find("boids") != string::npos)
			{
				line.erase(0, line.find_first_of(":")+2);
				numBoids = stoi(line);
			}
			else
			{
				line.erase(0, line.find_first_of(":")+2);
				numObstacles = stoi(line);
			}
				
		}
		file.close();
	}
	else
		cout << "Unable to open file.\n";

	maxSpeed = min(10.0, (numBoids / 300.0) * 15.0);
	maxSpeed = max(2.0f, maxSpeed);
}

int main(void)
{
    io::GLFWContext windows;
    auto window = windows.create(io::Window::dimensions{1024, 768}, "CPSC 587: Schools, Flocks & Herds - Teresa");
    window.enableVsync(true);
    window.keyboardCommands()
        | io::Key(GLFW_KEY_SPACE,
            [&](auto const &event) {
            if (event.action == GLFW_PRESS)
			{
                tend = !tend;
				separationRadius = (separationRadius == 3.0) ? 3.5 : 3.0;
			}
		});

    TurnTableControls controls(window, view.camera);

    glClearColor(0.f, 0.075f, 0.15f, 1.0f);

	InitialStateFromFile();
    InitBoids();
	InitObstacles();

    window.run([&](float frameTime) 
	{
		mousePosition = vec3f(((controls.cursorPosition.x - (window.width() / 2)) / (window.width() / 2)) * width, 
							  -((controls.cursorPosition.y - (window.height() / 2)) / (window.height() / 2)) * height, 0);
		view.projection.updateAspectRatio(window.width(), window.height());

		for(int i = 0; i < iterations; i++)
			MoveBoids();
	
        for (Boid * b : boids)
        {
			vec3f direction = b->position - b->prevPosition;
			direction /= length(direction);
			vec3f target = b->position + (0.1f * direction);

			vec3f right = cross(direction, vec3f(0,1,0));
			right /= length(right);
			vec3f up = cross(right, direction);
			up /= length(up);

			// auto m = translate(mat4f{1.f}, b->position);
			m = inverse(lookAt(b->position, b->prevPosition, up));
			m = scale(m, vec3f{0.25f});
			// m = translate(m, b->position); // Needed this otherwise the fish would teleport...

			addInstance(spheres, m);
        }
		draw(spheres, view);

		for (vec3f o : obstacles)
        {
			auto m = translate(mat4f{1.f}, o);
			addInstance(oSpheres, m);
        }
		draw(oSpheres, view);

        u += frameTime;
        
    });
    exit(EXIT_SUCCESS);
}
