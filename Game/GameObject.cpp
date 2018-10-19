#include "GameObject.hpp"
#include "World-2DTiles/World.hpp"
#include "World-2DTiles/Zone.hpp"
#include "GameModel.hpp"
#include <boost/foreach.hpp>



namespace RL_shared
{


void GameObject::notifyTimeStep( GameModel&, GameTimeCoordinate )
{
}

bool GameObject::removeMe(GameModel&) const
{
	return false;
}

bool GameObject::isFastMovingObject(void) const
{
	return false;
}

bool GameObject::alwaysProcess(void) const
{
	return false;
}

bool GameObject::canOverlap( const GameModel& model, const WorldObject::WorldLocation& loc ) const
{
	const World& world( model.world() );
	if (!world.zoneExists(loc.zone))
		return true;

	const Zone& zone( world.zone(loc.zone) );

	return canOverlap( world, zone, loc.x, loc.z );
}
bool GameObject::canOverlap( const World& world, const Zone& zone, int x, int z ) const
{
	if (!zone.isWithin(x,z))
		return false;
	if (!canOverlap( zone.terrainAt(x, z) ))
		return false;

	const ObjectList& objects( zone.objectsAt(x, z) );
	BOOST_FOREACH( DBKeyValue obj_key, objects )
	{
		if (world.objectExists(obj_key))
		{
			const WorldObject& obj( world.object(obj_key) );
			if (!canOverlap(obj))
				return false;
		}
	}

	return true;
}


}

