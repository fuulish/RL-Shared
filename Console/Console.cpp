#include "Console.hpp"

#define WIN32_LEAN_AND_MEAN
// #include <windows.h>
// #include <conio.h>
#include <curses.h>

#include <stdio.h>




namespace RL_shared
{




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







const int CONSOLE_SIZE_X = 80;
const int CONSOLE_SIZE_Y = 40;






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
