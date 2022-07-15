#ifndef RL_SHARED_ACTOR_HPP
#define RL_SHARED_ACTOR_HPP


#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>
#include "WorldObject.hpp"
#include "ActionEngine/ActionEngine.hpp"
#include <vector>


namespace RL_shared
{



class Actor : public WorldObject
{
public:

	typedef std::vector< DBKeyValue > Actions;

	const Actions& actions(void) const	{ return m_actions; }
 
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & boost::serialization::base_object<WorldObject>(*this);
		ar & m_actions;
	}


private:

	Actions m_actions;


	friend class ActionEngine;
	Actions& ncActions(void)	{ return m_actions; }

	void removeDeadActions( const ActionEngine& );
};


}


#endif
