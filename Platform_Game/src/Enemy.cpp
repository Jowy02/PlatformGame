#include "Enemy.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"

Enemy::Enemy() : Entity(EntityType::ENEMY)
{

}

Enemy::~Enemy() {
	delete pathfinding;
}

bool Enemy::Awake() {
	return true;
}

bool Enemy::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	flyingEnemy = parameters.attribute("flying").as_bool();
	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;
	initialPos = position;
	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, 16,16, bodyType::KINEMATIC);

	//Assign collider type
	pbody->ctype = ColliderType::ENEMY;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	// Initialize pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	return true;
}

bool Enemy::Update(float dt)
{

	if (playerNear && !flyingEnemy)
	{
		pathfinding->PropagateBFS();
	}
	if (pathfinding->expansionCnt >= pathfinding->maxExpansion && !found)
	{
		Vector2D pos = GetPosition();
		Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
		pathfinding->ResetPath(tilePos);
	}
	
	if (playerNear && flyingEnemy)
	{
		pathfinding->PropagateDijkstra();
	}
	if (pathfinding->expansionCnt >= pathfinding->maxExpansion && !found)
	{
		Vector2D pos = GetPosition();
		Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
		pathfinding->ResetPath(tilePos);
	}

	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics
	b2Transform pbodyPos = pbody->body->GetTransform();
	if (pathfinding->found && !oneTime){
		found = true;
	}

	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
	
	if (found) {
		if (!oneTime)
		{
			for (int i = 0; i < sizeof(vec); i++) {
				vec[cnt] = { 0,0 };
			}
			for (const auto& tile : pathfinding->pathTiles) {
				vec[cnt] = {tile};
				cnt++;
			}
			oneTime = true;
			pathfinding->found = false;
			tileEnemypos = vec[cnt-1].getX();
			if (vec[cnt - 1].getX() >= vec[0].getX())dir = 0;
			else dir = 1;
			cnt -= 2;
			
		}
		if (tileEnemypos != vec[cnt].getX())
		{
			move++;
			if (dir == 0)velocity.x = -0.2 * dt;
			else velocity.x = 0.2 * dt;

			if (move == p) {
				if (dir == 0)tileEnemypos--;
				else tileEnemypos++;
				p += 3;
			}
		}
		else cnt--;

		if (cnt == 0) {
			p = 3;
			tileEnemypos = 0;
			move = 0;
			found = false;
			oneTime = false;
			cnt = 0;

		}
	}

	pbody->body->SetLinearVelocity(velocity);
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
		debug = !debug;

	if(debug) pathfinding->DrawPath();;
	return true;
}

bool Enemy::CleanUp()
{
	tileEnemypos = 0;
	move = 0;
	found = false;
	oneTime = false;
	cnt = 0;
	return true;
}

void Enemy::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Enemy::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Enemy::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}