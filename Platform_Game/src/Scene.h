#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>

struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Enemy hitbox
	void EnemyHitbox();

	void Save();
	void Load();

	Vector2D GetPlayerPosition();

public:
	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;

	}
	int mapLevel = 0;

	std::vector<Enemy*> enemyList;

private:
	SDL_Texture* img;

	SDL_Texture* mouseTileTex = nullptr;
	std::string tilePosDebug = "[0,0]";
	bool once = false;

	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	pugi::xml_document configFile;

	//sfx
	int actualCheck = 0;
	int check;

	int margin = 8;
	int enemyRange = 100;
};