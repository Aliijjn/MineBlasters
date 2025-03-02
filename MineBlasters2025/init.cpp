#include "mineBlasters.hpp"

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

void	initStartOptions(GameInfo& game)
{
	StartOptions*	s = game.player.startOptions;

	std::vector<std::string>	firstNames = { "Buck", "Jed", "Colt", "Hank", "Wade", "Zeke", "Clint", "Boone", "Earl", "Silas", "Clyde", "Jesse", "Gus", "Eli", "Levi", "Rhett", "Tex", "Harlan", "Doc", "Bodie", "Roy", "Jeb", "Virgil", "Frank", "Clay", "Monty", "Bart", "Caleb", "Luke", "Sam", "Chet", "Billy", "Stetson", "Denver", "Charlie", "Cody", "Morgan", "Quentin", "Travis", "Bob", "Seth", "Ezekiel" };
	std::vector<std::string>	nickNames  = { "Ace", "Badger", "Big Tex", "Blackjack", "Boots", "Buckshot", "Bull", "Cactus", "Calico", "Copper", "Crow", "Curly", "Dakota", "Deacon", "Dusty", "El Lobo", "Fiddle", "Flint", "Grizzly", "Grit", "Hawk", "High Noon", "Hombre", "Iron Jack", "Kid", "Lariat", "Lefty", "Lightning", "Lobo", "Longshot", "Lucky", "Mad Dog", "Mesa", "Mustang", "Nightrider", "Pecos", "Preacher", "Quickdraw", "Ranger", "Rawhide", "Red", "Rowdy", "Rusty", "Shifty", "Silver", "Slim", "Snakebite", "Spurs", "Tumbleweed", "Whiskey", "Wild Bill", "Wolf" };
	std::vector<std::string>	lastNames  = { "Abbott", "Bennett", "Bowen", "Bradford", "Briggs", "Burke", "Carson", "Clay", "Collins", "Connelly", "Cooper", "Crawford", "Dawson", "Dixon", "Douglas", "Duncan", "Ellis", "Evans", "Ferguson", "Fletcher", "Gentry", "Graves", "Greer", "Hardin", "Harmon", "Harrison", "Henderson", "Hogan", "Holt", "Keller", "Langley", "Lawson", "Logan", "Marshall", "McAllister", "Mercer", "Nash", "Palmer", "Porter", "Quinn", "Ramsey", "Reeves", "Riley", "Schaefer", "Shepherd", "Sloan", "Sullivan", "Tanner", "Vaughn", "Webb", "Weston" };

	for (int i = 0; i < StartOptions::MAX; i++)
	{
		do
		{
			s[i].name = firstNames[tRand(firstNames.size())] + " ";
			if (tRand(3) == 0)
				s[i].name += "\"" + nickNames[tRand(nickNames.size())] + "\" ";
			s[i].name += lastNames[tRand(lastNames.size())];
		}
		while (s[i].name.length() > 23);
		s[i].weapon = game.weapons.basic[tRand(game.weapons.basic.size())];
	}
}

void	initPlayerEntity(GameInfo& game)
{
	PlayerEntity& player = game.player;

	player = PlayerEntity
	{
		.facingRight = false,
		.isMoving = false,
		.isFrozen = false,
		.isAlive = true,
		.coinCount = GameRules::cheats == true ? 1000 : 0,
		.health = 100,
		.maxHealth = 100,
		.pos = { 1, 1 },
		.dir = { 0, 0 },
		.reserveAmmo = { 10, 10, 10, 10 },
		.primary = { false, 0, 0, 6, &game.weapons.all[0] },
		.secondary = {},
		.topPrompt = TextPopUp{ 0.008f, Vec2{ game.window.vram.x / 2.0f, game.window.vram.y / 2.0f + 55 }, &game.sprites.fontLarge },
		.centrePrompt = TextPopUp{ 0.008f, Vec2{ game.window.vram.x / 2.0f, game.window.vram.y / 2.0f + 10 }, &game.sprites.font },
		.bottomPrompt = TextPopUp{ 0.0f,   Vec2{ game.window.vram.x / 2.0f, 10.0f                          }, &game.sprites.font }
	};
}

static void	displayLevel(GameInfo& game)
{
	game.player.topPrompt.Add(game, (game.misc.isInShop ? "Shop " : "Level ") + std::to_string(game.misc.level), 4500, true);
}

void resetStats(GameInfo& game)
{
	game.stats.coinsCollected = 0;
	game.stats.damageDealt = 0;
	game.stats.enemiesKilled = 0;
	game.stats.shotsFired = 0;
	game.stats.startTime = game.misc.MS;
	game.stats.timeOfDeath = 0;
}

static void	resetEntities(GameInfo& game)
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

void	resetLevel(GameInfo& game)
{
	resetEntities(game);
	game.sprites.overlays[Sprites::INTRO].StartNow(game);
	game.misc.levelStart = game.misc.MS;
	game.map = initMap(game, game.misc.level);
	initPlayerPos(game);
	displayLevel(game);
}

void	selectCharacter(GameInfo& game, uint8_t	characterIndex)
{
	game.player.primary = { false, 0, 0, 0, game.player.startOptions[characterIndex].weapon };
	game.player.primary.ammo = game.player.primary.ptr->_capacity;
	game.state = GameState::PLAYING;
	resetLevel(game);
}

void	changeGameState(GameInfo& game, GameState gameState)
{
	game.state = gameState;
	if (game.state == GameState::CHARACTER_SELECT)
	{
		initStartOptions(game);
		game.misc.seed = tRand(1'000'000);
	}
}

void	respawn(GameInfo& game)
{
	game.misc.level = 1;
	game.misc.isInShop = false;
	initPlayerEntity(game);
	resetStats(game);
	resetLevel(game);
}

void	nextLevel(GameInfo& game)
{
	if (game.misc.isInShop == true)
	{
		game.misc.level++;
		saveGame(game);
	}
	game.misc.isInShop = !game.misc.isInShop;
	resetLevel(game);
}
