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

SDL_Window * displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix, ballMatrix, leftMatrix, rightMatrix;
glm::vec3 left_paddle_movement = glm::vec3(0, 0, 0);
glm::vec3 right_paddle_movement = glm::vec3(0, 0, 0);
glm::vec3 left_paddle_position = glm::vec3(-4, 0, 0);
glm::vec3 right_paddle_position = glm::vec3(4, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);




void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("William Ko Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 750, 500, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 750, 500);

	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
	
	viewMatrix = glm::mat4(1.0f);
	leftMatrix = glm::mat4(1.0f);
	rightMatrix = glm::mat4(1.0f);
	ballMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(0.0f, 0.0f, 0.0f, 0.0f);

	glUseProgram(program.programID);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

void ProcessInput() {
	left_paddle_movement = glm::vec3(0, 0, 0);
	right_paddle_movement = glm::vec3(0, 0, 0);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
				ball_movement.y = 0.75f;
				ball_movement.x = 1.0f;
			}
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_W]) {
		if (left_paddle_position.y <= 2.75) {
			left_paddle_movement.y = 2.5f;
		}
	}
	if (keys[SDL_SCANCODE_S]) {
		if (left_paddle_position.y >= -2.75) {
			left_paddle_movement.y = -2.5f;
		}
	}
	if (keys[SDL_SCANCODE_UP]) {
		if (right_paddle_position.y <= 2.75) {
			right_paddle_movement.y = 2.5f;
		}	
	}
	if (keys[SDL_SCANCODE_DOWN]) {
		if (right_paddle_position.y >= -2.75) {
			right_paddle_movement.y = -2.5f;
		}	
	}
}

float lastTicks = 0.0f;

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	if (ball_position.y >= 3.65 || ball_position.y <= -3.65) {
		ball_movement.y *= -1.0f;
	}
	
	if (ball_position.x >= 4.9|| ball_position.x <= -4.9){
		gameIsRunning = false;
	}

	float rightxdist = fabs(right_paddle_position.x - ball_position.x) - ((0.3f + 0.2f) / 2.0f);
	float rightydist = fabs(right_paddle_position.y - ball_position.y) - ((2.0f + 0.2f) / 2.0f);

	float leftxdist = fabs(left_paddle_position.x - ball_position.x) - ((0.3f + 0.2f) / 2.0f);
	float leftydist = fabs(left_paddle_position.y - ball_position.y) - ((2.0f + 0.2f) / 2.0f);

	if (rightxdist < 0 && rightydist < 0) {
		ball_movement.x *= -1.0f;
		ball_movement.y *= -1.0f;
	}

	else if (leftxdist < 0 && leftydist < 0) {
		ball_movement.x *= -1.0f;
		ball_movement.y *= -1.0f;
	}


	ball_position += ball_movement * deltaTime;
	
	left_paddle_position += left_paddle_movement * deltaTime;
	
	right_paddle_position += right_paddle_movement * deltaTime;
	
	ballMatrix = glm::mat4(1.0f);
	ballMatrix = glm::translate(ballMatrix, ball_position);

	leftMatrix = glm::mat4(1.0f);
	leftMatrix = glm::translate(leftMatrix, left_paddle_position);

	rightMatrix = glm::mat4(1.0f);
	rightMatrix = glm::translate(rightMatrix, right_paddle_position);
	
}


void Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	program.SetColor(0.0f, 0.0f, 0.0f, 1.0f);
	program.SetModelMatrix(leftMatrix);
	float vertices[] = { -0.15f, 1.0f, 0.15f, 1.0f, -0.15f, -1.0f, 0.15f, 1.0f, -0.15f, -1.0f, 0.15f, -1.0f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

	program.SetModelMatrix(rightMatrix);
	float rightVertices[] = { 0.15f, 1.0f, -0.15f, 1.0f, 0.15f, -1.0f, -0.15f, 1.0f, 0.15f, -1.0f, -0.15f, -1.0f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightVertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

	program.SetModelMatrix(ballMatrix);
	float ballVertices[] = { -0.1f, -0.1f, -0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, -0.1f, -0.1f, 0.1f, -0.1f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

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