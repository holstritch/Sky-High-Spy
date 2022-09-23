#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

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


// entry point for the playbuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::StartAudioLoop("music");
	Play::CreateGameObject(TYPE_AGENT8, { 640, 360 }, 50, "agent8");
	Play::CentreAllSpriteOrigins();
	Play::SetSpriteOrigin("agent8_fly", 50, 100);
}

// called by playbuffer once for each frame 
bool MainGameUpdate(float elapsedTime) 
{
	gameState.timer += elapsedTime;

	Play::DrawBackground();
	PlayerControls();
	SpawnAsteroids();
	UpdateAgent8();
	Play::DrawFontText("105px", "REMAINING GEMS: " + std::to_string(gameState.remainingGems), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::DrawFontText("64px", "ARROW KEYS TO ROTATE AND SPACE BAR TO LAUNCH", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);
	Play::PresentDrawingBuffer();

	return Play::KeyDown(VK_ESCAPE);
}

void PlayerControls() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	Play::UpdateGameObject(obj_agent8, 1);

	// movement and anim
	if (Play::KeyDown(VK_LEFT))
	{
		Play::SetSprite(obj_agent8, "agent8_left", 0.2f);

	}
	if (Play::KeyDown(VK_RIGHT))
	{
		Play::SetSprite(obj_agent8, "agent8_right", 0.2f);
	}
}
void SpawnAsteroids() 
{
	GameObject& obj_asteroid = Play::GetGameObjectByType(TYPE_ASTEROID);
	Play::CreateGameObject(TYPE_ASTEROID, { 300, 300 }, 50, "asteroid");
	Play::DrawObject(obj_asteroid);
}

void UpdateAgent8() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	Play::DrawObject(obj_agent8);

	switch (gameState.agentState) 
	{
	case Agent8State::STATE_WALK:
		PlayerControls();

		break;

	case Agent8State::STATE_FLY:
		//flying
		break;

	case Agent8State::STATE_DEAD:
		// dead
		break;

	} // end of switch
}

// called once the player quits the game

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}