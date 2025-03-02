#include "mineBlasters.hpp"

static void	initWindow(GameInfo& game)
{
	game.window = { Screen::size, {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)} };
	initWindow(game.window, "MineBlasters", GameRules::vSync, false, "");
}

static void	initSprites(GameInfo& game)
{
	game.sprites.tiles.try_emplace(Block::PATH,				"sprites/tiles/path.bmp");
	game.sprites.tiles.try_emplace(Block::CHEST1_CLOSED,	"sprites/tiles/chest1_closed.bmp");
	game.sprites.tiles.try_emplace(Block::CHEST1_OPEN,		"sprites/tiles/chest1_open.bmp");
	game.sprites.tiles.try_emplace(Block::CHEST2_CLOSED,	"sprites/tiles/chest2_closed.bmp");
	game.sprites.tiles.try_emplace(Block::CHEST2_OPEN,		"sprites/tiles/chest2_open.bmp");
	game.sprites.tiles.try_emplace(Block::CHEST3_CLOSED,	"sprites/tiles/chest3_closed.bmp");
	game.sprites.tiles.try_emplace(Block::CHEST3_OPEN,		"sprites/tiles/chest3_open.bmp");
	game.sprites.tiles.try_emplace(Block::EXIT,				"sprites/tiles/exit.bmp");
	game.sprites.tiles.try_emplace(Block::SHOP,				"sprites/tiles/shop.bmp");
	game.sprites.tiles.try_emplace(Block::SHOP_DESK,		"sprites/tiles/shop_desk.bmp");

	for (int i = 0; i <= Neighbor::MAX; i++)
	{
		game.sprites.tiles.try_emplace(i, "sprites/wall/" + std::to_string(i) + ".bmp");
	}
	for (int i = 0; i < Item::coinAnimationCount; i++)
	{
		game.sprites.gold.try_emplace(i, "sprites/animations/gold/" + std::to_string(i) + ".bmp");
	}
	for (int i = -1; i <= 5; i++)
	{
		game.sprites.bullets.try_emplace(i, "sprites/animations/bullet/" + std::to_string(i) + ".bmp");
	}
	for (int i = Item::LIGHT_AMMO; i <= Item::SHOTGUN_AMMO; i++)
	{
		game.sprites.items.try_emplace(i, "sprites/ammo/" + std::to_string(i - Item::LIGHT_AMMO) + ".bmp");
	}
	for (int i = -2; i < Ammo::MAX_TYPES; i++)
	{
		game.sprites.crosshairs.try_emplace(i, "sprites/crosshairs/" + std::to_string(i) + ".bmp");
	}
	for (int i = 0; i < 4; i++)
	{
		game.sprites.playerStanding.try_emplace(i, "sprites/animations/player/standing" + std::to_string(i) + ".bmp");
	}
	for (int i = 0; i < 8; i++)
	{
		game.sprites.playerRunning.try_emplace(i, "sprites/animations/player/running" + std::to_string(i) + ".bmp");
	}
	game.sprites.overlays.reserve(4);
	game.sprites.overlays.emplace_back(Fade::PEAKS,      1000, "sprites/overlays/blood.bmp");
	game.sprites.overlays.emplace_back(Fade::DECREMENTS, 2000, "sprites/overlays/intro.bmp");
	game.sprites.overlays.emplace_back(Fade::INCREMENTS, 1000, "sprites/overlays/intro.bmp");
	game.sprites.overlays.emplace_back(Fade::PERSISTS,   1000, "sprites/overlays/death.bmp");

	loadFont(game.sprites.font, "sprites/fonts/default.bmp");
	loadFont(game.sprites.fontLarge, "sprites/fonts/large.bmp");
}

static void	initEntities(GameInfo& game)
{
	std::vector<DropPool>	dropPool = {
		{ 1,   Item::COIN,		  10, 1 },
		{ 13,  Item::COIN,		   3, 1 },
		{ 30,  Item::COIN,		   2, 1 },
		{ 50,  Item::COIN,         1, 1 },
		{ 80,  Item::COIN,         0, 1 },
		{ 85,  Item::LIGHT_AMMO,   1, 4 },
		{ 90,  Item::MEDIUM_AMMO,  1, 3 },
		{ 95,  Item::HEAVY_AMMO,   1, 2 },
		{ 100, Item::SHOTGUN_AMMO, 1, 2 },
	};

	game.staticEntities.push_back({ 100, 10, Player::speed / 4, 0.75f, 1.0f, 1550, { 0.4f, 0.75f }, { 0.4f, 0.05f }, "Zombie", dropPool });
	for (int i = 0; i < 8; i++)
	{
		game.staticEntities[0].walkingAnimation.try_emplace(i, "sprites/animations/zombie/walking" + std::to_string(i) + ".bmp");
	}
	for (int i = 0; i < 16; i++)
	{
		game.staticEntities[0].deathAnimation.try_emplace(i, "sprites/animations/zombie/dying" + std::to_string(i) + ".bmp");
	}
}

static void	initWeapons(GameInfo& game)
{
	auto& all = game.weapons.all;
	auto& basic = game.weapons.basic;
	auto& fancy = game.weapons.fancy;

	all.reserve(16);
	//               name              price ammoType       reloadType    pellets maxAmmo damage  fireRate velocity accuracy baseReload, singleReload fullReload sprite                    spriteUI 
	all.emplace_back("Revolver",       -1,   Ammo::MEDIUM,  SPEED_LOADER,  1,      6,     50.0f,  90.0f,   2.0f,    5.0f,    1000,       1000,        1000,      "sprites/weapons/0G.bmp", "sprites/weapons/0UI.bmp");
	all.emplace_back("Lever Action",   -1,   Ammo::MEDIUM,  SINGLE,        1,     12,     35.0f,  180.0f,  2.5f,    3.0f,     500,        500,           0,      "sprites/weapons/1G.bmp", "sprites/weapons/1UI.bmp");
	all.emplace_back("Shotgun",        40,   Ammo::SHOTGUN, SINGLE,	      12,      2,     10.0f,  120.0f,  1.5f,    12.0f,   1500,        500,           0,      "sprites/weapons/2G.bmp", "sprites/weapons/2UI.bmp");
	all.emplace_back("Bolt Action",    50,   Ammo::HEAVY,   SPEED_LOADER,  1,      5,     150.0f, 55.0f,   3.5f,    2.0f,     800,        800,        1600,      "sprites/weapons/3G.bmp", "sprites/weapons/3UI.bmp");
	all.emplace_back("Machine Pistol", 25,   Ammo::LIGHT,   SPEED_LOADER,  1,     10,     40.0f,  240.0f,  2.0f,    4.0f,     400,        400,        1600,      "sprites/weapons/4G.bmp", "sprites/weapons/4UI.bmp");
	all.emplace_back("Derringer",      -1,   Ammo::LIGHT,   SPEED_LOADER,  1,      4,     50.0f,  120.0f,  1.8f,    6.5f,     800,        800,        2000,      "sprites/weapons/5G.bmp", "sprites/weapons/5UI.bmp");
	all.emplace_back("Sawed-Off",      -1,   Ammo::SHOTGUN, SINGLE,	      12,      2,     10.0f,  120.0f,  1.5f,    30.0f,   1500,        500,           0,      "sprites/weapons/6G.bmp", "sprites/weapons/6UI.bmp");
	all.emplace_back("Avtomat",        100,  Ammo::HEAVY,   MAGAZINE,	   1,     15,     140.0f, 240.0f,  3.2f,    3.0f,       0,           0,       3500,      "sprites/weapons/7G.bmp", "sprites/weapons/7UI.bmp");
	all.emplace_back("Trapdoor Rifle", -1,   Ammo::HEAVY,   MAGAZINE,	   1,      1,     180.0f, 30.0f,   3.8f,    1.5f,       0,           0,       2000,      "sprites/weapons/8G.bmp", "sprites/weapons/8UI.bmp");

	for (int i = 0; i < all.size(); i++)
	{
		if (all[i]._price == -1)
			basic.push_back(&all[i]);
		else
			fancy.push_back(&all[i]);
	}
}

static void	initMenus(GameInfo& game)
{
	game.menus.background[GameState::MENU].Load("sprites/menu/0.bmp");
	game.menus.background[GameState::CHARACTER_SELECT].Load("sprites/menu/1.bmp");
	game.menus.background[GameState::DEAD].Load("sprites/menu/4.bmp");

	Image&	menuButton = game.menus.menuButton;
	menuButton.Load("sprites/buttons/menu.bmp");

	Font&	f = game.sprites.font;
	Font&	fXL = game.sprites.fontLarge;
	float	offset = menuButton.size.y - 10.0f;
	Vec2	pos = { (game.window.vram.x - menuButton.size.x) / 2, game.window.vram.y - menuButton.size.y };
	
	game.menus.simpleButtons[GameState::MENU].emplace_back(Vec2{ pos.x, pos.y - 60 },             "New Game",  menuButton, fXL, std::bind(changeGameState, std::ref(game), GameState::CHARACTER_SELECT));
	game.menus.simpleButtons[GameState::MENU].emplace_back(Vec2{ pos.x, pos.y - 60 - offset },    "Load Game", menuButton, fXL, std::bind(loadGame, std::ref(game)));
	game.menus.simpleButtons[GameState::MENU].emplace_back(Vec2{ pos.x, pos.y - 60 - offset * 2}, "Quit Game", menuButton, fXL, std::bind(changeGameState, std::ref(game), GameState::EXIT_GAME));

	game.menus.simpleButtons[GameState::PAUSED].emplace_back(pos,                               "Continue",  menuButton, fXL, std::bind(changeGameState, std::ref(game), GameState::PLAYING));
	game.menus.simpleButtons[GameState::PAUSED].emplace_back(Vec2{ pos.x, pos.y - offset },     "To Menu",   menuButton, fXL, std::bind(changeGameState, std::ref(game), GameState::MENU));
	game.menus.simpleButtons[GameState::PAUSED].emplace_back(Vec2{ pos.x, pos.y - offset * 2 }, "Quit Game", menuButton, fXL, std::bind(changeGameState, std::ref(game), GameState::EXIT_GAME));

	Image& charButton = game.menus.charButton;
	Image& backButton = game.menus.backButton;
	charButton.Load("sprites/buttons/character.bmp");
	backButton.Load("sprites/buttons/back.bmp");

	Vec2	halfScreen = toVec2(game.window.vram, 0.5);
	float	offsetBorder = 5.0f;
	float	offsetY = 20.0f;
	Vec2	posArr[4] = {
		{ halfScreen.x - offsetBorder - charButton.size.x, halfScreen.y - offsetY + offsetBorder },
		{ halfScreen.x + offsetBorder,                     halfScreen.y - offsetY + offsetBorder },
		{ halfScreen.x - offsetBorder - charButton.size.x, halfScreen.y - offsetY - offsetBorder - charButton.size.y },
		{ halfScreen.x + offsetBorder,                     halfScreen.y - offsetY - offsetBorder - charButton.size.y }
	};

	for (int i = 0; i < 4; i++)
	{
		game.menus.simpleButtons[GameState::CHARACTER_SELECT].emplace_back(posArr[i], "", charButton, fXL, std::bind(selectCharacter, std::ref(game), i));
	}
	game.menus.simpleButtons[GameState::CHARACTER_SELECT].emplace_back(Vec2{ 7, (float)game.window.vram.y - backButton.size.y }, "Back", backButton, fXL, std::bind(changeGameState, std::ref(game), GameState::MENU));

	Image& deadButton = game.menus.deadButton;
	deadButton.Load("sprites/buttons/dead.bmp");

	game.menus.simpleButtons[GameState::DEAD].emplace_back(Vec2{ game.window.vram.x / 2 - 10.0f - deadButton.size.x, 10.0f }, "To Menu",   deadButton, fXL, std::bind(changeGameState, std::ref(game), GameState::MENU));
	game.menus.simpleButtons[GameState::DEAD].emplace_back(Vec2{ game.window.vram.x / 2 + 10.0f,                     10.0f }, "Quit Game", deadButton, fXL, std::bind(changeGameState, std::ref(game), GameState::EXIT_GAME));
}

void	staticInit(GameInfo& game)
{
	FreeConsole();
	getTime(game);
	initWindow(game);
	initSprites(game);
	initEntities(game);
	initWeapons(game);
	initPlayerEntity(game);
	resetStats(game);
	initMenus(game);
	game.state = GameState::MENU;
	//resetLevel(game);
}