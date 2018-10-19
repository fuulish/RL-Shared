#include "WorldObject.hpp"
#include "World-2DTiles/World.hpp" 
#include "World-2DTiles/Zone.hpp" 
#include "Abstract/IGameModel.hpp"



namespace RL_shared
{



bool WorldObject::moveTo(IGameModel& model, WorldLocation new_loc, bool /*instant*/)
{
	World& world( model.world() );

	WorldLocation cur_loc( location() );

	if (WorldLocation::INVALID_ZONE == new_loc.zone)
	{
		if (WorldLocation::INVALID_ZONE != cur_loc.zone)
		{
			bool moved( world.zone(cur_loc.zone).takeObject(world, key(), cur_loc.x, cur_loc.z) );
			if (moved)
			{
				model.unRegisterObjectInZone(key(), cur_loc.zone);
				onMove(model);
			}
			return moved;
		}
		return true;
	}

	bool moved( world.zone(new_loc.zone).putObject(world, key(), new_loc.x, new_loc.z) );
	if (moved)
	{
		if (new_loc.zone != cur_loc.zone)
		{
			model.unRegisterObjectInZone(key(), cur_loc.zone);
			model.registerObjectInZone(key(), new_loc.zone);
		}

		onMove(model);
	}
	return moved;
}



}
