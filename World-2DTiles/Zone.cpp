#include "Zone.hpp"
#include "WorldObjects/WorldObject.hpp"
#include "World.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <algorithm>



namespace RL_shared
{


Zone::Zone(DBKeyValue key, boost::uint16_t size_x, boost::uint16_t size_z)
: m_key(key), m_size_x(size_x), m_size_z(size_z)
{
	m_space.resize( size_x*size_z );
}

Zone::Location& Zone::at(int x, int z)
{
	return m_space[x+(m_size_x*z)];
}
const Zone::Location& Zone::at(int x, int z) const
{
	return m_space[x+(m_size_x*z)];
}
bool Zone::within(int x, int z) const
{
	return (x >= 0) && (z >= 0) && (x < m_size_x) && (z < m_size_z);
}

TerrainType Zone::terrainAt(int x, int z) const
{
	if (!within(x,z))
		return 0;

	return at(x,z).terrain;
}
TerrainType Zone::recordedTerrainAt(int x, int z) const
{
	if (!within(x,z))
		return 0;

	return at(x,z).recorded_terrain;
}
WorldObjectType Zone::recordedObjectAt(int x, int z) const
{
	if (!within(x,z))
		return 0;

	return at(x,z).recorded_object;
}
const ObjectList& Zone::objectsAt(int x, int z) const
{
	if (!within(x,z))
		throw std::runtime_error("Out of zone: " + boost::lexical_cast<std::string>(x) + ", " + boost::lexical_cast<std::string>(z));

	return at(x,z).objects;
}

void Zone::putTerrain(TerrainType type, int x, int z)
{
	if (!within(x,z))
		return;

	at(x,z).terrain = type;
}
void Zone::recordTerrain(int x, int z)
{
	if (!within(x,z))
		return;

	Location& a_space = at(x,z);
	a_space.recorded_terrain = a_space.terrain;
}
void Zone::recordTerrain(int x, int z, TerrainType value)
{
	if (!within(x,z))
		return;

	Location& a_space = at(x,z);
	a_space.recorded_terrain = value;
}
void Zone::recordObject(WorldObjectType type, int x, int z)
{
	if (!within(x,z))
		return;

	at(x,z).recorded_object = type;
}

bool Zone::putObject(World& world, DBKeyValue obj_key, int x, int z)
{
	if (!within(x,z))
		return false;

	WorldObject& obj( world.object( obj_key ) );

	Location& target_loc( at(x,z) );

	if (!obj.canOverlap( target_loc.terrain ))
		return false;

	ObjectList& objects( target_loc.objects );
	BOOST_FOREACH( DBKeyValue test_obj_key, objects )
	{
		try
		{
			WorldObject& test_obj( world.object( test_obj_key ) );
			if (!(test_obj.canOverlap(obj) && obj.canOverlap(test_obj)))
				return false;
		}
		catch( World::NoObjectWithKey& )
		{
		}
	}

	WorldObject::WorldLocation source_loc = obj.location();
	if (WorldObject::WorldLocation::INVALID_ZONE != source_loc.zone)
	{
		world.zone(source_loc.zone).takeObject(world, obj_key, source_loc.x, source_loc.z);
	}
	objects.push_back( obj_key );
	WorldObject::WorldLocation loc_world(m_key, x, z);
	obj.setLocation( loc_world );
	return true;
}
bool Zone::takeObject(World& world, DBKeyValue obj_key, int x, int z)
{
	if (!within(x,z))
		return false;

	ObjectList& objects( at(x,z).objects );
	ObjectList::iterator finder = std::find( objects.begin(), objects.end(), obj_key );
	if (objects.end() != finder)
	{
		objects.erase(finder);

		if (world.objectExists(obj_key))
		{
			WorldObject& obj( world.object( obj_key ) );
			WorldObject::WorldLocation loc = obj.location();

			if ((loc.zone == m_key) && (loc.x == x) && (loc.z == z)) {
				WorldObject::WorldLocation loc_world;
				obj.setLocation(loc_world);
			}
		}

		finder = std::find( objects.begin(), objects.end(), obj_key );
		while (objects.end() != finder)
			objects.erase(finder);

		return true;
	}

	return false;
}




}

