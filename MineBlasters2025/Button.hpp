#pragma once

#include <cstdint>
#include <functional>

#include "Vector2.hpp"
#include "graphics.hpp"

class SimpleButton
{
	public:
		SimpleButton(Vec2 pos, std::string text, Image& image, Font& font, std::function<void()> func);
		void	CheckClick(Vec2 cursorPos);
		void	Render(WindowInfo window);
		~SimpleButton();

	private:
		Vec2					_pos, _textPos;
		std::string				_text;
		Image*					_image;
		Font*					_font;
		std::function<void()>	_callback;
};
