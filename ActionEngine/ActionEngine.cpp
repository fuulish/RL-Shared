#include "ActionEngine.hpp"
#include "WorldObjects/Actor.hpp"
#include "assert.hpp"
#include <boost/foreach.hpp>



namespace RL_shared
{


namespace
{
bool actionLessThan( boost::shared_ptr< const BaseAction > lhs, boost::shared_ptr< const BaseAction > rhs )
{
	return (lhs->timeRemaining()) < (rhs->timeRemaining());
}
}


void ActionEngine::addAction( boost::shared_ptr< BaseAction > action )
{
	ASSERT(INVALID_KEY == action->key());

	action->setKey(m_next_key);
	m_action_db[m_next_key] = action;
	++m_next_key;

	m_pending_actions.push_back( action );

	ASSERT(action->actor());

	action->actor()->ncActions().push_back( action->key() );
}

//TODO: supporting t != dt may be unnecessary
void ActionEngine::advance( GameTimeCoordinate t, GameTimeCoordinate dt, AGameModel& model )
{
	addPendingActions();

	std::sort( m_actions.begin(), m_actions.end(), actionLessThan ); //TODO: possibly unnecessary.

	GameTimeCoordinate ta = 0;
	do
	{

		ta += dt;
		if (ta > t)
			dt -= (ta-t);

		BOOST_FOREACH( Actions::value_type action, m_actions )
		{
			action->advance( dt, model );
		}

	} while (ta < t);

	addPendingActions();

	//Re-sort the actions, even if none were added, in case some have decreased by more than t 
	//(this may happen, for example, if one is interrupted; in general, actions are free to report whatever 
	//time remaining they like, so the engine may not assume that they are sorted except immediately after sorting them). 
	std::sort( m_actions.begin(), m_actions.end(), actionLessThan );

	std::vector< boost::shared_ptr< Actor > > actors;

	while ((!m_actions.empty()) && (m_actions.front()->timeRemaining() <= 0))
	{
		actors.push_back( m_actions.front()->actor() );

		ActionDB::iterator pos( m_action_db.find(m_actions.front()->key()) );
		ASSERT(m_action_db.end() != pos);
		if (m_action_db.end() != pos)
			m_action_db.erase( pos );

		m_actions.pop_front();
	}

	BOOST_FOREACH( boost::shared_ptr< Actor > actor, actors )
	{
		if (actor)
			actor->removeDeadActions(*this);
	}
}

GameTimeCoordinate ActionEngine::shortestTimeRemaining() const
{
	GameTimeCoordinate shortest_time(0);

	if (!m_actions.empty())
		shortest_time = m_actions.front()->timeRemaining();
	else if (!m_pending_actions.empty())
		shortest_time = m_pending_actions.front()->timeRemaining();

	BOOST_FOREACH( boost::shared_ptr< BaseAction > action, m_actions )
	{
		shortest_time = (std::min)(shortest_time, action->timeRemaining());
	}
	BOOST_FOREACH( boost::shared_ptr< BaseAction > action, m_pending_actions )
	{
		shortest_time = (std::min)(shortest_time, action->timeRemaining());
	}

	return shortest_time;
}

void ActionEngine::addPendingActions( void )
{
	if (!m_pending_actions.empty())
	{
		std::size_t old_size( m_actions.size() );
		m_actions.resize( old_size + m_pending_actions.size() );
		std::copy( m_pending_actions.begin(), m_pending_actions.end(), m_actions.begin() + old_size );

		m_pending_actions.clear();
	}
}


boost::shared_ptr< BaseAction > ActionEngine::getAction( DBKeyValue key ) const
{
	ActionDB::const_iterator pos( m_action_db.find(key) );
	if (m_action_db.end() != pos)
		return pos->second;
	return boost::shared_ptr< BaseAction >();
}


}
