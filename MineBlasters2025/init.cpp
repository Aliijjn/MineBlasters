#include "mineBlasters.hpp"

static void	initWindow(GameInfo& game)
{
	game.window = { Screen::size, {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)} };
	initWindow(game.window, "MineBlasters", GameRules::vSync, false, "");
}

static void	initSprites(GameInfo& game)
{
	game.sprites.tiles.emplace(Block::PATH,          Image{"sprites/tiles/path.bmp"         });
	game.sprites.tiles.emplace(Block::CHEST1_CLOSED, Image{"sprites/tiles/chest1_closed.bmp"});
	game.sprites.tiles.emplace(Block::CHEST1_OPEN,   Image{"sprites/tiles/chest1_open.bmp"  });
	game.sprites.tiles.emplace(Block::CHEST2_CLOSED, Image{"sprites/tiles/chest2_closed.bmp"});
	game.sprites.tiles.emplace(Block::CHEST2_OPEN,   Image{"sprites/tiles/chest2_open.bmp"  });
	game.sprites.tiles.emplace(Block::CHEST3_CLOSED, Image{"sprites/tiles/chest3_closed.bmp"});
	game.sprites.tiles.emplace(Block::CHEST3_OPEN,   Image{"sprites/tiles/chest3_open.bmp"  });
	game.sprites.tiles.emplace(Block::EXIT,          Image{"sprites/tiles/exit.bmp"         });
	game.sprites.tiles.emplace(Block::SHOP,          Image{"sprites/tiles/shop.bmp"         });
	game.sprites.tiles.emplace(Block::SHOP_DESK,     Image{"sprites/tiles/shop_desk.bmp"    });

	for (int i = 0; i <= Neighbor::MAX; i++)
	{
		game.sprites.tiles.emplace(i, Image{"sprites/wall/" + std::to_string(i) + ".bmp"});
	}
	for (int i = 0; i < Item::coinAnimationCount; i++)
	{
		game.sprites.gold.emplace(i, Image{"sprites/animations/gold/" + std::to_string(i) + ".bmp"});
	}
	for (int i = -1; i <= 5; i++)
	{
		game.sprites.bullets.emplace(i, Image{"sprites/animations/bullet/" + std::to_string(i) + ".bmp"});
	}
	for (int i = Item::LIGHT_AMMO; i <= Item::SHOTGUN_AMMO; i++)
	{
		game.sprites.items.emplace(i, Image{"sprites/ammo/" + std::to_string(i - Item::LIGHT_AMMO) + ".bmp"});
	}
	for (int i = -1; i < Ammo::MAX_TYPES; i++)
	{
		game.sprites.crosshairs.emplace(i, Image{"sprites/crosshairs/" + std::to_string(i) + ".bmp"});
	}
	for (int i = 0; i < 4; i++)
	{
		game.sprites.playerStanding.emplace(i, Image{"sprites/animations/player/standing" + std::to_string(i) + ".bmp"});
	}
	for (int i = 0; i < 8; i++)
	{
		game.sprites.playerRunning.emplace(i, Image{"sprites/animations/player/running" + std::to_string(i) + ".bmp"});
	}

	game.sprites.overlays.push_back(Overlay{ Fade::PEAKS,      1000, "sprites/overlays/blood.bmp" });
	game.sprites.overlays.push_back(Overlay{ Fade::DECREMENTS, 2000, "sprites/overlays/intro.bmp" });
	game.sprites.overlays.push_back(Overlay{ Fade::INCREMENTS, 1000, "sprites/overlays/intro.bmp" });
	game.sprites.overlays.push_back(Overlay{ Fade::PERSISTS,   1000, "sprites/overlays/death.bmp" });

	game.sprites.font      = loadFont("sprites/fonts/default.bmp");
	game.sprites.fontLarge = loadFont("sprites/fonts/large.bmp");
}

static void	initPlayerEntity(GameInfo& game)
{
	PlayerEntity& player = game.player;

	player = PlayerEntity
	{
		.facingRight  = false,
		.isMoving     = false,
		.isFrozen     = false,
		.isAlive      = true,
		.coinCount    = GameRules::cheats == true ? 1000 : 0,
		.health       = 100,
		.maxHealth    = 100,
		.pos          = { 1, 1 },
		.dir          = { 0, 0 },
		.reserveAmmo  = { 10, 10, 10, 10 },
		.primary      = { false, 0, 0, 6, &game.weapons.all[0] },
		.secondary    = {},
		.topPrompt    = TextPopUp{ 0.008f, Vec2{ game.window.vram.x / 2.0f, game.window.vram.y / 2.0f + 55 }, &game.sprites.fontLarge },
		.centrePrompt = TextPopUp{ 0.008f, Vec2{ game.window.vram.x / 2.0f, game.window.vram.y / 2.0f + 10 }, &game.sprites.font },
		.bottomPrompt = TextPopUp{ 0.0f,   Vec2{ game.window.vram.x / 2.0f, 10.0f                          }, &game.sprites.font }
	};
}

static void	initPlayerPos(GameInfo& game)
{
	PlayerEntity& player = game.player;

	player.pos = { 1, 1 };
	for (int i = 0; game.map[player.pos.y][player.pos.x] != Block::PATH; i++)
	{
		if (i % 2 == 0)
			player.pos.x++;
		else
			player.pos.y++;
	}
	player.pos.x++;
	player.pos.y++;
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

	game.staticEntities.push_back({ 100, 10, Player::speed / 4, 0.75f, 1.0f, 1550, { 0.4f, 0.75f }, { 0.4f, 0.05f }, "Zombie", dropPool});
	for (int i = 0; i < 8; i++)
	{
		game.staticEntities[0].walkingAnimation.emplace(i, Image("sprites/animations/zombie/walking" + std::to_string(i) + ".bmp"));
	}
	for (int i = 0; i < 16; i++)
	{
		game.staticEntities[0].deathAnimation.emplace(i, Image("sprites/animations/zombie/dying" + std::to_string(i) + ".bmp"));
	}
}

static void	initWeapons(GameInfo& game)
{
	auto&	all   = game.weapons.all;
	auto&	basic = game.weapons.basic;
	auto&	fancy = game.weapons.fancy;

	//               name           isFancy ammoType       reloadType    pellets maxAmmo damage  fireRate velocity accuracy baseReload, singleReload fullReload sprite                               spriteUI 
	all.push_back({ "Revolver",     false,  Ammo::LIGHT,   SPEED_LOADER, 1,      6,      50.0f,  90.0f,   2.0f,    5.0f,    1000,       1000,        1000,      Image{"sprites/weapons/0G.bmp"}, Image{"sprites/weapons/0UI.bmp"} });
	all.push_back({ "Lever Action", false,  Ammo::MEDIUM,  SINGLE,       1,      12,     40.0f,  180.0f,  2.5f,    3.0f,     500,        500,           0,      Image{"sprites/weapons/1G.bmp"}, Image{"sprites/weapons/1UI.bmp"} });
	all.push_back({ "Shotgun",      true,   Ammo::SHOTGUN, SINGLE,		 12,     2,      10.0f,  120.0f,  1.5f,    12.0f,   1500,        500,           0,      Image{"sprites/weapons/2G.bmp"}, Image{"sprites/weapons/2UI.bmp"} });
	all.push_back({ "Bolt Action",  true,   Ammo::HEAVY,   SPEED_LOADER, 1,      5,      150.0f, 55.0f,   3.5f,    2.0f,     800,        800,        1600,      Image{"sprites/weapons/3G.bmp"}, Image{"sprites/weapons/3UI.bmp"} });
	all.push_back({ "Broom Handle", true,   Ammo::LIGHT,   SPEED_LOADER, 1,      10,     40.0f,  300.0f,  2.0f,    4.0f,     400,        400,        1600,      Image{"sprites/weapons/4G.bmp"}, Image{"sprites/weapons/4UI.bmp"} });

	for (int i = 0; i < all.size(); i++)
	{
		if (all[i].isFancy == true)
			fancy.push_back(&all[i]);
		else
			basic.push_back(&all[i]);
	}
}

static void	displayLevel(GameInfo& game)
{
	game.player.topPrompt.Add(game, (game.misc.isInShop ? "Shop " : "Level ") + std::to_string(game.misc.level), 4500, true);
}

static void resetStats(GameInfo& game)
{
	game.stats.coinsCollected = 0;
	game.stats.damageDealt = 0;
	game.stats.enemiesKilled = 0;
	game.stats.shotsFired = 0;
	game.stats.startTime = game.misc.MS;
	game.stats.timeOfDeath = 0;
}

static void	resetVars(GameInfo& game)
{
	game.bullets.clear();
	game.droppedWeapons.clear();
	game.items.clear();
	game.entities.clear();
	game.shops.clear();

	for (Overlay& o : game.sprites.overlays)
		o.Stop();

	game.player.isFrozen = false;
	game.player.isAlive = true;
}

static void	init(GameInfo& game)
{
	resetVars(game);
	game.sprites.overlays[Sprites::INTRO].StartNow(game);
	game.misc.levelStart = game.misc.MS;
	game.map = initMap(game, game.misc.level);
	initPlayerPos(game);
	displayLevel(game);
}

void	staticInit(GameInfo& game)
{
	game.misc.seed = tRand(1'000'000);
	getTime(game);
	initWindow(game);
	initSprites(game);
	initEntities(game);
	initWeapons(game);
	initPlayerEntity(game);
	resetStats(game);
	init(game);
}

void	respawn(GameInfo& game)
{
	game.misc.level = 1;
	game.misc.isInShop = false;
	initPlayerEntity(game);
	resetStats(game);
	init(game);
}

void	nextLevel(GameInfo& game)
{
	if (game.misc.isInShop == true)
	{
		game.misc.level++;
	}
	game.misc.isInShop = !game.misc.isInShop;
	init(game);
}