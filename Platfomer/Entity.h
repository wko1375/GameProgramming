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
#include "Map.h"

enum EntityType { PLAYER, PLATFORM, ENEMY };
enum AIState { IDLE, WALKING, JUMPING, PATROLLING };
enum Behavior { WALKER, JUMPER, PATROLLER };
enum Direction { LEFT, RIGHT, UP, DOWN };

class Entity {
public:
	EntityType entityType;
	AIState aistate;
	Direction direction;

	glm::vec3 position;
	glm::vec3 movement;
	glm::vec3 acceleration;
	glm::vec3 velocity;

	int killCount;
	bool colTop;
	bool colBottom;
	bool colLeft;
	bool colRight;
	bool isActive;
	float speed;
	int width;
	int height;


	GLuint textureID;

	glm::mat4 modelMatrix;

	Entity();

	bool CheckCollision(Entity other);
	void CheckCollisionVert(Entity other);
	void Update(float deltaTime, Entity e, Map* map, Entity* enemies, int enemyCount);
	void Render(ShaderProgram* program);
	void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);

};