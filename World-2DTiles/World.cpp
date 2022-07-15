#include "World.hpp"
#include "Zone.hpp"
#include "WorldObjects/WorldObject.hpp"
#include "Include/assert.hpp"



namespace RL_shared
{


void World::clear(void)
{
	m_zones.clear();
	m_world_objects.clear();
	m_active_world_objects.clear();
}

Zone& World::zone(DBKeyValue key)
{
	if (!zoneExists(key))
		throw NoObjectWithKey(key);
	return *(m_zones[key]);
}

const Zone& World::zone(DBKeyValue key) const
{
	if (!zoneExists(key))
		throw NoObjectWithKey(key);
	return *(m_zones[key]);
}

WorldObject& World::object(DBKeyValue key)
{
	if (!objectExists(key))
		throw NoObjectWithKey(key);
	return *(m_world_objects[key]);
}

const WorldObject& World::object(DBKeyValue key) const
{
	if (!objectExists(key))
		throw NoObjectWithKey(key);
	return *(m_world_objects[key]);
}

WorldObjectPtr World::objectPtr(DBKeyValue key)
{
	if ((size_t)key < m_world_objects.size())
	{
		return m_world_objects[key];
	}
	return WorldObjectPtr();
}

WorldObjectCPtr World::objectPtr(DBKeyValue key) const
{
	if ((size_t)key < m_world_objects.size())
	{
		return m_world_objects[key];
	}
	return WorldObjectCPtr();
}

DBKeyValue World::createZone(boost::uint16_t size_x, boost::uint16_t size_z)
{
	DBKeyValue new_key( m_zones.size() );
	ASSERT( new_key >= 0 );
	ZonePtr new_zone( new Zone(new_key, size_x, size_z) );
	m_zones.push_back( new_zone );
	ASSERT( m_zones.size() == (new_key+1) );
	ASSERT( m_zones[ new_key ] == new_zone );
	return new_key;
}

DBKeyValue World::addWorldObject(WorldObjectPtr new_obj)
{
	ASSERT( new_obj );
	ASSERT( INVALID_KEY == new_obj->key() );
	DBKeyValue new_key( m_world_objects.size() );
	ASSERT( new_key >= 0 );
	new_obj->setKey( new_key );
	m_world_objects.push_back( new_obj );
	m_active_world_objects.push_back( new_obj );
	return new_key;
}

bool World::zoneExists(DBKeyValue key) const
{
	return (INVALID_KEY != key) && ((size_t)key < m_zones.size()) && (m_zones[key]);
}
bool World::objectExists(DBKeyValue key) const
{
	return (INVALID_KEY != key) && ((size_t)key < m_world_objects.size()) && (m_world_objects[key]);
}

void World::removeZone(DBKeyValue key)
{
	if ((size_t)key < m_zones.size())
	{
		m_zones[key] = ZonePtr();

		WorldObjects::iterator end_iter( m_world_objects.end() );
		for (WorldObjects::iterator iter( m_world_objects.begin() ); iter != end_iter; ++iter )
		{
			if ((*iter) && ((*iter)->location().zone == key))
			{
				*iter = WorldObjectPtr();
			}
		}

		//Leave alone to reduce chance of a new object being mistaken for the old one due to having the same key
		//if ((m_zones.size()-1) == key)
		//{
		//	m_zones.pop_back();
		//}
	}
}
void World::removeWorldObject(DBKeyValue key)
{
	if ((size_t)key < m_world_objects.size())
	{
		WorldObjectPtr obj( m_world_objects[key] );

		WorldObject::WorldLocation obj_loc( obj->location() );
		DBKeyValue obj_zone( obj_loc.zone );
		if (WorldObject::WorldLocation::INVALID_ZONE != obj_zone)
		{
			if (zoneExists(obj_zone))
			{
				zone(obj_zone).takeObject(*this, obj->key(), obj_loc.x, obj_loc.z);
			}
		}

		m_world_objects[key] = WorldObjectPtr();

		WorldObjects::iterator find_obj( std::find( m_active_world_objects.begin(), m_active_world_objects.end(), obj ) );
		if (m_active_world_objects.end() != find_obj)
		{
			std::swap( *find_obj, m_active_world_objects.back() );
			m_active_world_objects.pop_back();
		}

		//Leave alone to reduce chance of a new object being mistaken for the old one due to having the same key
		//if ((m_world_objects.size()-1) == key)
		//{
		//	m_world_objects.pop_back();
		//}
	}
}


}
