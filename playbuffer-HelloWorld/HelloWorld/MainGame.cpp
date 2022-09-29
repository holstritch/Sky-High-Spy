#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#define PLAY_ADD_GAMEOBJECT_MEMBERS float timeStamp = 0.0f;
#include "Play.h"

constexpr int DISPLAY_WIDTH = 1280;
constexpr int DISPLAY_HEIGHT = 720;
constexpr int DISPLAY_SCALE = 1;

constexpr int wrapBorderSize = 20.0f;

constexpr float Agent8RotSpeed = 0.05f;
constexpr float Agent8FlyingRotSpeed = 0.02f;
constexpr float Agent8Speed = 8.0f;

// storing the current asteroid
int myAsteroid = 0;

enum class Agent8State
{
	STATE_APPEAR = 0,
	STATE_ATTACHED,
	STATE_FLYING,
	STATE_DEAD,
	STATE_OFFSCREEN,
};

struct GameState 
{
	int remainingGems = 0;
	float timer = 0;
	int spriteId = 0;
	int particleCountDown = 0;
	Agent8State agentState = Agent8State::STATE_APPEAR;
};

GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_ASTEROID,
	TYPE_METEOR,
	TYPE_ASTEROID_PIECE,
	TYPE_GEM,
	TYPE_PARTICLE,
};

// function prototypes
float RandomFloatNumber();
void ScreenWrap(GameObject& obj, Vector2f origin);
void PlayerControls();
void SpawnPiecesAndGem();
void UpdateBrokenPieces();
void UpdateGem();
void SpawnParticle(GameObject& obj);
void UpdateParticles();
void SpawnAsteroids();
void UpdateAsteroids();
void SpawnMeteor();
void UpdateMeteor();
void UpdateAgent8();

// entry point for the playbuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	//Play::StartAudioLoop("music");
	Play::CreateGameObject(TYPE_AGENT8, { 640, 360 }, 50, "agent8_left");
	Play::CentreAllSpriteOrigins();
	Play::SetSpriteOrigin("agent8_fly", 30, 110);
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
	UpdateBrokenPieces();
	UpdateGem();
	UpdateParticles();
	Play::DrawFontText("105px", "REMAINING GEMS: " + std::to_string(gameState.remainingGems), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	if (gameState.agentState != Agent8State::STATE_OFFSCREEN) 
	{
		Play::DrawFontText("64px", "ARROW KEYS TO ROTATE AND SPACE BAR TO LAUNCH", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);
	}
	Play::PresentDrawingBuffer();
	return Play::KeyDown(VK_ESCAPE);
}

float RandomFloatNumber()
{
	// 0 to 1
	float f = (float)(rand() % 1000) / 1000.0f;
	// -0.5 to 1
	return f * 1.0f - 0.5f;
}

void ScreenWrap(GameObject& obj, Vector2f origin)
{
	if (obj.pos.x - origin.x - wrapBorderSize > DISPLAY_WIDTH)
	{
		obj.pos.x = 0.0f - wrapBorderSize + origin.x;
	}
	else if (obj.pos.x + origin.x + wrapBorderSize < 0)
	{
		obj.pos.x = DISPLAY_WIDTH + wrapBorderSize - origin.x;
	}

	if (obj.pos.y - origin.y - wrapBorderSize > DISPLAY_HEIGHT)
	{
		obj.pos.y = 0.0f - wrapBorderSize + origin.y;
	}
	else if (obj.pos.y + origin.y + wrapBorderSize < 0)
	{
		obj.pos.y = DISPLAY_HEIGHT + wrapBorderSize - origin.y;
	}
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
		obj_agent8.velocity = { 0, 0 };

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
			SpawnPiecesAndGem();
			// set velocity for flying & move away from gem
			obj_agent8.velocity.x = sin(obj_agent8.rotation) * Agent8Speed;
			obj_agent8.velocity.y = -cos(obj_agent8.rotation) * Agent8Speed;
			obj_agent8.pos += obj_agent8.velocity * 8;

			// destroy the asteroid agent8 jumps off
			Play::DestroyGameObject(myAsteroid);	
		}
	}
	// flying movement
	if (gameState.agentState == Agent8State::STATE_FLYING) 
	{
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
		obj_agent8.velocity.x = sin(obj_agent8.rotation) * Agent8Speed;
		obj_agent8.velocity.y = - cos(obj_agent8.rotation) * Agent8Speed;

		// flying agent8 wrap around screen
		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_agent8.spriteId);
		ScreenWrap(obj_agent8, origin);
	}
	if (gameState.agentState == Agent8State::STATE_DEAD) 
	{
		// shoot agent8 offscreen in pos facing
	}
}

void SpawnPiecesAndGem() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	int myAsteroidPieceIdL = Play::CreateGameObject(TYPE_ASTEROID_PIECE, obj_agent8.pos, 50, "asteroid_pieces");
	int myAsteroidPieceIdR = Play::CreateGameObject(TYPE_ASTEROID_PIECE, obj_agent8.pos, 50, "asteroid_pieces");
	int myAsteroidPieceIdUp = Play::CreateGameObject(TYPE_ASTEROID_PIECE, obj_agent8.pos, 50, "asteroid_pieces");

	// reset pos and move pieces 
	GameObject& myAsteroidL = Play::GetGameObject(myAsteroidPieceIdL);
	myAsteroidL.velocity = { -8, 0 };
	GameObject& myAsteroidR = Play::GetGameObject(myAsteroidPieceIdR);
	myAsteroidR.velocity = { 8, 0 };
	GameObject& myAsteroidUp = Play::GetGameObject(myAsteroidPieceIdUp);
	myAsteroidUp.velocity = { 0, -8 };

	// set sprite frame
	myAsteroidL.frame = 1;
	myAsteroidR.frame = 2;
	myAsteroidUp.frame = 0;

	Play::CreateGameObject(TYPE_GEM, obj_agent8.pos, 10, "gem");
}

void UpdateBrokenPieces()
{
	std::vector<int> vPiecesIds = Play::CollectGameObjectIDsByType(TYPE_ASTEROID_PIECE);

	for (int id_piece : vPiecesIds)
	{
		GameObject& obj_piece = Play::GetGameObject(id_piece);

		Play::DrawObject(obj_piece);

		Play::UpdateGameObject(obj_piece);

		if (!Play::IsVisible(obj_piece))
		{
			Play::DestroyGameObject(id_piece);
		}
		SpawnParticle(obj_piece);
	}
}
 
void UpdateGem()
{
	if (gameState.agentState == Agent8State::STATE_FLYING || gameState.agentState == Agent8State::STATE_ATTACHED)
	{
		GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

		std::vector<int> vGemIds = Play::CollectGameObjectIDsByType(TYPE_GEM);

		for (int id_gem : vGemIds)
		{
			GameObject& obj_gem = Play::GetGameObject(id_gem);
			Play::DrawObject(obj_gem);
			Play::UpdateGameObject(obj_gem);

			if (Play::IsColliding(obj_agent8, obj_gem)) // immediately collides oops
			{
				gameState.remainingGems++;
				Play::DestroyGameObject(id_gem);
			}
		}
	}
}

void SpawnParticle(GameObject& obj) 
{
	int particleId = Play::CreateGameObject(TYPE_PARTICLE, obj.pos, 50, "particle");
	gameState.particleCountDown = 2;
	GameObject& obj_particle = Play::GetGameObject(particleId);
	obj_particle.velocity = { RandomFloatNumber(), RandomFloatNumber() };
	obj_particle.timeStamp = gameState.timer;
}

void UpdateParticles() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	
	if (gameState.agentState == Agent8State::STATE_FLYING) //&& gameState.particleCountDown <= 0
	{ 
		SpawnParticle(obj_agent8);
	}
	gameState.particleCountDown--;

	std::vector<int> vParticleIds = Play::CollectGameObjectIDsByType(TYPE_PARTICLE);

	for (int id_particle : vParticleIds)
	{
		GameObject& obj_particle = Play::GetGameObject(id_particle);

		Play::DrawObject(obj_particle);

		Play::UpdateGameObject(obj_particle);

		// particles destroy after x amount of time
		if ((gameState.timer - obj_particle.timeStamp) > 1.0f)
		{
			Play::DestroyGameObject(id_particle);
		}
	}
}
void SpawnAsteroids() 
{
	for (int i = 0; i < 5; i++) 
	{
		int myAsteroidId = Play::CreateGameObject(TYPE_ASTEROID, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 50, "asteroid");
		GameObject& myAsteroid = Play::GetGameObject(myAsteroidId);

		myAsteroid.velocity = { rand() % 2 + (-1), rand() % 2 + 1 };
	} 
}

void UpdateAsteroids() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	// for each asteroid id
	std::vector<int> vAsteroidIds = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);

	for (int id_asteroid : vAsteroidIds) 
	{
		GameObject& obj_asteroid = Play::GetGameObject(id_asteroid);

		// change direction of sprite 
		obj_asteroid.rotation = atan2(obj_asteroid.velocity.y, obj_asteroid.velocity.x) + (PLAY_PI / 2);
		Play::UpdateGameObject(obj_asteroid);

		// asteroids wrap around screen
		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_asteroid.spriteId);
		ScreenWrap(obj_asteroid, origin);

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

	// change direction of sprite
	obj_meteor.rotation = atan2(obj_meteor.velocity.y, obj_meteor.velocity.x) + (PLAY_PI / 2);
	Play::UpdateGameObject(obj_meteor);

	// meteor wraps around screen
	Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_meteor.spriteId);
	ScreenWrap(obj_meteor, origin);
	
	if (Play::IsColliding(obj_agent8, obj_meteor))
	{
		obj_agent8.velocity.x = sin(obj_agent8.rotation) * Agent8Speed;
		obj_agent8.velocity.y = -cos(obj_agent8.rotation) * Agent8Speed;
		
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
	
		if (Play::IsLeavingDisplayArea(obj_agent8))
		{
			gameState.agentState = Agent8State::STATE_OFFSCREEN;
		}
		break;
	case Agent8State::STATE_OFFSCREEN:
		Play::DrawFontText("151px", "GAME OVER", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE);
		Play::DrawFontText("64px", "PRESS ESCAPE TO EXIT", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);
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