#ifndef RL_SHARED_INTERFACESTATE_HPP_INCLUDED
#define RL_SHARED_INTERFACESTATE_HPP_INCLUDED




#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/cstdint.hpp>




namespace RL_shared
{

	class AOutputWindow;
	class AUserInputItem;
	class AGameModel;

	typedef boost::int32_t GameTimeCoordinate;



	class InterfaceState;
	typedef boost::shared_ptr< InterfaceState > InterfaceStatePtr;
	typedef boost::shared_ptr< const InterfaceState > InterfaceStateConstPtr;


	class InterfaceStateMachine;



//Abstract base class for states in the Interface state machine.
class InterfaceState
{
public:

	explicit InterfaceState( boost::weak_ptr< InterfaceStateMachine > ism )
		: m_ISM( ism )
	{
	}

    virtual ~InterfaceState()
    {
    }

    virtual void enterFromParent( AGameModel& ) = 0;
    virtual void exitToParent( AGameModel& ) = 0;
    virtual void enterFromChild( AGameModel& ) = 0;
    virtual void exitToChild( AGameModel& ) = 0;

	///Returns a <bool,bool> pair in which:
	/// first is true if the input was a game command which should advance the game state.
	/// second is true if the game state was not advanced, but a redraw is required anyway. 
	typedef std::pair< bool, bool > CommandResult;
    virtual CommandResult interpretInput( const AUserInputItem&, AGameModel& ) = 0;

	virtual void notifyAHGameModelAdvance( AGameModel&, GameTimeCoordinate dt, bool is_current_state ) = 0;

	///Returns true if this state is to be popped.
    virtual bool finished(void) = 0;

    virtual void draw( AOutputWindow&, AGameModel& ) const = 0;
    virtual bool drawsWholeWindow(void) const = 0;

	///Returns true if the state needs input after being drawn.
	virtual bool needsInput(void) const = 0;

	void setNextState( InterfaceStatePtr new_state );
	boost::weak_ptr< InterfaceStateMachine > interfaceStateMachine(void) const { return m_ISM; }

private:

	boost::weak_ptr< InterfaceStateMachine > m_ISM;
};




}




#endif
