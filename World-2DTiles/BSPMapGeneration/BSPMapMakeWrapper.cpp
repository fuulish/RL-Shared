#include "BSPMapMakeWrapper.hpp"
#include "BSPMapMake.hpp"





namespace RL_shared
{

namespace
{
	double PRandom(void)
	{
		return (std::max)(0.0, (std::min)(1.0, ((static_cast<double>(rand()) * (1.0/(double)RAND_MAX))) ) );
	}
}


class BSPMapMakerCallbacksImpl : public IBSPMapMakerCallbacks
{
public:

	BSPMapMakerCallbacksImpl( MakeDefaultTiledRepresentation& make_rep )
	: m_make_rep(make_rep), m_maker(0)
	{
	}

	void setBSPMapMake( BSPMapMake* maker )
	{
		m_maker = maker;
	}


	virtual bool splitAllowed(int roomindex, int N, int S, int E, int W) ;
	virtual bool makeDoorAllowed(int room1index, int room2index) ;
	virtual void splitRoomCallback(int originalroomindex, int newsideroomindex, int middleroomindex) ;
	virtual void roomFromDrawDoorCallback(int newroomindex, int room1index, int room2index, int doortype) ;
	virtual void mirrorRoomCallback(int originalroomindex, int newroomindex) ;
	virtual int numDoorsForRoom(int roomindex, int N, int S, int E, int W) ;
	virtual int chooseDoorType(int room1index, int room2index, int N1, int S1, int E1, int W1, int N2, int S2, int E2, int W2);
	virtual void drawRoom(int roomindex, int N, int S, int E, int W);
	virtual void drawDoor(int doorindex, int roomindex1, int roomindex2, int type, int ori, int N, int S, int E, int W);

private:

	MakeDefaultTiledRepresentation& m_make_rep;
	BSPMapMake* m_maker;
};

bool BSPMapMakerCallbacksImpl::splitAllowed(int roomindex, int N, int S, int E, int W)
{
	int Xsize(E-W), Zsize(N-S);

	if ((Xsize <= 2) || (Zsize <= 2))
		return false; //keep long narrow rooms intact to form corridors

	return true;
}
bool BSPMapMakerCallbacksImpl::makeDoorAllowed(int room1index, int room2index)
{
	return true;
}
void BSPMapMakerCallbacksImpl::splitRoomCallback(int originalroomindex, int newsideroomindex, int middleroomindex)
{
}
void BSPMapMakerCallbacksImpl::roomFromDrawDoorCallback(int newroom, int room1index, int room2index, int doortype)
{
}
void BSPMapMakerCallbacksImpl::mirrorRoomCallback(int originalroomindex, int newroomindex)
{
}
int BSPMapMakerCallbacksImpl::numDoorsForRoom(int roomindex, int N, int S, int E, int W)
{
    if ( ((2 == (N-S))||(2 == (E-W))) && (((N-S)>6)||((E-W)>6)) )
        return 2;

	return 1 + (9 == (roomindex % 10));
}
int BSPMapMakerCallbacksImpl::chooseDoorType(int room1index, int room2index, int N1, int S1, int E1, int W1, int N2, int S2, int E2, int W2)
{
	int doortype;

	int NSoverlap = (std::min)(N1, N2) - (std::max)(S1, S2);
	int EWoverlap = (std::min)(E1, E2) - (std::max)(W1, W2);
    int r1mw = (std::min)(N1-S1, E1-W1);
    int r2mw = (std::min)(N2-S2, E2-W2);

	if ((1 == NSoverlap) || (1 == EWoverlap))
		return 0;
	
	if ( (2 == NSoverlap) || (2 == EWoverlap) )
        if ((2 == r1mw) && (2 == r2mw))
            return 2;

    if ( (NSoverlap > (2*r1mw)) || (NSoverlap > (2*r2mw)) || (EWoverlap > (2*r1mw)) || (EWoverlap > (2*r2mw)) )
        return 0;

    if ( (NSoverlap == (N1-S1)) || (EWoverlap == (E1-W1)) )
        if ( (NSoverlap == (N2-S2)) || (EWoverlap == (E2-W2)) )
            return 0;

	int randy = rand() & 15;
	//doortype = randy < 11 ? 0 : (randy < 14) || ((std::max)(NSoverlap, EWoverlap) < 4) ? 2 : 1;
	doortype = randy < 11 ? 0 : 2;

	return doortype;
}
void BSPMapMakerCallbacksImpl::drawRoom(int roomindex, int N, int S, int E, int W)
{
	m_make_rep.makeRoom(roomindex, N, S, E, W);
}
void BSPMapMakerCallbacksImpl::drawDoor(int doorindex, int roomindex1, int roomindex2, int type, int ori, int N, int S, int E, int W)
{
	MakeDefaultTiledRepresentation::DoorType door_type( MakeDefaultTiledRepresentation::NORMAL );
	switch (type)
	{
		default:
		case 0: break;
		case 1: 
		case 2: door_type = MakeDefaultTiledRepresentation::REMOVE_WALL; break; 
	};
	m_make_rep.makeDoor(doorindex, roomindex1, roomindex2, door_type, 
		0 == ori ? MakeDefaultTiledRepresentation::X_ALIGNED : MakeDefaultTiledRepresentation::Z_ALIGNED, 
		N, S, E, W);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void BSPMapGenerator( 
	MakeDefaultTiledRepresentation& make_rep, 
	boost::uint16_t map_x_dim, boost::uint16_t map_z_dim, 
	boost::uint16_t num_divisions, 
	MapStyles::MapStyle style
)
{
	BSPMapMakerCallbacksImpl params( make_rep );
	BSPMapMake maker( &params );
	params.setBSPMapMake( &maker );
	maker.setUseSplitAlternator(true);

	maker.setMaxRoomSize((map_x_dim + map_z_dim) / 4);
	make_rep.initialise(map_x_dim, map_z_dim);

	if (MapStyles::CorridorMess == style)
	{
		maker.setRoomSize(4);
		maker.setHollowWalls(true);
		maker.setHollowWallThickness(4);
		int firstroom(maker.makeRoom(map_z_dim - 1, 1, 1, map_x_dim - 1));
		maker.divideRooms(firstroom, num_divisions / 4);
		maker.setRoomSize(3);
		maker.setHollowWalls(false);
		maker.setWallThickness(1);
		maker.divideRooms(firstroom, num_divisions / 2);
		maker.connectRooms(firstroom, 1.0);
		maker.generateDrawDoors(0, false);
		maker.drawRooms();
		maker.drawDoors();
	}
	else if (MapStyles::RoomMess == style)
	{
		maker.setRoomSize(2);
		int firstroom(maker.makeRoom(map_z_dim - 1, 1, 1, map_x_dim - 1));
		maker.divideRooms(firstroom, num_divisions);
		maker.connectRooms(firstroom, 1.0);
		maker.generateDrawDoors(0, false);
		maker.drawRooms();
		maker.drawDoors();
	}
	else if (MapStyles::HFrame == style)
	{
		maker.setRoomSize(3);
		int firstroom( maker.makeRoom(map_z_dim - 1, 1, 1, 3) ); // western N-S corridor
		maker.makeRoom(map_z_dim - 1, map_x_dim - 3, 1, map_x_dim - 1); // eastern N-S corridor
		maker.makeRoom((map_z_dim / 2) + 1, 4, (map_z_dim / 2) - 1, map_x_dim - 4); // E-W corridor
		int north_area(maker.makeRoom(map_z_dim - 1, 4, (map_z_dim / 2) + 2, map_x_dim - 4));
		maker.divideRooms(north_area, num_divisions / 2);
		int south_area(maker.makeRoom((map_z_dim / 2) - 2, 4, 1, map_x_dim - 4));
		maker.divideRooms(south_area, num_divisions / 2);
		maker.connectRooms(firstroom, 1.0);
		maker.generateDrawDoors(0, false);
		maker.drawRooms();
		maker.drawDoors();
	}
	else if (MapStyles::IFrame == style)
	{
		maker.setRoomSize(3);
		int firstroom( maker.makeRoom(map_z_dim - 1, 1, map_z_dim - 3, map_x_dim - 1) ); // northern E-W corridor
		maker.makeRoom(3, 1, 1, map_x_dim - 1); // southern E-W corridor
		maker.makeRoom(map_z_dim - 4, (map_x_dim / 2) - 1, 4, (map_x_dim / 2) + 1);
		int west_area(maker.makeRoom(map_z_dim - 4, 1, 4, (map_x_dim / 2) - 2));
		maker.divideRooms(west_area, num_divisions / 2);
		int east_area(maker.makeRoom(map_z_dim - 4, (map_x_dim / 2) + 2, 4, map_x_dim - 1));
		maker.divideRooms(east_area, num_divisions / 2);
		maker.connectRooms(firstroom, 1.0);
		maker.generateDrawDoors(0, false);
		maker.drawRooms();
		maker.drawDoors();
	}
	else if (MapStyles::Airlocks == style)
	{
		maker.setWallThickness(4);
		maker.setRoomSize(8);
		maker.setDoorSize(4);
		int firstroom(maker.makeRoom(map_z_dim - 1, 1, 1, map_x_dim - 1));
		maker.divideRooms(firstroom, num_divisions / 16);
		maker.connectRooms(firstroom, 1.0);
		maker.generateDrawDoors(0, false);
		maker.setWallThickness(1);
		maker.setRoomSize(2);
		maker.setDoorSize(1);
		maker.drawDoorsToRooms();
		maker.divideRooms(firstroom, num_divisions / 2);
		maker.connectRooms(firstroom, 1.0);
		maker.generateDrawDoors(0, false);
		maker.drawRooms();
		maker.drawDoors();
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



MakeDefaultTiledRepresentation::MakeDefaultTiledRepresentation()
: m_zone_key(INVALID_KEY)
{
}

void MakeDefaultTiledRepresentation::initialise(int size_x, int size_z)
{
	for (int dz = 0; dz < size_z; ++dz)
	{
		for (int dx = 0; dx < size_x; ++dx)
		{
			this->writeWallTile(dx, dz);
		}
	}
}

void MakeDefaultTiledRepresentation::makeRoom(int room_key, int N, int S, int E, int W)
{
	this->makeRoom( m_zone_key, room_key, N, S, E, W );

	for (int dz = S; dz < N; ++dz)
	{
		for (int dx = W; dx < E; ++dx)
		{
			this->writeRoomTile(dx, dz, room_key, N, S, E, W);
		}
	}
}

void MakeDefaultTiledRepresentation::makeDoor(
	int door_key, int room_key_1, int room_key_2, 
	DoorType type, DoorOri ori, int N, int S, int E, int W
	)
{
	this->makeDoor( m_zone_key, door_key, room_key_1, room_key_2, type, ori, N, S, E, W );

	for (int dz = S; dz < N; ++dz)
	{
		for (int dx = W; dx < E; ++dx)
		{
			this->writeDoorTile(dx, dz, door_key, room_key_1, room_key_2, type, ori, N, S, E, W);
		}
	}
}






}
