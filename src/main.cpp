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
PhongStyle::InstancedRenderContext oSpheres;
auto view = View(TurnTable(), Perspective());

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

float width = 25;
float height = 15;
float depth = 10;

float deltaTime = 1.f/60.0;
bool paused = false;

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

    return boid;
}

void InitBoids()
{
    int n = 300;
	for (int i = 0; i < n; i++)
    {
		boids.push_back(
            CreateBoid(vec3f(random(-width, width), random(-height, height), random(-depth, depth)), 
                       vec3f(random(-maxSpeed, maxSpeed), random(-maxSpeed, maxSpeed), random(-maxSpeed, maxSpeed))));
    }

	// spheres = createInstancedRenderable(
    //     Sphere(
	// 			Centroid(0,0,0),
	// 			Radius(0.3),
	// 			AzimuthPoints(6),
	// 			AltitudePoints(6)),
    //     Phong(
    //         Colour(0.0, 0.5, 1.0),
    //         LightPosition(30.0, 30.0, 50.0)
    //     )
    // );

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
	int n = 8;
	for (int i = 0; i < n; i++)
    {
		obstacles.push_back(vec3f(random(-width, width), random(-height, height), random(-depth, depth)));
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

	// if (boid->position.x > width)
	// 	velocity.x = -speed;
	// else if (boid->position.x < -width)
	// 	velocity.x = speed;

	// if (boid->position.y > height)
	// 	velocity.y = -speed;
	// else if (boid->position.y < -height)
	// 	velocity.y = speed;

	// if (boid->position.z > depth)
	// 	velocity.z = -speed;
	// else if (boid->position.z < -depth)
	// 	velocity.z = speed;

	// return velocity;
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
		// if (count >= 5) break;

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
		// if (count >= 5) break;

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

void MoveBoids()
{
	vec3f v1, v2, v3, v4;
	for (Boid * b : boids)
	{
		v1 = Cohesion(b);
		v2 = Separation(b);
		v3 = Alignment(b);
		// v4 = BoundPosition(b);

		b->velocity += (v1 + v2 + v3);// + v4);

		if (length(b->velocity) > maxSpeed)
			b->velocity = (b->velocity / length(b->velocity)) * maxSpeed;

		BoundPosition(b);

		b->prevPosition = b->position;
		b->position += b->velocity * deltaTime;
	}
}

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

    glClearColor(0.f, 0.075f, 0.15f, 1.0f);

    InitBoids();
	InitObstacles();

    window.run([&](float frameTime) 
	{
		view.projection.updateAspectRatio(window.width(), window.height());

		if (!paused)
		{
			for(int i = 0; i < iterations; i++)
				MoveBoids();
		}
		
        for (Boid * b : boids)
        {
			vec3f direction = b->position - b->prevPosition;
			direction /= length(direction);
			vec3f target = b->position + (0.01f * direction);

			vec3f right = cross(direction, vec3f(0,1,0));
			vec3f up = cross(right, direction);

			auto m = translate(mat4f{1.f}, b->position);
			m *= lookAt(b->position, target, up);
			m = translate(m, b->position); // Needed this otherwise the fish would teleport...
			m = scale(m, vec3f{0.25f});

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
