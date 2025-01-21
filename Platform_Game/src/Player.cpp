#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"


Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}


bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(0, 0);
	savePos = Vector2D(0, 0);
	return true;

}

bool Player::Start() {

	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	savePos = position;
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	backidle.LoadAnimations(parameters.child("animations").child("backidle"));

	walk.LoadAnimations(parameters.child("animations").child("walk")); 
	backwalk.LoadAnimations(parameters.child("animations").child("backwalk"));

	jump.LoadAnimations(parameters.child("animations").child("jump"));
	backjump.LoadAnimations(parameters.child("animations").child("backjump"));

	dead.LoadAnimations(parameters.child("animations").child("dead"));

	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/Coin.ogg");
	jumpSfx = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/Jump Small.ogg");
	enemyKillSfx = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/Kick.ogg");
	dieSfx = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Music/Mario Dies.ogg");
	hitSfx = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/Bump.ogg");

	
	return true;
}


bool Player::Update(float dt)
{	
	if (Engine::GetInstance().scene.get()->activeMenu == true) return true;

	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
	// L08 TODO 5: Add physics to the player - updated player position using physics
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN){ 
		if (godmode==true){
			b2Vec2 prueba;
			prueba.x = (position.getX() / 50) + 0.3;
			prueba.y = (position.getY() / 50) + 0.3;
			pbody->body->SetTransform(prueba, 0);

			godmode = false;
		}
		else{
			deadAnimation = false;
			oneTime = false;
			godmode = true;
		}
	}

	if(godmode==false){	
		if(!deadAnimation && !hitL && !hitR && health>0)
		{
			// Move left
			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN) LOG("PLAYER POS : %d", position.getX());
			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) 
			{
				velocity.x = -marioVel * dt;
				currentAnimation = &backwalk;
				look = true;
			}

			// Move right
			else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) 
			{
				velocity.x = marioVel * dt;
				currentAnimation = &walk;
				look = false;
			}
			else{
				if(look)currentAnimation = &backidle;
				else currentAnimation = &idle;
			}

			//Jump
			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false || kill)
			{
				// Apply an initial upward force
				if (!kill)Engine::GetInstance().audio.get()->PlayFx(jumpSfx);
				else Engine::GetInstance().audio.get()->PlayFx(enemyKillSfx);
				pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
				isJumping = true;
				kill = false;
			}

			// If the player is jumpling, we don't want to apply gravity, we use the current velocity prduced by the jump
			if(isJumping)
			{
				velocity.y = pbody->body->GetLinearVelocity().y;

				if (look)
				{
					currentAnimation = &backjump; 
				}
				else currentAnimation = &jump;
			}

			// Apply the velocity to the player
			pbody->body->SetLinearVelocity(velocity);
		
			b2Transform pbodyPos = pbody->body->GetTransform();
			position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
			position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
		}
		else if (deadAnimation)
		{
			currentAnimation = &dead;

			if(!oneTime){
				pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -0.7f), true);
				Engine::GetInstance().audio.get()->PlayFx(dieSfx);
				oneTime = true;
			}

			velocity.y = pbody->body->GetLinearVelocity().y;
			pbody->body->SetLinearVelocity(velocity);

			b2Transform pbodyPos = pbody->body->GetTransform();
			position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
			position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

		}
		else if (hitL)
		{
			
			if (cnt <= 10)
			{
				currentAnimation = &dead;

				velocity.x = -0.2 * dt;
				pbody->body->SetLinearVelocity(velocity);

				b2Transform pbodyPos = pbody->body->GetTransform();
				position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
				position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
				cnt++;
			}
			else {
				hitL = false;
				health -=10;
				cnt = 0;
			}

		}
		else if (hitR)
		{

			if (cnt <= 10)
			{
				currentAnimation = &dead;

				velocity.x = 0.2 * dt;
				pbody->body->SetLinearVelocity(velocity);

				b2Transform pbodyPos = pbody->body->GetTransform();
				position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
				position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
				cnt++;
			}
			else {
				hitR = false;
				health -=10;
				cnt = 0;
			}
		}
		else if (health <= 0) {
			currentAnimation = &dead;

			if (!oneTime) {
				pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -0.2f), true);
				Engine::GetInstance().audio.get()->PlayFx(dieSfx);
				oneTime = true;
				cnt = 0;
			}

			velocity.y = pbody->body->GetLinearVelocity().y;
			pbody->body->SetLinearVelocity(velocity);

			b2Transform pbodyPos = pbody->body->GetTransform();
			position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
			position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
			cnt++;
			
		}
		if ((int)position.getY() >= 270) deadAnimation = true;

		if (plusVel) {
			marioVel = 0.3f;
			coldown++;
			if (coldown >= 200) {
				marioVel = 0.2f;
				plusVel = false;
			}
		}

	}

	else
	{
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
			position.setY(position.getY() - speed);

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
			position.setY(position.getY() + speed);

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			position.setX(position.getX() - speed);

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			position.setX(position.getX() + speed);
	}
	


	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();
	return true;
}


bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		//reset the jump flag when touching the ground
		isJumping = false;
		kill = false;
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().scene.get()->itemList[physB->id]->taken = true;
		Engine::GetInstance().physics.get()->DeletePhysBody(physB);
		coins++;
		break;
	case ColliderType::BOOST:
		LOG("Collision ITEM");
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().physics.get()->DeletePhysBody(physB);
		plusVel = true;
		
		break;
	case ColliderType::HEAL:
		LOG("Collision ITEM");
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().physics.get()->DeletePhysBody(physB);
		if (health < 200){
			health += 50;
			if (health > 200)health = 200;
		}
		break;
	case ColliderType::ENEMY:
		LOG("Collision ENEMY");
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
// Deletes the body of the item from the physics world
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}
void Player::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Player::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}