#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "Item.h"
#include <vector>
#include "GuiControlButton.h"

struct SDL_Texture;
enum buttonInteract
{
	TITLE,RESUME,SETTINGS,EXIT,MUSIC,FX,
	FULL_SCREEN,CONTINUE,START,CREDITS
};
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
	void StartNewGame();
	void ChangeLevel(int level);

	bool OnGuiMouseClickEvent(GuiControl* control);

	Vector2D GetPlayerPosition();

public:
	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;
	}

	std::vector<Enemy*> enemyList;
	std::vector<Item*> itemList;

	bool activeMenu = true;
	int mapLevel = 0;
	bool set = false;

private:
	SDL_Texture* img;

	SDL_Texture* mouseTileTex = nullptr;
	std::string tilePosDebug = "[0,0]";
	bool once = false;
	bool exit = true;
	bool visableBt = false;

	bool start = true;

	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	pugi::xml_document configFile;

	//sfx
	int actualCheck = 0;
	int check;

	int margin = 8;
	int enemyRange = 100;

	GuiControlButton* StartBt;
	GuiControlButton* ContinuetBt;
	GuiControlButton* CreditstBt;

	GuiControlButton* BackTitleBt;
	GuiControlButton* ResumeBt;
	GuiControlButton* SettingsBt;
	GuiControlButton* ScapeBt;

	GuiControlButton* MusicSlider;
	GuiControlButton* FxSlider;
	GuiControlButton* FulScreenCb;

	bool Fullscreen = false;
	bool saveGame;
	 
	SDL_Texture* Menu = nullptr;
	SDL_Texture* Transition = nullptr;
	SDL_Texture* SettingM = nullptr;
	SDL_Texture* CreditsM = nullptr;
	SDL_Texture* startM = nullptr;
	
	bool showTransition = false;
	bool credits = false;

	int transitionCnt = 0;

	Timer inGameTimer;
	int minutes = 0;
};