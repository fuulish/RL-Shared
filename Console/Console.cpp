#include "Console.hpp"
#include "Include/system.hpp"
#include <chrono>

#if defined(IS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#elif defined(IS_UNIX)
#include <curses.h>
#endif
#include <cassert>
#include <thread>

#include <cstring>


#if defined(IS_UNIX)
void WriteConsoleOutput(HANDLE win, CHAR_INFO * chs, COORD size, COORD dest, SMALL_RECT* rect)
{
	for (int i=0; i<size.X; ++i) {
		for (int j=0; j<size.Y; ++j) {
			mvwaddch(win, j, i,    chs[ j*size.X + i ].Char.AsciiChar);
			// XXX: works, but for the wrong reasons, FIX IT
			attr_t attr = chs[ j*size.X + i ].Attributes & A_ATTRIBUTES;
			chtype colr = (chs[ j*size.X + i ].Attributes & A_COLOR) >> NCURSES_ATTR_SHIFT;
			mvwchgat(win, j, i, 1, attr, colr, NULL);
		}
	}
}

typedef struct {
	int dwSize;
	bool bVisible;
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
	initscr();
	assert(has_colors() && can_change_color());
	start_color();
	HANDLE win = newwin(CONSOLE_SIZE_Y, CONSOLE_SIZE_X, 0, 0);
	keypad(win, TRUE);

	wrefresh(win);

	return win;
}
#endif

namespace RL_shared
{




#if defined(IS_WINDOWS)
WORD convertConsoleColour( Console::Colour foreground, Console::Colour background )
{
    WORD colour = 0;

    switch (foreground)
    {
        case Console::Red:              colour |= FOREGROUND_RED;                                                               break;
        case Console::Green:            colour |= FOREGROUND_GREEN;                                                             break;
        case Console::Blue:             colour |= FOREGROUND_BLUE;                                                              break;
        case Console::Yellow:           colour |= FOREGROUND_RED | FOREGROUND_GREEN;                                            break;
        case Console::Cyan:             colour |= FOREGROUND_GREEN | FOREGROUND_BLUE;                                           break;
        case Console::Magenta:          colour |= FOREGROUND_RED | FOREGROUND_BLUE;                                             break;
        case Console::Grey:             colour |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;                          break;
        case Console::BrightRed:        colour |= FOREGROUND_INTENSITY | FOREGROUND_RED;                                        break;
        case Console::BrightGreen:      colour |= FOREGROUND_INTENSITY | FOREGROUND_GREEN;                                      break;
        case Console::BrightBlue:       colour |= FOREGROUND_INTENSITY | FOREGROUND_BLUE;                                       break;
        case Console::BrightYellow:     colour |= FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;                     break;
        case Console::BrightCyan:       colour |= FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE;                    break;
        case Console::BrightMagenta:    colour |= FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE;                      break;
        case Console::White:            colour |= FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;   break;
        default:;
    };

    switch (background)
    {
        case Console::Red:              colour |= BACKGROUND_RED;                                                               break;
        case Console::Green:            colour |= BACKGROUND_GREEN;                                                             break;
        case Console::Blue:             colour |= BACKGROUND_BLUE;                                                              break;
        case Console::Yellow:           colour |= BACKGROUND_RED | BACKGROUND_GREEN;                                            break;
        case Console::Cyan:             colour |= BACKGROUND_GREEN | BACKGROUND_BLUE;                                           break;
        case Console::Magenta:          colour |= BACKGROUND_RED | BACKGROUND_BLUE;                                             break;
        case Console::Grey:             colour |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;                          break;
        case Console::BrightRed:        colour |= BACKGROUND_INTENSITY | BACKGROUND_RED;                                        break;
        case Console::BrightGreen:      colour |= BACKGROUND_INTENSITY | BACKGROUND_GREEN;                                      break;
        case Console::BrightBlue:       colour |= BACKGROUND_INTENSITY | BACKGROUND_BLUE;                                       break;
        case Console::BrightYellow:     colour |= BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN;                     break;
        case Console::BrightCyan:       colour |= BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE;                    break;
        case Console::BrightMagenta:    colour |= BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE;                      break;
        case Console::White:            colour |= BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;   break;
        default:;
    };

    return colour;
}
#elif defined(IS_UNIX)
int convertConsoleColour( Console::Colour foreground, Console::Colour background )
{
    int color_len = 3,
           colour = 1 << (2 * color_len), // avoid re-mapping default terminal colour
        is_bright = 1 << color_len,
       color_mask = (1 << color_len) - 1;

    short fore = 0, back = 0;

    switch (foreground)
    {
        case Console::Red:              fore = COLOR_RED;                 break;
        case Console::Green:            fore = COLOR_GREEN;               break;
        case Console::Blue:             fore = COLOR_BLUE;                break;
        case Console::Yellow:           fore = COLOR_YELLOW;              break;
        case Console::Cyan:             fore = COLOR_CYAN;                break;
        case Console::Magenta:          fore = COLOR_MAGENTA;             break;
        case Console::Grey:             fore = COLOR_BLACK;               break;
        case Console::BrightRed:        fore = is_bright | COLOR_RED;     break;
        case Console::BrightGreen:      fore = is_bright | COLOR_GREEN;   break;
        case Console::BrightBlue:       fore = is_bright | COLOR_BLUE;    break;
        case Console::BrightYellow:     fore = is_bright | COLOR_YELLOW;  break;
        case Console::BrightCyan:       fore = is_bright | COLOR_CYAN;    break;
        case Console::BrightMagenta:    fore = is_bright | COLOR_MAGENTA; break;
        case Console::White:            fore = is_bright | COLOR_WHITE;   break;
        default:;
    };

    switch (background)
    {
        case Console::Red:              back = COLOR_RED;                 break;
        case Console::Green:            back = COLOR_GREEN;               break;
        case Console::Blue:             back = COLOR_BLUE;                break;
        case Console::Yellow:           back = COLOR_YELLOW;              break;
        case Console::Cyan:             back = COLOR_CYAN;                break;
        case Console::Magenta:          back = COLOR_MAGENTA;             break;
        case Console::Grey:             back = COLOR_BLACK;               break;
        case Console::BrightRed:        back = is_bright | COLOR_RED;     break;
        case Console::BrightGreen:      back = is_bright | COLOR_GREEN;   break;
        case Console::BrightBlue:       back = is_bright | COLOR_BLUE;    break;
        case Console::BrightYellow:     back = is_bright | COLOR_YELLOW;  break;
        case Console::BrightCyan:       back = is_bright | COLOR_CYAN;    break;
        case Console::BrightMagenta:    back = is_bright | COLOR_MAGENTA; break;
        case Console::White:            back = is_bright | COLOR_WHITE;   break;
        default:;
    };

    int bbb, fff;

    bbb = (color_mask & back) << color_len;
    fff = color_mask & fore;
    colour |= bbb | fff;

    assert(OK == (init_pair(colour, color_mask & fore, color_mask & back)));

    colour <<= NCURSES_ATTR_SHIFT;

    if (is_bright & fore)
	    colour |= A_BOLD;
    if (is_bright & back)
	    colour |= A_BLINK;

    return colour;
}
#endif













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

void Console::cleanup(void)
{
#if defined(IS_UNIX)
	endwin();
#endif
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

	wrefresh(m_data->hFrontBuffer);

	//Sleep(50);
}
void Sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void Console::sleep(int ms)
{
	Sleep(ms);
}
KeyCode Console::readKey(void)
{
	bool escaped( false );

	char ch = _getch();
#if defined(_WIN32)
	if (0 == ch)
	{
		escaped = true;
		ch = _getch();
	}
	else {
		_getch();
	}
#endif

    return KeyCode( ch, escaped ); 
}
Console::ConsoleDims Console::getConsoleDimensions(void)
{
    ConsoleDims dims = {CONSOLE_SIZE_X, CONSOLE_SIZE_Y};
    return dims;
}



}
