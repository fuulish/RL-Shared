#include "Console.hpp"

#define WIN32_LEAN_AND_MEAN
// #include <windows.h>
// #include <conio.h>
#include <curses.h>
#include <cassert>
#include <thread>
#include <chrono>

#include <cstring>


// as substitute for the windows things
void WriteConsoleOutput(HANDLE win, CHAR_INFO * chs, COORD size, COORD dest, SMALL_RECT* rect)
{
	for (int i=0; i<size.X; ++i) {
		for (int j=0; j<size.Y; ++j) {
			mvwaddch(win, j, i,    chs[ j*size.X + i ].Char.AsciiChar);
			mvwchgat(win, j, i, 1, chs[ j*size.X + i ].Attributes, 0, NULL); // XXX: still need to reset the color
		}
	}
}

void Sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

typedef struct {
	int bla;
	bool theTruth;
} CONSOLE_CURSOR_INFO;

bool SetConsoleCursorInfo(HANDLE win, CONSOLE_CURSOR_INFO *lpConsoleCursorInfo)
{
	noecho();
	cbreak();
	curs_set(0);
	nodelay(win, FALSE);
	return true;
}

const int CONSOLE_SIZE_X = 80;
const int CONSOLE_SIZE_Y = 40;


bool SetConsoleScreenBufferSize(HANDLE win, COORD crd)
{
    getmaxyx(win, crd.Y, crd.X);
    assert((crd.X >= CONSOLE_SIZE_X) && (crd.Y >= CONSOLE_SIZE_Y));
    // XXX potentially  int wresize(WINDOW *win, int lines, int columns);  could help

    return true;
}

typedef bool BOOL;
bool SetConsoleWindowInfo(HANDLE win, BOOL absolute, SMALL_RECT *rect)
{
	return true;
}

bool SetConsoleActiveScreenBuffer(HANDLE win)
{
	return true;
}

typedef unsigned long DWORD;
#define  GENERIC_READ   0
#define  GENERIC_WRITE  1
#define  CONSOLE_TEXTMODE_BUFFER  0

// second arg should be a DWORD, not a pointer
HANDLE CreateConsoleScreenBuffer(DWORD access, DWORD *shareMode, void *secAttr, DWORD dwFlags, void *bufferData)
{
	HANDLE win = initscr();
	start_color();

	return win;
}

namespace RL_shared
{




int convertConsoleColour( Console::Colour foreground, Console::Colour background )
{
    int colour = 0;

    short fore, back;

    // XXX: need to ensure that start_color was called

    switch (foreground)
    {
        case Console::Red:              fore = COLOR_RED;     break;
        case Console::Green:            fore = COLOR_GREEN;   break;
        case Console::Blue:             fore = COLOR_BLUE;    break;
        case Console::Yellow:           fore = COLOR_YELLOW;  break;
        case Console::Cyan:             fore = COLOR_CYAN;    break;
        case Console::Magenta:          fore = COLOR_MAGENTA; break;
        case Console::Grey:             fore = COLOR_BLACK;   break; // should be bright black
        case Console::BrightRed:        fore = COLOR_RED;     break; // need brightness/intensity attribute
        case Console::BrightGreen:      fore = COLOR_GREEN;   break; // need brightness/intensity attribute
        case Console::BrightBlue:       fore = COLOR_BLUE;    break; // need brightness/intensity attribute
        case Console::BrightYellow:     fore = COLOR_YELLOW;  break; // need brightness/intensity attribute
        case Console::BrightCyan:       fore = COLOR_CYAN;    break; // need brightness/intensity attribute
        case Console::BrightMagenta:    fore = COLOR_MAGENTA; break; // need brightness/intensity attribute
        case Console::White:            fore = COLOR_WHITE;   break; // need brightness/intensity attribute
        default:;
    };

    switch (background)
    {
        case Console::Red:              back = COLOR_RED;     break;
        case Console::Green:            back = COLOR_GREEN;   break;
        case Console::Blue:             back = COLOR_BLUE;    break;
        case Console::Yellow:           back = COLOR_YELLOW;  break;
        case Console::Cyan:             back = COLOR_CYAN;    break;
        case Console::Magenta:          back = COLOR_MAGENTA; break;
        case Console::Grey:             back = COLOR_BLACK;   break; // should be bright black
        case Console::BrightRed:        back = COLOR_RED;     break; // need brightness/intensity attribute
        case Console::BrightGreen:      back = COLOR_GREEN;   break; // need brightness/intensity attribute
        case Console::BrightBlue:       back = COLOR_BLUE;    break; // need brightness/intensity attribute
        case Console::BrightYellow:     back = COLOR_YELLOW;  break; // need brightness/intensity attribute
        case Console::BrightCyan:       back = COLOR_CYAN;    break; // need brightness/intensity attribute
        case Console::BrightMagenta:    back = COLOR_MAGENTA; break; // need brightness/intensity attribute
        case Console::White:            back = COLOR_WHITE;   break; // need brightness/intensity attribute
        default:;
    };

    colour = fore | back;

    if (OK != (init_pair(colour, fore, back)))
	    return 0; // XXX: double-check that this makes sense/works

    return colour;
}













struct Console::ConsoleData
{
	ConsoleData(void);

    void    hideCursor(void);

    HANDLE  hFrontBuffer;
	CHAR_INFO back_buffer[CONSOLE_SIZE_Y*CONSOLE_SIZE_X];
};

Console::ConsoleData::ConsoleData(void)
: hFrontBuffer(0)
{
	memset(back_buffer, 0, sizeof(CHAR_INFO)*CONSOLE_SIZE_X*CONSOLE_SIZE_Y);
}

void Console::ConsoleData::hideCursor(void)
{
    CONSOLE_CURSOR_INFO ci = {10,FALSE};
    SetConsoleCursorInfo(hFrontBuffer, &ci);
}












Console::Console(void)
{
	m_data.reset( new ConsoleData() );

    m_data->hFrontBuffer = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        NULL,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL
    );

    COORD screen_size;
    screen_size.X = CONSOLE_SIZE_X;
    screen_size.Y = CONSOLE_SIZE_Y;
    SetConsoleScreenBufferSize(m_data->hFrontBuffer, screen_size);

	SMALL_RECT window_rect;
	window_rect.Top = 0;
	window_rect.Left = 0;
	window_rect.Bottom = CONSOLE_SIZE_Y-1;
	window_rect.Right = CONSOLE_SIZE_X-1;
    SetConsoleWindowInfo(m_data->hFrontBuffer, TRUE, &window_rect);

    m_data->hideCursor();
    SetConsoleActiveScreenBuffer(m_data->hFrontBuffer);
}
void Console::clearScreen(void)
{
	memset(m_data->back_buffer, 0, sizeof(CHAR_INFO)*CONSOLE_SIZE_X*CONSOLE_SIZE_Y);
}
void Console::draw(int nX, int nY, char chr, Colour fore, Colour back)
{
    if((nX < 0) || (nX >= CONSOLE_SIZE_X))
        return;
    if((nY < 0) || (nY >= CONSOLE_SIZE_Y))
        return;

	CHAR_INFO write;
	write.Char.UnicodeChar = 0;
	write.Char.AsciiChar = chr;
	write.Attributes = convertConsoleColour( fore, back );

	m_data->back_buffer[(nY*CONSOLE_SIZE_X)+nX] = write;
}
void Console::drawText(int nX, int nY, const char* text, Colour fore, Colour back)
{
	for(; *text; ++nX, ++text)
	{
		draw(nX, nY, *text, fore, back);
	}
}
void Console::updateScreen(void)
{
	COORD size;
	size.X = CONSOLE_SIZE_X;
	size.Y = CONSOLE_SIZE_Y;

	COORD dest;
	dest.X = 0;
	dest.Y = 0;

	SMALL_RECT rect;
	rect.Top = 0;
	rect.Bottom = CONSOLE_SIZE_Y-1;
	rect.Left = 0;
	rect.Right = CONSOLE_SIZE_X-1;

	WriteConsoleOutput( 
        m_data->hFrontBuffer,
        m_data->back_buffer,
        size,
        dest,
        &rect
        );

	//Sleep(50);
}
void Console::sleep(int ms)
{
	Sleep(ms);
}
KeyCode Console::readKey(void)
{
	bool escaped( false );

	char ch = _getch();
	if (0 == ch)
	{
		escaped = true;
		ch = _getch();
	}
	else {
		_getch();
	}

    return KeyCode( ch, escaped ); 
}
Console::ConsoleDims Console::getConsoleDimensions(void)
{
    ConsoleDims dims = {CONSOLE_SIZE_X, CONSOLE_SIZE_Y};
    return dims;
}



}
