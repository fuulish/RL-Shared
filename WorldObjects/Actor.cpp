#include "Actor.hpp"
#include <boost/foreach.hpp>


namespace RL_shared
{


void Actor::removeDeadActions(const ActionEngine& engine)
{
	bool repeat(true);
	while (repeat)
	{
		repeat = false;

		Actions::iterator iter( m_actions.begin() );
		for (; iter != m_actions.end(); ++iter)
		{
			if (!engine.actionExists(*iter))
			{
				m_actions.erase( iter );
				repeat = true;
				break;
			}
		}
	}
}



}
