#ifndef RL_SHARED_CONSOLE_MESSAGE_DISPLAY_HPP
#define	RL_SHARED_CONSOLE_MESSAGE_DISPLAY_HPP


#include "Messages/UserMessageLog.hpp"
#include "Messages/UserMessageWindow.hpp"
#include "Console/Console.hpp"
#include "Include/assert.hpp"


namespace RL_shared
{


class MessageDisplay
{
public:

	enum {LOG_SIZE = 50};

	struct WindowParams
	{
		int x_low, x_high, y_low, y_high;
		WindowParams( int xl, int xh, int yl, int yh ) : x_low(xl), x_high(xh), y_low(yl), y_high(yh)
		{
			ASSERT( x_low <= x_high );
			ASSERT( y_low <= y_high );
		}
		bool operator==( const WindowParams& rhs ) const
		{
			return (rhs.x_low == x_low) && (rhs.x_high == x_high) && (rhs.y_low == y_low) && (rhs.y_high == y_high);
		}
		bool operator!=( const WindowParams& rhs ) const
		{
			return !(*this == rhs);
		}
	};

	explicit MessageDisplay( const WindowParams& );

	void setWindow( const WindowParams& );

	void addString( const char * );
	void beginNewMessage(void);
	void scrollUp(void);
	void scrollDown(void);

	void draw( Console&, Console::Colour hl_col, Console::Colour col ) const;

private:

	UserMessageLog m_msgs;
	UserMessageWindow m_msg_window;

	WindowParams m_window;

	bool m_new_msg;

	MessageDisplay( const MessageDisplay& );
	MessageDisplay& operator=( const MessageDisplay& );
};


}



#endif
