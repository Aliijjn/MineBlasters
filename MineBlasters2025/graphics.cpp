#include "graphics.hpp"

LRESULT CALLBACK windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
void(_stdcall* glWindowPos2f)(float x, float y);
void(_stdcall* glGenerateMipmap)(unsigned target);
unsigned(_stdcall* wglSwapIntervalEXT)(unsigned status);

// graphic
BITMAPINFO  bmi;
HWND        window;
HDC         WindowDC;
WNDCLASSA   wc;

// openGL
PIXELFORMATDESCRIPTOR pfd;

// my stuff
Pixel* vram;
bool    lMouse, rMouse;

LRESULT CALLBACK    windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        lMouse = true;
        break;
    case WM_LBUTTONUP:
        lMouse = false;
        break;
    case WM_RBUTTONDOWN:
        rMouse = true;
        break;
    case WM_RBUTTONUP:
        rMouse = false;
        break;
    case WM_QUIT:
    case WM_CLOSE:
        ExitProcess(0);
    }
    return DefWindowProcA(window, msg, wParam, lParam);
}

void    retrieveInput()
{
    MSG     msg;

    while (PeekMessageA(&msg, window, 0, 0, 0))
    {
        GetMessageA(&msg, window, 0, 0);
        DispatchMessageA(&msg);
    }
}

int64_t	getTime()
{
    auto	now = std::chrono::system_clock::now();
    auto	duration = now.time_since_epoch();

    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

bool    getLeftMouseClick(int64_t coolDown)
{
    static int64_t  lastClick;
    int64_t         now = getTime();
    
    if (lMouse == false)
        return false;
    if (lastClick + coolDown > now)
        return false;
    lastClick = now;
    return true;
}

bool    getRightMouseClick()
{
    return rMouse;
}

bool    keyPress(char key)
{
    return GetKeyState(key) & 0x80;
}

bool    keyPress(char key, int64_t coolDownMS)
{
    static std::map<char, int64_t>  timeStamps;

    auto    keyCoolDown = timeStamps.find(key);
    int64_t time = getTime();

    if (keyPress(key) == true && (keyCoolDown == timeStamps.end() || keyCoolDown->second <= time))
    {
        timeStamps.insert_or_assign(key, time + coolDownMS);
        return true;
    }
    return false;
}

Vec2     getCursorPos(WindowInfo& windowInfo)
{
    POINT   cursor;
    RECT    client;  
    Vec2    result;

    GetCursorPos(&cursor);
    ScreenToClient(window, &cursor);
    GetClientRect(window, &client);
    cursor.y = client.bottom - cursor.y;
    result.x = (float)(cursor.x) * windowInfo.vram.x / client.right;
    result.y = (float)(cursor.y) * windowInfo.vram.y / client.bottom;
    return result;
}

static float   posToOpenGL(float pos, int max)
{
    return pos * 2 / max - 1;
}

// ------------------------ vvv window management vvv -------------------------

void	initWindow(WindowInfo windowInfo, std::string windowName, bool vSync, bool showCursor, std::string iconPath)
{
    // windows
    WNDCLASSA wc{};

    wc.lpfnWndProc = windowMessageHandler;                                                                      // Pointer to the window procedure
    wc.hInstance = GetModuleHandle(NULL);                                                                       // Handle to the application instance
    wc.lpszClassName = "class";                                                                                 // Name of the window class
    wc.hCursor = LoadCursor(NULL, IDC_HAND);                                                                    // Default cursor
    wc.hIcon = (HICON)LoadImageA(NULL, iconPath.c_str(), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE | LR_LOADFROMFILE); // Create Icon
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);                                                              // Default background color
    wc.style = CS_HREDRAW | CS_VREDRAW;                                                                         // Window style (optional)

    ShowCursor(showCursor);

    // Register the window class
    RegisterClassA(&wc);
    vram = new Pixel[windowInfo.vram.x * windowInfo.vram.y]{};
    window = CreateWindowExA(0, "class", windowName.c_str(), WS_VISIBLE | WS_SYSMENU, 0, 0, windowInfo.screen.x, windowInfo.screen.y, 0, 0, wc.hInstance, 0);
    WindowDC = GetDC(window);

    bmi.bmiHeader.biWidth = windowInfo.vram.x;
    bmi.bmiHeader.biHeight = windowInfo.vram.y;
    bmi.bmiHeader.biBitCount = sizeof(Pixel) * 8;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biSize = sizeof(BITMAPINFO);

    RECT client;
    GetClientRect(window, &client);
    SetWindowPos(window, 0, 0, 0,
        windowInfo.screen.x + (windowInfo.screen.x - client.right),
        windowInfo.screen.y + (windowInfo.screen.y - client.bottom), 0
    );

    // openGL
    pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,
    24,0,0,0,0,0,0,0,0,0,
    0,0,0,0,32,0,0,PFD_MAIN_PLANE,
    0,0,0,0
    };
#pragma comment(lib, "opengl32.lib")
    SetPixelFormat(WindowDC, ChoosePixelFormat(WindowDC, &pfd), &pfd);
    wglMakeCurrent(WindowDC, wglCreateContext(WindowDC));
    SwapBuffers(WindowDC);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelZoom((float)windowInfo.screen.x / windowInfo.vram.x, (float)windowInfo.screen.y / windowInfo.vram.y);
    glWindowPos2f = (void(_stdcall*)(float, float))wglGetProcAddress("glWindowPos2f");
    glGenerateMipmap = (void(_stdcall*)(unsigned))wglGetProcAddress("glGenerateMipmap");
    wglSwapIntervalEXT = (unsigned(_stdcall*)(unsigned))wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(vSync);
    renderFrame();
}

void    closeWindow()
{
    delete[] vram;
}

void    errorBox(std::string body, std::string title)
{
    MessageBoxA(0, body.c_str(), title.c_str(), MB_OK);
}

static void    bindTexture(Image* texture)
{
    glGenTextures(1, &texture->GLindex);
    glBindTexture(GL_TEXTURE_2D, texture->GLindex);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->size.x, texture->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->content);
    glGenerateMipmap(GL_TEXTURE_2D);
}

int     getImageID()
{
    static int i;
    return i++;
}

Image::Image()
{
    content = nullptr;
    ID = getImageID();
    //std::cout << "+ ID: " << ID << "\n";
}

Image::Image(std::string path)
{
    this->Load(path);
    ID = getImageID();
    //std::cout << "+ ID: " << ID << "\n";
}

void    Image::Load(std::string path)
{
    delete[] content;
    content = nullptr;

    HANDLE      file;
    char        header[BMP::HEADER_MAX_SIZE];
    char*       buffer;
    uint32_t    fileSize;
    uint32_t    endOfHeader;

    file = CreateFileA(path.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
    {
        MessageBoxA(0, ("Couldn't open " + path).c_str(), "Error opening file", MB_OK);
        return;
    }
    ReadFile(file, (LPVOID)header, BMP::HEADER_MAX_SIZE, 0, 0);
    fileSize = *(uint32_t*)(header + BMP::FILE_SIZE);
    endOfHeader = *(uint32_t*)(header + BMP::HEADER_OFFSET);
    size.x = *(uint32_t*)(header + BMP::IMAGE_X_SIZE);
    size.y = *(uint32_t*)(header + BMP::IMAGE_Y_SIZE);
    SetFilePointer(file, endOfHeader, 0, 0);
    content = new Pixel[(fileSize - endOfHeader) / 4];
    buffer = new char[fileSize - endOfHeader];
    ReadFile(file, buffer, fileSize - endOfHeader, 0, 0);
    for (int i = 0; i < (fileSize - endOfHeader) / 4; i++)
    {
        content[i].b = buffer[i * 4 + 0];
        content[i].g = buffer[i * 4 + 1];
        content[i].r = buffer[i * 4 + 2];
        content[i].a = buffer[i * 4 + 3];
    }
    CloseHandle(file);
    delete[] buffer;
    bindTexture(this);
}

//Image& Image::operator=(const Image& cpy)
//{
//    if (this != &cpy)
//    {
//        delete[] content;
//        size = cpy.size;
//        content = new Pixel[size.x * size.y];
//        std::copy(cpy.content, cpy.content + (size.x * size.y), content);
//        GLindex = cpy.GLindex;
//    }
//    return *this;
//}

Image::~Image()
{
    delete[] content; //memory leaks :))
    content = nullptr;
    //std::cout << "- ID: " << ID << "\n";
}

bool    rowIsEmpty(const Image& image, int x, int y, int maxY)
{
    for (int y2 = 0; y2 < maxY; y2++)
    {
        if (*(uint32_t*)&image.content[x + (y + y2) * image.size.x] != 0)
        {
            return false;
        }
    }
    return true;
}

void    loadFont(Font& font, std::string path)
{
    font.image.Load(path);
    font.charSize = { font.image.size.x / 16, font.image.size.y / 8 };
    for (int i = 0; i < 128; i++)
    {
        int x = 0;
        while (x < font.charSize.x && rowIsEmpty(font.image, i % 16 * font.charSize.x + x, (7 - i / 16) * font.charSize.y, font.charSize.y) == false)
        {
            x++;
        }
        font.sizes[i] = x;
    }
    font.sizes[' '] = font.charSize.x / 4;
}

//--------------------------------- ^^^ init ^^^ ----------------------------------

void    drawRectange(Vec2 pos, Vec2 size, WindowInfo windowInfo, Pixel colour)
{
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glColor4ub(colour.r, colour.g, colour.b, colour.a);
            glVertex2f(posToOpenGL(pos.x, windowInfo.vram.x),          posToOpenGL(pos.y, windowInfo.vram.y));
            glVertex2f(posToOpenGL(pos.x, windowInfo.vram.x),          posToOpenGL(pos.y + size.y, windowInfo.vram.y));
            glVertex2f(posToOpenGL(pos.x + size.x, windowInfo.vram.x), posToOpenGL(pos.y + size.y, windowInfo.vram.y));
            glVertex2f(posToOpenGL(pos.x + size.x, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));
        glColor4ub(255, 255, 255, 255);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void    drawImage(Vec2 pos, WindowInfo windowInfo, Image* image, bool flipped)
{
    float   startX = flipped ? pos.x + image->size.x : pos.x;
    float   endX = flipped ? pos.x : pos.x + image->size.x;

    glBindTexture(GL_TEXTURE_2D, image->GLindex);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(posToOpenGL(startX, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));
        glTexCoord2f(0, 1);
        glVertex2f(posToOpenGL(startX, windowInfo.vram.x), posToOpenGL(pos.y + image->size.y, windowInfo.vram.y));
        glTexCoord2f(1, 1);
        glVertex2f(posToOpenGL(endX, windowInfo.vram.x), posToOpenGL(pos.y + image->size.y, windowInfo.vram.y));
        glTexCoord2f(1, 0);
        glVertex2f(posToOpenGL(endX, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));;
    glEnd();
}

void    drawImage(Vec2 pos, Vec2 tilePos, float size, WindowInfo windowInfo, Image* image, uint8_t colour)
{
    float   startX = pos.x;
    float   endX = pos.x + size * image->size.x;

    glBindTexture(GL_TEXTURE_2D, image->GLindex);
    glBegin(GL_QUADS);
    glColor3b(colour, colour, colour);
        glTexCoord2f(tilePos.x, tilePos.y);
        glVertex2f(posToOpenGL(startX, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));
        glTexCoord2f(tilePos.x, tilePos.y + size);
        glVertex2f(posToOpenGL(startX, windowInfo.vram.x), posToOpenGL(pos.y + size * image->size.y, windowInfo.vram.y));
        glTexCoord2f(tilePos.x + size, tilePos.y + size);
        glVertex2f(posToOpenGL(endX, windowInfo.vram.x), posToOpenGL(pos.y + size * image->size.y, windowInfo.vram.y));
        glTexCoord2f(tilePos.x + size, tilePos.y);
        glVertex2f(posToOpenGL(endX, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));
    glColor3b(127, 127, 127);
    glEnd();
}

void    drawImage(Vec2 pos, WindowInfo windowInfo, Image* image, float transparency)
{
    float   startX = pos.x;
    float   endX = pos.x + image->size.x;

    glColor4ub(255, 255, 255, transparency);
    glBindTexture(GL_TEXTURE_2D, image->GLindex);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(posToOpenGL(startX, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));
        glTexCoord2f(0, 1);
        glVertex2f(posToOpenGL(startX, windowInfo.vram.x), posToOpenGL(pos.y + image->size.y, windowInfo.vram.y));
        glTexCoord2f(1, 1);
        glVertex2f(posToOpenGL(endX, windowInfo.vram.x), posToOpenGL(pos.y + image->size.y, windowInfo.vram.y));
        glTexCoord2f(1, 0);
        glVertex2f(posToOpenGL(endX, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));
    glColor4ub(255, 255, 255, 255);
    glEnd();
}

static void    drawChar(const Font& font, WindowInfo windowInfo, Vec2 pos, Vec2 charPos, Vec2 charSize, float transparency)
{
    glBindTexture(GL_TEXTURE_2D, font.image.GLindex);
    glColor4b(127, 127, 127, transparency);
    glBegin(GL_QUADS);

        glTexCoord2f(charPos.x, charPos.y);
        glVertex2f(posToOpenGL(pos.x, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));
        glTexCoord2f(charPos.x, charPos.y + charSize.y);
        glVertex2f(posToOpenGL(pos.x, windowInfo.vram.x), posToOpenGL(pos.y + font.image.size.y * charSize.y, windowInfo.vram.y));
        glTexCoord2f(charPos.x + charSize.x, charPos.y + charSize.y);
        glVertex2f(posToOpenGL(pos.x + font.image.size.x * charSize.x, windowInfo.vram.x), posToOpenGL(pos.y + font.image.size.y * charSize.y, windowInfo.vram.y));
        glTexCoord2f(charPos.x + charSize.x, charPos.y);
        glVertex2f(posToOpenGL(pos.x + font.image.size.x * charSize.x, windowInfo.vram.x), posToOpenGL(pos.y, windowInfo.vram.y));;
   
    glColor4b(127, 127, 127, 127);
    glEnd();

}

static void    getCharLen(const Font& font, float& len, char c)
{
    if (c == '\t')
        len = (int)(len / 20) * 20 + 20;
    else
        len += font.sizes[c] + 1;
}

int    drawString(const Font& font, WindowInfo windowInfo, Vec2 pos, std::string str, float transparency)
{
    float len = 0;

    for (char c : str)
    {
        Vec2    charPos = 
        {
            (float)(c % 16 * font.charSize.x) / font.image.size.x,
            (float)((7 - c / 16) * font.charSize.y) / font.image.size.y
        };
        Vec2    charSize =
        {
            (float)font.sizes[c] / font.image.size.x,
            (float)font.charSize.y / font.image.size.y
        };
        drawChar(font, windowInfo, { pos.x + len, pos.y - 1 }, charPos, charSize, transparency);
        getCharLen(font, len, c);
    }
    return len;
}

int    drawString(const Font& font, WindowInfo windowInfo, Vec2 pos, std::string str, Font::Align alignX, Font::Align alignY, float transparency)
{
    float   len = 0;
    float   height = 1;

    if (alignX != Font::Align::LEFT)
    {
        for (char c : str)
        {
            getCharLen(font, len, c);
        }
    }
    if (alignX == Font::Align::CENTRE)
    {
        len /= 2;
    }
    if (alignY == Font::Align::CENTRE)
    {
        height += font.charSize.y / 2;
    }
    else if (alignY == Font::Align::BOTTOM)
    {
        height += font.charSize.y;
    }
    return drawString(font, windowInfo, { pos.x - len, pos.y - height }, str, transparency);
}

void    renderFrame()
{
    SwapBuffers(WindowDC);
}
