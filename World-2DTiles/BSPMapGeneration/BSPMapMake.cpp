
#include "BSPMapMake.hpp"
#include <utility>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <math.h>







namespace RL_shared
{



namespace
{

//Random from 0.0 to 1.0
double PRandom(void)
{
	return (std::max)(0.0, (std::min)(1.0, ((static_cast<double>(rand()) * (1.0/(double)RAND_MAX))) ) );
}

//Random from -1.0 to 1.0
double Random(void)
{
	return (std::max)(-1.0, (std::min)(1.0, (PRandom() - 0.5) * 2.0 ) );
}

int FLOOR( double f )
{
	return static_cast<int>(floor(f));
}

}











/////////////////////////////////////////////////////////////////////////////////


enum { MAXROOMS = 1024, MAXDOORS = 1024 };


/////////////////////////////////////////////////////////////////////////////////


struct Room
{
	int		extents[4];	//{N,S,E,W} in world grid coords

	Room(void)
	{
		extents[0] = extents[1] = extents[2] = extents[3] = 0;
	}
	Room(int N, int S, int E, int W)
	{
		extents[0] = N;
		extents[1] = S;
		extents[2] = E;
		extents[3] = W;
	}

    struct Size{ int ns, ew; };

    Size size() const
    {
        Size sz;
        sz.ns = extents[0]-extents[1];
        sz.ew = extents[2]-extents[3];
        sz.ns = (sz.ns < 0 ? -sz.ns : sz.ns);
        sz.ew = (sz.ew < 0 ? -sz.ew : sz.ew);
        return sz;
    }
    int longSize() const
    {
        Size sz( size() );
        return sz.ns > sz.ew ? sz.ns : sz.ew;
    }
    int shortSize() const
    {
        Size sz( size() );
        return sz.ns < sz.ew ? sz.ns : sz.ew;
    }
};

struct Door
{
	int room[2];

	Door(void)
	{
		room[0] = room[1] = -1;
	}
};


/////////////////////////////////////////////////////////////////////////////////


struct RoomStore
{
	Room	m_room_pool[MAXROOMS];
	int		m_next_room;

	RoomStore(void) : m_next_room(0)
	{
	}

	void reset(void)
	{
		m_next_room = 0;
	}

	int getRoom(void)
	{
		if (m_next_room >= MAXROOMS)
			return -1;
		return m_next_room++;
	}
};


struct DoorStore
{
	Door	m_door_pool[MAXDOORS];
	int		m_next_door;

	DoorStore(void) : m_next_door(0)
	{
	}

	void reset(void)
	{
		m_next_door = 0;
	}

	int getDoor(void)
	{
		if (m_next_door >= MAXDOORS)
			return -1;
		return m_next_door++;
	}
};


/////////////////////////////////////////////////////////////////////////////////



struct DrawDoor
{
	int	door;
	int NSoverlap;
	int EWoverlap;

	int	doortype; //0 == small, 1 == large, 2 == remove wall
	int doorpos;
	int doorsize;

	int extents[4];	//N, S, E, W

	DrawDoor(void)
	{
		door = -1;
		doortype = doorpos = doorsize = 0;
		extents[0] = extents[1] = extents[2] = extents[3] = -1;
	}

	void setDoor(int idoor, DoorStore&, RoomStore&);
	void chooseDoorType(DoorStore&, RoomStore&, IBSPMapMakerCallbacks*);
	void chooseDoorPosition(int doorsize_min, int dooroffs_min);
	void calculateExtents(DoorStore&, RoomStore&, int wall_thickness);
};


struct DrawDoorStore
{
	DrawDoor	m_draw_door_pool[MAXDOORS];
	int			m_next_draw_door;

	DrawDoorStore(void) : m_next_draw_door(0)
	{
	}

	void reset(void)
	{
		m_next_draw_door = 0;
	}

	int getDrawDoor(void)
	{
		if (m_next_draw_door >= MAXDOORS)
			return -1;
		return m_next_draw_door++;
	}
};



/////////////////////////////////////////////////////////////////////////////////



class BSPMapMakeImpl
{
public:

	BSPMapMakeImpl( IBSPMapMakerCallbacks* );

	int makeRoom(int N, int W, int S, int E);
	int mirrorRoomsInX(int firstroom, int lastroom, int X);
	int mirrorRoomsInZ(int firstroom, int lastroom, int Z);
	int divideRooms(int firstroomindex, int numdivides);
	void connectRooms(int firstroomindex, double targetcoverage);
	int generateDrawDoors(int firstdoor, bool smalldoorsonly);
	void drawDoorsToRooms(void);
	void drawRooms(void);
	void drawDoors(void);

	void setHollowWalls(bool b);
	void setHollowWallThickness(int n);
	void setWallThickness(int n);
	void setDoorSize(int n);
	void setDoorOffset(int n);
	void setRoomSize(int n);
	void setUseSplitAlternator(bool b);
	void setSplitAlternator(int i);
	void setMaxRoomSize(int i);

	bool getHollowWalls(void) const;
	int  getHollowWallThickness(void) const;
	int  getWallThickness(void) const;
	int  getDoorSize(void) const;
	int  getDoorOffset(void) const;
	int  getRoomSize(void) const;
	bool getUseSplitAlternator(void) const;
	int  getSplitAlternator(void) const;
	int  getMaxRoomSize(void) const;

private:


	RoomStore m_room_store;
	DoorStore m_door_store;
	DrawDoorStore m_draw_door_store;
	bool m_explored[MAXROOMS];

	IBSPMapMakerCallbacks* m_pCBs;
	int m_wall_thickness;
	int m_doorsize_min;
	int m_dooroffs_min;//minimum offset into wall containing door. Must be <= (m_roomsize_min-m_doorsize_min)/2 or full connectivity might be impossible.
	int m_roomsize_min;
	int m_hollow_wall_thickness;//	= (2*m_wall_thickness)+(2*m_dooroffs_min)+m_doorsize_min;	//this is the minimum it must be to ensure that the "hollow rooms" can all be linked to
	int m_split_alternator;
    int m_max_room_size;
	bool m_generate_hollow_walls;
	bool m_use_split_alternator;

	void checkHollowWallThickness(void);
	void checkDoorOffset(void);
	void clearExplored(void);
	int countReachableRooms(int room);
	bool splitRoom(int roomindex, bool force_horizontal = false, bool force_vertical = false);
	bool roomsAreConnected(int room1, int room2);
	bool roomsAreNeighbours(const Room& room1, const Room& room2);
	int findPotentialDoor(int room);
	bool makeDoor(int room, int connectto);
	void makeDoor(int room);
	bool roomsAreConjoined(int room1, int room2, std::vector<int>& visited) const;
};


BSPMapMakeImpl::BSPMapMakeImpl( IBSPMapMakerCallbacks * pCBs )
: m_pCBs( pCBs ), m_wall_thickness(1), m_doorsize_min(1), m_dooroffs_min(0), m_roomsize_min(2), 
m_hollow_wall_thickness((2*m_wall_thickness)+(2*m_dooroffs_min)+m_doorsize_min), 
m_split_alternator(0), m_max_room_size(128), m_generate_hollow_walls(false), m_use_split_alternator(false)
{
}
















#define	MIN_ROOM_FOR_DOOR	(m_doorsize_min+(2*m_dooroffs_min))



void BSPMapMakeImpl::setHollowWalls(bool b)
{
	m_generate_hollow_walls = b;
}
void BSPMapMakeImpl::setHollowWallThickness(int n)
{
	m_hollow_wall_thickness = n;
}
void BSPMapMakeImpl::checkHollowWallThickness(void)
{
	m_hollow_wall_thickness = (std::max)( m_hollow_wall_thickness, (2*m_wall_thickness)+(2*m_dooroffs_min)+m_doorsize_min );
}
void BSPMapMakeImpl::setWallThickness(int n)
{
	m_wall_thickness = n;
}
void BSPMapMakeImpl::setDoorSize(int n)
{
	m_doorsize_min = n;
}
void BSPMapMakeImpl::setDoorOffset(int n)
{
	m_dooroffs_min = n;
}
void BSPMapMakeImpl::checkDoorOffset(void)
{
	m_dooroffs_min = (std::min)( m_dooroffs_min, (m_roomsize_min-m_doorsize_min)/2 );
}
void BSPMapMakeImpl::setRoomSize(int n)
{
	m_roomsize_min = n;
}
void BSPMapMakeImpl::setMaxRoomSize(int i)
{
	m_max_room_size = i;
}
void BSPMapMakeImpl::setSplitAlternator(int i)
{
	m_split_alternator = i & 1;
}
void BSPMapMakeImpl::setUseSplitAlternator(bool b)
{
	m_use_split_alternator = b;
}


bool BSPMapMakeImpl::getHollowWalls(void) const
{
	return m_generate_hollow_walls;
}
int BSPMapMakeImpl::getHollowWallThickness(void) const
{
	return m_hollow_wall_thickness;
}
int BSPMapMakeImpl::getWallThickness(void) const
{
	return m_wall_thickness;
}
int BSPMapMakeImpl::getDoorSize(void) const
{
	return m_doorsize_min;
}
int BSPMapMakeImpl::getDoorOffset(void) const
{
	return m_dooroffs_min;
}
int BSPMapMakeImpl::getRoomSize(void) const
{
	return m_roomsize_min;
}
int BSPMapMakeImpl::getMaxRoomSize(void) const
{
	return m_max_room_size;
}
bool BSPMapMakeImpl::getUseSplitAlternator(void) const
{
	return m_use_split_alternator;
}
int BSPMapMakeImpl::getSplitAlternator(void) const
{
	return m_split_alternator;
}








/////////////////////////////////////////////////////////////////////////////////









int BSPMapMakeImpl::mirrorRoomsInX(int firstroom, int lastroom, int X)
{
	int lastindex = lastroom;
	for (int mirrorroom = firstroom; mirrorroom <= lastroom; ++mirrorroom)
	{
		Room& room = m_room_store.m_room_pool[mirrorroom];

		int newroomindex = m_room_store.getRoom();

		if (-1 != newroomindex)
		{
			lastindex = newroomindex;
			Room& newroom = m_room_store.m_room_pool[newroomindex];
			newroom.extents[2] = X - room.extents[3];
			newroom.extents[3] = X - room.extents[2];
			newroom.extents[0] = room.extents[0];
			newroom.extents[1] = room.extents[1];

			m_pCBs->mirrorRoomCallback(mirrorroom, newroomindex);
		}
	}
	return lastindex;
}

int BSPMapMakeImpl::mirrorRoomsInZ(int firstroom, int lastroom, int Z)
{
	int lastindex = lastroom;
	for (int mirrorroom = firstroom; mirrorroom <= lastroom; ++mirrorroom)
	{
		Room& room = m_room_store.m_room_pool[mirrorroom];

		int newroomindex = m_room_store.getRoom();

		if (-1 != newroomindex)
		{
			lastindex = newroomindex;
			Room& newroom = m_room_store.m_room_pool[newroomindex];
			newroom.extents[0] = Z - room.extents[1];
			newroom.extents[1] = Z - room.extents[0];
			newroom.extents[2] = room.extents[2];
			newroom.extents[3] = room.extents[3];

			m_pCBs->mirrorRoomCallback(mirrorroom, newroomindex);
		}
	}
	return lastindex;
}





bool BSPMapMakeImpl::splitRoom(int roomindex, bool force_horizontal, bool force_vertical)
{
	Room& room = m_room_store.m_room_pool[roomindex];

	if (!m_pCBs->splitAllowed(roomindex, room.extents[0], room.extents[1], room.extents[2], room.extents[3]))
		return false;


	int split_thickness = m_generate_hollow_walls ? m_hollow_wall_thickness : m_wall_thickness;
	int min_splittable_size = ((2*m_roomsize_min)+split_thickness);


	int sizeNS = (room.extents[0]-room.extents[1]);
	int sizeEW = (room.extents[2]-room.extents[3]);

	if	(  (sizeNS <= min_splittable_size)
		&& (sizeEW <= min_splittable_size) )
		return false;

	bool decider( force_horizontal && (!force_vertical) );

	if (!(force_horizontal || force_vertical))
	{
		(++m_split_alternator) &= 1;
		decider = m_use_split_alternator ? (m_split_alternator & 1) : (rand()>(RAND_MAX/2));
	}

	if	(  (sizeEW <= min_splittable_size)
		|| ((sizeNS >= min_splittable_size) && decider)
		)
	{	//split with horizontal wall

		int split = m_roomsize_min + (std::max)(0, static_cast<int>(PRandom() * static_cast<double>(sizeNS-min_splittable_size)) );

		int newroomindex = m_room_store.getRoom();
		if (-1 != newroomindex)
		{
			Room& newroom = m_room_store.m_room_pool[newroomindex];
			newroom.extents[0] = room.extents[0] - split - split_thickness;
			newroom.extents[1] = room.extents[1];
			newroom.extents[2] = room.extents[2];
			newroom.extents[3] = room.extents[3];
			room.extents[1] = room.extents[0] - split;

			int newroomindex2 = -1;
			if (m_generate_hollow_walls)
			{
				newroomindex2 = m_room_store.getRoom();
				if (-1 != newroomindex2)
				{
					Room& hollowroom = m_room_store.m_room_pool[newroomindex2];
					hollowroom.extents[0] = room.extents[1] - m_wall_thickness;
					hollowroom.extents[1] = room.extents[1] - split_thickness + m_wall_thickness;
					hollowroom.extents[2] = room.extents[2];
					hollowroom.extents[3] = room.extents[3];
				}
			}
			m_pCBs->splitRoomCallback(roomindex, newroomindex, newroomindex2);
		}
		else
			return false;
	}
	else
	{	//split with vertical wall

		int split = m_roomsize_min + (std::max)(0, static_cast<int>(PRandom() * static_cast<double>(sizeEW-min_splittable_size)) );

		int newroomindex = m_room_store.getRoom();
		if (-1 != newroomindex)
		{
			Room& newroom = m_room_store.m_room_pool[newroomindex];
			newroom.extents[0] = room.extents[0];
			newroom.extents[1] = room.extents[1];
			newroom.extents[2] = room.extents[2] - split - split_thickness;
			newroom.extents[3] = room.extents[3];
			room.extents[3] = room.extents[2] - split;

			int newroomindex2 = -1;
			if (m_generate_hollow_walls)
			{
				newroomindex2 = m_room_store.getRoom();
				if (-1 != newroomindex2)
				{
					Room& hollowroom = m_room_store.m_room_pool[newroomindex2];
					hollowroom.extents[2] = room.extents[3] - m_wall_thickness;
					hollowroom.extents[3] = room.extents[3] - split_thickness + m_wall_thickness;
					hollowroom.extents[0] = room.extents[0];
					hollowroom.extents[1] = room.extents[1];
				}
			}
			m_pCBs->splitRoomCallback(roomindex, newroomindex, newroomindex2);
		}
		else
			return false;
	}

	return true;
}





bool BSPMapMakeImpl::roomsAreConnected(int room1, int room2)
{
	for (int finddoor=0; finddoor<m_door_store.m_next_door; ++finddoor)
	{
		Door& testdoor = m_door_store.m_door_pool[finddoor];

		if ( (testdoor.room[0] == room1) && (testdoor.room[1] == room2) )
			return true;
		if ( (testdoor.room[1] == room1) && (testdoor.room[0] == room2) )
			return true;
	}

	return false;
}




bool BSPMapMakeImpl::roomsAreNeighbours(const Room& room1, const Room& room2)
{
	int NSoverlap = (std::min)(room1.extents[0], room2.extents[0]) - (std::max)(room1.extents[1], room2.extents[1]);
	int EWoverlap = (std::min)(room1.extents[2], room2.extents[2]) - (std::max)(room1.extents[3], room2.extents[3]);

	return ((-m_wall_thickness == NSoverlap) && (EWoverlap >= MIN_ROOM_FOR_DOOR)) || ((-m_wall_thickness == EWoverlap) && (NSoverlap >= MIN_ROOM_FOR_DOOR));
}




int BSPMapMakeImpl::findPotentialDoor(int room)
{
	Room& this_room = m_room_store.m_room_pool[room];

	int numneighboursfound = 0;	//unconnected neighbours
	for (int findroom=0; findroom<m_room_store.m_next_room; ++findroom)
	{
		if (findroom == room)
			continue;

		Room& testroom = m_room_store.m_room_pool[findroom];

		if (!roomsAreNeighbours(this_room, testroom))
			continue;

		if (roomsAreConnected(room, findroom))
			continue;

		++numneighboursfound;
	}

	if (0 == numneighboursfound)
		return -1;

	int pick = (std::min)( numneighboursfound-1, FLOOR( PRandom() * numneighboursfound ) );
	int connectto;
	for (connectto=0; connectto<m_room_store.m_next_room; ++connectto)
	{
		if (connectto == room)
			continue;

		Room& testroom = m_room_store.m_room_pool[connectto];

		if (!roomsAreNeighbours(this_room, testroom))
			continue;

		if (roomsAreConnected(room, connectto))
			continue;

		if (0 == pick)
			break;
		--pick;
	}

	return connectto;
}


bool BSPMapMakeImpl::makeDoor(int room, int connectto)
{
	if (!m_pCBs->makeDoorAllowed(room, connectto))
		return false;

	int newdoorindex = m_door_store.getDoor();

	if (-1 != newdoorindex)
	{
		Door& newdoor = m_door_store.m_door_pool[newdoorindex];
		newdoor.room[0] = room;
		newdoor.room[1] = connectto;
		return true;
	}
	return false;
}

void BSPMapMakeImpl::makeDoor(int room)
{
	int connectto = findPotentialDoor(room);
	if (-1 == connectto)
		return;

	makeDoor(room, connectto);
}






void BSPMapMakeImpl::clearExplored(void)
{
	memset(m_explored, false, MAXROOMS*sizeof(bool));
}

int BSPMapMakeImpl::countReachableRooms(int room)
{
	int tally = 1;

	m_explored[room] = true;

	for (int finddoor=0; finddoor<m_door_store.m_next_door; ++finddoor)
	{
		Door& testdoor = m_door_store.m_door_pool[finddoor];

		if ( (testdoor.room[0] == room) && (!m_explored[testdoor.room[1]]) )
			tally += countReachableRooms(testdoor.room[1]);

		if ( (testdoor.room[1] == room) && (!m_explored[testdoor.room[0]]) )
			tally += countReachableRooms(testdoor.room[0]);
	}

	return tally;
}




int BSPMapMakeImpl::makeRoom(int N, int W, int S, int E)
{
	int roomindex = m_room_store.getRoom();

	if (-1 != roomindex)
	{
		Room& room1 = m_room_store.m_room_pool[roomindex];
		room1.extents[0] = N;
		room1.extents[1] = S;
		room1.extents[2] = E;
		room1.extents[3] = W;
	}
	return roomindex;
}

int BSPMapMakeImpl::divideRooms(int firstroomindex, int numdivides)
{
	if (-1 == firstroomindex)
		return m_room_store.m_next_room-1;

	checkDoorOffset();
	checkHollowWallThickness();

    int split_room = firstroomindex;
	while ((numdivides > 0) && (split_room < m_room_store.m_next_room))
    {
        if (PRandom() > ((double)m_room_store.m_room_pool[split_room].longSize() / (double)m_max_room_size))
            {++split_room;}
        else if (splitRoom(split_room))
            {--numdivides;}
        else
            {++split_room;}
    }
	return m_room_store.m_next_room-1;
}


void BSPMapMakeImpl::connectRooms(int firstroomindex, double targetcoverage)
{
	drawDoorsToRooms();	//in case the rules have changed since the last ConnectRooms call

	int actualnumrooms = m_room_store.m_next_room - firstroomindex;


	if (actualnumrooms < 2)
		return;


	for (int make_doors=firstroomindex; make_doors<m_room_store.m_next_room; ++make_doors)
	{
	    Room& this_room = m_room_store.m_room_pool[ make_doors ];
		int numdoors = m_pCBs->numDoorsForRoom(make_doors, this_room.extents[0], this_room.extents[1], this_room.extents[2], this_room.extents[3]);
		while (numdoors--)
			makeDoor(make_doors);
	}


	//Test for closed off sections
	int targetnumrooms = static_cast<int>(targetcoverage * static_cast<double>(actualnumrooms));

	clearExplored();
	int reachable = countReachableRooms(firstroomindex);

	while (reachable < targetnumrooms)
	{
		//find a room that isn't explored but is neighbours with one that is, and put a door in.
		bool foundone = false;
		for (int findaroom=firstroomindex+1; (!foundone) && (findaroom<m_room_store.m_next_room); ++findaroom)
		{
			if (m_explored[findaroom])
				continue;

			Room& this_room = m_room_store.m_room_pool[findaroom];
			for (int findaroom2=firstroomindex; findaroom2<m_room_store.m_next_room; ++findaroom2)
			{
				if (findaroom2 == findaroom)
					continue;

				Room& testroom = m_room_store.m_room_pool[findaroom2];

				if (roomsAreNeighbours(this_room, testroom) && m_explored[findaroom2])
				{
					foundone = true;
					if (!makeDoor(findaroom, findaroom2))
						return;
					break;
				}
			}
		}

		int lastreachable = reachable;

		clearExplored();
		reachable = countReachableRooms(firstroomindex);

		if (lastreachable == reachable)
			return;
	}

	return;
}


















void DrawDoor::setDoor(int idoor, DoorStore& door_store, RoomStore& room_store)
{
	door = idoor;

	Door& this_door = door_store.m_door_pool[door];
	Room& room1 = room_store.m_room_pool[this_door.room[0]];
	Room& room2 = room_store.m_room_pool[this_door.room[1]];

	NSoverlap = (std::min)(room1.extents[0], room2.extents[0]) - (std::max)(room1.extents[1], room2.extents[1]);
	EWoverlap = (std::min)(room1.extents[2], room2.extents[2]) - (std::max)(room1.extents[3], room2.extents[3]);
}

void DrawDoor::chooseDoorType(DoorStore& door_store, RoomStore& room_store, IBSPMapMakerCallbacks* pCBs)
{
	if (-1 == door)
		return;

	const Door& this_door = door_store.m_door_pool[door];
	const Room& room1 = room_store.m_room_pool[this_door.room[0]];
	const Room& room2 = room_store.m_room_pool[this_door.room[1]];

	//different doorway configs: small, large (offset to offset), and remove wall

	doortype = pCBs->chooseDoorType(this_door.room[0], this_door.room[1], room1.extents[0], room1.extents[1], room1.extents[2], room1.extents[3], room2.extents[0], room2.extents[1], room2.extents[2], room2.extents[3]);
}

void DrawDoor::chooseDoorPosition(int doorsize_min, int dooroffs_min)
{
	double fdoorpos;
	if ((std::max)(NSoverlap, EWoverlap) > (5*doorsize_min))
	{
		//generate random value biased towards 0 or 1
		fdoorpos = Random();
		fdoorpos = 0.5 * (1.0 - (fdoorpos*fdoorpos*fdoorpos));
		fdoorpos = rand()>(RAND_MAX/2) ? 0.5+fdoorpos : 0.5-fdoorpos;
		fdoorpos = (std::max)(0.0, (std::min)(1.0, fdoorpos) );
	}
	else
	{
		fdoorpos = PRandom();
	}

	if (EWoverlap > 0)
	{	//North or south wall door.
		doorsize =	0 == doortype ? doorsize_min :
					1 == doortype ? EWoverlap - (2*dooroffs_min): 
					EWoverlap;
		doorpos =	0 == doortype ? dooroffs_min + FLOOR(fdoorpos*static_cast<double>(EWoverlap-doorsize-(2*dooroffs_min))) :
					1 == doortype ? dooroffs_min:
					0;
	}
	else if (NSoverlap > 0)
	{	//East or west door
		doorsize =	0 == doortype ? doorsize_min :
					1 == doortype ? NSoverlap - (2*dooroffs_min): 
					NSoverlap;
		doorpos =	0 == doortype ? dooroffs_min + FLOOR(fdoorpos*static_cast<double>(NSoverlap-doorsize-(2*dooroffs_min))) :
					1 == doortype ? dooroffs_min:
					0;
	}
}


void DrawDoor::calculateExtents(DoorStore& door_store, RoomStore& room_store, int wall_thickness)
{
	if (-1 == door)
		return;

	Door& this_door = door_store.m_door_pool[door];
	Room& room1 = room_store.m_room_pool[this_door.room[0]];
	Room& room2 = room_store.m_room_pool[this_door.room[1]];

	if (EWoverlap > 0)
	{	//North or south wall door.
		extents[0] = (std::min)(room1.extents[0], room2.extents[0]) + wall_thickness;
		extents[1] = extents[0] - wall_thickness;
		extents[3] = (std::max)(room1.extents[3], room2.extents[3]) + doorpos;
		extents[2] = extents[3] + doorsize;
	}
	else if (NSoverlap > 0)
	{	//East or west door
		extents[3] = (std::max)(room1.extents[3], room2.extents[3]) - wall_thickness;
		extents[2] = extents[3] + wall_thickness;
		extents[0] = (std::min)(room1.extents[0], room2.extents[0]) - doorpos;
		extents[1] = extents[0] - doorsize;
	}
}






















void BSPMapMakeImpl::drawRooms(void)
{
	for (int drawroom=0; drawroom<m_room_store.m_next_room; ++drawroom)
	{
		Room& this_room = m_room_store.m_room_pool[drawroom];

		m_pCBs->drawRoom(drawroom, this_room.extents[0], this_room.extents[1], this_room.extents[2], this_room.extents[3]);
	}
}

bool BSPMapMakeImpl::roomsAreConjoined(int room1, int room2, std::vector<int>& visited) const
{
    if (visited.end() != std::find( visited.begin(), visited.end(), room2 ))
        return false;

    visited.push_back(room2);

    for (int test_draw_door=0; test_draw_door < m_draw_door_store.m_next_draw_door; ++test_draw_door )
    {
        const DrawDoor& this_draw_door( m_draw_door_store.m_draw_door_pool[test_draw_door] );
        if ((this_draw_door.door < 0) || (this_draw_door.door >= m_door_store.m_next_door))
            continue;
        const Door& this_door( m_door_store.m_door_pool[this_draw_door.door] );
        if (0 != this_draw_door.doortype)
        {
            if ( ((this_door.room[0] == room1) && (this_door.room[1] == room2)) || ((this_door.room[0] == room2) && (this_door.room[1] == room1)) )
                return true;

            if (this_door.room[0] == room2)
                if (roomsAreConjoined(room1, this_door.room[1], visited))
                    return true;
            if (this_door.room[1] == room2)
                if (roomsAreConjoined(room1, this_door.room[0], visited))
                    return true;
        }
    }

    return false;
}

int BSPMapMakeImpl::generateDrawDoors(int firstdoor, bool smalldoorsonly)
{
	for (int door=0; door<m_door_store.m_next_door; ++door)
	{
		Door& this_door = m_door_store.m_door_pool[door];

		int newdrawdoor = m_draw_door_store.getDrawDoor();
		if (-1 != newdrawdoor)
		{
			DrawDoor& this_draw_door = m_draw_door_store.m_draw_door_pool[newdrawdoor];
			this_draw_door.setDoor(door, m_door_store, m_room_store);
			if (smalldoorsonly)
				this_draw_door.doortype = 0;
			else
				this_draw_door.chooseDoorType(m_door_store, m_room_store, m_pCBs);
		}
	}

    //TODO make this optional. For some settings, e.g. ruins, having isolated doors is appropriate

    //Disallow "normal" doors where both sides are part of the same extended room.
    bool passed(true);
    for (;;)
    {
        passed = true;
	    for (int draw_door=0; draw_door < m_draw_door_store.m_next_draw_door; ++draw_door)
	    {
		    DrawDoor& this_draw_door = m_draw_door_store.m_draw_door_pool[ draw_door ];

		    if (0 == this_draw_door.doortype)
		    {
    		    Door& this_door = m_door_store.m_door_pool[ this_draw_door.door ];
    		    std::vector<int> visited;
    		    if (roomsAreConjoined(this_door.room[0], this_door.room[1], visited))
    		    {
    		        this_draw_door.doortype = 2;
    		        passed = false;
    		        break;
    		    }
    	    }
        }
        if (passed)
            break;
    }


	for (int draw_door=0; draw_door < m_draw_door_store.m_next_draw_door; ++draw_door)
	{
		DrawDoor& this_draw_door = m_draw_door_store.m_draw_door_pool[draw_door];
		this_draw_door.chooseDoorPosition(m_doorsize_min, m_dooroffs_min);
		this_draw_door.calculateExtents(m_door_store, m_room_store, m_wall_thickness);
	}

	return m_draw_door_store.m_next_draw_door;
}

void BSPMapMakeImpl::drawDoors(void)
{
	for (int drawdoor=0; drawdoor<m_draw_door_store.m_next_draw_door; ++drawdoor)
	{
		DrawDoor& this_draw_door = m_draw_door_store.m_draw_door_pool[drawdoor];

		if (-1 == this_draw_door.door)
			continue;

		Door& this_door = m_door_store.m_door_pool[this_draw_door.door];

		m_pCBs->drawDoor(drawdoor, this_door.room[0], this_door.room[1], this_draw_door.doortype, 
			this_draw_door.EWoverlap > 0 ? 0 : 1, 
			this_draw_door.extents[0], this_draw_door.extents[1], this_draw_door.extents[2], this_draw_door.extents[3]);
	}
}


void BSPMapMakeImpl::drawDoorsToRooms(void)
{
	for (int drawdoor=0; drawdoor<m_draw_door_store.m_next_draw_door; ++drawdoor)
	{
		DrawDoor& this_draw_door = m_draw_door_store.m_draw_door_pool[drawdoor];
		if (-1 == this_draw_door.door)
			continue;

		if (this_draw_door.EWoverlap > 0)
		{
			if ((this_draw_door.extents[0] - this_draw_door.extents[1]) <= (2*m_wall_thickness))
				continue;
			if ((this_draw_door.extents[2] - this_draw_door.extents[3]) < ((2*m_dooroffs_min)+m_doorsize_min))
				continue;
		}
		else if (this_draw_door.NSoverlap > 0)
		{
			if ((this_draw_door.extents[2] - this_draw_door.extents[3]) <= (2*m_wall_thickness))
				continue;
			if ((this_draw_door.extents[0] - this_draw_door.extents[1]) < ((2*m_dooroffs_min)+m_doorsize_min))
				continue;
		}


		int newroomindex = m_room_store.getRoom();
		if (-1 != newroomindex)
		{
			Room& newroom = m_room_store.m_room_pool[newroomindex];

			if (this_draw_door.EWoverlap > 0)
			{
				newroom.extents[2] = this_draw_door.extents[2];
				newroom.extents[3] = this_draw_door.extents[3];
				newroom.extents[0] = this_draw_door.extents[0] - m_wall_thickness;
				newroom.extents[1] = this_draw_door.extents[1] + m_wall_thickness;
			}
			else if (this_draw_door.NSoverlap > 0)
			{
				newroom.extents[0] = this_draw_door.extents[0];
				newroom.extents[1] = this_draw_door.extents[1];
				newroom.extents[2] = this_draw_door.extents[2] - m_wall_thickness;
				newroom.extents[3] = this_draw_door.extents[3] + m_wall_thickness;
			}

			Door& door = m_door_store.m_door_pool[this_draw_door.door];
			m_pCBs->roomFromDrawDoorCallback(newroomindex, door.room[0], door.room[1], this_draw_door.doortype);
		}
	}

	m_door_store.reset();
	m_draw_door_store.reset();
}






/////////////////////////////////////////////////////////////////////////////////////////////////////


BSPMapMake::BSPMapMake( IBSPMapMakerCallbacks* pCBs )
: m_impl( new BSPMapMakeImpl(pCBs) )
{
}

int BSPMapMake::makeRoom(int N, int W, int S, int E)
{
	return m_impl->makeRoom(N,W,S,E);
}
int BSPMapMake::mirrorRoomsInX(int firstroom, int lastroom, int X)
{
	return m_impl->mirrorRoomsInX(firstroom, lastroom, X);
}
int BSPMapMake::mirrorRoomsInZ(int firstroom, int lastroom, int Z)
{
	return m_impl->mirrorRoomsInZ(firstroom, lastroom, Z);
}
int BSPMapMake::divideRooms(int firstroomindex, int numdivides)
{
	return m_impl->divideRooms(firstroomindex, numdivides);
}
void BSPMapMake::connectRooms(int firstroomindex, double targetcoverage)
{
	m_impl->connectRooms(firstroomindex, targetcoverage);
}
int BSPMapMake::generateDrawDoors(int firstdoor, bool smalldoorsonly)
{
	return m_impl->generateDrawDoors(firstdoor, smalldoorsonly);
}
void BSPMapMake::drawDoorsToRooms(void)
{
	m_impl->drawDoorsToRooms();
}
void BSPMapMake::drawRooms(void)
{
	m_impl->drawRooms();
}
void BSPMapMake::drawDoors(void)
{
	m_impl->drawDoors();
}

void BSPMapMake::setHollowWalls(bool b)
{
	m_impl->setHollowWalls(b);
}
void BSPMapMake::setHollowWallThickness(int n)
{
	m_impl->setHollowWallThickness(n);
}
void BSPMapMake::setWallThickness(int n)
{
	m_impl->setWallThickness(n);
}
void BSPMapMake::setDoorSize(int n)
{
	m_impl->setDoorSize(n);
}
void BSPMapMake::setDoorOffset(int n)
{
	m_impl->setDoorOffset(n);
}
void BSPMapMake::setRoomSize(int n)
{
	m_impl->setRoomSize(n);
}
void BSPMapMake::setUseSplitAlternator(bool b)
{
	m_impl->setUseSplitAlternator(b);
}
void BSPMapMake::setSplitAlternator(int i)
{
	m_impl->setSplitAlternator(i);
}
void BSPMapMake::setMaxRoomSize(int i)
{
	m_impl->setMaxRoomSize(i);
}

bool BSPMapMake::getHollowWalls(void) const
{
	return m_impl->getHollowWalls();
}
int  BSPMapMake::getHollowWallThickness(void) const
{
	return m_impl->getHollowWallThickness();
}
int  BSPMapMake::getWallThickness(void) const
{
	return m_impl->getWallThickness();
}
int  BSPMapMake::getDoorSize(void) const
{
	return m_impl->getDoorSize();
}
int  BSPMapMake::getDoorOffset(void) const
{
	return m_impl->getDoorOffset();
}
int  BSPMapMake::getRoomSize(void) const
{
	return m_impl->getRoomSize();
}
bool BSPMapMake::getUseSplitAlternator(void) const
{
	return m_impl->getUseSplitAlternator();
}
int  BSPMapMake::getSplitAlternator(void) const
{
	return m_impl->getSplitAlternator();
}
int  BSPMapMake::getMaxRoomSize(void) const
{
	return m_impl->getMaxRoomSize();
}



}
