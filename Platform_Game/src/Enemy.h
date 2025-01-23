#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

struct SDL_Texture;

class Enemy : public Entity
{
public:

	Enemy();
	virtual ~Enemy();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void Stop();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ResetPath();

public:
	bool found = false;
	bool playerNear = false;
	Vector2D initialPos;
	bool startFight = false;
	bool hited = false;

	bool endFight = false;
private:

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;

	Animation idle_R;
	Animation invisible_idle_R;
	Animation idle_L;
	Animation invisible_idle_L;
	Animation hit;

	PhysBody* pbody;
	Pathfinding* pathfinding;


	const char* typeEnemy;

	Vector2D vec[30];
	bool flyingEnemy;
	bool oneTime = false;
	bool debug = false;
	int cnt;
	bool p = false;
	bool rest = false;

	int changedir = 0;
	int r = 0;
	int ry = 0;
	int dir = 0; //hacer defines coon izquierda = 0 y derecha = 1
	int dirUD = 0;

	bool boss = false;
	int fase = 0;
	int boosPath = 0;
	int hitColdwon = 0;

};
