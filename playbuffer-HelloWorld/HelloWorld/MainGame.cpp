#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr int DISPLAY_WIDTH = 1280;
constexpr int DISPLAY_HEIGHT = 720;
constexpr int DISPLAY_SCALE = 1;

constexpr float Agent8RotSpeed = 0.05f;

enum class Agent8State
{
	STATE_WALK = 0,
	STATE_FLY,
	STATE_DEAD,
};

struct GameState 
{
	int remainingGems = 0;
	float timer = 0;
	int spriteId = 0;
	Agent8State agentState = Agent8State::STATE_WALK;
};

GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_ASTEROID,
};

// function prototypes
void PlayerControls();
void SpawnAsteroids();
void UpdateAgent8();
void UpdateAsteroids();


// entry point for the playbuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::StartAudioLoop("music");
	Play::CreateGameObject(TYPE_AGENT8, { 640, 360 }, 50, "agent8");
	Play::CentreAllSpriteOrigins();
	Play::SetSpriteOrigin("agent8_fly", 50, 100);
	Play::SetSpriteOrigin("agent8_left", 50, 110);
	Play::SetSpriteOrigin("agent8_right", 50, 110);
	SpawnAsteroids();
}

// called by playbuffer once for each frame 
bool MainGameUpdate(float elapsedTime) 
{
	gameState.timer += elapsedTime;

	Play::DrawBackground();
	PlayerControls();
	UpdateAsteroids();
	UpdateAgent8();
	Play::DrawFontText("105px", "REMAINING GEMS: " + std::to_string(gameState.remainingGems), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "ARROW KEYS TO ROTATE AND SPACE BAR TO LAUNCH", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);
	Play::PresentDrawingBuffer();

	return Play::KeyDown(VK_ESCAPE);
}

void PlayerControls() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj_asteroid = Play::GetGameObjectByType(TYPE_ASTEROID);

	obj_agent8.pos = obj_asteroid.pos;
	// movement and anim on asteroid
	if (gameState.agentState == Agent8State::STATE_WALK) 
	{
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

		Play::UpdateGameObject(obj_agent8);
	}
}
void SpawnAsteroids() 
{
	for (int i = 0; i < 3; i++) 
	{
		int myAsteroidId = Play::CreateGameObject(TYPE_ASTEROID, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 50, "asteroid");
		GameObject& myAsteroid = Play::GetGameObject(myAsteroidId);

		myAsteroid.velocity = { rand() % 2 + 1, rand() % 2 + 1};
	}
}

void UpdateAsteroids() 
{
	std::vector<int> vAsteroidIds = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	for (int id_asteroids : vAsteroidIds) 
	{
		GameObject& obj_asteroid = Play::GetGameObject(id_asteroids);

		Play::UpdateGameObject(obj_asteroid);

		if (Play::IsColliding(obj_agent8, obj_asteroid))
		{
			gameState.agentState = Agent8State::STATE_WALK;
		}

		// asteroids stay on screen
		if (obj_asteroid.pos.y > DISPLAY_HEIGHT) 
		{
			obj_asteroid.pos.y = + 20;
		}

		else if (obj_asteroid.pos.y < 0)
		{
			obj_asteroid.pos.y = obj_asteroid.pos.y + 20, DISPLAY_HEIGHT;
		}

		else if (obj_asteroid.pos.x > DISPLAY_WIDTH) 
		{
			obj_asteroid.pos.x = +20;
		}

		else if (obj_asteroid.pos.x < 0)
		{
			obj_asteroid.pos.y = obj_asteroid.pos.x + 20, DISPLAY_HEIGHT;
		}

		Play::DrawObject(obj_asteroid);
	}

}

void UpdateAgent8() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	Play::DrawObjectRotated(obj_agent8);

	switch (gameState.agentState) 
	{
	case Agent8State::STATE_WALK:
		PlayerControls();

		break;

	case Agent8State::STATE_FLY:
		Play::SetSprite(obj_agent8, "agent8_fly", 0.2f);
		break;

	case Agent8State::STATE_DEAD:
		Play::SetSprite(obj_agent8, "agent8_dead", 0.2f);
		break;

	} // end of switch
}

// called once the player quits the game

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}