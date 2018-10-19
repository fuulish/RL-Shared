#ifndef RL_SHARED_KEY_PRESS_HPP
#define	RL_SHARED_KEY_PRESS_HPP



#include "Abstract/AUserInputItem.hpp"
#include "Console/Console.hpp"



namespace RL_shared
{


class KeyPress : public AUserInputItem
{
public:
	KeyCode		value;

	KeyPress(const KeyCode& in_key) : value(in_key)
	{
	}
};


}



#endif
