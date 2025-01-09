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
	//item->position = Vector2D(16, 192);

	for (pugi::xml_node itemNode = configParameters.child("entities").child("items").child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
	{
		Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
		item->SetParameters(itemNode);
		itemList.push_back(item);
	}

	// Create a enemy using the entity manager 
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
		enemy->SetParameters(enemyNode);
		enemyList.push_back(enemy);
	}

	SDL_Rect btPos = { 100, 100, 60,32 };
	BackTitleBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 0, "Title", btPos, this);

	btPos = { 250, 100, 60,32 };
	ResumeBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "Resume", btPos, this);

	btPos = { 400, 100, 60,32 };
	SettingsBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 2, "Settings", btPos, this);

	btPos = { 550, 100, 60,32 };
	ScapeBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 3, "Exit", btPos, this);


	btPos = { 250, 150, 60,16 };
	MusicSlider = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, 4, "Music", btPos, this);

	btPos = { 400, 150, 60,16 };
	FxSlider  = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, 5, "Fx", btPos, this);

	btPos = { 550, 134, 32,32 };
	FulScreenCb = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::CHECKBOX, 6, "Full Screen", btPos, this);

	//Init Butons
	btPos = { 250, 100, 60,32 };
	StartBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 7, "Start ", btPos, this);

	btPos = { 100, 100, 60,32 };
	ContinuetBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 8, "Continue", btPos, this);

	btPos = { 700, 100, 60,32 };
	CreditstBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 9, "Credits", btPos, this);
	CreditstBt->state = GuiControlState::NORMAL;
	ContinuetBt->state = GuiControlState::NORMAL;
	StartBt->state = GuiControlState::NORMAL;
	ScapeBt->state = GuiControlState::NORMAL;
	SettingsBt->state = GuiControlState::NORMAL;

	return ret;

}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());
	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Maps/MapMetadata.png");
	Menu = Engine::GetInstance().textures.get()->Load("Assets/Textures/Main Menu.png");

	check = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/1Up.ogg");

	enemyList[enemyList.size()-1]->Disable();
	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	//Change level(not finished)
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F1) == KEY_DOWN && mapLevel == 1 )
	{
		mapLevel = 0;
		Engine::GetInstance().map->CleanUp();
		Engine::GetInstance().map->Load("Assets/Maps/", "Stage 1.tmx");
		Load();
		enemyList[enemyList.size() - 1]->Disable();
		player->SetPosition({ 155, 192 });
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN && mapLevel == 0 || player->GetPosition().getX() >= 3500)
	{
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
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {

		if(mapLevel == 0)player->SetPosition({155, 199});
		else player->SetPosition({ 111, 192 });
	}
	if (player->GetPosition().getX() >= 350 && mapLevel == 1) enemyList[enemyList.size() - 1]->startFight = true;
	return true;
	inGameTimer.Start();

}

// Called each loop iteration
bool Scene::Update(float dt)
{
	SDL_Rect sliderKnob = { 200, 10, player->health,20 };

	//L03 TODO 3: Make the camera movement independent of framerate
	if (!activeMenu)
	{
		float camSpeed = 2;
		Vector2D p = player->position;

		if (p.getX() >= 110 && p.getX() <= 3270 || Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		{ 
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
	else {

		Engine::GetInstance().render.get()->DrawTexture(Menu, 0, 0);
	}

	return true;
}
void Scene::EnemyHitbox()
{
	for (int i = 0; i < enemyList.size(); i++)
	{
		if (enemyList[i]->active)
		{
			if (enemyList[i]->initialPos.getX()- enemyRange <= player->GetPosition().getX() && enemyList[i]->initialPos.getX() + enemyRange >= player->GetPosition().getX())
			{
				enemyList[i]->playerNear = true;
			}
			else
			{
				enemyList[i]->playerNear = false;
				enemyList[i]->Stop();
			}

			if (enemyList[i]->GetPosition().getX() - 15 <= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() + 15 >= player->GetPosition().getX())
			{
				if (enemyList[i]->GetPosition().getY() - 16 >= player->GetPosition().getY() && enemyList[i]->GetPosition().getY() - 25 <= player->GetPosition().getY()) {
					enemyList[i]->found = false;
					if(mapLevel == 0)enemyList[i]->Disable();
					if (mapLevel == 0)enemyList[i]->Stop();
					if (mapLevel == 1) enemyList[i]->hited = true;
					player->kill = true;
				}
			}

			if (enemyList[i]->GetPosition().getX() - 15 <= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() >= player->GetPosition().getX())
			{
				if (enemyList[i]->GetPosition().getY()- margin <= player->GetPosition().getY() && enemyList[i]->GetPosition().getY() + margin >= player->GetPosition().getY()){
					player->hitL = true;
				}

			}
			else if (enemyList[i]->GetPosition().getX() + 15 >= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() <= player->GetPosition().getX())
			{
				if (enemyList[i]->GetPosition().getY() - margin <= player->GetPosition().getY() && enemyList[i]->GetPosition().getY() + margin >= player->GetPosition().getY()){
					player->hitR = true;
				}
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
			ContinuetBt->state = GuiControlState::NORMAL;
			StartBt->state = GuiControlState::NORMAL;
			SettingsBt->state = GuiControlState::NORMAL;
			ScapeBt->state = GuiControlState::NORMAL;

			MusicSlider->state = GuiControlState::DISABLED;
			FxSlider->state = GuiControlState::DISABLED;
			FulScreenCb->state = GuiControlState::DISABLED;

		}
		else
		{
			BackTitleBt->state = GuiControlState::NORMAL;
			ResumeBt->state = GuiControlState::NORMAL;
			SettingsBt->state = GuiControlState::NORMAL;
			ScapeBt->state = GuiControlState::NORMAL;
			BackTitleBt->state = GuiControlState::NORMAL;


			MusicSlider->state = GuiControlState::DISABLED;
			FxSlider->state = GuiControlState::DISABLED;
			FulScreenCb->state = GuiControlState::DISABLED;
		}

	}

	//hacer en funcion Load() aparte y llamarla desde aqui
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN || player->GetPosition().getY() >= 300 || (player->health<=0&&player->cnt>=100)){
		if(mapLevel==0)Load();
		if(mapLevel == 1){
			player->deadAnimation = false;
			player->oneTime = false;
			player->health = 200;
			player->cnt = 0;
			player->SetPosition({ 111, 192 });
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

	for (pugi::xml_node enemyNode = configFile.child("config").child("scene").child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		if (enemyNode.attribute("dead").as_bool()) enemyList[i]->Enable();
		enemyList[i]->SetPosition({enemyNode.attribute("x").as_float(), enemyNode.attribute("y").as_float()});
		i++;
	}
	player->SetPosition({ (float)configFile.child("config").child("scene").child("entities").child("player").attribute("x").as_int(),(float)configFile.child("config").child("scene").child("entities").child("player").attribute("y").as_int() });
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

	for (pugi::xml_node enemyNode = saveFile.child("config").child("scene").child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		enemyNode.attribute("dead").set_value(enemyList[i]->active);
		enemyNode.attribute("x").set_value(enemyList[i]->GetPosition().getX()-8);
		i++;
		if (i >= enemyList.size() - 1) break;
	}

	Vector2D playerPos = player->GetPosition();
	saveFile.child("config").child("scene").child("entities").child("player").attribute("x").set_value(playerPos.getX() - 8);
	saveFile.child("config").child("scene").child("entities").child("player").attribute("y").set_value(playerPos.getY() - 8);

	saveFile.save_file("config.xml");//save modified file 
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	SDL_DestroyTexture(img);

	return true;
}

Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
}
bool Scene::OnGuiMouseClickEvent(GuiControl* control)
{
	// L15: DONE 5: Implement the OnGuiMouseClickEvent method

	if (control->id == 0) {
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
		activeMenu = true;
	}
	else if (control->id == 1) {
		ResumeBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;
		BackTitleBt->state = GuiControlState::DISABLED;

	}
	else if (control->id == 2) {
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
	}
	else if (control->id == 3) exit = false;
	else if (control->id == 4) {
		Engine::GetInstance().audio.get()->SetVolume(control->value);
	}
	else if (control->id == 5) {
		Engine::GetInstance().audio.get()->FxVolume = control->value;

	}
	else if (control->id == 6){
		Fullscreen = !Fullscreen;
		Engine::GetInstance().window.get()->SetScreen(Fullscreen);
	}
	else if (control->id == 7) {
		CreditstBt->state = GuiControlState::DISABLED;
		ContinuetBt->state = GuiControlState::DISABLED;
		StartBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;

		activeMenu = false;

		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Ground Theme.ogg", 0.f);
	}

	return true;
}