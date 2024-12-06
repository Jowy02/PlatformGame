#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	
	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

public:

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	//Audio fx
	int pickCoinFxId;
	int jumpSfx;
	int dieSfx;
	int enemyKillSfx;
	int hitSfx;


	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	float jumpForce = 0.35f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping
	bool godmode = false;

	bool look = false;
	bool oneTime = false;
	bool deadAnimation = false;

	bool hitR = false;
	bool hitL = false;

	bool kill = false;

	int cnt = 0; 

	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
	Animation backidle;

	Animation walk;
	Animation backwalk;

	Animation jump;
	Animation backjump;

	Animation dead;

	Vector2D savePos;
};