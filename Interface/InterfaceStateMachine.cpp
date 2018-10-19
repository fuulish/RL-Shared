#include "InterfaceStateMachine.hpp"
#include <vector>
#include <boost/foreach.hpp>






namespace RL_shared
{





class InterfaceStateMachineImpl
{
    typedef std::vector< InterfaceStatePtr > StateStack;
    StateStack m_state_stack;

	InterfaceStatePtr m_next_state;

public:

	InterfaceStateMachineImpl(void);
	~InterfaceStateMachineImpl();

	void pushInitialState( AGameModel& model, InterfaceStatePtr initial_state );

	void exitAll( AGameModel& );
    void draw( AOutputWindow&, AGameModel& ) const;
    std::pair< bool, bool > interpretInput( const AUserInputItem&, AGameModel& );
	bool notifyAHGameModelAdvance( AGameModel&, GameTimeCoordinate dt );
	bool finished( void );
	bool needsInput( void );

	void setNextState( InterfaceStatePtr state )
	{
		m_next_state = state;
	}

private:

	InterfaceStateMachineImpl( const InterfaceStateMachineImpl& );
	InterfaceStateMachineImpl& operator = ( const InterfaceStateMachineImpl& );

    InterfaceStatePtr getCurrentState( void );
    InterfaceStateConstPtr getCurrentState( void ) const;
	bool updateState( AGameModel& model );
    void pushState( InterfaceStatePtr newstate, AGameModel& );
    void popState( AGameModel& );
};





InterfaceStateMachineImpl::InterfaceStateMachineImpl(void)
{
}

InterfaceStateMachineImpl::~InterfaceStateMachineImpl()
{
}

void InterfaceStateMachineImpl::pushInitialState( AGameModel& model, InterfaceStatePtr initial_state )
{
	pushState( initial_state, model );
}

void InterfaceStateMachineImpl::exitAll( AGameModel& model )
{
    while (!m_state_stack.empty())
    {
        popState( model );
    }
}

void InterfaceStateMachineImpl::draw( AOutputWindow& window, AGameModel& model ) const
{
    //Draw the states from parents to children, so that "popover" states overwrite the others.
    //Start from either the beginning or the last state which draws over the whole window.

    StateStack::const_iterator draw_iterator = m_state_stack.begin();

    for (StateStack::const_reverse_iterator ri = m_state_stack.rbegin(); ri != m_state_stack.rend(); ++ri)
    {
        if ((*ri)->drawsWholeWindow())
        {
            draw_iterator = (++ri).base();
            break;
        }
    }

    for (; draw_iterator != m_state_stack.end(); ++draw_iterator)
    {
        (*draw_iterator)->draw( window, model );
    }
}

bool InterfaceStateMachineImpl::updateState( AGameModel& model )
{
	if (m_next_state)
	{
		InterfaceStatePtr next_state = m_next_state;
		m_next_state.reset();
		pushState( next_state, model );

		updateState(model);

		return true;
	}
	else if (getCurrentState() && getCurrentState()->finished())
	{
		popState( model );

		updateState(model);

		return true;
	}

	return false;
}

std::pair< bool, bool > InterfaceStateMachineImpl::interpretInput( const AUserInputItem& input, AGameModel& model )
{
	InterfaceStatePtr current_state = getCurrentState();

	if (!current_state)
		return std::pair< bool, bool >(false, false);

	std::pair< bool, bool > result( current_state->interpretInput( input, model ) );

	//Always force a redraw when there is a state change.
	result.second |= updateState(model);

	return result;
}

bool InterfaceStateMachineImpl::notifyAHGameModelAdvance( AGameModel& model, GameTimeCoordinate dt )
{
	BOOST_FOREACH( InterfaceStatePtr state, m_state_stack )
	{
		state->notifyAHGameModelAdvance(model, dt, getCurrentState() == state);
	}

	return updateState(model);
}

bool InterfaceStateMachineImpl::finished( void )
{
	return m_state_stack.empty();
}

bool InterfaceStateMachineImpl::needsInput( void ) 
{
	if (m_state_stack.empty())
		return false;
	return getCurrentState()->needsInput();
}

InterfaceStatePtr InterfaceStateMachineImpl::getCurrentState(void)
{
    if (m_state_stack.empty())
        return InterfaceStatePtr();
    return m_state_stack.back();
}

InterfaceStateConstPtr InterfaceStateMachineImpl::getCurrentState(void) const
{
    if (m_state_stack.empty())
        return InterfaceStateConstPtr();
    return m_state_stack.back();
}

void InterfaceStateMachineImpl::pushState( InterfaceStatePtr new_state, AGameModel& model )
{
    if (!m_state_stack.empty())
        m_state_stack.back()->exitToChild( model );

    m_state_stack.push_back( new_state );

    new_state->enterFromParent( model );
}

void InterfaceStateMachineImpl::popState( AGameModel& model )
{
	if (m_state_stack.empty())
		return;

    InterfaceStatePtr outgoing = m_state_stack.back();

    m_state_stack.back().reset();
    m_state_stack.pop_back();

    outgoing->exitToParent( model );

	if (!m_state_stack.empty())
	    m_state_stack.back()->enterFromChild( model );
}













InterfaceStateMachine::InterfaceStateMachine(void)
: m_impl( new InterfaceStateMachineImpl() )
{
}

void InterfaceStateMachine::pushInitialState( AGameModel& model, InterfaceStatePtr initial_state )
{
	m_impl->pushInitialState( model, initial_state );
}

void InterfaceStateMachine::exitAll( AGameModel& model )
{
	m_impl->exitAll( model );
}

void InterfaceStateMachine::draw( AOutputWindow& window, AGameModel& model ) const
{
	m_impl->draw( window, model );
}

std::pair< bool, bool > InterfaceStateMachine::interpretInput( const AUserInputItem& input, AGameModel& model )
{
	return m_impl->interpretInput( input, model );
}

bool InterfaceStateMachine::notifyAHGameModelAdvance( AGameModel& model, GameTimeCoordinate dt )
{
	return m_impl->notifyAHGameModelAdvance(model, dt);
}

bool InterfaceStateMachine::finished( void )
{
	return m_impl->finished();
}

bool InterfaceStateMachine::needsInput( void )
{
	return m_impl->needsInput();
}

void InterfaceStateMachine::setNextState( InterfaceStatePtr newstate )
{
	m_impl->setNextState( newstate );
}



}

