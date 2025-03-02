#include "mineBlasters.hpp"

void	renderCoin(GameInfo& game, Vec2 pos)
{
	Image* image = &game.sprites.gold.lower_bound((game.misc.MS % 1200) / (1200 / 12))->second;

	if (image != nullptr)
		drawImage(pos, game.window, image);
}

void	renderBullets(GameInfo& game)
{
	for (const Bullet& b : game.bullets)
	{
		Vec2	pos = {                                                                    /* / 2 is slightly off on y-axis */
			Screen::size.x / 2 + (b.pos.x - game.player.pos.x) * Block::size.x - Item::size.x / 2,
			Screen::size.y / 2 + (b.pos.y - game.player.pos.y) * Block::size.y - Item::size.y / 2
		};

		if (b.isActive)
			drawImage(pos, game.window, &game.sprites.bullets[-1]);
		else
			drawImage(pos, game.window, &game.sprites.bullets[(game.misc.MS - b.startMS) / 30]);
	}
}

void	renderEntities(GameInfo& game)
{
	for (const Item::Dropped& item : game.items)
	{
		Vec2	pos = {
			Screen::size.x / 2 + (item.pos.x - game.player.pos.x) * Block::size.x - Item::size.x / 2,
			Screen::size.y / 2 + (item.pos.y - game.player.pos.y) * Block::size.y - Item::size.y / 2
		};

		if (pos.x + Item::size.x > 0 && pos.x - Item::size.x < Screen::size.x &&
			pos.y + Item::size.y > 0 && pos.y - Item::size.y < Screen::size.y &&
			DDA(game, { game.player.pos.x, game.player.pos.y + Player::eyesOffset }, item.pos) == true)
		{
			if (item.type == Item::COIN)
			{
				renderCoin(game, pos);
			}
			else
			{
				drawImage(pos, game.window, &game.sprites.items[item.type]);
			}
		}
	}
	for (const DroppedWeapon& w : game.droppedWeapons)
	{
		Vec2	pos = {
			Screen::size.x / 2 + (w.pos.x - game.player.pos.x) * Block::size.x - w.ptr->_sprite.size.x / 2,
			Screen::size.y / 2 + (w.pos.y - game.player.pos.y) * Block::size.y - w.ptr->_sprite.size.y / 2
		};

		if (pos.x + Item::size.x > 0 && pos.x - Item::size.x < Screen::size.x &&
			pos.y + Item::size.y > 0 && pos.y - Item::size.y < Screen::size.y &&
			DDA(game, { game.player.pos.x, game.player.pos.y + Player::eyesOffset }, w.pos) == true)
		{
			drawImage(pos, game.window, &w.ptr->_sprite);
		}
	}
}

void	renderEnemies(GameInfo& game)
{
	for (const Entity& e : game.entities)
	{
		if (e.ptr == nullptr)
		{
			std::cerr << "entity is nullptr\n";
			continue;
		}

		const Vec2	offsets[4] =
		{
			{ e.ptr->hitBox.x * 0.4f,  e.ptr->hitBox.y * 0.4f  },
			{ e.ptr->hitBox.x * 0.4f,  -e.ptr->hitBox.y * 0.4f },
			{ e.ptr->hitBox.x * 0.4f,  e.ptr->hitBox.y * 0.4f  },
			{ -e.ptr->hitBox.x * 0.4f, -e.ptr->hitBox.y * 0.4f }
		};

		for (int i = 0; i < 4; i++)
		{
			if (DDA(game, { game.player.pos.x, game.player.pos.y + Player::eyesOffset }, { addVec2(e.pos, offsets[i]) }) == true)
			{
				Image* sprite;

				if (e.timeOfDeath == 0)
					sprite = &e.ptr->walkingAnimation[(game.misc.MS % 800) / 100];
				else
					sprite = &e.ptr->deathAnimation[((game.misc.MS - e.timeOfDeath) % 1600) / 100];

				Vec2	pos =
				{
					Screen::size.x / 2 + (e.pos.x - game.player.pos.x) * Block::size.x - sprite->size.x / 2,
					Screen::size.y / 2 + (e.pos.y - game.player.pos.y) * Block::size.y - sprite->size.y / 2
				};

				drawImage(pos, game.window, sprite, !(e.facingRight));
				continue;
			}
		}
	}
}

void	renderPlayer(GameInfo& game)
{																			                                                  // v Magic number D:
	Vec2	playerPos = { (Screen::size.x - Player::size.x) / 2, (Screen::size.y - Player::size.y) / 2 - (float)Player::size.y / 8 };
	Image*	currentImage;

	if (game.player.isMoving == true)
		currentImage = &game.sprites.playerRunning[(game.misc.MS % 800) / 100];
	else
		currentImage = &game.sprites.playerStanding[(game.misc.MS % 1600) / 400];
	drawImage(playerPos, game.window, currentImage, !game.player.facingRight);
}

void	renderCrosshair(GameInfo& game)
{
	StaticWeapon*	ptr = game.player.primary.ptr;
	Image*			image;

	if (game.state != GameState::PLAYING)
	{
		image = &game.sprites.crosshairs[-2];
	}
	else if (ptr == nullptr)
	{
		image = &game.sprites.crosshairs[-1];
	}
	else
	{
		image = &game.sprites.crosshairs[ptr->_ammoType];
	}
	drawImage(subtractVec2(game.misc.cursorPos, toVec2(image->size, 0.5f)), game.window, image);
}

void	renderUI(GameInfo& game)
{
	drawString(game.sprites.font, game.window, { 8, game.window.vram.y - 20.0f }, "FPS: " + std::to_string(countFPS()));
	drawString(game.sprites.font, game.window, { 8, game.window.vram.y - 32.0f }, "LOD: " + std::to_string(game.misc.LOD));

	Weapon	w[] = {game.player.primary, game.player.secondary};
	Font&	f = game.sprites.font;
	Font&	fXL = game.sprites.fontLarge;

	for (int i = 0; i <= 1; i++)
	{
		int32_t	posY = 55 - i * 25;

		if (w[i].ptr != nullptr)
		{
			drawImage({24, (float)posY + 8}, game.window, &w[i].ptr->_spriteUI);
		}
	}
	if (w[0].ptr != nullptr || w[1].ptr != nullptr)
	{
		drawString(fXL, game.window, { 9, 58 }, "\36");
		drawString(fXL, game.window, { 5, 43 }, "Q");
		drawString(fXL, game.window, { 9, 28 }, "\37");
	}
	for (int i = 0; i < Ammo::types; i++)
	{
		float		posY = 46 - i * 12;
		std::string	s1 = Ammo::pool[i].name.substr(0, Ammo::pool[i].name.find_first_of(" \0"));
		std::string	s2 = "[" + std::to_string(game.player.reserveAmmo[i]) + "]";

		drawString(f, game.window, { game.window.vram.x - 75.0f, posY }, s1);
		drawString(f, game.window, { game.window.vram.x - 30.0f, posY }, s2);
	}

	drawString(fXL, game.window, { 10, 5 }, (w[0].ptr == nullptr) ? "0/0" : std::to_string(w[0].ammo) + "/" + std::to_string(game.player.reserveAmmo[w[0].ptr->_ammoType]));

	drawString(fXL, game.window, { 8.5, 160 }, "\34");
	drawString(fXL, game.window, { 25,  160 }, std::to_string((int)game.player.health));
	drawString(fXL, game.window, { 9,   185 }, "$");
	drawString(fXL, game.window, { 25,  185 }, std::to_string(game.player.coinCount));
}

void	manageTextPrompts(GameInfo& game)
{
	game.player.topPrompt.CheckAndRender(game);
	game.player.centrePrompt.CheckAndRender(game);
	game.player.bottomPrompt.CheckAndRender(game);
}

void	renderOverlays(GameInfo& game)
{
	for (Overlay& o : game.sprites.overlays)
	{
		o.Render(game);
	}
}

void	renderDeathStat(GameInfo& game, std::string stat, std::string val, int i, float transparency)
{
	float	x[] = { game.window.vram.x / 2.0f - 60.0f, game.window.vram.x / 2.0f + 60.0f };
	float	y = game.window.vram.y - 130.0f - 15.0f * i;

	drawString(game.sprites.font, game.window, { x[0], y }, stat, transparency);
	drawString(game.sprites.font, game.window, { x[1], y }, val, Font::Align::RIGHT, Font::Align::TOP, transparency);
}

std::string	msToString(int64_t MS)
{
	std::string	minutes = std::to_string(MS / 60'000);
	std::string	seconds = std::to_string(MS / 1'000 % 60);

	return (minutes.length() == 1 ? "0" : "") + minutes + ":" + (seconds.length() == 1 ? "0" : "") + seconds;
}

void	renderDeathScreen(GameInfo& game)
{
	Stats&		stats = game.stats;
	float		transparency = std::min<float>(1.0f, (float)(game.misc.MS - stats.timeOfDeath) / game.sprites.overlays[Sprites::DEATH]._maxDuration) * 127.0f;

	drawString(game.sprites.fontLarge, game.window, { game.window.vram.x / 2.0f, game.window.vram.y - 130.0f }, "Stats:", Font::Align::CENTRE, Font::Align::TOP, transparency);
	
	renderDeathStat(game, "Time alive:",      msToString(game.stats.timeOfDeath - game.stats.startTime), 1, transparency);
	renderDeathStat(game, "Coins collected:", std::to_string(stats.coinsCollected),                      2, transparency);
	renderDeathStat(game, "Shots fired:",     std::to_string(stats.shotsFired),                          3, transparency);
	renderDeathStat(game, "Damage dealt:",    std::to_string(stats.damageDealt),                         4, transparency);
	renderDeathStat(game, "Enemies Killed:",  std::to_string(stats.enemiesKilled),                       5, transparency);
}

void	renderCharacterSelect(GameInfo& game)
{
	Font&	f = game.sprites.font;
	Font&	fXL = game.sprites.fontLarge;
	Image&	charButton = game.menus.charButton;
	Vec2	halfScreen = toVec2(game.window.vram, 0.5);
	float	offsetBorder = 5.0f;
	float	offsetY = 20.0f;
	Vec2	posArr[4] = {
		{ halfScreen.x - offsetBorder - charButton.size.x, halfScreen.y - offsetY + offsetBorder },
		{ halfScreen.x + offsetBorder,                     halfScreen.y - offsetY + offsetBorder },
		{ halfScreen.x - offsetBorder - charButton.size.x, halfScreen.y - offsetY - offsetBorder - charButton.size.y },
		{ halfScreen.x + offsetBorder,                     halfScreen.y - offsetY - offsetBorder - charButton.size.y }
	};
	Vec2	namePos = { charButton.size.x / 2, charButton.size.y - 20 };
	Vec2	textPos = { 25, charButton.size.y - 50 };
	float	textOffset = 13;
	
	for (int i = 0; i < 4; i++)
	{
		StaticWeapon*	weapon = game.player.startOptions[i].weapon;
		Vec2	imagePos = addVec2(posArr[i], { (float)charButton.size.x - weapon->_spriteUI.size.x - 30, 30 });

		drawString(fXL, game.window, addVec2(posArr[i], namePos), game.player.startOptions[i].name, Font::Align::CENTRE, Font::Align::CENTRE);
		drawString(f, game.window, addVec2(posArr[i], textPos), "WEAPON:\t " + weapon->_name);
		drawString(f, game.window, addVec2(posArr[i], { textPos.x, textPos.y - textOffset     }), "DAMAGE:\t " + std::to_string((int)weapon->_damage * weapon->_pellets));
		drawString(f, game.window, addVec2(posArr[i], { textPos.x, textPos.y - textOffset * 2 }), "FIRE RATE:\t "    + std::to_string((int)weapon->_fireRate) + " rpm");
		drawString(f, game.window, addVec2(posArr[i], { textPos.x, textPos.y - textOffset * 3 }), "CAPACITY:\t "   + std::to_string((int)weapon->_capacity));
		drawImage(imagePos, game.window, &weapon->_spriteUI);
	}
}

void	renderMenu(GameInfo& game)
{
	if (game.state != GameState::PAUSED)
	{
		drawImage({ 0, 0 }, game.window, &game.menus.background[game.state]);
	}
	for (SimpleButton& b : game.menus.simpleButtons[game.state])
	{
		b.Render(game.window);
	}
	if (game.state == GameState::CHARACTER_SELECT)
	{
		renderCharacterSelect(game);
	}
	if (game.state == GameState::DEAD)
	{
		renderDeathScreen(game);
	}
	renderCrosshair(game);
}

void	render(GameInfo& game)
{
	if (game.state == GameState::PLAYING || game.state == GameState::PAUSED)
	{
		renderGrid(game);
		renderEntities(game);
		renderEnemies(game);
		renderPlayer(game);
		renderBullets(game);
		renderBlockingWalls(game);

		renderCrosshair(game);
		renderUI(game);
		manageTextPrompts(game);
		renderOverlays(game);
	}
	if (game.state != GameState::PLAYING)
	{
		renderMenu(game);
	}
	renderFrame();
}
