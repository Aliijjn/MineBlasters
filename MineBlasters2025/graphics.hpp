#pragma once

#include "Vector2.hpp"

#include <map>
#include <list>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <iostream>

#include <time.h>
#include <stdlib.h>
#include <Windows.h>
#include <mmsystem.h>

#include <gl/GL.h>

static constexpr int bufferSize = 128;

struct Pixel
{
    uint8_t		r, g, b, a;
};

struct WindowInfo
{
    IVec2   vram = { 0, 0 };
    IVec2   screen = { 0, 0 };
};

namespace BMP
{
    enum
    {
        FILE_SIZE = 2,
        HEADER_OFFSET = 10,
        IMAGE_X_SIZE = 18,
        IMAGE_Y_SIZE = 22,
        HEADER_MAX_SIZE = 54
    };
}

class Image
{
    public:
        Image();
        Image(std::string path);
        void    Load(std::string path);
        Image&  operator=(const Image& cpy);
        ~Image();
        Pixel*      content = nullptr;
        IVec2       size = {0, 0};
        uint32_t    GLindex = 0;
};

struct Font
{
    enum class Align : uint8_t
    {
        LEFT, CENTRE, RIGHT
    };
    Image   image;
    IVec2   charSize;
    uint8_t	sizes[128];
};

void	initWindow(WindowInfo windowInfo, std::string windowName = "Window", bool vSync = true, bool showCursor = true, std::string iconPath = "");
void    closeWindow();
void    errorBox(std::string title, std::string body);
void    renderFrame();
void    retrieveInput();
bool    getLeftMouseClick();
bool    getRightMouseClick();
bool    keyPress(char key);
bool    keyPress(char key, int64_t coolDownMS);
Vec2    getCursorPos(WindowInfo& windowInfo);

// Images
void    drawImage(Vec2 pos, WindowInfo windowInfo, Image* image, bool flipped = false);
void    drawImage(Vec2 pos, Vec2 tilePos, float size, WindowInfo windowInfo, Image* image, uint8_t colour);
void    drawImage(Vec2 pos, WindowInfo windowInfo, Image* image, float transparency);

// Fonts
Font    loadFont(std::string path);
int     drawString(const Font& font, WindowInfo windowInfo, Vec2 pos, std::string str, float transparency = 127.0f);
int     drawString(const Font& font, WindowInfo windowInfo, Vec2 pos, std::string str, Font::Align alignment, float transparency = 127.0f);
