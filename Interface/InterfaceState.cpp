#include "InterfaceState.hpp"
#include "InterfaceStateMachine.hpp"



namespace RL_shared
{



void InterfaceState::setNextState( InterfaceStatePtr new_state )
{
	boost::shared_ptr< InterfaceStateMachine > ism( m_ISM.lock() );
	if (ism)
	{
		ism->setNextState( new_state );
	}
}




}


