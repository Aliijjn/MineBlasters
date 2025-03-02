#include "Button.hpp"

bool	isInBox(Vec2 cursorPos, Vec2 pos, Vec2 size)
{
	return (cursorPos.x >= pos.x && cursorPos.x <= pos.x + size.x &&
		cursorPos.y >= pos.y && cursorPos.y <= pos.y + size.y);
}

SimpleButton::SimpleButton(Vec2 pos, std::string text, Image& image, Font& font, std::function<void()> func)
{
	_pos = pos;
	_text = text;
	_image = &image;
	_textPos = toVec2(_image->size, 0.5f);
	_font = &font;
	_callback = func;
}

void	SimpleButton::CheckClick(Vec2 cursorPos)
{
	if (isInBox(cursorPos, _pos, toVec2(_image->size)) == true && getLeftMouseClick(200) == true)
	{
		_callback();
	}
}

void	SimpleButton::Render(WindowInfo window)
{
	if (_image != nullptr)
		drawImage(_pos, window, _image, 255.0f);
	if (_font != nullptr && _text.empty() == false)
		drawString(*_font, window, addVec2(_pos, _textPos), _text, Font::Align::CENTRE, Font::Align::CENTRE);
}

SimpleButton::~SimpleButton() {}
