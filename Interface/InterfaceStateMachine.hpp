#ifndef RL_SHARED_INTERFACE_STATE_MACHINE_HPP_INCLUDED
#define RL_SHARED_INTERFACE_STATE_MACHINE_HPP_INCLUDED




#include "InterfaceState.hpp"
#include <utility>




namespace RL_shared
{

		
		
class InterfaceStateMachineImpl;



class InterfaceStateMachine
{
public:

	InterfaceStateMachine(void);

	void pushInitialState( AGameModel& model, InterfaceStatePtr initial_state );

	void exitAll( AGameModel& );
    void draw( AOutputWindow&, AGameModel& ) const;

	///Returns a <bool,bool> pair in which:
	/// first is true if the input was a game command which should advance the game state.
	/// second is true if the game state was not advanced, but a redraw is required anyway. 
    std::pair< bool, bool > interpretInput( const AUserInputItem&, AGameModel& );

	///Returns true if a new state is current.
	bool notifyAHGameModelAdvance( AGameModel&, GameTimeCoordinate dt );

	bool finished( void );

	bool needsInput( void );

private:

	friend class InterfaceState;
	void setNextState( InterfaceStatePtr );



	boost::shared_ptr< InterfaceStateMachineImpl > m_impl;


	InterfaceStateMachine( const InterfaceStateMachine& );
	InterfaceStateMachine& operator = ( const InterfaceStateMachine& );
};



}




#endif
