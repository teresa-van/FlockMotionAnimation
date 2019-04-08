#include "givr.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "io.h"
#include "turntable_controls.h"

//if we want to add a file, eg: #include "circle.h"

using namespace givr;
using namespace givr::camera;
using namespace givr::style;
using namespace givr::geometry;
using namespace std;

PhongStyle::InstancedRenderContext spheres;
auto view = View(TurnTable(), Perspective());

// auto phongStyle = Phong(
// 	LightPosition(0.0, 50.0, 75.0),
// 	Colour(1.0, 0.2, 0.3)
// );

float u = 0.;
mat4f m{1.f};

////////////////////////////////////////////////////////////////////

int scene = 1;
int iterations = 50;

float radiusAvoid = 16.0f;		
float radiusCoherence = 18.0f;	
float radiusAttract = 20.0f;	
float radiusMax = 25.0f;
float vScalar = 0.0005f;
float vAvoidScalar = 0.3f;
float vCoherenceScalar = 0.03f;
float vAttractScalar = 0.08f;
float velocityLimit = 5;

float deltaTime = 1.f/200.f;
bool paused = false;

////////////////////////////////////////////////////////////////////

struct Boid
{
    vec3f position;
    vec3f velocity;
};

vector<Boid*> boids;

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

    return boid;
}

void InitBoids()
{
    int n = 50;
	for (int i = 0; i < n; i++)
    {
		boids.push_back(
            CreateBoid(vec3f(random(-10, 10), random(-10, 10), random(-10, 10)), 
                       vec3f(random(0, 5), random(0, 5), random(0, 5))));
    }
	spheres = createInstancedRenderable(
        Sphere(
				Centroid(0,0,0),
				Radius(0.5),
				AzimuthPoints(3),
				AltitudePoints(3)),
        Phong(
            Colour(0., 0., 1.),
            LightPosition(10., 10.0, 10.0)
        )
    );
}

void UpdateVelocity(Boid * b1, Boid * b2)
{
    float radius = length(b1->position - b2->position);
	vec3f  distance = b2->position - b1->position;
	vec3f velocity = vec3f(0, 0, 0);

	if (radius <= radiusMax)
	{
		if (radius < radiusAvoid)
		{
			velocity += (vAvoidScalar * (-distance));
		}
		else if (radius < radiusCoherence)
		{
			velocity += vCoherenceScalar * b2->velocity;
		} 
		else
		{
			velocity += vAttractScalar * distance;
		}
	}
    
	b1->velocity += (vScalar * velocity);
}

void CalculateForces()
{
	for (int i = 0; i < boids.size(); i++)
	{
		for (int j = 0; j < boids.size(); j++)
		{
			if (i == j) continue;

			Boid * b1 = boids[i];
			Boid * b2 = boids[j];

			UpdateVelocity(b1, b2);
		}
	}
	for (int i = 0; i < boids.size(); i++)
	{
		Boid * boid = boids.at(i);
        boid->position += deltaTime * boid->velocity;
	}
}

// void DrawTriangle(int i1, int i2, int i3)
// {
// 	auto t = Triangle(
// 		Point1(particles[i1]->position.x, particles[i1]->position.y, particles[i1]->position.z),
// 		Point2(particles[i2]->position.x, particles[i2]->position.y, particles[i2]->position.z),
// 		Point3(particles[i3]->position.x, particles[i3]->position.y, particles[i3]->position.z)
// 	);
// 	auto triangle = createRenderable(t, phongStyle);
// 	draw(triangle, view);
// }

int main(void)
{
    io::GLFWContext windows;
    auto window = windows.create(io::Window::dimensions{1024, 768}, "CPSC 587: Schools, Flocks & Herds - Teresa");
    window.enableVsync(true);
    // window.keyboardCommands()
    //     | io::Key(GLFW_KEY_1,
    //         [&](auto const &event) {
    //         if (event.action == GLFW_PRESS)
    //             SingleSpring();
	// 			iterations = 20;
    //         })

    TurnTableControls controls(window, view.camera);

    glClearColor(1.f, 1.f, 1.f, 1.f);

    InitBoids();
    window.run([&](float frameTime) 
	{
		view.projection.updateAspectRatio(window.width(), window.height());

		if (!paused)
		{
			for(int i = 0; i < iterations; i++)
				CalculateForces();
		}
		
        for (Boid * b : boids)
        {
            vec3f pos = b->position;
			auto m = translate(mat4f{1.f}, pos);
			addInstance(spheres, m);
        }
		draw(spheres, view);

        u += frameTime;
        
    });
    exit(EXIT_SUCCESS);
}
