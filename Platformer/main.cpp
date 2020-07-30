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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Entity.h"
#include "Map.h"
#include <SDL_mixer.h>
#define PLATFORM_COUNT 21
#define ENEMY_COUNT 3

struct GameState {
	Entity* player;
	Map* map;
	Entity platforms[PLATFORM_COUNT];
	Entity enemies[ENEMY_COUNT];
	Entity* gamelose;
	Entity* gamewin;
	Entity* menu;
};

#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5

unsigned int level1_data[]=
{
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
 0, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2
};

Mix_Music* music;
Mix_Chunk* bounce;

enum GameMode { GAME_LEVEL, GAME_OVER, GAME_WIN, MENU };
GameMode mode = MENU;

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;
ShaderProgram program;

glm::mat4 viewMatrix, projectionMatrix;

GLuint spriteSheetTextureID;

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return textureID;
}

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	displayWindow = SDL_CreateWindow("William Ko Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 500, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	

#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 900, 500);
	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
	viewMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-10.0f, 10.0f, -5.0f, 5.0f, -1.0f, 1.0f);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 1.0f, 1.0f, 0.0f);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	music = Mix_LoadMUS("dooblydoo.mp3");
	Mix_PlayMusic(music, -1);

	bounce = Mix_LoadWAV("bounce.wav");

	glUseProgram(program.programID);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	GLuint playerTextureID = LoadTexture("player.png");
	GLuint platformTextureID = LoadTexture("platform.png");
	GLuint enemyTextureID = LoadTexture("ctg.png");

	state.player = new Entity();
	state.player->position = glm::vec3(-4.0f, -2.5f, 0.0f);
	state.player->movement = glm::vec3(0);
	state.player->acceleration = glm::vec3(0, 0, 0);
	state.player->speed = 1.0f;
	state.player->textureID = playerTextureID;
	state.player->entityType = PLAYER;
	state.player->width = 1;
	state.player->height = 1;

	state.gamelose = new Entity();
	state.gamelose->position = glm::vec3(0, 0, 0);
	state.gamelose->textureID = LoadTexture("lose.png");

	state.gamewin = new Entity();
	state.gamewin->position = glm::vec3(0, 0, 0);
	state.gamewin->textureID = LoadTexture("win.png");

	state.menu = new Entity();
	state.menu->position = glm::vec3(0, 0, 0);
	state.menu->textureID = LoadTexture("platformer.png");
	
	for (int i = 0; i < PLATFORM_COUNT; i++) {
		state.platforms[i].entityType = PLATFORM;
		state.platforms[i].textureID = platformTextureID;
		state.platforms[i].position = glm::vec3(-10 + i, -3.5f, 0);
	}

	state.enemies[0].position = glm::vec3(5.0f, -2.5f, 0.0f);
	state.enemies[0].movement = glm::vec3(0);
	state.enemies[0].speed = 1.0f;
	state.enemies[0].entityType = ENEMY;
	state.enemies[0].textureID = enemyTextureID;
	state.enemies[0].aistate = WALKING;
	state.enemies[0].width = 1;
	state.enemies[0].height = 1;

	state.enemies[1].position = glm::vec3(3.0f, -2.5f, 0.0f);
	state.enemies[1].movement = glm::vec3(0);
	state.enemies[1].speed = 1.0f;
	state.enemies[1].entityType = ENEMY;
	state.enemies[1].textureID = enemyTextureID;
	state.enemies[1].aistate = JUMPING;
	state.enemies[1].width = 1;
	state.enemies[1].height = 1;

	state.enemies[2].position = glm::vec3(7.0f, -2.5f, 0.0f);
	state.enemies[2].movement = glm::vec3(0);
	state.enemies[2].speed = 1.0f;
	state.enemies[2].entityType = ENEMY;
	state.enemies[2].textureID = enemyTextureID;
	state.enemies[2].aistate = PATROLLING;
	state.enemies[2].direction = RIGHT;
	state.enemies[2].width = 1;
	state.enemies[2].height = 1;



	GLuint mapTextureID = LoadTexture("tileset.png");
	state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 4, 1);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}



void ProcessInputGameLevel() {
	state.player->velocity = glm::vec3(0, -0.75f, 0);
	if (state.player->position.y <= -2.5) {
		state.player->velocity.y = 0;
	}
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
	if (keys[SDL_SCANCODE_LEFT]) {
		state.player->velocity.x = -2.0f;
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		state.player->velocity.x = 2.0f;
	}
	if (keys[SDL_SCANCODE_SPACE]) {
		state.player->velocity.y = 5.0f;
		Mix_PlayChannel(-1, bounce, 0);
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
	state.player->velocity = glm::vec3(0, -0.75f, 0);
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

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0.0f;
float accumulator = 0.0f;

void UpdateGameLevel(float deltaTime) {
	state.player->Update(deltaTime, *state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);
	state.enemies[0].Update(deltaTime, *state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);
	state.enemies[1].Update(deltaTime, *state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);
	state.enemies[2].Update(deltaTime, *state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);
	if (state.player->isActive == false) {
		mode = GAME_OVER;
	}
	if (state.player->killCount == 3) {
		mode = GAME_WIN;
	}
}

void UpdateGameOver(float deltaTime) {
	;
}

void UpdateGameWin(float deltaTime) {
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

void RenderGameLevel() {
	state.player->Render(&program);
	for (int i = 0; i < ENEMY_COUNT; i++) {
		state.enemies[i].Render(&program);
	}
	for (int i = 0; i < PLATFORM_COUNT; i++) {
		state.platforms[i].Render(&program);
	}
}

void RenderGameOver() {
	float vertices[] = { -2.5f, -2.5f, 2.5f, -2.5f, 2.5f, 2.5f, -2.5f, -2.5f, 2.5f, 2.5f, -2.5f, 2.5f };
	state.gamelose->Render(&program, vertices);

}

void RenderGameWin() {
	float vertices[] = { -2.5f, -2.5f, 2.5f, -2.5f, 2.5f, 2.5f, -2.5f, -2.5f, 2.5f, 2.5f, -2.5f, 2.5f };
	state.gamewin->Render(&program, vertices);

}

void RenderMenu() {
	float vertices[] = { -2.5f, -2.5f, 2.5f, -2.5f, 2.5f, 2.5f, -2.5f, -2.5f, 2.5f, 2.5f, -2.5f, 2.5f };
	state.menu->Render(&program, vertices);
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

void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();
	while (gameIsRunning) {
		ProcessInput();
		Update();
		Render();
	}

	Shutdown();
	return 0;
}
