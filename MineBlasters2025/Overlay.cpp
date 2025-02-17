#include "Overlay.hpp"
#include "mineBlasters.hpp"

Overlay::Overlay() {}

Overlay::Overlay(Fade fadeType, int64_t maxLen, std::string imagePath)
{
	this->Load(fadeType, maxLen, imagePath);
}

void		Overlay::Load(Fade fadeType, int64_t maxDuration, std::string imagePath)
{
	_fadeType = fadeType;
	_start = 0;
	_maxDuration = maxDuration;
	_image.Load(imagePath);
}

void		Overlay::StartNow(const GameInfo& game)
{
	_start = game.misc.MS;
}

void		Overlay::Stop()
{
	_start = 0;
}

void		Overlay::Render(const GameInfo& game)
{
	if (_start + _maxDuration > game.misc.MS || (_fadeType == Fade::PERSISTS && _start != 0))
	{
		double	currentDuration = game.misc.MS - _start;
		double	animationStrength = 0.0;

		// doubles between 0.0 and 1.0 (1.0 -> fully transparent)
		switch (_fadeType)
		{
			case Fade::INCREMENTS:
				animationStrength = std::min<double>(1.0, currentDuration / _maxDuration);
				break;
			case Fade::PERSISTS:
			case Fade::DECREMENTS:
				animationStrength = 1.0 - std::min<double>(1.0, currentDuration / _maxDuration);
				break;
			case Fade::PEAKS:
				animationStrength = 1.0 - std::min<double>(1.0, std::abs((double)_maxDuration / 2 - currentDuration) / ((double)_maxDuration / 2));
				break;
		}
		drawImage({ 0, 0 }, game.window, &_image, (float)animationStrength * 127.0f);
	}
}

Overlay& Overlay::operator=(const Overlay& cpy)
{
	if (this != &cpy)
	{
		_fadeType = cpy._fadeType;
		_start = cpy._start;
		_maxDuration = cpy._maxDuration;
		_image = cpy._image;
	}
	return *this;
}

Overlay::~Overlay() {}