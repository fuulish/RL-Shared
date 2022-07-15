#include "ConsoleView.hpp"
#include "KeyPress.hpp"
#include "Console/Console.hpp"
#include "Interface/InterfaceStateMachine.hpp"
#include "Abstract/IGameModel.hpp"
#include "Include/system.hpp"
#include <boost/shared_ptr.hpp>
#include <ctime>





namespace RL_shared
{



const std::clock_t FRAME_TIME = (std::max)((std::clock_t)1, CLOCKS_PER_SEC/50);



class ConsoleViewImpl
{
public:

    ConsoleViewImpl( IGameModel&, InterfaceStateMachine& ui );

    void run(void);

private:

	ConsoleViewImpl( const ConsoleViewImpl& );
	ConsoleViewImpl& operator = ( const ConsoleViewImpl& );



	Console m_console_window;

	IGameModel& m_model;

	InterfaceStateMachine& m_ui;
};




ConsoleViewImpl::ConsoleViewImpl( IGameModel& model, InterfaceStateMachine& ui )
: m_model( model )
, m_ui( ui )
{
}


void ConsoleViewImpl::run(void)
{
	bool done( false );
	bool awaiting_input( true );

	//artificial delay to control maximum "play rate".
	const std::clock_t frame_time( FRAME_TIME );

#if defined(IS_WINDOWS)
	std::clock_t frame_end( std::clock() );
#endif


	while (!done)
	{

		try
		{
#if defined(IS_WINDOWS)
			std::clock_t sleep_time(frame_end - std::clock());
			if ((sleep_time > 0) && (sleep_time < frame_time))
				m_console_window.sleep( sleep_time );
			frame_end = std::clock() + frame_time;
#endif

			m_ui.draw( m_console_window, m_model );
			m_console_window.updateScreen();

			if (awaiting_input)
			{
				bool progress(false);

				while (!progress)
				{

					KeyCode key( m_console_window.readKey() );

					std::pair< bool, bool > result( m_ui.interpretInput( KeyPress(key), m_model ) );
					if ( result.first )
					{
						GameTimeCoordinate dt(0);
						awaiting_input = m_model.advance_turn( dt );
						if (m_ui.notifyAHGameModelAdvance( m_model, dt ))
						{
							awaiting_input = m_ui.needsInput();
							result.second = true;
						}
					}

					done = m_ui.finished();

					progress = (result.first || result.second || done);

				}
			}
			else
			{
				GameTimeCoordinate dt(0);
				awaiting_input = m_model.advance_turn( dt );
				m_ui.notifyAHGameModelAdvance( m_model, dt ); //redraw assumed required, because the model told us to do another turn without waiting for input. 
			}
		}
		catch(std::bad_alloc)
		{
			m_console_window.clearScreen();
			m_console_window.drawText(0, 1, "A fatal error has occurred: System memory is exhausted", Console::BrightRed);
			m_console_window.drawText(0, 23, "Press any key to exit.", Console::BrightRed);
			m_console_window.updateScreen();
			m_console_window.readKey();
			done = true;
		}
		catch(std::exception& err)
		{
			m_console_window.clearScreen();
			m_console_window.drawText(0, 1, "A fatal error has occurred: ", Console::BrightRed);
			m_console_window.drawText(0, 3, err.what(), Console::Red);
			m_console_window.drawText(0, 23, "Press any key to exit.", Console::BrightRed);
			m_console_window.updateScreen();
			m_console_window.readKey();
			done = true;
		}
		catch(const char * string)
		{
			m_console_window.clearScreen();
			m_console_window.drawText(0, 1, "A fatal error has occurred: ", Console::BrightRed);
			m_console_window.drawText(0, 3, string, Console::Red);
			m_console_window.drawText(0, 23, "Press any key to exit.", Console::BrightRed);
			m_console_window.updateScreen();
			m_console_window.readKey();
			done = true;
		}
		catch(...)
		{
			m_console_window.clearScreen();
			m_console_window.drawText(0, 1, "An unknown fatal error has occurred!", Console::BrightRed);
			m_console_window.drawText(0, 23, "Press any key to exit.", Console::BrightRed);
			m_console_window.updateScreen();
			m_console_window.readKey();
			done = true;
		}

	}

}




ConsoleView::ConsoleView( IGameModel& model, InterfaceStateMachine& ui )
: m_impl( new ConsoleViewImpl( model, ui ) )
{
}

void ConsoleView::run(void)
{
	m_impl->run();
}



}
