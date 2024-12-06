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

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());
	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Maps/MapMetadata.png");

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
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

	if (p.getX() >= 110 && p.getX() <= 3270 || Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
	{ 
		if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
			Engine::GetInstance().render.get()->camera.x -= ceil(camSpeed * dt);

		if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
			Engine::GetInstance().render.get()->camera.x += ceil(camSpeed * dt);

		Engine::GetInstance().render.get()->camera.x = ((-p.getX())*camSpeed)+200;
	}

	Vector2D mousePos = Engine::GetInstance().input.get()->GetMousePosition();
	Vector2D mouseTile = Engine::GetInstance().map.get()->WorldToMap(mousePos.getX() - Engine::GetInstance().render.get()->camera.x,
																	 mousePos.getY() - Engine::GetInstance().render.get()->camera.y);
	EnemyHitbox();

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F4) == KEY_DOWN)
		for (int i = 0; i < enemyList.size(); i++) enemyList[i]->active = true;


	//Render a te	xture where the mouse is over to highlight the tile, use the texture 'mouseTileTex'
	Vector2D highlightTile = Engine::GetInstance().map.get()->MapToWorld(mouseTile.getX(), mouseTile.getY());
	SDL_Rect rect = { 0,0,16,16 };
	Engine::GetInstance().render.get()->DrawTexture(mouseTileTex,
		highlightTile.getX(),
		highlightTile.getY(),
		&rect);

	// saves the tile pos for debugging purposes
	if (mouseTile.getX() >= 0 && mouseTile.getY() >= 0 || once) {
		tilePosDebug = "[" + std::to_string((int)mouseTile.getX()) + " ," + std::to_string((int)mouseTile.getY()) + "] ";
		once = true;
	}

	//If mouse button is pressed modify enemy position
	if (Engine::GetInstance().input.get()->GetMouseButtonDown(1) == KEY_DOWN) {
		enemyList[0]->SetPosition(Vector2D(highlightTile.getX(), highlightTile.getY()));
		enemyList[0]->ResetPath();
	}

	return true;
}
void Scene::EnemyHitbox()
{
	for (int i = 0; i < enemyList.size(); i++)
	{
		if (enemyList[i]->active)
		{
			if (enemyList[i]->initialPos.getX()-100 <= player->GetPosition().getX() && enemyList[i]->initialPos.getX() + 100 >= player->GetPosition().getX())
			{
				enemyList[i]->playerNear = true;
			}
			else
			{
				enemyList[i]->playerNear = false;
				enemyList[i]->CleanUp();
			}

			if (enemyList[i]->GetPosition().getX() - 15 <= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() + 15 >= player->GetPosition().getX())
			{
				if (enemyList[i]->GetPosition().getY() - 16 >= player->GetPosition().getY() && enemyList[i]->GetPosition().getY() - 25 <= player->GetPosition().getY()) {
					enemyList[i]->found = false;
					enemyList[i]->Disable();
					player->kill = true;
				}
			}

			if (enemyList[i]->GetPosition().getX() - 15 <= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() >= player->GetPosition().getX())
			{
				if (enemyList[i]->GetPosition().getY() == player->GetPosition().getY()) player->hitL = true;
			}
			else if (enemyList[i]->GetPosition().getX() + 15 >= player->GetPosition().getX() && enemyList[i]->GetPosition().getX() <= player->GetPosition().getX())
			{
				if (enemyList[i]->GetPosition().getY() == player->GetPosition().getY()) player->hitR = true;
			}
		}
	}
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	//hacer en funcion Load() aparte y llamarla desde aqui
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) Load();
	
	//hacer en funcion Save() aparte y llamarla desde aqui
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN && !player->deadAnimation) Save();

	return ret;
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