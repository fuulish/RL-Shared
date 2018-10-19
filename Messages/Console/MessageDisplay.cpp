#include "MessageDisplay.hpp"
#include "Console/Console.hpp"



namespace RL_shared
{


MessageDisplay::MessageDisplay( const WindowParams& params )
: m_msgs(LOG_SIZE)
, m_window(params)
, m_new_msg(false)
{
}


void MessageDisplay::setWindow( const WindowParams& params )
{
	if (params != m_window)
	{
		m_window = params;
	}
}


void MessageDisplay::addString( const char * msg )
{
	m_msgs.addMessage( UserMessageLog::Msg( msg ) );
	m_new_msg = true;
}


void MessageDisplay::beginNewMessage(void)
{
	m_msgs.beginNewMessage();
	m_new_msg = false;
}


void MessageDisplay::scrollUp(void)
{
	m_msg_window.scrollBackward();
}


void MessageDisplay::scrollDown(void)
{
	m_msg_window.scrollForward();
}


namespace
{

void drawMessageWindow( 
	Console& window, 
	const UserMessageLog& msg_log, 
	const UserMessageWindow& msg_window, 
	int window_x_low, int window_y_low, int window_x_high, int window_y_high, 
	bool highlight_last,
	Console::Colour hl_col, Console::Colour col
	);
}


void MessageDisplay::draw( Console& console, Console::Colour hl_col, Console::Colour col ) const
{
	drawMessageWindow(console, 
		m_msgs, m_msg_window, 
		m_window.x_low, m_window.y_low, 
		m_window.x_high, m_window.y_high, 
		m_new_msg, hl_col, col
		);
}



namespace
{

void drawMessageWindow( 
	Console& window, 
	const UserMessageLog& msg_log, 
	const UserMessageWindow& msg_window, 
	int window_x_low, int window_y_low, int window_x_high, int window_y_high, 
	bool highlight_last, Console::Colour hl_col, Console::Colour col
	)
{
	ASSERT( window_x_low <= window_x_high );
	ASSERT( window_y_low <= window_y_high );

	Console::Colour old_msg_colour = col;
	Console::Colour new_msg_colour = highlight_last ? hl_col : old_msg_colour;

	const unsigned int window_width( window_x_high - window_x_low + 1 );

	unsigned int skip_line_count(0);
	unsigned int target_skip( msg_window.scrollOffset() );

	UserMessageLog::const_iterator msgs_begin( msg_log.begin() );
	UserMessageLog::const_iterator msgs_end( msg_log.end() );
	for (; (msgs_begin != msgs_end) && (skip_line_count < target_skip); ++msgs_begin)
	{
		unsigned int num_lines(msgs_begin->lines( window_width ));
		if (skip_line_count + num_lines > target_skip)
			break;
		skip_line_count += num_lines;
	}

	int y_pos( window_y_high );

	for (; (msgs_begin != msgs_end) && (y_pos >= window_y_low); ++msgs_begin)
	{
		using std::vector;
		using std::string;

		vector< string > lines;
		msgs_begin->toLines( window_width, lines );

		vector< string >::const_reverse_iterator lines_rbegin( lines.rbegin() );
		vector< string >::const_reverse_iterator lines_rend( lines.rend() );
		for (; (lines_rbegin != lines_rend) && (y_pos >= window_y_low); ++lines_rbegin)
		{
			if (skip_line_count < target_skip)
			{
				++skip_line_count;
				continue;
			}

			Console::Colour colour( msg_log.begin() == msgs_begin ? new_msg_colour : old_msg_colour );

			window.drawText( window_x_low, y_pos, lines_rbegin->c_str(), colour );

			--y_pos;
		}
	}
}

}


}

