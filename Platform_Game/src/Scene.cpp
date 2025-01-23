#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "Enemy.h"
#include "GuiControl.h"
#include "GuiManager.h"

Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	//L04: TODO 3b: Instantiate the player using the entity manager
	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("entities").child("player"));
	
	//L08 Create a new item using the entity manager and set the position to (200, 672) to test
	//Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	int id = 0;
	for (pugi::xml_node itemNode = configParameters.child("entities").child("items").child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
	{
		Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
		item->SetParameters(itemNode, id);
		id++;
		itemList.push_back(item);
	}

	// Create a enemy using the entity manager 
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
		enemy->SetParameters(enemyNode);
		enemyList.push_back(enemy);
	}

	SDL_Rect btPos = { 140, 363, 100,25 };
	BackTitleBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, TITLE, "Title", btPos, this);

	btPos = { 200, 363, 100,25 };
	ResumeBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, RESUME, "Resume", btPos, this);

	btPos = { 350, 363, 100,25 };
	SettingsBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, SETTINGS, "Settings", btPos, this);

	btPos = { 500, 363, 100,25 };
	ScapeBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, EXIT, "Exit", btPos, this);

	btPos = { 275, 200, 60,16 };
	MusicSlider = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, MUSIC, "Music", btPos, this);

	btPos = { 425, 200, 60,16 };
	FxSlider  = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, FX, "Fx", btPos, this);

	btPos = { 275, 250, 20,20 };
	FulScreenCb = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::CHECKBOX, FULL_SCREEN, "Full Screen", btPos, this);

	//Init Butons
	btPos = { 200, 363, 100,25 };
	StartBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, START, "Start ", btPos, this);

	btPos = { 50, 363, 100,25 };
	ContinuetBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, CONTINUE, "Continue", btPos, this);

	btPos = { 650, 363, 100,25 };
	CreditstBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, CREDITS, "Credits", btPos, this);


	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());

	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Maps/MapMetadata.png");
	Menu = Engine::GetInstance().textures.get()->Load("Assets/Textures/Main Menu.png");
	SettingM = Engine::GetInstance().textures.get()->Load("Assets/Textures/Pause.png");
	Transition = Engine::GetInstance().textures.get()->Load("Assets/Textures/Next_Stage.png");
	CreditsM = Engine::GetInstance().textures.get()->Load("Assets/Textures/Credits.png");
	startM = Engine::GetInstance().textures.get()->Load("Assets/Textures/Intro_Screen.png");

	check = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/1Up.ogg");

	enemyList[enemyList.size()-1]->Disable();
	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	//Change level
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F1) == KEY_DOWN && mapLevel == 1 )
	{
		mapLevel = 0;
		Engine::GetInstance().map->CleanUp();
		Engine::GetInstance().map->Load("Assets/Maps/", "Stage 1.tmx");
		Load();
		enemyList[enemyList.size() - 1]->Disable();
		for (int y = 0; y < itemList.size(); y++) {
			itemList[y]->Enable();
		}
		player->SetPosition({ 155, 192 });
		Engine::GetInstance().audio.get()->StopMusic();
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Ground Theme.ogg", 0.f);
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN && mapLevel == 0 || player->GetPosition().getX() >= 3500)
	{
		showTransition = true;
		mapLevel = 1;
		for (int y = 0; y < enemyList.size(); y++) {
			enemyList[y]->Disable();
		}
		for (int y = 0; y < itemList.size(); y++) {
			itemList[y]->Disable();
		}
		enemyList[enemyList.size()-1]->Enable();
		Engine::GetInstance().map->CleanUp();
		Engine::GetInstance().map->Load("Assets/Maps/", "Stage 2.tmx");
		player->SetPosition({ 111, 192 });

		Engine::GetInstance().audio.get()->StopMusic();
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {

		if(mapLevel == 0)player->SetPosition({155, 199});
		else player->SetPosition({ 111, 192 });
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F7) == KEY_DOWN) {
		if (mapLevel == 0)
		{
			if (actualCheck == 0) {
				float x = Engine::GetInstance().map.get()->checkpoints[0].getX();
				float y = Engine::GetInstance().map.get()->checkpoints[0].getY();
				player->SetPosition({ x, y });
				actualCheck = 1;
			}
			else if (actualCheck == 1) {
				float x = Engine::GetInstance().map.get()->checkpoints[1].getX();
				float y = Engine::GetInstance().map.get()->checkpoints[1].getY();
				player->SetPosition({ x, y });
				actualCheck = 2;
			}
			else if(actualCheck == 2){
				player->SetPosition({ 111, 192 });
				actualCheck = 0;
			}
		}
		else {
			player->SetPosition({ 111, 192 });
		}
	}
	if (player->GetPosition().getX() >= 350 && mapLevel == 1) enemyList[enemyList.size() - 1]->startFight = true;

	if(enemyList[enemyList.size() - 1]->endFight){
		if(transitionCnt == 0){
			Engine::GetInstance().audio.get()->StopMusic();
			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Stage Clear.ogg", 0.f);
		}
		transitionCnt++;
		if (transitionCnt >= 350) {

			SDL_Rect btPos = { 350, 363, 100,25 };
			SettingsBt->bounds = btPos;

			btPos = { 500, 363, 100,25 };
			ScapeBt->bounds = btPos;

			ResumeBt->state = GuiControlState::DISABLED;
			SettingsBt->state = GuiControlState::DISABLED;
			ScapeBt->state = GuiControlState::DISABLED;
			CreditstBt->state = GuiControlState::DISABLED;
			ContinuetBt->state = GuiControlState::DISABLED;
			StartBt->state = GuiControlState::DISABLED;
			BackTitleBt->state = GuiControlState::DISABLED;

			CreditstBt->state = GuiControlState::NORMAL;
			ContinuetBt->state = GuiControlState::NORMAL;
			StartBt->state = GuiControlState::NORMAL;
			ScapeBt->state = GuiControlState::NORMAL;
			SettingsBt->state = GuiControlState::NORMAL;

			Engine::GetInstance().audio.get()->StopMusic();
			Engine::GetInstance().map.get()->noUpdate = true;
			enemyList[enemyList.size() - 1]->endFight = false;

			activeMenu = true;
			transitionCnt = 0;
		}
	}
	return true;

}

// Called each loop iteration
bool Scene::Update(float dt)
{
	SDL_Rect sliderKnob = { 200, 10, player->health,20 };
	set = player->settings;
	//L03 TODO 3: Make the camera movement independent of framerate
	if (!activeMenu) {
		float camSpeed = 2;
		Vector2D p = player->position;

		if (p.getX() >= 110 && p.getX() <= 3270 || Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) { 
			if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
				Engine::GetInstance().render.get()->camera.x -= ceil(camSpeed * dt);

			if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
				Engine::GetInstance().render.get()->camera.x += ceil(camSpeed * dt);

			Engine::GetInstance().render.get()->camera.x = ((-p.getX())*camSpeed)+200;
		}

		EnemyHitbox();

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F4) == KEY_DOWN)
			for (int i = 0; i < enemyList.size(); i++) enemyList[i]->active = true;

		Engine::GetInstance().render->DrawText("COINS:", 10, 10, 40, 20);
		Engine::GetInstance().render->DrawText(std::to_string(player->coins).c_str(), 60, 10, 10, 20);

		Engine::GetInstance().render->DrawText("HEALTH:", 160, 10, 40, 20);
		Engine::GetInstance().render->DrawRectangle(sliderKnob, 255, 0, 0, 255, true, false);

		Engine::GetInstance().render->DrawText("Timer:", 590, 10, 40, 20);

		int scaleText = 1;
		if (inGameTimer.ReadSec() >= 10)scaleText = 2;
		Engine::GetInstance().render->DrawText(std::to_string(inGameTimer.ReadSec()).c_str(), 660, 10, scaleText*10, 20);
		if (inGameTimer.ReadSec() == 60){
			minutes++;
			inGameTimer.Start();
		}
		scaleText = 1;
		if (minutes >= 10)scaleText = 2;
		Engine::GetInstance().render->DrawText(std::to_string(minutes).c_str(), 645, 10, scaleText * 10, 20);
	}
	if (activeMenu){
		Engine::GetInstance().render.get()->camera.x = 0;
		if(transitionCnt >50){
			if(!player->settings)Engine::GetInstance().render.get()->DrawTexture(Menu, 0, 0);
			else Engine::GetInstance().render.get()->DrawTexture(SettingM, 0, 0);
			if(credits)Engine::GetInstance().render.get()->DrawTexture(CreditsM, 0, 0);
		}
		
		else {
			transitionCnt++;
			Engine::GetInstance().render.get()->DrawTexture(startM, 0, 0);
			if (transitionCnt == 49) {
				CreditstBt->state = GuiControlState::NORMAL;
				StartBt->state = GuiControlState::NORMAL;
				ScapeBt->state = GuiControlState::NORMAL;
				SettingsBt->state = GuiControlState::NORMAL;
				ContinuetBt->state = GuiControlState::JUST_VISIBLE;
			}
		}
		Engine::GetInstance().map.get()->noUpdate = true;
	}
	if (player->settings && !activeMenu) {
		Engine::GetInstance().render.get()->camera.x = 0;
		Engine::GetInstance().render.get()->DrawTexture(SettingM, 0, 0);
		Engine::GetInstance().map.get()->noUpdate = true;
	}
	else Engine::GetInstance().map.get()->noUpdate = false;
	return true;
}
void Scene::EnemyHitbox()
{
	for (int i = 0; i < enemyList.size(); i++) {
		if (enemyList[i]->active) {

			if (enemyList[i]->initialPos.getX()- enemyRange <= player->GetPosition().getX() && enemyList[i]->initialPos.getX() + enemyRange >= player->GetPosition().getX())
				enemyList[i]->playerNear = true;
			
			else{
				enemyList[i]->playerNear = false;
				enemyList[i]->Stop();
			}

			if (enemyList[i]->GetPosition().getX() - 15 <= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() + 15 >= player->GetPosition().getX()) {
				if (enemyList[i]->GetPosition().getY() - 16 >= player->GetPosition().getY() && enemyList[i]->GetPosition().getY() - 25 <= player->GetPosition().getY()) {
					enemyList[i]->found = false;
					if(mapLevel == 0)enemyList[i]->Disable();
					if (mapLevel == 0)enemyList[i]->Stop();
					if (mapLevel == 1) enemyList[i]->hited = true;
					player->kill = true;
				}
			}

			if (enemyList[i]->GetPosition().getX() - 15 <= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() >= player->GetPosition().getX()) {
				if (enemyList[i]->GetPosition().getY()- margin <= player->GetPosition().getY() && enemyList[i]->GetPosition().getY() + margin >= player->GetPosition().getY())
					player->hitL = true;
			}
			else if (enemyList[i]->GetPosition().getX() + 15 >= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() <= player->GetPosition().getX()) {
				if (enemyList[i]->GetPosition().getY() - margin <= player->GetPosition().getY() && enemyList[i]->GetPosition().getY() + margin >= player->GetPosition().getY())
					player->hitR = true;
			}
		}
	}
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	int Check = Engine::GetInstance().map.get()->checkpoints[actualCheck].getX();

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN){
		if (activeMenu) {

			CreditstBt->state = GuiControlState::NORMAL;

			if(saveGame)ContinuetBt->state = GuiControlState::NORMAL;
			else ContinuetBt->state = GuiControlState::JUST_VISIBLE;

			StartBt->state = GuiControlState::NORMAL;
			SettingsBt->state = GuiControlState::NORMAL;
			ScapeBt->state = GuiControlState::NORMAL;

			MusicSlider->state = GuiControlState::DISABLED;
			FxSlider->state = GuiControlState::DISABLED;
			FulScreenCb->state = GuiControlState::DISABLED;
			player->settings = false;

		}
		else {
			BackTitleBt->state = GuiControlState::NORMAL;
			ResumeBt->state = GuiControlState::NORMAL;
			SettingsBt->state = GuiControlState::NORMAL;
			ScapeBt->state = GuiControlState::NORMAL;
			BackTitleBt->state = GuiControlState::NORMAL;

			MusicSlider->state = GuiControlState::DISABLED;
			FxSlider->state = GuiControlState::DISABLED;
			FulScreenCb->state = GuiControlState::DISABLED;
			player->settings = true;
		}
		credits = false;
	}

	//hacer en funcion Load() aparte y llamarla desde aqui
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN || player->GetPosition().getY() >= 300 || (player->health<=0&&player->cnt>=100)){
		if (configFile.child("config").child("scene").child("entities").child("player").attribute("level").as_int() == 1 || mapLevel == 1){
			ChangeLevel(1);
			player->oneTime = false;
			player->deadAnimation = false;
		}
		else if (configFile.child("config").child("scene").child("entities").child("player").attribute("level").as_int() == 0){
			ChangeLevel(0);
			Load();
		}
	}

	//hacer en funcion Save() aparte y llamarla desde aqui
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN && !player->deadAnimation) Save();
	if (player->GetPosition().getX() >= Check && player->GetPosition().getX() <= Check + 10){
		Engine::GetInstance().audio.get()->PlayFx(check);
		if (actualCheck == 0)actualCheck++;
		else actualCheck--;
		Save();
	}

	if (showTransition) {
		Engine::GetInstance().render.get()->camera.x = 0;
		Engine::GetInstance().render.get()->DrawTexture(Transition, 0, 0);
		Engine::GetInstance().map.get()->noUpdate = true;
		player->settings = true;

		transitionCnt++;
		if (transitionCnt>= 100){
			showTransition = false;
			Engine::GetInstance().map.get()->noUpdate = false;
			player->settings = false;
			transitionCnt = 0;
			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/King Boo Theme.ogg", 0.f);
		}
	}

	return exit;
}
void Scene::Load()
{
	int i = 0;
	player->deadAnimation = false;
	player->oneTime = false;
	player->health = 200;
	player->cnt = 0;

	pugi::xml_parse_result result = configFile.load_file("config.xml");
	if (result == NULL) {
		LOG("ERROR");
		return;
	}

	for (pugi::xml_node enemyNode = configFile.child("config").child("scene").child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
		if (enemyNode.attribute("dead").as_bool()) enemyList[i]->Enable();
		else enemyList[i]->Disable();
		enemyList[i]->SetPosition({enemyNode.attribute("x").as_float(), enemyNode.attribute("y").as_float()});
		i++;
	}
	enemyList[enemyList.size() - 1]->Disable();
	float y = configFile.child("config").child("scene").child("entities").child("player").attribute("y").as_float();

	player->coins = configFile.child("config").child("scene").child("entities").child("player").attribute("coins").as_int();
	player->health = configFile.child("config").child("scene").child("entities").child("player").attribute("hp").as_int();

	if (mapLevel == 0 && y > 192) y = 192;
	player->SetPosition({ (float)configFile.child("config").child("scene").child("entities").child("player").attribute("x").as_int(), y});
}
void Scene::Save()
{
 	int i = 0;
	pugi::xml_document saveFile;
	pugi::xml_parse_result result = saveFile.load_file("config.xml");
	if (result == NULL) {
		LOG("ERROR");
		return;
	}
	for (pugi::xml_node itemNode = saveFile.child("config").child("scene").child("entities").child("items").child("item"); itemNode; itemNode = itemNode.next_sibling("item")) {
		if (itemList[i]->taken == true)itemNode.attribute("taken").set_value(true);
		else itemNode.attribute("taken").set_value(false);
		i++;
		if (i >= itemList.size() - 1) break;
	}
	i = 0;
	for (pugi::xml_node enemyNode = saveFile.child("config").child("scene").child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
		enemyNode.attribute("dead").set_value(enemyList[i]->active);
		enemyNode.attribute("x").set_value(enemyList[i]->GetPosition().getX()-8);
		i++;
		if (i >= enemyList.size() - 1) break;
	}
	saveGame = true;
	Vector2D playerPos = player->GetPosition();
	saveFile.child("config").child("scene").child("entities").child("player").attribute("coins").set_value(player->coins);
	saveFile.child("config").child("scene").child("entities").child("player").attribute("hp").set_value(player->health);
	saveFile.child("config").child("scene").child("entities").child("player").attribute("save").set_value(1);
	saveFile.child("config").child("scene").child("entities").child("player").attribute("x").set_value(playerPos.getX() - 8);
	saveFile.child("config").child("scene").child("entities").child("player").attribute("y").set_value(playerPos.getY() - 8);
	saveFile.child("config").child("scene").child("entities").child("player").attribute("level").set_value(mapLevel);
	saveFile.save_file("config.xml");//save modified file 
}

void Scene::StartNewGame()
{
	int i = 0;
	pugi::xml_document saveFile;
	pugi::xml_parse_result result = saveFile.load_file("startConfig.xml");
	if (result == NULL) {
		LOG("ERROR");
		return;
	}
	for (pugi::xml_node enemyNode = saveFile.child("config").child("scene").child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
		enemyList[i]->Enable();
		enemyList[i]->SetPosition({ enemyNode.attribute("x").as_float(), enemyNode.attribute("y").as_float() });
		i++;
	}
	enemyList[enemyList.size() - 1]->Disable();

	player->coins = saveFile.child("config").child("scene").child("entities").child("player").attribute("coins").as_int();
	player->health = saveFile.child("config").child("scene").child("entities").child("player").attribute("hp").as_int();

	player->SetPosition({saveFile.child("config").child("scene").child("entities").child("player").attribute("x").as_float() ,saveFile.child("config").child("scene").child("entities").child("player").attribute("y").as_float()});
	ChangeLevel(0);
}
// Called before quitting
bool Scene::CleanUp() {
	LOG("Freeing scene");

	SDL_DestroyTexture(img);

	return true;
}

Vector2D Scene::GetPlayerPosition() {
	return player->GetPosition();
}
bool Scene::OnGuiMouseClickEvent(GuiControl* control)
{
	// L15: DONE 5: Implement the OnGuiMouseClickEvent method

	if (control->id == TITLE) {
		ResumeBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		BackTitleBt->state = GuiControlState::DISABLED;

		CreditstBt->state = GuiControlState::NORMAL;

		if (saveGame)ContinuetBt->state = GuiControlState::NORMAL;
		else ContinuetBt->state = GuiControlState::JUST_VISIBLE;

		StartBt->state = GuiControlState::NORMAL;
		ScapeBt->state = GuiControlState::NORMAL;
		SettingsBt->state = GuiControlState::NORMAL;


		Engine::GetInstance().audio.get()->StopMusic();
		Engine::GetInstance().map.get()->noUpdate = true;
		player->settings = false;
		activeMenu = true;

		SDL_Rect btPos = { 350, 363, 100,25 };
		SettingsBt->bounds = btPos;

		btPos = { 500, 363, 100,25 };
		ScapeBt->bounds = btPos;

	}
	else if (control->id == RESUME) {
		ResumeBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;
		BackTitleBt->state = GuiControlState::DISABLED;
		player->settings = false;

	}
	else if (control->id == SETTINGS) {
		ResumeBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;
		CreditstBt->state = GuiControlState::DISABLED;
		ContinuetBt->state = GuiControlState::DISABLED;
		StartBt->state = GuiControlState::DISABLED;
		BackTitleBt->state = GuiControlState::DISABLED;

		MusicSlider->state = GuiControlState::NORMAL;
		FxSlider->state = GuiControlState::NORMAL;
		FulScreenCb->state = GuiControlState::NORMAL;
		player->settings = true;

	}
	else if (control->id == EXIT) exit = false;
	else if (control->id == MUSIC) {
		Engine::GetInstance().audio.get()->SetVolume(control->value);
	}
	else if (control->id == FX) {
		Engine::GetInstance().audio.get()->FxVolume = control->value;

	}
	else if (control->id == FULL_SCREEN){
		Fullscreen = !Fullscreen;
		Engine::GetInstance().window.get()->SetScreen(Fullscreen);
	}
	else if (control->id == CONTINUE) {
		CreditstBt->state = GuiControlState::DISABLED;
		ContinuetBt->state = GuiControlState::DISABLED;
		StartBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;

		activeMenu = false;
		Engine::GetInstance().map.get()->noUpdate = false;
		inGameTimer.Start();

		if (configFile.child("config").child("scene").child("entities").child("player").attribute("level").as_int() == 0){
			ChangeLevel(0);
			Load();
		}
		
		if (configFile.child("config").child("scene").child("entities").child("player").attribute("level").as_int() == 1) ChangeLevel(1);
	}
	else if (control->id == START) {
		mapLevel = 0;

		CreditstBt->state = GuiControlState::DISABLED;
		ContinuetBt->state = GuiControlState::DISABLED;
		StartBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;

		StartNewGame();
		activeMenu = false;
		player->settings = false;

		Engine::GetInstance().map.get()->noUpdate = false;
		inGameTimer.Start();

		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Ground Theme.ogg", 0.f);
	}
	else if (control->id == CREDITS) {
		CreditstBt->state = GuiControlState::DISABLED;
		ContinuetBt->state = GuiControlState::DISABLED;
		StartBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;
		credits = true;
	}
	if (control->id == START || control->id == CONTINUE) {

		SDL_Rect btPos = { 425, 300, 100,25 };
		BackTitleBt->bounds = btPos;

		btPos = { 425, 200, 100,25 };
		SettingsBt->bounds = btPos;

		btPos = { 275, 300, 100,25 };
		ScapeBt->bounds = btPos;

		btPos = { 275, 200, 100,25 };
		ResumeBt->bounds = btPos;
		showTransition = 0;
	}
	return true;
}
void Scene::ChangeLevel(int level)
{
	if (level == 0) {
		mapLevel = 0;
		Engine::GetInstance().map->CleanUp();
		Engine::GetInstance().map->Load("Assets/Maps/", "Stage 1.tmx");
		enemyList[enemyList.size() - 1]->Disable();
		Engine::GetInstance().audio.get()->StopMusic();
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Ground Theme.ogg", 0.f);
	}
	else if (level == 1) {
		mapLevel = 1;
		for (int y = 0; y < enemyList.size(); y++) {
			enemyList[y]->Disable();
		}
		for (int y = 0; y < itemList.size(); y++) {
			itemList[y]->Disable();
		}
		enemyList[enemyList.size() - 1]->Enable();
		Engine::GetInstance().map->CleanUp();
		Engine::GetInstance().map->Load("Assets/Maps/", "Stage 2.tmx");
		player->SetPosition({ 111, 192 });

		Engine::GetInstance().audio.get()->StopMusic();
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/King Boo Theme.ogg", 0.f);
	}
}