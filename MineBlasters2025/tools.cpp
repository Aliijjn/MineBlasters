#include "mineBlasters.hpp"

uint32_t	tRand(uint32_t modulo)
{
	uint32_t	x = __rdtsc();

	x += (x << 10);
	x ^= (x >> 6);
	x += (x << 3);
	x ^= (x >> 11);
	x += (x << 15);
	return x % modulo;
}

float	floatMod1(float f)
{
	return f - std::floor(f);
}

bool	isOutOfBounds(IVec2 pos, uint32_t size)
{
	return (pos.x < 0 || pos.y < 0 || pos.x >= size || pos.y >= size);
}

bool	isOutOfBounds(IVec2 pos, IVec2 size)
{
	return (pos.x < 0 || pos.y < 0 || pos.x >= size.x || pos.y >= size.y);
}

void	checkColission(GameInfo& game, Vec2	pos, Vec2& move, Vec2 hitBox)
{
	// collision at
	bool	topLeft = game.map[pos.y + move.y + hitBox.y / 2][pos.x + move.x - hitBox.x / 2] != Block::PATH;
	bool	topRight = game.map[pos.y + move.y + hitBox.y / 2][pos.x + move.x + hitBox.x / 2] != Block::PATH;
	bool	bottomLeft = game.map[pos.y + move.y - hitBox.y / 2][pos.x + move.x - hitBox.x / 2] != Block::PATH;
	bool	bottomRight = game.map[pos.y + move.y - hitBox.y / 2][pos.x + move.x + hitBox.x / 2] != Block::PATH;

	if ((topLeft && bottomLeft) || (topRight && bottomRight))
	{
		move.x = 0;
	}
	if ((topLeft && topRight) || (bottomLeft && bottomRight))
	{
		move.y = 0;
	}

	topLeft = game.map[pos.y + hitBox.y / 2][pos.x + move.x - hitBox.x / 2] != Block::PATH;
	topRight = game.map[pos.y + hitBox.y / 2][pos.x + move.x + hitBox.x / 2] != Block::PATH;
	bottomLeft = game.map[pos.y - hitBox.y / 2][pos.x + move.x - hitBox.x / 2] != Block::PATH;
	bottomRight = game.map[pos.y - hitBox.y / 2][pos.x + move.x + hitBox.x / 2] != Block::PATH;

	if (topLeft || bottomLeft || topRight || bottomRight)
	{
		move.x = 0;
	}

	topLeft = game.map[pos.y + move.y + hitBox.y / 2][pos.x - hitBox.x / 2] != Block::PATH;
	topRight = game.map[pos.y + move.y + hitBox.y / 2][pos.x + hitBox.x / 2] != Block::PATH;
	bottomLeft = game.map[pos.y + move.y - hitBox.y / 2][pos.x - hitBox.x / 2] != Block::PATH;
	bottomRight = game.map[pos.y + move.y - hitBox.y / 2][pos.x + hitBox.x / 2] != Block::PATH;

	if (topLeft || bottomLeft || topRight || bottomRight)
	{
		move.y = 0;
	}
}

// returns true if longer than maxLen
bool	dotProduct(Vec2 i1, Vec2 i2, float maxLen)
{
	float dx = i1.x - i2.x;
	float dy = i1.y - i2.y;

	return (dx * dx + dy * dy > maxLen * maxLen);
}

bool	DDA(GameInfo& game, Vec2 start, Vec2 end)
{
	Ray			ray{};
	Vec2		fractPos;
	const int	maxIter = 20;

	ray.dir = normaliseVec2(subtractVec2(end, start));
	ray.delta.x = abs(1.0f / ray.dir.x);
	ray.delta.y = abs(1.0f / ray.dir.y);
	ray.step.x = ray.dir.x < 0.0f ? -1 : 1;
	ray.step.y = ray.dir.y < 0.0f ? -1 : 1;
	fractPos = { start.x - (int)start.x, start.y - (int)start.y };
	ray.side.x = (ray.dir.x < 0.0f ? fractPos.x : 1.0f - fractPos.x) * ray.delta.x;
	ray.side.y = (ray.dir.y < 0.0f ? fractPos.y : 1.0f - fractPos.y) * ray.delta.y;
	ray.square_pos.x = start.x;
	ray.square_pos.y = start.y;

	for (int i = 0; i < maxIter; i++)
	{
		if (game.map[ray.square_pos.y][ray.square_pos.x] <= Block::WALL_MAX)
		{
			return false;
		}
		if (ray.square_pos.x == (int)end.x && ray.square_pos.y == (int)end.y)
		{
			return true;
		}
		if (ray.side.x < ray.side.y)
		{
			ray.square_pos.x += ray.step.x;
			ray.side.x += ray.delta.x;
		}
		else
		{
			ray.square_pos.y += ray.step.y;
			ray.side.y += ray.delta.y;
		}
	}
	return false;
}

int		manhattanLine(IVec2 start, IVec2 end)
{
	return abs(start.x - end.x) + abs(start.y - end.y);
}

int	countFPS()
{
	static int		frames;
	static int		prevFrames;
	auto			now = std::chrono::system_clock::now();
	auto			duration = now.time_since_epoch();
	int64_t			currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	static int64_t	prevTime;

	frames++;
	if (currentTime % 1000 < prevTime % 1000)
	{
		prevFrames = frames;
		frames = 0;
	}
	prevTime = currentTime;
	return prevFrames;
}

uint64_t    getTicks()
{
	LARGE_INTEGER    frequency, count;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&count);
	count.QuadPart /= (frequency.QuadPart / 1000);
	return count.QuadPart;
}

void	getTime(GameInfo& game)
{
	auto	now = std::chrono::system_clock::now();
	auto	duration = now.time_since_epoch();

	game.misc.MS = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
