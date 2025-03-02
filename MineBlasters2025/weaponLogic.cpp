#include "mineBlasters.hpp"

static void	manageShoot(GameInfo& game)
{
	static int64_t	fireTimeOut;

	if (game.player.primary.ptr == nullptr ||
		game.player.primary.ammo == 0 ||
		fireTimeOut > game.misc.MS ||
		game.player.primary.isReloading != NO_RELOAD)
	{
		return;
	}

	Weapon& weapon = game.player.primary;
	Vec2	pos = game.misc.cursorPos;
	Vec2	relativePos = normaliseVec2({ pos.x - game.window.vram.x / 2, pos.y - game.window.vram.y / 2 });

	for (int i = 0; i < weapon.ptr->_pellets; i++)
	{
		float	offset = ((float)tRand(weapon.ptr->_accuracy * 200) / 100 - weapon.ptr->_accuracy) / 100;
		Vec2	randomPos = normaliseVec2({ relativePos.x + offset, relativePos.y + offset });

		game.bullets.push_back(Bullet{ true, weapon.ptr->_damage, game.player.pos, multiplyVec2(randomPos, Player::speed * weapon.ptr->_velocity), game.misc.MS});
	}
	weapon.ammo--;
	fireTimeOut = game.misc.MS + 1000.0 / (weapon.ptr->_fireRate / 60.0);
	game.stats.shotsFired++;
}

static void	manageWeaponSwap(GameInfo& game)
{
	Weapon& w1 = game.player.primary;
	Weapon&	w2 = game.player.secondary;

	std::swap(w1, w2);
	if (w1.isReloading == true)
		w1.isReloading = false;
}

static void	startReload(GameInfo& game, Weapon& w1, uint16_t& reserveAmmo)
{
	if ((w1.ammo == 0 && w1.ptr->_reloadType == SPEED_LOADER) || w1.ptr->_reloadType == MAGAZINE)
	{
		w1.isReloading = FULL_RELOAD;
		w1.reloadEnd = game.misc.MS + w1.ptr->_baseReload + w1.ptr->_fullReload;
	}
	else
	{
		w1.isReloading = PARTIAL_RELOAD;
		w1.reloadEnd = game.misc.MS + w1.ptr->_baseReload + w1.ptr->_singleReload;
	}
	w1.reloadStart = game.misc.MS;
}

static void	endReload(GameInfo& game, Weapon& w1, uint16_t& reserveAmmo)
{
	if (w1.isReloading == FULL_RELOAD)
	{
		if (w1.ammo + reserveAmmo < w1.ptr->_capacity)
		{
			w1.ammo += reserveAmmo;
			reserveAmmo = 0;
		}
		else
		{
			reserveAmmo -= w1.ptr->_capacity - w1.ammo;
			w1.ammo = w1.ptr->_capacity;
		}
		w1.isReloading = NO_RELOAD;
	}
	else
	{
		reserveAmmo--;
		w1.ammo++;
		if (reserveAmmo > 0 && w1.ammo < w1.ptr->_capacity)
		{
			w1.reloadEnd = game.misc.MS + w1.ptr->_singleReload;
			w1.reloadStart = game.misc.MS;
		}
		else
		{
			w1.isReloading = NO_RELOAD;
		}
	}
}

static void	manageReload(GameInfo& game)
{
	Weapon& w1 = game.player.primary;

	if (w1.ptr == nullptr)
	{
		return;
	}
	uint16_t& reserveAmmo = game.player.reserveAmmo[w1.ptr->_ammoType];

	if (w1.ammo == w1.ptr->_capacity || reserveAmmo == 0)
	{
		return;
	}
	if (w1.isReloading != NO_RELOAD)
	{
		int64_t	timeLeftMS = w1.reloadEnd - game.misc.MS;

		game.player.bottomPrompt.Add(game, "reloading " + std::to_string(timeLeftMS / 1000) + "," + std::to_string(timeLeftMS / 100 % 10), 1, true);
	}
	if (w1.isReloading == NO_RELOAD && reserveAmmo != 0 && (w1.ammo == 0 || keyPress('R', 10) == true))
	{
		startReload(game, w1, reserveAmmo);
	}
	else if (w1.isReloading != NO_RELOAD && w1.reloadEnd < game.misc.MS)
	{
		endReload(game, w1, reserveAmmo);
	}
}

void	manageWeapons(GameInfo& game)
{
	manageReload(game);
	if ((keyPress(VK_SPACE) == true || getLeftMouseClick() == true) && game.misc.levelStart + 500 < game.misc.MS)
		manageShoot(game);
	if (keyPress('Q', 500) == true)
		manageWeaponSwap(game);
}
