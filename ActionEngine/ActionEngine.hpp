#ifndef RL_SHARED_ACTION_ENGINE_HPP
#define	RL_SHARED_ACTION_ENGINE_HPP



#include "Abstract/AGameModel.hpp"
#include "DBKeyValue.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/cstdint.hpp>
#include <deque>
#include <vector>
#include <map>



namespace RL_shared
{


typedef boost::int32_t GameTimeCoordinate;
class Actor;


class BaseAction
{
public:

	BaseAction(void) : m_key(INVALID_KEY)
	{
	}

	virtual void advance( GameTimeCoordinate, AGameModel& ) = 0;

	virtual GameTimeCoordinate timeRemaining(void) const = 0;

	virtual bool interrupt( AGameModel& )
	{
		return false;
	}
	virtual bool isLongAction( AGameModel& ) const
	{
		return false;
	}

	virtual boost::shared_ptr< Actor > actor(void) const = 0;

	DBKeyValue key(void) const
	{
		return m_key;
	}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & m_key;
	}

private:

	friend class ActionEngine;
	void setKey( DBKeyValue key )
	{
		m_key = key;
	}

	DBKeyValue m_key;
};




class ActionEngine
{
public:

	ActionEngine(void) : m_next_key(0)
	{
	}

	void clear(void)
	{
		m_actions.clear();
		m_pending_actions.clear();
		m_action_db.clear();
		m_next_key = 0;
	}

	bool empty(void) const
	{
		return (m_actions.empty() && m_pending_actions.empty());
	}

	void addAction( boost::shared_ptr< BaseAction > );
	void advance( GameTimeCoordinate total, GameTimeCoordinate step, AGameModel& );

	boost::shared_ptr< BaseAction > getAction( DBKeyValue key ) const;
	bool actionExists( DBKeyValue key ) const
	{
		return m_action_db.end() != m_action_db.find(key);
	}

	GameTimeCoordinate shortestTimeRemaining(void) const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & m_actions;
		ar & m_pending_actions;
		ar & m_action_db;
		ar & m_next_key;
	}

private:

	ActionEngine( const ActionEngine& );
	ActionEngine& operator= ( const ActionEngine& );

	typedef std::deque< boost::shared_ptr< BaseAction > > Actions;
	Actions m_actions;
	std::vector< boost::shared_ptr< BaseAction > > m_pending_actions;

	typedef std::map< DBKeyValue, boost::shared_ptr< BaseAction > > ActionDB;
	ActionDB m_action_db;
	DBKeyValue m_next_key;

	void addPendingActions( void );
};


}


#endif
