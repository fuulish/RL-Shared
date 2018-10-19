#ifndef RL_SHARED_CONSOLE_HPP
#define RL_SHARED_CONSOLE_HPP


#include <boost/shared_ptr.hpp>
#include <utility>
#include "Abstract/AOutputWindow.hpp"


namespace RL_shared
{


typedef std::pair< char, bool > KeyCode;


class Console : public AOutputWindow
{
public:

	enum Colour
	{
		Undefined = -1,

		Black = 0,
		Red,
		Green,
		Blue,
		Yellow,
		Cyan,
		Magenta,
		Grey,
		BrightRed,
		BrightGreen,
		BrightBlue,
		BrightYellow,
		BrightCyan,
		BrightMagenta,
		White
	};

    Console(void);

    void clearScreen(void);
    void draw(int nX, int nY, char chr, Colour forecol, Colour backcol = Black);
    void drawText(int nX, int nY, const char* text, Colour forecol, Colour backcol = Black);
    void updateScreen(void);

    KeyCode readKey(void);

    struct ConsoleDims
    {
        int x, y;
    };
    ConsoleDims getConsoleDimensions(void);

	void sleep(int ms);

private:

	struct ConsoleData;
	boost::shared_ptr<ConsoleData> m_data;
};


}


#endif
