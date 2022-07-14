#ifndef RL_SHARED_CONSOLE_VIEW_HPP
#define RL_SHARED_CONSOLE_VIEW_HPP


#include <boost/shared_ptr.hpp>


namespace RL_shared
{


class IGameModel;
class InterfaceStateMachine;
class ConsoleViewImpl;


class ConsoleView
{
public:

	ConsoleView( IGameModel&, InterfaceStateMachine& );

    void run(void);
    void cleanup(void);

private:

	ConsoleView( const ConsoleView& );
	ConsoleView& operator = ( const ConsoleView& );


	boost::shared_ptr< ConsoleViewImpl > m_impl;
};


}



#endif
