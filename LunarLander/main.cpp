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


struct GameState {
	Entity* player;
	Entity* platform;
	Entity* platform1;
	Entity* platform2;
	Entity* gamewin;
	Entity* gamelose;
};

enum GameMode {GAME_LEVEL, GAME_OVER, GAME_WIN};
GameMode mode = GAME_LEVEL;

GameState state;

SDL_Window * displayWindow;
bool gameIsRunning = true;
ShaderProgram program;

glm::mat4 viewMatrix, projectionMatrix;

GLuint mapTextureAtlasID;

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

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("William Ko Lunar Lander", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 750, 500, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 750, 500);
	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
	viewMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -1.0f, 1.0f);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(0.0f, 0.0f, 0.0f, 1.0f);

	glUseProgram(program.programID);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	state.player = new Entity();
	state.player->position = glm::vec3(0, 3.5f, 0);
	state.player->movement = glm::vec3(0);
	state.player->acceleration = glm::vec3(0, -0.75f, 0);
	state.player->speed = 1.0f;
	state.player->textureID = LoadTexture("spaceship.png");

	state.platform = new Entity;
	state.platform->textureID = LoadTexture("platform.png");
	state.platform->position = glm::vec3(-4.5f, -4.5f, 0);

	state.platform1 = new Entity;
	state.platform1->textureID = LoadTexture("platform.png");
	state.platform->position = glm::vec3(-3.5f, -4.5f, 0);

	state.platform2 = new Entity;
	state.platform2->textureID = LoadTexture("platform.png");
	state.platform2->position = glm::vec3(-2.5f, -4.5, 0);
	
	state.gamewin = new Entity;
	state.gamewin->textureID = LoadTexture("win.png");
	state.gamewin->position = glm::vec3(0, 0, 0);

	state.gamelose = new Entity;
	state.gamelose->textureID = LoadTexture("lose.png");
	state.gamelose->position = glm::vec3(0, 0, 0);
}


void ProcessInputGameLevel() {
	state.player->acceleration = glm::vec3(0, -0.75f, 0);
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
	if (keys[SDL_SCANCODE_UP]) {
		state.player->acceleration.y = 1.0f;
	}
	if (keys[SDL_SCANCODE_LEFT]) {
		state.player->acceleration.x = -1.0f;
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		state.player->acceleration.x = 1.0f;
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

void ProcessInput() {
	switch (mode) {
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

float lastTicks = 0.0f;

void UpdateGameLevel(float deltaTime) {
	state.player->Update(deltaTime);
	if (state.player->position.y <= -3.5f && state.player->position.x >= -3.5f && state.player->position.x <= -2.5f) {
		mode = GAME_WIN;
	}
	else if (state.player->position.x <= -4.5f || state.player->position.x >= 4.5f || state.player->position.y <= -4.5f) {
		mode = GAME_OVER;
	}
}

void UpdateGameOver(float deltaTime) {
	;
}

void UpdateGameWin(float deltaTime) {
	;
}

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	switch (mode) {
	case GAME_LEVEL:
		UpdateGameLevel(deltaTime);
		break;
	case GAME_OVER:
		UpdateGameOver(deltaTime);
		break;
	case GAME_WIN:
		UpdateGameWin(deltaTime);
		break;
	}

}

void RenderGameLevel(float v1[], float v2[], float v3[]) {
	state.player->Render(&program, v1);

	state.platform1->Render(&program, v2);
	state.platform2->Render(&program, v3);
	SDL_GL_SwapWindow(displayWindow);
}

void RenderGameOver() {
	float vertices[] = { -2.5f, -2.5f, 2.5f, -2.5f, 2.5f, 2.5f, -2.5f, -2.5f, 2.5f, 2.5f, -2.5f, 2.5f };
	state.gamelose->Render(&program, vertices);
	SDL_GL_SwapWindow(displayWindow);
}

void RenderGameWin() {
	float vertices[] = { -2.5f, -2.5f, 2.5f, -2.5f, 2.5f, 2.5f, -2.5f, -2.5f, 2.5f, 2.5f, -2.5f, 2.5f };
	state.gamewin->Render(&program, vertices);
	SDL_GL_SwapWindow(displayWindow);
}

void Render() {
	float landerVertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f };
	float platform1Vertices[] = { -4.0f, -5.0f, -3.0f, -5.0f, -3.0f, -4.0f, -4.0f, -5.0f, -3.0f, -4.0f, -4.0f, -4.0f };
	float platform2Vertices[] = { -3.0f, -5.0f, -2.0f, -5.0f, -2.0f, -4.0f, -3.0f, -5.0f, -2.0f, -4.0f, -3.0f, -4.0f };

	switch (mode) {
	case GAME_LEVEL:
		RenderGameLevel(landerVertices, platform1Vertices, platform2Vertices);
		break;
	case GAME_OVER:
		RenderGameOver();
		break;
	case GAME_WIN:
		RenderGameWin();
		break;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	
	
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
