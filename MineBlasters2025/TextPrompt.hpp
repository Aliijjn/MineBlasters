#pragma once

#include <deque>
#include <string>
#include "Vector2.hpp"

struct Font;
struct GameInfo;

class TextPopUp
{
	struct Text
	{
		int64_t		upTime;
		std::string	str;
	};

	public:
		TextPopUp();
		TextPopUp(float riseWithTime, Vec2 pos, Font* font);
		TextPopUp& operator=(const TextPopUp& cpy);
		void	Add(GameInfo& game, std::string str, int64_t upTime, bool override = false);
		void	CheckAndRender(GameInfo& game);
		~TextPopUp();

	private:
		int64_t				_creation = 0;
		std::deque<Text>	_text;

		// "constants"
		float				_riseWithTime = 0.0f;
		Vec2				_pos = { 0, 0 };
		Font*				_font = nullptr;
};
