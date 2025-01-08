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
	}

	// Create a enemy using the entity manager 
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
		enemy->SetParameters(enemyNode);
		enemyList.push_back(enemy);
	}
	SDL_Rect btPos = { 300, 100, 60,32 };
	ResumeBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "Resume", btPos, this);

	btPos = { 400, 100, 60,32 };
	SettingsBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 2, "Settings", btPos, this);

	btPos = { 500, 100, 60,32 };
	ScapeBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 3, "Escape", btPos, this);

	btPos = { 250, 150, 60,16 };
	MusicSlider = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, 4, "Music", btPos, this);

	btPos = { 400, 150, 60,16 };
	FxSlider  = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, 5, "Fx", btPos, this);

	btPos = { 550, 134, 32,32 };
	FulScreenCb = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::CHECKBOX, 6, "Full Screen", btPos, this);

	return ret;

}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());
	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Maps/MapMetadata.png");

	check = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/1Up.ogg");

	Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Ground Theme.ogg", 0.f);

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
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN && mapLevel == 0)
	{
		mapLevel = 1;
		Engine::GetInstance().map->CleanUp();
		Engine::GetInstance().map->Load("Assets/Maps/", "Stage 2.tmx");
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {

		player->SetPosition({155, 199});
	}
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	//L03 TODO 3: Make the camera movement independent of framerate
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
					enemyList[i]->Disable();
					enemyList[i]->Stop(); 
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

		ResumeBt->state = GuiControlState::NORMAL;
		SettingsBt->state = GuiControlState::NORMAL;
		ScapeBt->state = GuiControlState::NORMAL;

		MusicSlider->state = GuiControlState::DISABLED;
		FxSlider->state = GuiControlState::DISABLED;
		FulScreenCb->state = GuiControlState::DISABLED;
	}

	//hacer en funcion Load() aparte y llamarla desde aqui
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN || player->GetPosition().getY() >= 300) Load();
	
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
	LOG("Press Gui Control: %d", control->id);
	if (control->id == 1) {
		ResumeBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;
	}
	else if (control->id == 2) {
		ResumeBt->state = GuiControlState::DISABLED;
		SettingsBt->state = GuiControlState::DISABLED;
		ScapeBt->state = GuiControlState::DISABLED;

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

	return true;
}