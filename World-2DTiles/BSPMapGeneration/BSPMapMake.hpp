#ifndef	RL_SHARED_BSP_MAP_MAKE_HPP
#define	RL_SHARED_BSP_MAP_MAKE_HPP


#include <boost/cstdint.hpp>
#include <utility>


namespace RL_shared
{



class IBSPMapMakerCallbacks
{
public:

	virtual bool splitAllowed(int roomindex, int N, int S, int E, int W) = 0;
	virtual bool makeDoorAllowed(int room1index, int room2index) = 0;
	virtual void splitRoomCallback(int originalroomindex, int newsideroomindex, int middleroomindex) = 0;
	virtual void roomFromDrawDoorCallback(int newroomindex, int room1index, int room2index, int doortype) = 0;
	virtual void mirrorRoomCallback(int originalroomindex, int newroomindex) = 0;
	virtual int numDoorsForRoom(int roomindex, int N, int S, int E, int W) = 0;
	virtual int chooseDoorType(int room1index, int room2index, int N1, int S1, int E1, int W1, int N2, int S2, int E2, int W2) = 0;
	virtual void drawRoom(int roomindex, int N, int S, int E, int W) = 0;
	virtual void drawDoor(int doorindex, int roomindex1, int roomindex2, int type, int doorori, int N, int S, int E, int W) = 0;

};


class BSPMapMakeImpl;

class BSPMapMake
{
public:

	BSPMapMake( IBSPMapMakerCallbacks* );

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

	BSPMapMakeImpl* m_impl;
};


}



#endif
