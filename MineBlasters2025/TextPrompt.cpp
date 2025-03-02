#include "TextPrompt.hpp"
#include "mineBlasters.hpp"

TextPopUp::TextPopUp()
{
}

TextPopUp::TextPopUp(float riseWithTime, Vec2 pos, Font* font)
{
	if (font == nullptr)
	{
		MessageBoxA(0, "Font is a nullptr", "Error with font", MB_OK);
	}
	_riseWithTime = riseWithTime;
	_pos = pos;
	_font = font;
}

TextPopUp& TextPopUp::operator=(const TextPopUp& cpy)
{
	if (this != &cpy)
	{
		_creation = cpy._creation;
		_text = cpy._text;
		_riseWithTime = cpy._riseWithTime;
		_pos = cpy._pos;
		_font = cpy._font;
	}
	return *this;
}

void	TextPopUp::Add(GameInfo& game, std::string str, int64_t upTime, bool override)
{
	if (override == true || _text.size() == 0)
	{
		_creation = game.misc.MS;
	}
	if (override == true && _text.size() > 0)
	{
		_text[0] = { upTime, str };
	}
	else
	{
		_text.push_back({ upTime, str });
	}
}

void	TextPopUp::CheckAndRender(GameInfo& game)
{
	if (_text.size() > 0 && _creation + _text[0].upTime < game.misc.MS)
	{
		_text.pop_front();
		_creation = game.misc.MS;
	}
	if (_text.size() > 0)
	{
		float	transparancy = 127.0f;
		Vec2	newPos = _pos;

		if (_riseWithTime != 0 && _text[0].upTime != 0)
		{
			transparancy = 127.0f - ((float)(game.misc.MS - _creation) / _text[0].upTime) * 127.0f;
			newPos.y += _riseWithTime * (game.misc.MS - _creation);
		}
		drawString(*_font, game.window, newPos, _text[0].str, Font::Align::CENTRE, Font::Align::TOP, transparancy);
	}
}

TextPopUp::~TextPopUp()
{
}