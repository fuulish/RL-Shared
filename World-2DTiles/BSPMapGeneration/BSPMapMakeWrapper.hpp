#ifndef RL_SHARED_BSP_MAP_MAKE_WRAPPER_HPP
#define	RL_SHARED_BSP_MAP_MAKE_WRAPPER_HPP



#include "DBKeyValue.hpp"
#include <boost/shared_ptr.hpp>





namespace RL_shared
{






class MakeDefaultTiledRepresentation
{
public:

	enum DoorType
	{
		NORMAL = 0,
		REMOVE_WALL, 

		NUM_DOOR_TYPES
	};

	enum DoorOri
	{
		X_ALIGNED,	//N-S facing door; walls to E and W
		Z_ALIGNED	//E-W facing door; walls to N and S
	};

	MakeDefaultTiledRepresentation();

	virtual ~MakeDefaultTiledRepresentation()
	{
	}

	virtual void initialise(int size_x, int size_z);
	virtual void makeRoom(int room_key, int N, int S, int E, int W);
	virtual void makeDoor(int door_key, int room_key_1, int room_key_2, DoorType type, DoorOri ori, int N, int S, int E, int W);

	///Called once for each room after generation has finished. 
	///NOTE: Room extents form a half-open range. N and E are open.
	virtual void makeRoom(DBKeyValue zone_key, int room_key, int N, int S, int E, int W) = 0;
	///Called once for each door after generation has finished. 
	///NOTE: Door extents form a half-open range. N and E are open.
	virtual void makeDoor(DBKeyValue zone_key, int door_key, int room_key_1, int room_key_2, DoorType type, DoorOri ori, int N, int S, int E, int W) = 0;

	virtual void writeWallTile(int x, int z) = 0;
	virtual void writeRoomTile(int x, int z, int room_key, int N, int S, int E, int W) = 0;
	virtual void writeDoorTile(int x, int z, int door_key, int room_key_1, int room_key_2, DoorType type, DoorOri ori, int N, int S, int E, int W) = 0;

private:

	DBKeyValue m_zone_key;
};





namespace MapStyles
{
    enum MapStyle
    {
        RoomMess, 
        CorridorMess, 
		HFrame, 
		IFrame, 
		Airlocks
    };
}


void BSPMapGenerator( 
	MakeDefaultTiledRepresentation&, 
	boost::uint16_t map_x_dim, boost::uint16_t map_z_dim, 
	boost::uint16_t num_divisions, 
	MapStyles::MapStyle style
	);





}





#endif
