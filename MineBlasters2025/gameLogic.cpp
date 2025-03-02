#include "mineBlasters.hpp"

void	checkLOD(GameInfo& game)
{
	static int64_t	previousAction;

	if (game.misc.MS > previousAction + 200 && keyPress(VK_UP) == true && game.misc.LOD < 10)
	{
		game.misc.LOD++;
		previousAction = game.misc.MS;
	}
	else if (game.misc.MS > previousAction + 200 && keyPress(VK_DOWN) == true && game.misc.LOD > 0)
	{
		game.misc.LOD--;
		previousAction = game.misc.MS;
	}
}

void	movePlayer(GameInfo& game)
{
	Vec2	dir{};

	if (keyPress('D') == true)
		dir.x++;
	if (keyPress('A') == true)
		dir.x--;
	if (keyPress('W') == true)
		dir.y++;
	if (keyPress('S') == true)
		dir.y--;
	if (!dir.x && !dir.y)
	{
		game.player.isMoving = false;
		return;
	}
	game.player.isMoving = true;
	if (dir.x)
	{
		game.player.facingRight = dir.x > 0;
	}
	if (dir.x && dir.y)
	{
		// close enough to /= sqrt(2)
		dir.x *= 0.7;
		dir.y *= 0.7;
	}
	dir = multiplyVec2(dir, Player::speed);
	checkColission(game, game.player.pos, dir, Player::wallHitBox);
	game.player.pos = addVec2(game.player.pos, dir);
}

void	spitOutItems(GameInfo& game, Vec2 pos, uint16_t type, uint16_t copies, uint16_t count)
{
	for (uint32_t i = 0; i < copies; i++)
	{
		Vec2			randomDir = normaliseVec2({ (float)tRand(100) - 49.5f, (float)tRand(100) - 49.5f });
		Item::Dropped	item = Item::Dropped
		{
			.pos = { pos.x, pos.y },
			.dir = multiplyVec2(randomDir, Item::speed),
			.MS = game.misc.MS,
			.type = type,
			.count = count
		};
		game.items.push_back(item);
	}
}

void	spitOutWeapon(GameInfo& game, Vec2 pos, bool isFancy, StaticWeapon* ptr)
{
	DroppedWeapon	d{};
	int				randomNbr = game.misc.seed + (int)pos.x + (int)pos.y * 3 + game.misc.level * 5;

	d.pos = pos;
	d.dir = multiplyVec2(normaliseVec2(subtractVec2(game.player.pos, pos)), Item::speed);
	d.ptr = ptr;
	if (ptr == nullptr)
	{
		if (isFancy == true)
			d.ptr = game.weapons.fancy[randomNbr % game.weapons.fancy.size()];
		else
			d.ptr = game.weapons.basic[randomNbr % game.weapons.basic.size()];
	}
	d.ammo = d.ptr->_capacity;

	game.droppedWeapons.push_back(d);
}

void	openChest(GameInfo& game, Vec2 itemPos, uint16_t& block)
{
	int randomNbr = game.misc.seed + (int)itemPos.x + (int)itemPos.y * 3 + game.misc.level * 5;

	if (block == Block::CHEST1_CLOSED)
	{
		spitOutItems(game, itemPos, Item::COIN, 10);
		spitOutWeapon(game, itemPos, true);
	}
	if (block == Block::CHEST2_CLOSED)
	{
		spitOutItems(game, itemPos, Item::COIN, 3);
		spitOutWeapon(game, itemPos, false);
	}
	if (block == Block::CHEST3_CLOSED)
	{
		int	i = randomNbr % Ammo::types;
		int	j = (i + 1) % Ammo::types;

		spitOutItems(game, itemPos, Ammo::pool[i].type, 1, Ammo::pool[i].dropCount);
		spitOutItems(game, itemPos, Ammo::pool[j].type, 1, Ammo::pool[j].dropCount);
		// spitOutWeapon(game, itemPos, false);
	}
	block++; // to CHEST(n)_OPEN
}

void	manageInteractables(GameInfo& game, Vec2 newPos, bool pressedE)
{
	uint16_t& block = game.map[newPos.y][newPos.x];

	if (block == Block::CHEST1_CLOSED ||
		block == Block::CHEST2_CLOSED ||
		block == Block::CHEST3_CLOSED)
	{
		game.player.bottomPrompt.Add(game, "Press [E] to open chest", 1, true);
		if (pressedE == true)
		{
			Vec2	itemPos = { std::floor(newPos.x) + 0.5f, std::floor(newPos.y) + 0.5f };

			openChest(game, itemPos, block);
		}
	}
	if (block == Block::EXIT)
	{
		game.player.bottomPrompt.Add(game, "Press [E] to descent", 1, true);
		if (pressedE == true)
		{
			game.player.isFrozen = true;
			game.misc.nextLevel = game.misc.MS + game.sprites.overlays[Sprites::OUTRO]._maxDuration;
			game.sprites.overlays[Sprites::OUTRO].StartNow(game);
			return;
		}
	}
	if (block == Block::SHOP)
	{
		auto it = game.shops.find(toIVec2(newPos));

		if (it == game.shops.end())
		{
			game.player.bottomPrompt.Add(game, "This shop is out of stock", 1, true);
		}
		else
		{
			int	cost = it->second->_price;

			game.player.bottomPrompt.Add(game, "Press [E] to buy \"" + it->second->_name + "\" for " + std::to_string(cost) + "$", 1, true);
			if (pressedE && game.player.coinCount >= cost)
			{
				spitOutWeapon(game, newPos, true, it->second);
				game.shops.erase(it);
				game.player.coinCount -= cost;
			}
			else if (pressedE)
			{
				game.player.centrePrompt.Add(game, "not enough $$$", 2000, true);
			}
		}
	}
}

void	checkInteractables(GameInfo& game)
{
	bool	pressedE = keyPress('E');

	for (int32_t y = -2; y <= 2; y++)
	{
		for (int32_t x = -2; x <= 2; x++)
		{
			Vec2		newPos = { game.player.pos.x + x + 0.5f, game.player.pos.y + y + 0.5f };
			
			if (isOutOfBounds(
				{ (int32_t)newPos.x, (int32_t)newPos.y }, 
				{ (int32_t)game.map[0].size(), (int32_t)game.map.size() }) == false &&
				dotProduct(game.player.pos, newPos, Player::chestRange) == false)
			{
				manageInteractables(game, newPos, pressedE);
			}
		}
	}
}

void	checkNextLevel(GameInfo& game)
{
	if (game.misc.nextLevel != 0 && game.misc.nextLevel < game.misc.MS)
	{
		nextLevel(game);
		game.misc.nextLevel = 0;
	}
}

void	gameLogic(GameInfo& game)
{
	getTime(game);
	if (keyPress(VK_ESCAPE, 300) == true)
	{
		if (game.state == GameState::PLAYING)
			game.state = GameState::PAUSED;
		else if (game.state == GameState::PAUSED)
			game.state = GameState::PLAYING;
	}
	if (game.state == GameState::PLAYING)
	{
		checkNextLevel(game);
		if (game.player.isFrozen == false)
		{
			movePlayer(game);
			manageWeapons(game);
			checkInteractables(game);
		}
		manageEntities(game);
		checkLOD(game);
	}
	else
	{
		for (SimpleButton& b : game.menus.simpleButtons[game.state])
		{
			b.CheckClick(game.misc.cursorPos);
		}
	}
	//std::cout << (int)game.state << "\n";
}