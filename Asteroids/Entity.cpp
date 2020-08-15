#include "Entity.h"
#include <vector>

Entity::Entity() {
	width = 0;
	height = 0;
	position = glm::vec3(0);
	movement = glm::vec3(0);
	velocity = glm::vec3(0);
	acceleration = glm::vec3(0);
	textureID = NULL;
	modelMatrix = glm::mat4(1.0f);
	entityType = PLAYER;
	score = 0;
	isActive = true;
	rotation = N;
	direction = UP;
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

void Entity::Render(ShaderProgram* program) {
	if (!isActive) {
		return;
	}
	
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
	
void Entity::Update(float deltaTime, Entity e, std::vector<Entity> &asteroids, std::vector<Entity> &bullets) {
	modelMatrix = glm::mat4(1.0f);
	if (!isActive) {
		return;
	}
	if (entityType == BULLET) {
		if (position.x > 5.1f || position.y > 5.3f || position.x < -5.1f || position.y < -5.3f) {
			isActive = false;
			return;
		}
		position.y += velocity.y * deltaTime;
		position.x += velocity.x * deltaTime;
		modelMatrix = glm::translate(modelMatrix, position);
		return;
	}
	else if (entityType == ASTEROID) {
		if (asteroidSize == BIG) {
			if (position.x > 6.0f || position.y > 6.0f || position.x < -6.0f || position.y < -6.0f) {
				isActive = false;
				return;
			}
		}
		else if (asteroidSize == MEDIUM) {
			if (position.x > 5.5f || position.y > 5.5f || position.x < -5.5f || position.y < -5.5f) {
				isActive = false;
				return;
			}
		}
		else {
			if (position.x > 5.25f || position.y > 5.25f || position.x < -5.25f || position.y < -5.25f) {
				isActive = false;
				return;
			}
		}
		
		for (int i = 0; i < bullets.size(); i++) {
			if (CheckCollision(bullets[i])) {
				isActive = false;
				bullets[i].isActive = false;
				explode(asteroids);
				return;
			}
		}
		position.y += velocity.y * deltaTime;
		position.x += velocity.x * deltaTime;
		modelMatrix = glm::translate(modelMatrix, position);
		return;
	}
	else if (entityType == PLAYER) {
		for (int i = 0; i < asteroids.size(); i++) {
			if (CheckCollision(asteroids[i])) {
				if (lives >= 1) {
					for (int i = 0; i < asteroids.size(); i++) {
						asteroids[i].isActive = false;
						asteroids.clear();
					}
					position = glm::vec3(0, 0, 0);
					lives -= 1;
					return;
				}
				else {
					isActive = false;
					return;
				}
			}
		}
		velocity += acceleration * deltaTime;
		position.y += velocity.y * deltaTime;
		position.x += velocity.x * deltaTime;
		modelMatrix = glm::translate(modelMatrix, position);

		if (rotation == R) {
			modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			rotation = N;
		}
		else if (rotation == L) {
			modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			rotation = N;
		}
		velocity = glm::vec3(0);
		rotate = 0.0f;
		return;
	}
}

void Entity::explode(std::vector<Entity> &asteroids) {
	Entity* a1 = new Entity();
	Entity* a2 = new Entity();
	a1->entityType = ASTEROID;
	a2->entityType = ASTEROID;

	if (asteroidSize == BIG) {
		a1->height = 1;
		a2->height = 1;
		a1->width = 1;
		a2->width = 1;
		a1->asteroidSize = MEDIUM;
		a2->asteroidSize = MEDIUM;
	}

	else if (asteroidSize == MEDIUM) {
		a1->height = 0.5;
		a2->height = 0.5;
		a1->width = 0.5;
		a2->width = 0.5;
		a1->asteroidSize = SMALL;
		a2->asteroidSize = SMALL;
	}

	else if(asteroidSize == SMALL){
		for (int i = 0; i < asteroids.size(); i++) {
			if (asteroids[i].velocity == this->velocity && asteroids[i].position == this->position) {
				asteroids.erase(asteroids.begin() + i);
				break;
			}
		}
		return;
	}

	if (velocity.x == 2.0f || velocity.x == -2.0f) {
		a1->position = glm::vec3(position.x, position.y, 0.0f);
		a2->position = glm::vec3(position.x, position.y, 0.0f);
	}
	else if (velocity.y == 2.0f || velocity.y == -2.0f) {
		a1->position = glm::vec3(position.x, position.y, 0.0f);
		a2->position = glm::vec3(position.x, position.y, 0.0f);
	}

	a1->velocity = glm::vec3(velocity.y, velocity.x, 0.0f);
	a2->velocity = glm::vec3(-velocity.y, -velocity.x, 0.0f);

	a1->textureID = textureID;
	a2->textureID = textureID;

	for (int i = 0; i < asteroids.size(); i++) {
		if (asteroids[i].velocity == this->velocity && asteroids[i].position == this->position) {
			asteroids.erase(asteroids.begin() + i);
			break;
		}
	}

	asteroids.push_back(*a1);
	asteroids.push_back(*a2);
}

Entity Entity::Shoot() {
	Entity* bullet = new Entity();
	bullet->entityType = BULLET;

	bullet->position = glm::vec3(position.x, position.y, 0.0f);
	bullet->modelMatrix = glm::mat4(1.0f);
	

	if (direction == UP) {
		bullet->velocity = glm::vec3(0.0f, 2.0f, 0.0f);
	}
	else if (direction == DOWN) {
		bullet->velocity = glm::vec3(0.0f, -2.0f, 0.0f);
		bullet->modelMatrix = glm::rotate(bullet->modelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (direction == RIGHT) {
		bullet->velocity = glm::vec3(2.0f, 0.0f, 0.0f);
		bullet->modelMatrix = glm::rotate(bullet->modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {
		bullet->velocity = glm::vec3(-2.0f, 0.0f, 0.0f);
		bullet->modelMatrix = glm::rotate(bullet->modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	bullet->width = 0.4;
	bullet->height = 0.6;
	return *bullet;
}