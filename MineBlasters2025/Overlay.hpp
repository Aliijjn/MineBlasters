#pragma once

#include <string>
#include <cstdint>
#include "graphics.hpp"

struct GameInfo;

enum class Fade : uint8_t
{
	INCREMENTS,
	DECREMENTS,
	PEAKS,
	PERSISTS
};

class Overlay
{
	public:
		Overlay();
		Overlay(Fade fadeType, int64_t maxLen, std::string imagePath);
		void		Load(Fade fadeType, int64_t maxDuration, std::string imagePath);
		void		StartNow(const GameInfo& game);
		void		Stop();
		void		Render(const GameInfo& game);
		Overlay&	operator=(const Overlay& cpy);
		~Overlay();

		Fade	_fadeType = Fade::INCREMENTS;
		int64_t	_start = 0, _maxDuration = 0;
		Image	_image;
};