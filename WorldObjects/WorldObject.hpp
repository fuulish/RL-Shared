#ifndef RL_SHARED_WORLD_OBJECT_HPP
#define	RL_SHARED_WORLD_OBJECT_HPP



#include "DBKeyValue.hpp"
#include "Types.hpp"



namespace RL_shared
{


class IGameModel;


class WorldObject
{
public:

	WorldObject(void) : m_key(INVALID_KEY)
	{
	}
	virtual ~WorldObject()
	{
	}

	DBKeyValue key(void) const	{ return m_key; }

	struct WorldLocation
	{
		enum {INVALID_ZONE = INVALID_KEY};
		DBKeyValue zone;
		int x, z;
		WorldLocation(void) : zone(INVALID_ZONE), x(0), z(0)
		{
		}
		WorldLocation(DBKeyValue in_zone, int in_x, int in_z) : zone(in_zone), x(in_x), z(in_z)
		{
		}
		bool operator == (const WorldLocation& rhs) const
		{
			return (rhs.zone == zone) && (rhs.x == x) && (rhs.z == z);
		}
		bool operator != (const WorldLocation& rhs) const
		{
			return !(*this == rhs);
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & zone;
			ar & x;
			ar & z;
		}
	};
	WorldLocation location(void) const			{ return m_loc; }
	void setLocation(WorldLocation new_loc)	{ m_loc = new_loc; }

	//Uses virtual dispatch. Do not call from constructor.
	bool moveTo(IGameModel&, WorldLocation new_loc, bool instant);


	virtual bool canOverlap(const WorldObject&) const = 0;
	virtual bool canOverlap(TerrainType) const = 0;


    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & m_key;
		ar & m_loc;
	}

private:

	DBKeyValue m_key;
	WorldLocation m_loc;


	friend class World;
	void setKey( DBKeyValue new_key )	{ m_key = new_key; }

	WorldObject(WorldObject&);
	WorldObject& operator=(WorldObject&);

	virtual void onMove(IGameModel&)	{}
};


};



#endif
