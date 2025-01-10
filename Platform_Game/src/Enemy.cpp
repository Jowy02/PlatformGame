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
	boss = parameters.attribute("boss").as_bool();

	//Load animations

	idle_R.LoadAnimations(parameters.child("animations").child("idleR"));
	idle_L.LoadAnimations(parameters.child("animations").child("idleL"));

	currentAnimation = &idle_L;
	invisible_idle_R.LoadAnimations(parameters.child("animations").child("invisible_idleR"));
	invisible_idle_L.LoadAnimations(parameters.child("animations").child("invisible_idleL"));

	hit.LoadAnimations(parameters.child("animations").child("hit"));

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
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
	Vector2D enemyPos = GetPosition();
	if (Engine::GetInstance().scene.get()->activeMenu == true) return true;

	if (boss)
	{
		if (startFight) {

			velocity.x = 0.0 * dt;
			velocity.y = 0.0 * dt;
			if (fase == 0 && !hited){ 
				if (boosPath < 60)velocity.x = -0.2 * dt;
				else if (boosPath < 100 )velocity.y = 0.2 * dt;
				else if (boosPath < 160) velocity.x = -0.2 * dt;
				else if (boosPath < 200){
					velocity.y = -0.2 * dt;
					currentAnimation = &idle_R;
				}
				else if (boosPath < 260) velocity.x = 0.2 * dt;
				else if (boosPath < 300) velocity.y = 0.2 * dt;
				else if (boosPath < 360) velocity.x = 0.2 * dt;
				else if (boosPath < 400){
					velocity.y = -0.2 * dt;
					currentAnimation = &idle_L;
				}

				if (boosPath >= 400)boosPath = 0;
			}
			else if (fase == 1 && !hited) {

				if (boosPath < 125)velocity.x = -0.2 * dt;
				else if (boosPath < 135){
					velocity.y = 0.2 * dt;
					currentAnimation = &idle_R;
				}
				else if (boosPath < 260) velocity.x = 0.2 * dt;
				else if (boosPath < 270){
					velocity.y = 0.2 * dt;
					currentAnimation = &idle_L;
				}
				else if (boosPath < 395) velocity.x = -0.2 * dt;
				else if (boosPath < 405){
					velocity.y = 0.2 * dt;
					currentAnimation = &idle_R;
				}
				else if (boosPath < 530) velocity.x = 0.2 * dt;
				else if (boosPath < 540){
					velocity.y = 0.2 * dt;
					currentAnimation = &idle_L;
				}
				else if (boosPath < 665) velocity.x = -0.2 * dt;
				else if (boosPath < 675){
					velocity.y = 0.2 * dt;
					currentAnimation = &idle_R;
				}
				else if (boosPath < 800) velocity.x = 0.2 * dt;
				else if (boosPath < 850){
					velocity.y = -0.2 * dt;
					currentAnimation = &idle_L;
				}
				if (boosPath >= 850)boosPath = 0;
			}
			else if (fase == 2 && !hited) {

				if (boosPath < 80){
					velocity.x = -0.32 * dt;
					velocity.y = 0.1 * dt;
				}
				else if (boosPath < 120){
					velocity.y = -0.2 * dt;
					currentAnimation = &invisible_idle_R;
				}

				else if (boosPath < 198) {
					velocity.x = 0.32 * dt;
					velocity.y = 0.1 * dt;
				}
				else if (boosPath < 238){
					velocity.y = -0.2 * dt;
					currentAnimation = &invisible_idle_L;
				}

				if (boosPath >= 238)boosPath = 0;

			}
			else if (fase == 3) {
				
				if (boosPath < 200) velocity.y = 0.2 * dt;
			}
			else {
				if(hitColdwon>50)
				{ 
					fase++;
					if (fase == 2)currentAnimation = &invisible_idle_L;
					else if (fase!=3) currentAnimation = &idle_L;

					boosPath = 0;
					if (fase != 3){
						b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(700), PIXEL_TO_METERS(50));
						pbody->body->SetTransform(bodyPos, 0);
					}
					hited = false;
					hitColdwon = 0;
				}
				else {
					hitColdwon++;
					currentAnimation = &hit;
				}

			}

			boosPath++;
		}

		pbody->body->SetLinearVelocity(velocity);
		b2Transform pbodyPos = pbody->body->GetTransform();
		position.setX((METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2));
		position.setY((METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2));
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();
	}
	else {


		if (playerNear && !flyingEnemy)
		{
			pathfinding->PropagateBFS();
		}
		else if (playerNear && flyingEnemy )
		{
			pathfinding->PropagateDijkstra();
		}

		if (pathfinding->expansionCnt >= pathfinding->maxExpansion && !found || rest)
		{
			Vector2D pos = GetPosition();
			Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
			pathfinding->ResetPath(tilePos, flyingEnemy);
			rest = false;
		}

		if (pathfinding->FexpansionCnt >= 100 && !found || rest)
		{
    		Vector2D pos = GetPosition();
			Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
			pathfinding->ResetPath(tilePos, flyingEnemy);
		}

		// L08 TODO 4: Add a physics to an item - update the position of the object from the physics
		b2Transform pbodyPos = pbody->body->GetTransform();
		if (pathfinding->found && !oneTime){
			found = true;
		}

		if (found) {
			if (!oneTime)
			{
				for (int i = 0; i < sizeof(vec); i++) {
					vec[cnt] = { 0,0 };
				}
				for (const auto& tile : pathfinding->pathTiles) {
					vec[cnt] = {tile*16};
					cnt++;
				}
				oneTime = true;
				pathfinding->found = false;
			
				if (vec[cnt - 1].getX() >= vec[0].getX())dir = 0;
				else dir = 1;

				if (vec[cnt - 1].getY() >= vec[0].getY())dirUD = 0;
				else dirUD = 1;
				cnt -= 1;
			}
		
			if(vec[cnt].getY() == vec[cnt-1].getY() && cnt !=0 || cnt == 0 && vec[cnt].getY() == vec[cnt+1].getY())
			{ 
				velocity.y = 0.0 * dt;
				if(dir == 1)
				{ 
					if (enemyPos.getX() <= vec[cnt].getX())
					{
						velocity.x = 0.2 * dt;
					}
					 if (enemyPos.getX() >= vec[cnt].getX()) cnt--;
				
				}
				else 
				{
					if (enemyPos.getX() >= vec[cnt].getX())
					{
						velocity.x = -0.2 * dt;
					}
					if (enemyPos.getX()<= vec[cnt].getX()) cnt--;
				}
			}
			else 
			{
				velocity.x = 0.0 * dt;
				if (dirUD == 1)
				{
					if (enemyPos.getY() <= vec[cnt].getY())
					{
						velocity.y = 0.2 * dt;
					}
					if (enemyPos.getY()-8 >= vec[cnt].getY()) cnt--;
				}
				else
				{
					if (enemyPos.getY() >= vec[cnt].getY())
					{
						velocity.y = -0.2 * dt;
					}
					if (enemyPos.getY() <= vec[cnt].getY()) cnt--;
				}
			}

			if(cnt < 0) {
				found = false;
				oneTime = false;
				cnt = 0;
				velocity.y = 0.0;
			}
		}

		pbody->body->SetLinearVelocity(velocity);
		position.setX((METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2) + r);
		position.setY((METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2) + ry);

		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
			debug = !debug;

		if(debug) pathfinding->DrawPath();
	}

	return true;
}
void Enemy::Stop()
{
	pbody->body->SetLinearVelocity({0.0,0.0});

	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos, flyingEnemy);

	found = false;
	oneTime = false;
	cnt = 0;
}

bool Enemy::CleanUp()
{
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
	pathfinding->ResetPath(tilePos, flyingEnemy);
}