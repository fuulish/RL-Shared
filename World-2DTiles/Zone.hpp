#ifndef	RL_SHARED_ZONE_HPP
#define	RL_SHARED_ZONE_HPP


#include "WorldObjects/Types.hpp"
#include "DBKeyValue.hpp"
#include <list>
#include <vector>


namespace RL_shared
{


class World;

typedef std::list< DBKeyValue > ObjectList;



class Zone
{
public:

	Zone(DBKeyValue key, boost::uint16_t size_x, boost::uint16_t size_z);

	DBKeyValue key(void) const	{ return m_key; }
	boost::uint16_t sizeX(void) const	{ return m_size_x; }
	boost::uint16_t sizeZ(void) const	{ return m_size_z; }
	bool isWithin(int x, int z) const	{ return within(x,z); }

	TerrainType terrainAt(int x, int z) const;
	TerrainType recordedTerrainAt(int x, int z) const;
	WorldObjectType recordedObjectAt(int x, int z) const;
	const ObjectList& objectsAt(int x, int z) const; //throws std::runtime_error

	void putTerrain(TerrainType, int x, int z);
	void recordTerrain(int x, int z);
	void recordTerrain(int x, int z, TerrainType);
	void recordObject(WorldObjectType, int x, int z);


    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & m_key;
		ar & m_size_x;
		ar & m_size_z;
		ar & m_space;
	}

	//Default constructor intended only for serialization use.
	Zone(void) : m_key(INVALID_KEY), m_size_x(0), m_size_z(0)
	{
	}

private:

	DBKeyValue	m_key;

	boost::uint16_t m_size_x;
	boost::uint16_t m_size_z;

	struct Location
	{
		TerrainType terrain;
		TerrainType recorded_terrain;
		WorldObjectType recorded_object;
		ObjectList objects;

		Location(void) : terrain(0), recorded_terrain(0), recorded_object(0)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & terrain;
			ar & recorded_terrain;
			ar & recorded_object;
			ar & objects;
		}
	};
	std::vector< Location > m_space; //was using boost::scoped_array, but it is not supported by boost::serialization. 


	Location& at(int x, int z);
	const Location& at(int x, int z) const;
	bool within(int x, int z) const;

	Zone(const Zone&);
	Zone& operator=(Zone&);

	friend class World;
	friend class WorldObject;
	bool putObject(World&, DBKeyValue, int x, int z); //throws World::NoObjectWithKey
	bool takeObject(World&, DBKeyValue, int x, int z); //throws World::NoObjectWithKey
};


}


#endif
