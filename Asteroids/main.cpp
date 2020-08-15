#define GL_SILENCE_DEPRECIATION
#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <vector>
#include <SDL.h>
#include<SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Entity.h"
#include <random>
#include <ctime>
#include <cstdlib>
SDL_Window * displayWindow;
bool gameIsRunning = true;
ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

enum GameMode { GAME_LEVEL, GAME_OVER, GAME_WIN, MENU };
GameMode mode = MENU;

#define ENEMY_COUNT 10
#define BULLET_COUNT 100
#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0.0f;
float accumulator = 0.0f;
int timePassed = 0;

struct GameState {
	Entity* player;
	Entity* menu;
	Entity* gameOver;
	Entity* gameWin;
	std::vector<Entity> asteroids;
	std::vector<Entity> bullets;
};

GameState state;

GLuint LoadTexture(const char* filePath) {
	int w, h, n;
	unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	stbi_image_free(image);
	return textureID;
}

float RandomFloat(float min, float max)
{
	assert(max > min);
	float random = ((float)rand()) / (float)RAND_MAX;
	float range = max - min;
	return (random * range) + min;
}

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	displayWindow = SDL_CreateWindow("Final Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 400, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 400, 400);
	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	GLuint playerTextureID = LoadTexture("playership.png");

	state.player = new Entity();
	state.player->position = glm::vec3(0);
	state.player->velocity = glm::vec3(0);
	state.player->width = 1;
	state.player->height = 1;
	state.player->textureID = playerTextureID;
	state.player->entityType = PLAYER;

	state.menu = new Entity();
	state.menu->position = glm::vec3(0, 0, 0);
	state.menu->textureID = LoadTexture("finalprojectmenulogo.png");

	state.gameOver = new Entity();
	state.gameOver->position = glm::vec3(0, 0, 0);
	state.gameOver->textureID = LoadTexture("lose.png");

	state.gameWin = new Entity();
	state.gameWin->position = glm::vec3(0, 0, 0);
	state.gameWin->textureID = LoadTexture("win.png");

	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

Entity generateAsteroid(Direction d) {
	GLuint asteroidTextureID = LoadTexture("meteorBrown_big3.png");
	Entity* asteroid = new Entity();
	asteroid->textureID = asteroidTextureID;
	asteroid->entityType = ASTEROID;
	asteroid->asteroidSize = BIG;

	float xPos;
	float yPos;
	float xVel;
	float yVel;

	if (d == UP) {
		xPos = RandomFloat(-2.0f, 2.0f);
		yPos = 5.0f;
		xVel = RandomFloat(-0.5f, 0.5f);
		yVel = -2.0f;
	}
	else if (d == RIGHT) {
		xPos = 5.0f;
		yPos = RandomFloat(-2.0f, 2.0f);
		xVel = -2.0f;
		yVel = RandomFloat(-0.5f, 0.5f);
	}
	else if (d == LEFT) {
		xPos = RandomFloat(-2.0f, 2.0f);
		yPos = -5.0f;
		xVel = 2.0f;
		yVel = RandomFloat(-0.5f, 0.5f);
	}
	else {
		xPos = -5.0f;
		yPos = RandomFloat(-2.0f, 2.0f);
		xVel = RandomFloat(-0.5f, 0.5f);
		yVel = 2.0f;
	}

	asteroid->position = glm::vec3(xPos, yPos, 0.0f);
	asteroid->velocity = glm::vec3(xVel, yVel, 0.0f);
	asteroid->width = 2;
	asteroid->height = 2;

	return *asteroid;
}

void ProcessInputGameLevel() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_w:
				state.player->acceleration.y = 75.0f;
				break;
			case SDLK_s:
				state.player->acceleration.y = -75.0f;
				break;
			case SDLK_a:
				state.player->acceleration.x = -75.0f;
				break;
			case SDLK_d:
				state.player->acceleration.x = 75.0f;
				break;
			case SDLK_RIGHT:
				state.player->rotation = R;
				if (state.player->direction == UP) {
					state.player->direction = RIGHT;
				}
				else if (state.player->direction == RIGHT) {
					state.player->direction = DOWN;
				}
				else if (state.player->direction == DOWN) {
					state.player->direction = LEFT;
				}
				else {
					state.player->direction = UP;
				}
				break;
			case SDLK_LEFT:
				state.player->rotation = L;
				if (state.player->direction == UP) {
					state.player->direction = LEFT;
				}
				else if (state.player->direction == RIGHT) {
					state.player->direction = UP;
				}
				else if (state.player->direction == DOWN) {
					state.player->direction = RIGHT;
				}
				else {
					state.player->direction = DOWN;
				}
				break;
			case SDLK_SPACE:
				Entity b = state.player->Shoot();
				b.textureID = LoadTexture("laserBlue07.png");
				state.bullets.push_back(b);
				break;
			}
		}
	}
}

void ProcessInputGameOver() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;
		}
	}
}

void ProcessInputGameWin() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;
		}
	}
}

void ProcessInputMenu() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;
		}
	}
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_RETURN]) {
		mode = GAME_LEVEL;
	}
}

void RenderMenu() {
	float vertices[] = { -5.0f, -2.5f, 5.0f, -2.5f, 5.0f, 2.5f, -5.0f, -2.5f, 5.0f, 2.5f, -5.0f, 2.5f };
	state.menu->Render(&program, vertices);
}

void RenderGameLevel() {
	state.player->Render(&program);
	for (int i = 0; i < state.asteroids.size(); i++) {
		if (state.asteroids[i].asteroidSize == BIG) {
			float vertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
			state.asteroids[i].Render(&program, vertices);
		}
		else if (state.asteroids[i].asteroidSize == MEDIUM) {
			float vertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f,-0.5f, 0.5f };
			state.asteroids[i].Render(&program, vertices);
		}
		else if (state.asteroids[i].asteroidSize == SMALL) {
			float vertices[] = { -0.25f, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f, -0.25f, -0.25f, 0.25f, 0.25f,-0.25f, 0.25f };
			state.asteroids[i].Render(&program, vertices);
		}	
	}
	for (int i = 0; i < state.bullets.size(); i++) {
		float vertices[] = { -0.2f, -0.3f, 0.2f, -0.3f, 0.2f, 0.3f, -0.2f, -0.3f, 0.2f, 0.3f, -0.2f, 0.3f };
		state.bullets[i].Render(&program, vertices);
	}
}

void RenderGameOver() {
	state.gameOver->Render(&program);
}

void RenderGameWin() {
	state.gameWin->Render(&program);
}

void UpdateGameLevel(float deltaTime) {
	if (state.player->isActive == false) {
		mode = GAME_OVER;
		return;
	}

	timePassed += 1;
	state.player->Update(FIXED_TIMESTEP, *state.player, state.asteroids, state.bullets);
	int randDirection = rand() % 4;

	if (timePassed % 100 == 0) {
		Entity asteroid;
		if (randDirection == 0) {
			asteroid = generateAsteroid(UP);
		}
		else if (randDirection == 1) {
			asteroid = generateAsteroid(DOWN);
		}
		else if (randDirection == 2) {
			asteroid = generateAsteroid(LEFT);
		}
		else if (randDirection == 3) {
			asteroid = generateAsteroid(RIGHT);
		}
		state.asteroids.push_back(asteroid);
	}

	for (int i = 0; i < state.asteroids.size(); i++) {
		if (state.asteroids[i].isActive == false) {
			state.asteroids.erase(state.asteroids.begin() + i);
			continue;
		}
		state.asteroids[i].Update(FIXED_TIMESTEP, state.asteroids[i], state.asteroids, state.bullets);
	}

	for (int i = 0; i < state.bullets.size(); i++) {
		if (state.bullets[i].isActive == false) {
			state.bullets.erase(state.bullets.begin() + i);
			continue;
		}
		state.bullets[i].Update(FIXED_TIMESTEP,state.bullets[i], state.asteroids, state.bullets);
	}
}

void UpdateGameWin(float deltaTime) {
	;
}

void UpdateGameOver(float deltaTime) {
	;
}

void UpdateMenu(float deltaTime) {
	;
}

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;
	deltaTime += accumulator;

	if (deltaTime < FIXED_TIMESTEP) {
		accumulator = deltaTime;
		return;
	}
	while (deltaTime >= FIXED_TIMESTEP) {
		switch (mode) {
		case MENU:
			UpdateMenu(FIXED_TIMESTEP);
			break;
		case GAME_LEVEL:
			UpdateGameLevel(FIXED_TIMESTEP);
			break;
		case GAME_OVER:
			UpdateGameOver(FIXED_TIMESTEP);
			break;
		case GAME_WIN:
			UpdateGameWin(FIXED_TIMESTEP);
			break;
		}
		deltaTime -= FIXED_TIMESTEP;
	}
	accumulator = deltaTime;
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (mode) {
	case MENU:
		RenderMenu();
		break;
	case GAME_LEVEL:
		RenderGameLevel();
		break;
	case GAME_OVER:
		RenderGameOver();
		break;
	case GAME_WIN:
		RenderGameWin();
		break;
	}
	SDL_GL_SwapWindow(displayWindow);
}

void ProcessInput() {
	switch (mode) {
	case MENU:
		ProcessInputMenu();
		break;
	case GAME_LEVEL:
		ProcessInputGameLevel();
		break;
	case GAME_OVER:
		ProcessInputGameOver();
		break;
	case GAME_WIN:
		ProcessInputGameWin();
		break;
	}
}

void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	srand(time(0));
	Initialize();
	while (gameIsRunning) {
		ProcessInput();
		Update();
		Render();
	}

	Shutdown();
	return 0;
}