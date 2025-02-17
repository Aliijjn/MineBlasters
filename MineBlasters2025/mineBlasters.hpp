#pragma once

#include <map>
#include <list>
#include <deque>
#include <mutex>
#include <stack>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <future>
#include <fstream>
#include <iostream>

#include <math.h>
#include <time.h>
#include <intrin.h>
#include <stdlib.h>
#include <Windows.h>
#include <mmsystem.h>

#include <gl/GL.h>

#include "graphics.hpp"
#include "Vector2.hpp"
#include "TextPrompt.hpp"
#include "Overlay.hpp"

struct Ray
{
	Vec2	pos;
	Vec2	dir;
	Vec2	delta;
	Vec2	side;
	IVec2	step;
	IVec2	square_pos;
};

namespace Neighbor
{
	enum
	{
		TOP_RIGHT,
		TOP_LEFT,
		BOTTOM_LEFT,
		BOTTOM_RIGHT,
		TOP,
		RIGHT,
		LEFT,
		BOTTOM,
		MAX
	};

	static constexpr IVec2	check[Neighbor::MAX]
	{
		{1, 1},
		{-1, 1},
		{-1, -1},
		{1, -1},
		{0, 1},
		{1, 0},
		{-1, 0},
		{0, -1}
	};
}

namespace Block
{
	static constexpr IVec2		size = { 32, 32 };
	static constexpr Vec2		count = { 20, 11.25 };
	static constexpr uint8_t	wallMax = 15;

	enum
	{
		EMPTY = 0,
		WALL_MAX = 255,
		NO_COLLISION, // anything beyond this has no collision with entities
		PATH,
		CHEST1_CLOSED,
		CHEST1_OPEN,
		CHEST2_CLOSED,
		CHEST2_OPEN,
		CHEST3_CLOSED,
		CHEST3_OPEN,
		ZOMBIE_SPAWN,
		EXIT,
		SHOP,
		SHOP_DESK,
		UNKNOWN
	};
}

namespace Player
{
	static constexpr IVec2		size = { 32, 32 };
	static constexpr Vec2		hitBox = { 16.0f / Block::size.x, 32.0f / Block::size.y };
	static constexpr Vec2		wallHitBox = { 20.0f / Block::size.x, 2.0f / Block::size.y };
	static constexpr float		eyesOffset = 0.0f;
	static constexpr float		chestRange = 1.25f;
	static constexpr float		speed = 0.009f;
}

namespace Chunk
{
	constexpr uint32_t	size = 20;
	constexpr float		wallOdds = 0.25f;
}

namespace Screen
{
	static constexpr IVec2	size = {
		Block::size.x * Block::count.x, 
		Block::size.y * Block::count.y 
	};
}

namespace GameRules
{
	static constexpr int8_t LOD = 5;
	static constexpr bool	vSync = false;
	static constexpr bool	cheats = false;
}

namespace Ammo
{
	enum
	{
		LIGHT = 0,
		MEDIUM,
		HEAVY,
		SHOTGUN,
		MAX_TYPES
	};
	constexpr IVec2	spriteSize = {16, 16};
}

enum
{
	SINGLE,
	SPEED_LOADER,
	MAGAZINE
};

enum
{
	NO_RELOAD,
	PARTIAL_RELOAD,
	FULL_RELOAD
};

struct StaticWeapon
{
	std::string	name;

	bool		isFancy;
	uint8_t		ammoType, reloadType, pellets;
	uint16_t	maxAmmo;
	float		damage, fireRate, velocity, accuracy;
	int64_t		baseReload, singleReload, fullReload;
	Image		sprite, spriteUI;
};

struct DroppedWeapon
{
	Vec2			pos, dir;
	uint16_t		ammo;
	StaticWeapon*	ptr;
};

struct Weapon
{
	uint8_t			isReloading;
	int64_t			reloadStart, reloadEnd;
	uint16_t		ammo;
	StaticWeapon*	ptr;
};

struct Bullet
{
	bool		isActive;
	float		damage;
	Vec2		pos, dir;
	int64_t		startMS;
};

struct DropPool
{
	uint32_t	odds, type, copies, stackSize;
};

struct StaticEntity
{
	enum
	{
		ZOMBIE
	};
	float					maxHealth, damage, speed, range, attackCooldown;
	int64_t					deathTimeOut;
	Vec2					hitBox, wallHitBox;
	std::string				name;
	std::vector<DropPool>	dropPool;
	std::map<int, Image>	walkingAnimation;
	std::map<int, Image>	attackAnimation;
	std::map<int, Image>	deathAnimation;
};

struct Entity
{
	bool			facingRight;
	float			health;
	int64_t			lastAttack, timeOfDeath;
	Vec2			pos, dir;
	StaticEntity*	ptr;
};

struct PlayerEntity
{
	bool		facingRight, isMoving, isFrozen, isAlive;
	uint16_t	coinCount;
	float		health, maxHealth;
	Vec2		pos, dir = { 0, 0 };

	uint16_t	reserveAmmo[Ammo::MAX_TYPES];
	Weapon		primary, secondary;

	TextPopUp	topPrompt, centrePrompt, bottomPrompt;
};

namespace Item
{
	static constexpr IVec2		size = { 16, 16 };
	static constexpr float		speed = 0.0075;
	static constexpr float		friction = 0.992f;
	static constexpr uint8_t	coinAnimationCount = 12;

	struct Dropped
	{
		Vec2		pos, dir;
		int64_t		MS;
		uint16_t	type;
		uint16_t	count;
	};

	enum
	{
		COIN = 1,
		LIGHT_AMMO,
		MEDIUM_AMMO,
		HEAVY_AMMO,
		SHOTGUN_AMMO
	};
}

namespace Ammo
{
	using namespace Item;

	struct StaticAmmo
	{
		uint16_t	type, dropCount;
		std::string	name;
	};

	constexpr uint16_t		types = SHOTGUN_AMMO - LIGHT_AMMO + 1;
	inline StaticAmmo		pool[types] =
	{
		{ LIGHT_AMMO,   6, "light ammo" },
		{ MEDIUM_AMMO,  5,  "medium ammo" },
		{ HEAVY_AMMO,   3,  "heavy ammo" },
		{ SHOTGUN_AMMO, 4,  "shotgun ammo" }
	};
}

struct Misc
{
	bool		isInShop = false;
	int8_t		LOD = GameRules::LOD;
	uint32_t	seed, level = 1;
	int64_t		MS = 0, levelStart = 0, nextLevel = 0;
	Vec2		cursorPos; // not relative!!
};

struct Sprites
{
	enum
	{
		BLOOD, INTRO, OUTRO, DEATH
	};

	std::map<uint16_t, Image>	tiles;
	std::map<uint16_t, Image>	items;
	std::map<uint8_t,  Image>	gold;
	std::map<int8_t,   Image>	bullets;
	std::map<int8_t,   Image>	crosshairs;
	std::map<int8_t,   Image>	playerStanding;
	std::map<int8_t,   Image>	playerRunning;
	std::vector<Overlay>		overlays;
	Font						font;
	Font						fontLarge;
};

struct Weapons
{
	std::vector<StaticWeapon>	all;
	std::vector<StaticWeapon*>	basic;
	std::vector<StaticWeapon*>	fancy;
};

struct Stats
{
	int64_t		startTime, timeOfDeath;
	uint64_t	coinsCollected, shotsFired, \
				damageDealt, enemiesKilled;
};

//enum class GameState : uint8_t
//{
//	RUNNING, DEAD, START
//};

struct GameInfo
{
	PlayerEntity								player;
	Weapons										weapons;
	std::vector<std::vector<uint16_t>>			map;
	std::vector<DroppedWeapon>					droppedWeapons;
	std::vector<Item::Dropped>					items;
	std::vector<Bullet>							bullets;
	std::vector<StaticEntity>					staticEntities;
	std::vector<Entity>							entities;
	std::map<IVec2, StaticWeapon*>				shops;

	Sprites										sprites;
	Misc										misc;
	Stats										stats;
	WindowInfo									window;
};

std::vector<std::vector<uint16_t>>	initMap(GameInfo& game, uint32_t lvl);

// logic
void	gameLogic(GameInfo& game);
void	manageEntities(GameInfo& game);
void	manageWeapons(GameInfo& game);

// spit out stuff
void	spitOutItems(GameInfo& game, Vec2 pos, uint16_t type, uint16_t copies, uint16_t count = 1);
void	spitOutWeapon(GameInfo& game, Vec2 pos, bool isFancy = false, StaticWeapon* ptr = nullptr);

// render
void	render(GameInfo& game);
void	renderGrid(GameInfo& game);
void	renderBlockingWalls(GameInfo& game);

// init
void	staticInit(GameInfo& game);
void	respawn(GameInfo& game);
void	nextLevel(GameInfo& game);

// tools
uint32_t	tRand(uint32_t modulo);
float		floatMod1(float f);
bool		isOutOfBounds(IVec2 pos, uint32_t size);
bool		isOutOfBounds(IVec2 pos, IVec2 size);
void		checkColission(GameInfo& game, Vec2	pos, Vec2& move, Vec2 hitBox);
bool		dotProduct(Vec2 i1, Vec2 i2, float maxLen);
bool		DDA(GameInfo& game, Vec2 start, Vec2 end);
int			manhattanLine(IVec2 start, IVec2 end);
int			countFPS();
uint64_t    getTicks();
void		getTime(GameInfo& game);
