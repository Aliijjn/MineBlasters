#include "mineBlasters.hpp"

void	renderWall(GameInfo& game, Vec2 pos, uint16_t val)
{
	drawImage(pos, game.window, &game.sprites.tiles[Block::EMPTY]);
	for (uint8_t i = 0; i < Neighbor::MAX; i++)
	{
		if (val & (1 << i))
		{
			drawImage(pos, game.window, &game.sprites.tiles[i + 1]);
		}
	}
}

static void	renderTile(GameInfo& game, Vec2 pos, Vec2 tilePos, uint16_t val, int8_t shade, float size)
{
	Vec2		newPos = { pos.x + tilePos.x * 32, pos.y + tilePos.y * 32 };

	drawImage(newPos, tilePos, size, game.window, &game.sprites.tiles[Block::PATH], 63 + 16 * shade);
	if (val != Block::PATH && shade != 0)
	{
		drawImage(newPos, tilePos, size, game.window, &game.sprites.tiles[val], 63 + 16 * shade);
	}
}

static bool smartPathFind(GameInfo& game, IVec2 pos, IVec2 dir)
{
	// base cases
	if (game.map[pos.y][pos.x] <= Block::WALL_MAX)
		return false;
	if (dir.x == 0 && dir.y == 0)
		return true;
	// recursive shenanigans
	if (dir.x > 0 && smartPathFind(game, { pos.x + 1, pos.y }, { dir.x - 1, dir.y }) == true)
		return true;
	if (dir.x < 0 && smartPathFind(game, { pos.x - 1, pos.y }, { dir.x + 1, dir.y }) == true)
		return true;
	if (dir.y > 0 && smartPathFind(game, { pos.x, pos.y + 1 }, { dir.x, dir.y - 1 }) == true)
		return true;
	if (dir.y < 0 && smartPathFind(game, { pos.x, pos.y - 1 }, { dir.x, dir.y + 1 }) == true)
		return true;
	return false;
}

// checks if a tile cannot be seen because of curves in the map
static bool	tileIsHidden(GameInfo& game, IVec2 tile)
{
	IVec2	playerPos = toIVec2({ game.player.pos.x, game.player.pos.y + Player::eyesOffset });
	IVec2	dir = subtractIVec2(tile, playerPos);

	return (!smartPathFind(game, playerPos, dir));
}

static int8_t	checkShade(GameInfo& game, IVec2 index, Vec2 tilePos, float size, int depth)
{
	const float	min = /*(depth == game.misc.LOD) ? size / 4 :*/ 0.00001;
	const float max = size - min;
	const Vec2	AAoffsets[4] = { {tilePos.x + min, tilePos.y + min}, {tilePos.x + min, tilePos.y + max}, {tilePos.x + max, tilePos.y + max}, {tilePos.x + max, tilePos.y + min} };
	int8_t		shade = 0;

	for (int i = 0; i < 4; i++)
	{
		if (DDA(game, { game.player.pos.x, game.player.pos.y + Player::eyesOffset }, { index.x + AAoffsets[i].x, index.y + AAoffsets[i].y }))
		{
			shade++;
		}
	}
	return shade;
}

struct RenderCall
{
	int8_t		shade;
	uint16_t	val;
	float		size;
	Vec2		pos, tilePos;
};

static int8_t	checkTiles(GameInfo& game, std::vector<RenderCall>& renderCalls, Vec2 pos, IVec2 index, uint16_t val, Vec2 tilePos = { 0, 0 }, int8_t depth = 0)
{
	float	size = (float)1 / (1 << depth);
	int8_t	shades[4]{};
	Vec2	offsets[4] = { {0, 0}, {size / 2, 0}, {0, size / 2}, {size / 2, size / 2} };

	if (depth == 0)
	{
		if (checkShade(game, index, tilePos, size, depth) == 4)
		{
			renderCalls.push_back({ 4, val, size, pos, tilePos });
			return -1;
		}
		if (tileIsHidden(game, index) == true)
		{
			renderCalls.push_back({ 0, val, size, pos, tilePos });
			return -1;
		}
	}
	if (depth < game.misc.LOD)
	{
		for (int8_t i = 0; i < 4; i++)
		{
			shades[i] = checkTiles(game, renderCalls, pos, index, val, addVec2(tilePos, offsets[i]), depth + 1);
		}
		if (shades[0] != -1 && shades[0] == shades[1] && shades[0] == shades[2] && shades[0] == shades[3])
		{
			if (depth == 0)
			{
				renderCalls.push_back({ shades[0], val, size, pos, tilePos });
			}
			return shades[0];
		}
		for (int8_t i = 0; i < 4; i++)
		{
			if (shades[i] != -1)
			{
				renderCalls.push_back({ shades[i], val, size / 2, pos, addVec2(tilePos, offsets[i]) });
			}
		}
		return -1;
	}
	return checkShade(game, index, tilePos, size, depth);
}

struct Tiles
{
	Vec2		pos;
	IVec2		index;
	uint16_t	val;
};

int					renderPtr;
std::mutex			ptrMutex;
std::vector<Tiles>	toRender;

static void	initThreads(GameInfo& game, std::vector<RenderCall>& renderCalls)
{
	while (1)
	{
		Tiles	job;
		{
			std::unique_lock<std::mutex> lock(ptrMutex);
			if (renderPtr >= toRender.size())
			{
				return;
			}
			job = toRender[renderPtr++];
		}
		checkTiles(game, renderCalls, job.pos, job.index, job.val);
	}
}

bool	getPositions(GameInfo& game, Vec2& pos, IVec2& index, const IVec2 relativePos)
{
	pos = {
		(relativePos.x - floatMod1(game.player.pos.x)) * Block::size.x,
		(relativePos.y - floatMod1(game.player.pos.y) + 0.5f) * Block::size.y
	};
	index = {
		(int)(std::floor(game.player.pos.x) - std::floor(Block::count.x / 2) + relativePos.x),
		(int)(std::floor(game.player.pos.y) - std::floor(Block::count.y / 2) + relativePos.y)
	};
	return isOutOfBounds(index, { (int)game.map[0].size(), (int)game.map.size() });
}

void	renderGrid(GameInfo& game)
{
	uint32_t	numThreads = std::thread::hardware_concurrency() / 2;

	if (numThreads <= 0)
		numThreads = 1;

	for (int32_t y = -1; y <= Block::count.y + 1; y++)
	{
		for (int32_t x = -1; x <= Block::count.x + 1; x++)
		{
			Vec2	pos;
			IVec2	index;

			if (getPositions(game, pos, index, {x, y}) == true)
			{
				drawImage(pos, game.window, &game.sprites.tiles[Block::EMPTY]);
				continue;
			}
			uint16_t	val = game.map[index.y][index.x];

			if (val <= Block::WALL_MAX)
			{
				renderWall(game, pos, val);
			}
			else
			{
				toRender.push_back({ pos, index, val });
			}
		}
	}

	std::vector<std::vector<RenderCall>>	renderCalls(numThreads);
	std::vector<std::future<void>>			futures;

	renderPtr = 0;
	for (int i = 0; i < numThreads; i++)
	{
		futures.push_back(std::async(std::launch::async, initThreads, std::ref(game), std::ref(renderCalls[i])));
	}
	for (auto& f : futures)
	{
		f.get();
	}
	for (auto& it : renderCalls)
	{
		for (const RenderCall& r : it)
		{
			renderTile(game, r.pos, r.tilePos, r.val, r.shade, r.size);
		}
	}
	toRender.clear();
}

void	renderBlockingWalls(GameInfo& game)
{
	for (int32_t y = -1; y <= Block::count.y + 1; y++)
	{
		for (int32_t x = -1; x <= Block::count.x + 1; x++)
		{
			Vec2	pos;
			IVec2	index;

			if (getPositions(game, pos, index, { x, y }) == false)
			{
				uint16_t	val = game.map[index.y][index.x];

				if (val <= Block::WALL_MAX && val & (1 << Neighbor::TOP))
				{
					renderWall(game, pos, val);
				}
			}
		}
	}
}