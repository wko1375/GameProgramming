#include "Entity.h"

Entity::Entity() {
	width = 0;
	height = 0;
	position = glm::vec3(0);
	movement = glm::vec3(0);
	acceleration = glm::vec3(0);
	velocity = glm::vec3(0);
	speed = 0;
	textureID = NULL;
	modelMatrix = glm::mat4(1.0f);
	isActive = true;
	entityType = PLAYER;
	killCount = 0;
	lives = 3;
}

bool Entity::CheckCollision(Entity other) {
	if (!isActive || !(other.isActive)) {
		return false;
	}
	float xdist = fabs(position.x - other.position.x) - ((width + other.width) / 2.0f);
	float ydist = fabs(position.y - other.position.y) - ((height + other.height) / 2.0f);

	if (xdist < 0 && ydist < 0) {
		return true;
	}
	else {
		return false;
	}
}

void Entity::CheckCollisionVert(Entity other) {
	if (CheckCollision(other)) {
		position.x = -10.0f;
		float ydist = fabs(position.y - other.position.y);
		float penetrationY = fabs(ydist - (height / 2) - (other.height / 2));
		if (velocity.y > 0) {
			position.y -= penetrationY;
			velocity.y = 0;
			colTop = true;
			isActive = false;
		}
		else if (velocity.y < 0) {
			position.y += penetrationY;
			velocity.y = 0;
			colBottom = true;
			other.isActive = false;
		}
	}
}

void Entity::Update(float deltaTime, Entity e, Entity* platforms, int PLATFORM_COUNT, Entity* enemies, int enemyCount) {
	if (entityType == PLAYER) {
		for (int i = 0; i < enemyCount; i++) {
			if (CheckCollision(enemies[i])) {
				if (position.y > enemies[i].position.y + 0.5) {
					killCount += 1;
					enemies[i].isActive = false;
				}
				else {
					if (lives > 1) {
						lives -= 1;
						position = glm::vec3(-4.0f, -2.5f, 0.0f);
					}
					else {
						isActive = false;
					}
				}
			}
		}
	}

	if (entityType == ENEMY) {
		if (aistate == WALKING) {
			velocity.x = -1.0f;
			position.x += velocity.x * deltaTime;
		}
		if (aistate == JUMPING) {
			if (position.y >= 0.0f) {
				direction = DOWN;
			}
			else if (position.y == -2.5f) {
				direction = UP;
			}
			if (direction == UP) {
				velocity.y = 0.5f;
				position.y += velocity.y * deltaTime;
			}
			if (direction == DOWN) {
				velocity.y = -0.5f;
				position.y += velocity.y * deltaTime;
			}
		}
		if (aistate == PATROLLING) {
			if (position.x >= 9.5f) {
				direction = LEFT;
			}
			if (position.x <= 4.0f) {
				direction = RIGHT;
			}
			if (direction == RIGHT) {
				velocity.x = 1.0f;
				position.x += velocity.x * deltaTime;
			}
			if (direction == LEFT) {
				velocity.x = -1.0f;
				position.x += velocity.x * deltaTime;
			}
		}
	}
	if (entityType == PLAYER) {
		velocity += acceleration * deltaTime;
		position.y += velocity.y * deltaTime;
		position.x += velocity.x * deltaTime;
	}
	velocity = glm::vec3(0);
}


void Entity::Render(ShaderProgram* program) {
	if (!isActive) {
		return;
	}
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	program->SetModelMatrix(modelMatrix);
	float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };


	glBindTexture(GL_TEXTURE_2D, textureID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}
void Entity::Render(ShaderProgram* program, float vertices[]) {
	if (!isActive) {
		return;
	}
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	program->SetModelMatrix(modelMatrix);
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };


	glBindTexture(GL_TEXTURE_2D, textureID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

