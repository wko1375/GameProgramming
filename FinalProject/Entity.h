#define GL_SILENCE_DEPRECATION
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include <vector>

enum EntityType {PLAYER, ASTEROID, BULLET, BARRIER, BACKGROUND};
enum AsteroidSize{BIG, MEDIUM, SMALL};
enum Direction{UP, DOWN, LEFT, RIGHT};
enum Rotation{R, L, N};

class Entity {
public:
	EntityType entityType;
	AsteroidSize asteroidSize;
	Direction direction;
	Rotation rotation;

	glm::vec3 position;
	glm::vec3 movement;
	glm::vec3 velocity;
	glm::vec3 acceleration;

	float speed;
	int width;
	int height;
	bool isActive;
	float rotate;
	int lives;

	GLuint textureID;
	glm::mat4 modelMatrix;

	Entity();

	bool CheckCollision(Entity other);
	void Update(float deltaTime, Entity e, std::vector<Entity> &asteroids, std::vector<Entity> &bullets, int &score, std::vector<Entity> &barriers);
	void Render(ShaderProgram* program);
	void Render(ShaderProgram* program, float vertices[]);

	Entity Shoot();
	void explode(std::vector<Entity> &asteroids);
};