#ifndef	RL_SHARED_WORLD_HPP
#define	RL_SHARED_WORLD_HPP



#include "DBKeyValue.hpp"
#include <boost/shared_ptr.hpp>
#include <vector>



namespace RL_shared
{


class Zone;
class WorldObject;
typedef boost::shared_ptr< WorldObject > WorldObjectPtr;
typedef boost::shared_ptr< const WorldObject > WorldObjectCPtr;
typedef boost::shared_ptr< Zone > ZonePtr;


class World
{
public:

	World(void)
	{
	}

	class NoObjectWithKey : std::runtime_error
	{
	public:
		NoObjectWithKey(DBKeyValue key) : std::runtime_error( (std::string("No object with key ") + key.asString()).c_str() ) {
		}
	};

	typedef std::vector< ZonePtr > Zones;
	typedef std::vector< WorldObjectPtr > WorldObjects;

	void clear(void);

	bool zoneExists(DBKeyValue) const;
	Zone& zone(DBKeyValue); //throws NoObjectWithKey
	const Zone& zone(DBKeyValue) const; //throws NoObjectWithKey
	bool objectExists(DBKeyValue) const;
	WorldObject& object(DBKeyValue); //throws NoObjectWithKey
	const WorldObject& object(DBKeyValue) const; //throws NoObjectWithKey
	WorldObjectPtr objectPtr(DBKeyValue);
	WorldObjectCPtr objectPtr(DBKeyValue) const;

	DBKeyValue createZone(boost::uint16_t size_x, boost::uint16_t size_z);
	void removeZone(DBKeyValue);
	DBKeyValue addWorldObject(WorldObjectPtr);
	void removeWorldObject(DBKeyValue);

	const Zones& zones(void) const			{ return m_zones; }
	const WorldObjects& objects(void) const	{ return m_active_world_objects; }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & m_zones;
		ar & m_world_objects;
		ar & m_active_world_objects;
	}

private:

	Zones m_zones;

	WorldObjects m_world_objects;
	WorldObjects m_active_world_objects;


	World(World&);
	World& operator=(World&);
};



}


#endif
