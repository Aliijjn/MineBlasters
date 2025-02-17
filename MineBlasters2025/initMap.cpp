#include "mineBlasters.hpp"

static bool	isValidGrid(const std::vector<std::vector<uint8_t>>& chunks,
			std::vector<std::vector<bool>>& visited, IVec2 pos, IVec2 end, uint32_t size)
{
	if (isOutOfBounds(pos, size) || chunks[pos.y][pos.x] == 1 || visited[pos.y][pos.x] == true)
	{
		return false;
	}
	visited[pos.y][pos.x] = true;
	if (pos.x == end.x && pos.y == end.y)
		return true;
	if (isValidGrid(chunks, visited, { pos.x - 1, pos.y }, end, size) == true)
		return true;
	if (isValidGrid(chunks, visited, { pos.x + 1, pos.y }, end, size) == true)
		return true;
	if (isValidGrid(chunks, visited, { pos.x, pos.y - 1 }, end, size) == true)
		return true;
	if (isValidGrid(chunks, visited, { pos.x, pos.y + 1 }, end, size) == true)
		return true;
	return false;
}

static bool	isValidGrid(const std::vector<std::vector<uint8_t>>& chunks, uint32_t size)
{
	for (int32_t y = 0; y < size; y++)
	{
		for (int32_t x = 0; x < size; x++)
		{
			if (chunks[y][x] == 1)
			{
				continue;
			}
			std::vector<std::vector<bool>> visited(size, std::vector<bool>(size));

			if (isValidGrid(chunks, visited, { x, y }, { (int32_t)size - 1, (int32_t)size - 1 }, size) == false)
			{
				return false;
			}
		}
	}
	return true;
}

static void	initGrids(std::vector<std::vector<uint8_t>>& chunks, uint32_t size)
{
	const uint32_t	wallcount = ((float)(size * size)) * Chunk::wallOdds;

	do
	{
		for (auto& it : chunks)
		{
			for (uint8_t& i : it)
			{
				i = 0;
			}
		}
		for (uint32_t i = 0; i < wallcount; i++)
		{
			while (1)
			{
				IVec2	wall = { tRand(size), tRand(size) };

				if (((wall.x == 0 && wall.y == 0) ||
					(wall.x == size - 1 && wall.y == size - 1) ||
					chunks[wall.y][wall.x] == 1) == false)
				{
					chunks[wall.y][wall.x] = 1;
					break;
				}
			}
		}
	} while (isValidGrid(chunks, size) == false);
}

static void	initConnections(std::vector<std::vector<uint8_t>>& chunks, uint32_t size)
{
	std::vector<std::vector<uint8_t>> connections(size, std::vector<uint8_t>(size));

	for (int32_t y = 0; y < size; y++)
	{
		for (int32_t x = 0; x < size; x++)
		{
			if (chunks[y][x] != 0)
				continue;
			if (isOutOfBounds({ x + 1, y }, size) == false && chunks[y][x + 1] == 0)
				connections[y][x] += 0b1;
			if (isOutOfBounds({ x, y - 1 }, size) == false && chunks[y - 1][x] == 0)
				connections[y][x] += 0b10;
			if (isOutOfBounds({ x - 1, y }, size) == false && chunks[y][x - 1] == 0)
				connections[y][x] += 0b100;
			if (isOutOfBounds({ x, y + 1 }, size) == false && chunks[y + 1][x] == 0)
				connections[y][x] += 0b1000;
		}
	}
	chunks = connections;
}

template <typename T> 
static void	printMap(std::vector<std::vector<T>>& chunks, uint32_t size, bool detailed = true)
{
	return;
	for (auto it : chunks)
	{
		for (T i : it)
		{
			if (detailed == true)
			{
				std::cout << (int)i << " ";
				if ((int)i < 10)
					std::cout << "  ";
				else if ((int)i < 100)
					std::cout << " ";
			}
			else
				std::cout << (i >= Block::PATH ? "." : "X") << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

uint8_t	checkNeighbors(const std::vector<std::vector<uint16_t>>& map, IVec2 pos, uint32_t size)
{
	uint8_t	result = 0;

	for (uint8_t i = 0; i < Neighbor::MAX; i++)
	{
		IVec2	newPos = { pos.x + Neighbor::check[i].x, pos.y + Neighbor::check[i].y };

		if (isOutOfBounds(newPos, size) == false && map[newPos.y][newPos.x] >= (1 << Neighbor::MAX))
		{
			result += 1 << i;
		}
	}
	return result;
}

static std::vector<std::vector<uint16_t>>	readChunk(std::ifstream& infile)
{
	std::vector<std::vector<uint16_t>>	chunk(Chunk::size, std::vector<uint16_t>(Chunk::size));
	std::string							line;
	const char*							validChars = "0.cCzESs";

	for (uint32_t y = 0; y < Chunk::size; y++)
	{
		std::getline(infile, line);
		if ((uint32_t)line.length() != Chunk::size)
		{
			throw std::exception(("Size of line is " + std::to_string(y + 1) + ". Should be " + std::to_string(Chunk::size)).c_str());
		}
		size_t index = line.find_first_not_of(validChars);
		if (index != line.npos)
		{
			std::string	chr(line[index], 1);
			throw std::exception(("Invalid character \"" + chr + "\" @ line " + std::to_string(y + 1) + " column " + std::to_string(index + 1)).c_str());
		}
		for (uint32_t x = 0; x < Chunk::size; x++)
		{
			if (line[x] == '0')
				chunk[y][x] = Block::EMPTY;
			else if (line[x] == '.')
				chunk[y][x] = Block::PATH;
			else if (line[x] == 'c')
				chunk[y][x] = Block::CHEST3_CLOSED;
			else if (line[x] == 'C')
				chunk[y][x] = tRand(10) < 7 ? Block::CHEST2_CLOSED : Block::CHEST1_CLOSED;
			else if (line[x] == 'z')
				chunk[y][x] = Block::ZOMBIE_SPAWN;
			else if (line[x] == 'E')
				chunk[y][x] = Block::EXIT;
			else if (line[x] == 'S')
				chunk[y][x] = Block::SHOP;
			else if (line[x] == 's')
				chunk[y][x] = Block::SHOP_DESK;
			else
				chunk[y][x] = Block::UNKNOWN;
		}
	}
	// check for wall orientation
	printMap(chunk, Chunk::size, true);
	for (int32_t y = 0; y < Chunk::size; y++)
	{
		for (int32_t x = 0; x < Chunk::size; x++)
		{
			if (chunk[y][x] == 0)
			{
				chunk[y][x] = checkNeighbors(chunk, { x, y }, Chunk::size);
			}
		}
	}
	printMap(chunk, Chunk::size, true);
	return chunk;
}

static void	getChunkTable(std::map<uint32_t, std::vector<std::vector<uint16_t>>>& chunkTable, std::string folder, uint32_t size, uint32_t count)
{
	std::string		fileName;
	std::ifstream	infile;

	for (int i = 0; i < count; i++)
	{
		fileName = folder + "/" + std::to_string(i) + ".txt";
		try
		{
			infile.open(fileName);
			if (infile.is_open() == false)
			{
				throw std::exception("Couldn't open file");
			}
			chunkTable.emplace(i, readChunk(infile));
			infile.close();
		}
		catch (std::exception e)
		{
			MessageBoxA(0, (fileName + ": " + e.what()).c_str(), "Error opening file", MB_OK);
			exit(1);
		}
	}
	//for (auto it : chunkTable)
	//{
	//	std::cout << "map " << it.first << ":\n";
	//	printMap(it.second, size, true);
	//}
}

static void	createFullMap(const std::vector<std::vector<uint8_t>>& chunks,
						  const std::map<uint32_t, std::vector<std::vector<uint16_t>>>& chunkTable,
						  std::vector<std::vector<uint16_t>>& map, uint32_t size)
{
	for (uint32_t macroY = 0; macroY < size; macroY++)
	{
		for (uint32_t macroX = 0; macroX < size; macroX++)
		{
			const std::vector<std::vector<uint16_t>>	currentGrid = chunkTable.find(chunks[macroY][macroX])->second;

			for (uint32_t y = 0; y < Chunk::size; y++)
			{
				for (uint32_t x = 0; x < Chunk::size; x++)
				{
					map[macroY * Chunk::size + y][macroX * Chunk::size + x] = currentGrid[y][x];
				}
			}
		}
	}
}

void	checkFullMap(GameInfo& game, std::vector<std::vector<uint16_t>>& map, uint32_t size)
{
	for (int32_t y = 0; y < map.size(); y++)
	{
		for (int32_t x = 0; x < map[y].size(); x++)
		{
			if (map[y][x] == Block::ZOMBIE_SPAWN)
			{
				map[y][x] = Block::PATH;
				if (x >= Chunk::size || y >= Chunk::size)
				{
					game.entities.push_back({ false, game.staticEntities[StaticEntity::ZOMBIE].maxHealth, 0, 0, { x + 0.5f, y + 0.5f }, { 0, 0 }, &game.staticEntities[StaticEntity::ZOMBIE] });
				}
			}
			if (map[y][x] == Block::EXIT && (x < (size - 1) * Chunk::size || y < (size - 1) * Chunk::size))
			{
				map[y][x] = Block::PATH;
			}
			if (map[y][x] == Block::SHOP)
			{
				game.shops.emplace(IVec2{ x, y }, game.weapons.fancy[tRand(game.weapons.fancy.size())]);
			}
		}
	}
}

std::vector<std::vector<uint16_t>>	initMap(GameInfo& game, uint32_t lvl)
{
	const uint32_t											size = game.misc.isInShop ? 1 : lvl + 1;
	std::vector<std::vector<uint8_t>>						chunks(size, std::vector<uint8_t>(size));
	std::map<uint32_t, std::vector<std::vector<uint16_t>>>	chunkTable;
	std::vector<std::vector<uint16_t>>						map(size * Chunk::size, std::vector<uint16_t>(size * Chunk::size));

	initGrids(chunks, size);
	initConnections(chunks, size);
	getChunkTable(chunkTable, game.misc.isInShop ? "chunks/shop" : "chunks", size, game.misc.isInShop ? 1 : 16);
	createFullMap(chunks, chunkTable, map, size);
	checkFullMap(game, map, size);
	return map;
}