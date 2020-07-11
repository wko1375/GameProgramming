#include "Entity.h"

Entity::Entity()
{
	entityType = PLATFORM;
	isStatic = true;
	isActive = true;
	position = glm::vec3(0);
	speed = 0;
	width = 1;
	height = 1;
}

bool Entity::CheckCollision(Entity other)
{
	if (isStatic == true) return false;
	if (isActive == false || other.isActive == false) return false;

	float xdist = fabs(position.x - other.position.x) - ((width + other.width) / 2.0f);
	float ydist = fabs(position.y - other.position.y) - ((height + other.height) / 2.0f);

	if (xdist < 0 && ydist < 0)
	{
		lastCollision = other.entityType;
		return true;
	}

	return false;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
	for (int i = 0; i < objectCount; i++)
	{
		Entity object = objects[i];

		if (CheckCollision(object))
		{
			float ydist = fabs(position.y - object.position.y);
			float penetrationY = fabs(ydist - (height / 2) - (object.height / 2));
			if (velocity.y > 0) {
				position.y -= penetrationY;
				velocity.y = 0;
				collidedTop = true;
			}
			else if (velocity.y < 0) {
				position.y += penetrationY;
				velocity.y = 0;
				collidedBottom = true;
			}
			if (entityType == PLAYER && lastCollision == ENEMY && (collidedRight || collidedLeft)) {
				isStatic = true;
				velocity = glm::vec3(0, 0, 0);
				acceleration = glm::vec3(0, 0, 0);
			}
			else if (entityType == PLAYER && lastCollision == ENEMY && collidedBottom) {
				objects[i].isActive = false;
				lastCollision = PLATFORM;
				killcount++;
			}
		}
	}
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
	for (int i = 0; i < objectCount; i++)
	{
		Entity object = objects[i];

		if (CheckCollision(object))
		{
			float xdist = fabs(position.x - object.position.x);
			float penetrationX = fabs(xdist - (width / 2) - (object.width / 2));
			if (velocity.x > 0) {
				position.x -= penetrationX;
				velocity.x = 0;
				collidedRight = true;
			}
			else if (velocity.x < 0) {
				position.x += penetrationX;
				velocity.x = 0;
				collidedLeft = true;
			}
			if (entityType == PLAYER && lastCollision == ENEMY && (collidedRight || collidedLeft)) {
				isStatic = true;
				velocity = glm::vec3(0, 0, 0);
				acceleration = glm::vec3(0, 0, 0);
			}
		}
	}
}


void Entity::Jump()
{
	if (collidedBottom)
	{
		if (entityType == ENEMY) {
			velocity.y = 2.0f;
		}
		else {
			velocity.y = 5.0f;
		}
	}
}


void Entity::AIwalker(Entity player)
{
	switch (aiState) {
	case IDLE:
		if (glm::distance(position, player.position) < 2.5) {
			aiState = WALKING;
		}
		break;
	case WALKING:
		if (player.position.x > position.x) {
			velocity.x = 0.7;
		}
		else {
			velocity.x = -0.7f;
		}
		break;
	}

}

void Entity::AIfloater(Entity player)
{
	switch (aiState) {
	case IDLE:
		if (glm::distance(position, player.position) < 3.0) {
			aiState = FLOATING;
		}
		break;
	case FLOATING:
		if (player.position.x > position.x) {
			velocity.x = 2.3;
		}
		else {
			velocity.y = 0.25;
			if (glm::distance(position, player.position) > 3.0) {
				velocity.y = -0.25f;
			}
		}
		break;
	}

}

void Entity::AIjumper(Entity player) {
	//glClearColor(0.0f, 0.90f, 0.9f, 0.3f);
	switch (aiState) {
	case IDLE:
		if (glm::distance(position, player.position) < 10.0) {
			aiState = JUMPING;
		}
		break;
	case JUMPING:
		Jump();
		break;
	}
}

void Entity::AIupdate(Entity player) {
	switch (aiType) {
	case WALKER:
		AIwalker(player);
		break;
	case JUMPER:
		AIjumper(player);
		break;
	case FLOATER:
		AIfloater(player);
		break;
	}
}


void Entity::Update(float deltaTime, Entity player, Entity* objects, int objectCount, Entity* enemies, int enemyCount)
{
	collidedTop = false;
	collidedBottom = false;
	collidedLeft = false;
	collidedRight = false;

	velocity += acceleration * deltaTime;




	position.y += velocity.y * deltaTime;        // Move on Y
	CheckCollisionsY(objects, objectCount);    // Fix if needed

	if (entityType == ENEMY)
	{
		AIupdate(player);
	}
	if (entityType == PLAYER)
	{
		CheckCollisionsY(enemies, enemyCount);
	}
	position.x += velocity.x * deltaTime;        // Move on X
	CheckCollisionsX(objects, objectCount);    // Fix if needed

	if (entityType == PLAYER)
	{
		CheckCollisionsX(enemies, enemyCount);
	}
}




void Entity::Render(ShaderProgram* program) {
	if (isActive == false) {
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