#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr int DISPLAY_WIDTH = 1280;
constexpr int DISPLAY_HEIGHT = 720;
constexpr int DISPLAY_SCALE = 1;

constexpr int wrapBorderSize = 20.0f;

constexpr float Agent8RotSpeed = 0.05f;
constexpr float Agent8FlyingRotSpeed = 0.02f;
constexpr float Agent8Speed = 4.0f;

// storing the current asteroid
int myAsteroid = 0;

enum class Agent8State
{
	STATE_APPEAR = 0,
	STATE_ATTACHED,
	STATE_FLYING,
	STATE_DEAD,
};

struct GameState 
{
	int remainingGems = 0;
	float timer = 0;
	int spriteId = 0;
	Agent8State agentState = Agent8State::STATE_APPEAR;
};

GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_ASTEROID,
	TYPE_METEOR,
};

// function prototypes
void PlayerControls();
void SpawnAsteroids();
void UpdateAgent8();
void UpdateAsteroids();
void SpawnMeteor();
void UpdateMeteor();


// entry point for the playbuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	//Play::StartAudioLoop("music");
	Play::CreateGameObject(TYPE_AGENT8, { 640, 360 }, 50, "agent8_left");
	Play::CentreAllSpriteOrigins();
	Play::SetSpriteOrigin("agent8_fly", 50, 110);
	Play::SetSpriteOrigin("agent8_left", 50, 110);
	Play::SetSpriteOrigin("agent8_right", 50, 110);
	SpawnAsteroids();
	SpawnMeteor();

}

// called by playbuffer once for each frame 
bool MainGameUpdate(float elapsedTime) 
{
	gameState.timer += elapsedTime;

	Play::DrawBackground();
	PlayerControls();
	UpdateAsteroids();
	UpdateAgent8();
	UpdateMeteor();
	Play::DrawFontText("105px", "REMAINING GEMS: " + std::to_string(gameState.remainingGems), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "ARROW KEYS TO ROTATE AND SPACE BAR TO LAUNCH", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);
	Play::PresentDrawingBuffer();

	return Play::KeyDown(VK_ESCAPE);
}

void PlayerControls() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	float Agent8Angle = 0.1f;

	// movement and anim on asteroid
	if (gameState.agentState == Agent8State::STATE_ATTACHED) 
	{
		// position on current asteroid
		GameObject& obj_asteroid = Play::GetGameObject(myAsteroid);
		obj_agent8.pos = obj_asteroid.pos;

		if (Play::KeyDown(VK_LEFT))
		{
			Play::SetSprite(obj_agent8, "agent8_left", 0.2f);
			obj_agent8.rotation -= Agent8RotSpeed;
		}
		if (Play::KeyDown(VK_RIGHT))
		{
			Play::SetSprite(obj_agent8, "agent8_right", 0.2f);
			obj_agent8.rotation += Agent8RotSpeed;
		}
		if (Play::KeyPressed(VK_SPACE))
		{
			gameState.agentState = Agent8State::STATE_FLYING;
			// destroy the asteroid agent8 jumps off
			Play::DestroyGameObject(myAsteroid);
			// create asteroid pieces 
			// for each asteroid destroyed, spawn 3 asteroid pieces in its place, move them off screen then destroy
			
		}
		Play::UpdateGameObject(obj_agent8);
	}
	// flying movement
	if (gameState.agentState == Agent8State::STATE_FLYING) 
	{
		// reset position
		obj_agent8.pos = obj_agent8.oldPos;
		obj_agent8.pos += obj_agent8.velocity;
		
		//rotate
		if (Play::KeyDown(VK_LEFT)) 
		{
			obj_agent8.rotation -= Agent8FlyingRotSpeed;
		}
		if (Play::KeyDown(VK_RIGHT))
		{
			obj_agent8.rotation += Agent8FlyingRotSpeed;
		}
		// move in direction of angle 
		obj_agent8.pos.x = obj_agent8.pos.x + sin(obj_agent8.rotation) * Agent8Speed;
		obj_agent8.pos.y = obj_agent8.pos.y - cos(obj_agent8.rotation) * Agent8Speed;

		// flying agent8 wrap around screen
		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_agent8.spriteId);

		if (obj_agent8.pos.x - origin.x - wrapBorderSize > DISPLAY_WIDTH)
		{
			obj_agent8.pos.x = 0.0f - wrapBorderSize + origin.x;
		}
		else if (obj_agent8.pos.x + origin.x + wrapBorderSize < 0)
		{
			obj_agent8.pos.x = DISPLAY_WIDTH + wrapBorderSize - origin.x;
		}

		if (obj_agent8.pos.y - origin.y - wrapBorderSize > DISPLAY_HEIGHT)
		{
			obj_agent8.pos.y = 0.0f - wrapBorderSize + origin.y;
		}
		else if (obj_agent8.pos.y + origin.y + wrapBorderSize < 0)
		{
			obj_agent8.pos.y = DISPLAY_HEIGHT + wrapBorderSize - origin.y;
		}

		Play::UpdateGameObject(obj_agent8);
	}
}
void SpawnAsteroids() 
{
	for (int i = 0; i < 3; i++) 
	{
		int myAsteroidId = Play::CreateGameObject(TYPE_ASTEROID, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 50, "asteroid");
		GameObject& myAsteroid = Play::GetGameObject(myAsteroidId);

		myAsteroid.velocity = { rand() % 2 + (-1), rand() % 2 + 1 };

	} 

}

void UpdateAsteroids() 
{
	std::vector<int> vAsteroidIds = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
	GameObject& obj_meteor = Play::GetGameObjectByType(TYPE_METEOR);
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	for (int id_asteroids : vAsteroidIds) 
	{
		GameObject& obj_asteroid = Play::GetGameObject(id_asteroids);

		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_asteroid.spriteId);
		// change direction of sprite 
		obj_asteroid.rotation = atan2(obj_asteroid.velocity.y, obj_asteroid.velocity.x) + (PLAY_PI / 2);
		Play::UpdateGameObject(obj_asteroid);

		// asteroids wrap around screen
		if (obj_asteroid.pos.x - origin.x - wrapBorderSize > DISPLAY_WIDTH)
		{
			obj_asteroid.pos.x = 0.0f - wrapBorderSize + origin.x;
		}
		else if (obj_asteroid.pos.x + origin.x + wrapBorderSize < 0)
		{
			obj_asteroid.pos.x = DISPLAY_WIDTH + wrapBorderSize - origin.x;
		}

		if (obj_asteroid.pos.y - origin.y - wrapBorderSize > DISPLAY_HEIGHT)
		{
			obj_asteroid.pos.y = 0.0f - wrapBorderSize + origin.y;
		}
		else if (obj_asteroid.pos.y + origin.y + wrapBorderSize < 0)
		{
			obj_asteroid.pos.y = DISPLAY_HEIGHT + wrapBorderSize - origin.y;
		}

		Play::DrawObjectRotated(obj_asteroid);
	}

}

void SpawnMeteor() 
{
	int myMeteorId= Play::CreateGameObject(TYPE_METEOR, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 50, "meteor");
	GameObject& obj_meteor = Play::GetGameObject(myMeteorId);

	obj_meteor.velocity = { rand() % 5 + (-1), rand() % 5 + 1 };
}

void UpdateMeteor() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj_meteor = Play::GetGameObjectByType(TYPE_METEOR);
	Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_meteor.spriteId);

	// change direction of sprite
	obj_meteor.rotation = atan2(obj_meteor.velocity.y, obj_meteor.velocity.x) + (PLAY_PI / 2);
	Play::UpdateGameObject(obj_meteor);

	// meteor wraps around screen
	if (obj_meteor.pos.x - origin.x - wrapBorderSize > DISPLAY_WIDTH)
	{
		obj_meteor.pos.x = 0.0f - wrapBorderSize + origin.x;
	}
	else if (obj_meteor.pos.x + origin.x + wrapBorderSize < 0)
	{
		obj_meteor.pos.x = DISPLAY_WIDTH + wrapBorderSize - origin.x;
	}

	if (obj_meteor.pos.y - origin.y - wrapBorderSize > DISPLAY_HEIGHT)
	{
		obj_meteor.pos.y = 0.0f - wrapBorderSize + origin.y;
	}
	else if (obj_meteor.pos.y + origin.y + wrapBorderSize < 0)
	{
		obj_meteor.pos.y = DISPLAY_HEIGHT + wrapBorderSize - origin.y;
	}
	
	if (Play::IsColliding(obj_agent8, obj_meteor))
	{
		gameState.agentState = Agent8State::STATE_DEAD;
	}
	Play::DrawObjectRotated(obj_meteor);
}

void UpdateAgent8() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj_meteor = Play::GetGameObjectByType(TYPE_METEOR);
	Play::DrawObjectRotated(obj_agent8);

	switch (gameState.agentState)
	{
	case Agent8State::STATE_APPEAR:
		{
			gameState.agentState = Agent8State::STATE_FLYING;

			// collect asteroids as list of ints 
			std::vector<int> vAsteroidIds = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
			for (int id_asteroid : vAsteroidIds)
			{
				// int then gets each asteroid object 
				GameObject& obj_asteroid = Play::GetGameObject(id_asteroid);
				// obj_asteroid is now the current attached asteroid 
				if (Play::IsColliding(obj_agent8, obj_asteroid))
				{
					obj_agent8.pos = obj_asteroid.pos;
					// declaring myAsteroid as the current attached asteroid
					myAsteroid = id_asteroid;
					gameState.agentState = Agent8State::STATE_ATTACHED;

				}
			}
		}
		break;
	case Agent8State::STATE_ATTACHED:
		PlayerControls();
		break;
	case Agent8State::STATE_FLYING:
		{
			Play::SetSprite(obj_agent8, "agent8_fly", 0);
			PlayerControls();

			std::vector<int> vAsteroidIds = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
			for (int id_asteroid : vAsteroidIds)
			{
				GameObject& obj_asteroid = Play::GetGameObject(id_asteroid);

				if (Play::IsColliding(obj_agent8, obj_asteroid))
				{
					obj_agent8.pos = obj_asteroid.pos;
					myAsteroid = id_asteroid;
					gameState.agentState = Agent8State::STATE_ATTACHED;
				}
			}
		}
		break;
	case Agent8State::STATE_DEAD:
		Play::SetSprite(obj_agent8, "agent8_dead", 0.2f);

		// TO DO: move agent8 off screen

		// TO DO: change direction of sprite
	
		if (Play::IsLeavingDisplayArea(obj_agent8)) 
		{
			gameState.agentState = Agent8State::STATE_APPEAR;
		}
		break;

	} // end of switch

	Play::UpdateGameObject(obj_agent8);
}

// called once the player quits the game

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}