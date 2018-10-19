#ifndef RL_SHARED_I_GAME_MODEL_HPP
#define RL_SHARED_I_GAME_MODEL_HPP



#include "AGameModel.hpp"
#include "DBKeyValue.hpp"
#include <boost/cstdint.hpp>



namespace RL_shared
{


typedef boost::int32_t GameTimeCoordinate;

class World;

class IGameModel : public AGameModel
{
public:

	//For turn-based views.
	//Advances until model decides that something has "happened". 
	//Returns true if new input required, false otherwise. 
	//dt is filled with the amount of game time advanced.
	virtual bool advance_turn( GameTimeCoordinate& dt ) = 0;

	//For real-time views.
	//Advances by time step given. 
	virtual void advance_step( GameTimeCoordinate time_step ) = 0;


	virtual World& world(void) = 0;

	virtual void registerObjectInZone( RL_shared::DBKeyValue obj_key, RL_shared::DBKeyValue zone_key ) = 0;
	virtual void unRegisterObjectInZone( RL_shared::DBKeyValue obj_key, RL_shared::DBKeyValue zone_key ) = 0;
};


}




#endif
