#include "mineBlasters.hpp"

void	loop(GameInfo& game)
{
	uint64_t	ticks = getTicks();
	uint64_t	currentTicks;

	while (1)
	{
		retrieveInput();
		game.misc.cursorPos = getCursorPos(game.window);
		for (currentTicks = getTicks(); ticks < currentTicks; ticks++)
		{
			gameLogic(game);
		}
		render(game);
	}
}

void	cleanUp(GameInfo& game)
{
	closeWindow();
}

int main()
{
	GameInfo	game{};

	staticInit(game);
	loop(game);
	cleanUp(game);
}
