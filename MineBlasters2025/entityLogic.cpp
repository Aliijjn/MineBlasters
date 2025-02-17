#include "mineBlasters.hpp"

void	manageEntity(GameInfo& game, Vec2& pos, Vec2& dir)
{
	uint16_t	block = game.map[pos.y + dir.y][pos.x + dir.x];

	if (block > Block::NO_COLLISION)
	{
		pos.x += dir.x;
		pos.y += dir.y;
	}
	else
	{
		block = game.map[pos.y + dir.y][pos.x];

		if (block > Block::NO_COLLISION)
		{
			dir.x = -dir.x;
		}
		else
		{
			dir.y = -dir.y;
		}
	}
	dir.x *= Item::friction;
	dir.y *= Item::friction;
}

void	pickupWeapon(GameInfo& game, int i)
{
	DroppedWeapon w{};

	if (game.player.primary.ptr != nullptr && game.player.secondary.ptr != nullptr)
	{
		// add as primary
		Vec2	cursorPos = multiplyVec2(normaliseVec2(subtractVec2(game.misc.cursorPos, toVec2(game.window.vram, 0.5))), Player::speed);

		w = { game.player.pos, cursorPos, game.player.primary.ammo, game.player.primary.ptr};
		std::swap(w, game.droppedWeapons[i]);
	}
	else if (game.player.primary.ptr != nullptr && game.player.secondary.ptr == nullptr)
	{
		// add as primary, make prev primary, secondary
		w = game.droppedWeapons[i];
		std::swap(game.player.primary, game.player.secondary);
		game.droppedWeapons.erase(game.droppedWeapons.begin() + i);
	}
	else
	{
		// swap with primary
		w = game.droppedWeapons[i];
		game.droppedWeapons.erase(game.droppedWeapons.begin() + i);
	}
	game.player.primary = { NO_RELOAD, 0, 0, w.ammo, w.ptr };
}

bool	bulletWallCollision(GameInfo& game, Bullet& b)
{
	uint16_t	blockHit = game.map[b.pos.y][b.pos.x];

	if (blockHit & 1 << Neighbor::BOTTOM)
		return game.map[b.pos.y - 0.28f][b.pos.x] <= Block::WALL_MAX;
	if (blockHit & 1 << Neighbor::TOP)
		return game.map[b.pos.y + 0.28f][b.pos.x] <= Block::WALL_MAX;
	return blockHit <= Block::WALL_MAX;
}

void	manageBullets(GameInfo& game)
{
	for (int i = 0; i < game.bullets.size(); i++)
	{
		Bullet& b = game.bullets[i];

		if (b.isActive)
		{
			b.pos = addVec2(b.pos, b.dir);
		}
		if (b.isActive == true && (bulletWallCollision(game, b) == true || b.startMS + 3000 <= game.misc.MS))
		{
			// disable boolet
			b.isActive = false;
			b.startMS = game.misc.MS;
		}
		if (b.isActive == false && b.startMS + 150 <= game.misc.MS)
		{
			game.bullets.erase(game.bullets.begin() + i);
			i--;
		}
	}
}

void	manageMiscEntities(GameInfo& game)
{
	for (int i = 0; i < game.droppedWeapons.size(); i++)
	{
		DroppedWeapon& d = game.droppedWeapons[i];

		manageEntity(game, d.pos, d.dir);
		if (dotProduct(d.pos, game.player.pos, 1.5f) == false)
		{
			game.player.bottomPrompt.Add(game, "Press [F] for \"" + d.ptr->name + "\"", 1, true);
			if (keyPress('F', 300) == true)
			{
				pickupWeapon(game, i);
			}
		}
	}
	for (int i = 0; i < game.items.size(); )
	{
		Item::Dropped& d = game.items[i];

		manageEntity(game, d.pos, d.dir);
		if (d.MS + 500 < game.misc.MS && dotProduct(d.pos, game.player.pos, 2.0f) == false)
		{
			if (dotProduct(d.pos, game.player.pos, 0.1f) == false)
			{
				if (d.type == Item::COIN)
				{
					game.player.coinCount++;
					game.stats.coinsCollected++;
				}
				else if (d.type >= Item::LIGHT_AMMO && d.type <= Item::SHOTGUN_AMMO)
				{
					game.player.centrePrompt.Add(game, "+" + std::to_string(d.count) + " " + Ammo::pool[d.type - Ammo::LIGHT_AMMO].name, 2000);
					game.player.reserveAmmo[d.type - Item::LIGHT_AMMO] += d.count;
				}
				game.items.erase(game.items.begin() + i);
				continue;
			}
			else
			{
				d.dir = multiplyVec2(normaliseVec2(subtractVec2(game.player.pos, d.pos)), Item::speed / 2);
			}
		}
		i++;
	}
}

void	manageHit(GameInfo& game, Entity& entity, Bullet& bullet)
{
	entity.health -= bullet.damage;
	game.stats.damageDealt += bullet.damage;
	bullet.isActive = false;
	bullet.startMS = game.misc.MS;
	if (entity.health <= 0)
	{
		uint32_t	r = tRand(100);

		if (r % 33 == 1)
		{
			spitOutWeapon(game, entity.pos);
		}
		for (DropPool d : entity.ptr->dropPool)
		{
			if (r < d.odds)
			{
				spitOutItems(game, entity.pos, d.type, d.copies, d.stackSize);
				break;
			}
		}
		entity.timeOfDeath = game.misc.MS;
		game.stats.enemiesKilled++;
	}
}

void	manageEnemies(GameInfo& game)
{
	for (int e = 0; e < game.entities.size(); e++)
	{
		Entity& entity = game.entities[e];
		const Vec2	offsets[4] =
		{
			{ entity.ptr->hitBox.x *  0.5f, entity.ptr->hitBox.y *  0.5f },
			{ entity.ptr->hitBox.x *  0.5f, -entity.ptr->hitBox.y * 0.5f },
			{ entity.ptr->hitBox.x *  0.5f, entity.ptr->hitBox.y *  0.5f },
			{ -entity.ptr->hitBox.x * 0.5f, -entity.ptr->hitBox.y * 0.5f }
		};

		if (entity.timeOfDeath != 0)
		{
			if (entity.timeOfDeath + entity.ptr->deathTimeOut < game.misc.MS)
			{
				game.entities.erase(game.entities.begin() + e);
				e--;
			}
			continue;
		}
		for (int b = 0; b < game.bullets.size(); b++)
		{
			Bullet& bullet = game.bullets[b];

			if (bullet.isActive == true && AABBMiddle(entity.pos, entity.ptr->hitBox, bullet.pos, { 0.125f, 0.125f }) == true)
			{
				manageHit(game, entity, bullet);
			}
		}
		if (game.player.isAlive == true && entity.lastAttack + (int64_t)(entity.ptr->attackCooldown * 1000) < game.misc.MS && dotProduct(entity.pos, game.player.pos, entity.ptr->range) == false)
		{
			entity.lastAttack = game.misc.MS;
			game.player.health -= entity.ptr->damage;
			if (game.player.health <= 0)
			{
				game.player.isAlive = false;
				game.player.isFrozen = true;
				game.sprites.overlays[Sprites::DEATH].StartNow(game);
				game.stats.timeOfDeath = game.misc.MS;
			}
			else
			{
				game.sprites.overlays[Sprites::BLOOD].StartNow(game);
			}
		}
		for (int i = 0; i < 4; i++)
		{
			if (DDA(game, addVec2(entity.pos, offsets[i]), game.player.pos) == true)
			{
				entity.dir = multiplyVec2(normaliseVec2Safe(subtractVec2(game.player.pos, entity.pos)), entity.ptr->speed);
			}
		}
		if (entity.dir.x != 0 && entity.dir.y != 0)
		{
			Vec2	tempDir = entity.dir;

			checkColission(game, entity.pos, tempDir, entity.ptr->wallHitBox);
			tempDir = multiplyVec2(normaliseVec2Safe(tempDir), entity.ptr->speed);
			checkColission(game, entity.pos, tempDir, entity.ptr->wallHitBox);
			entity.pos = addVec2(entity.pos, tempDir);
			entity.facingRight = entity.dir.x > 0;
		}
	}
}

void	manageEntities(GameInfo& game)
{
	manageBullets(game);
	manageEnemies(game);
	manageMiscEntities(game);
}