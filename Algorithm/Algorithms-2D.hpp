#ifndef RL_SHARED_ALGORITHMS_2D_HPP
#define	RL_SHARED_ALGORITHMS_2D_HPP



#include <vector>



namespace RL_shared
{






unsigned long isqrt(unsigned long x);







class ISampleCellFunctor
{
public:
	virtual ~ISampleCellFunctor()
	{
	}
	//Returns true to stop the line cast, false otherwise. 
	virtual bool operator()(int x, int y) = 0;
};

class VisitCellFunctor
{
public:
	virtual ~VisitCellFunctor()
	{
	}
	virtual void operator()(int x, int y) 
	{
	}
};

//Bresenham line cast.
void lineCast(
	int x0, int y0, int x1, int y1, 
	const ISampleCellFunctor& sample_cell_functor,
	VisitCellFunctor& visit_cell_functor
	);







struct PathPoint
{
	int x, z;
	PathPoint()
		: x(-1), z(-1)
	{
	}
	PathPoint( int in_x, int in_z )
		: x( in_x ), z( in_z )
	{
	}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & x;
		ar & z;
	}
};

typedef std::vector< PathPoint > Path;

class World;
class Zone;
const static int COST_RADIX = 10000;
const static int COST_DIAG = 14142;
typedef bool (*ExploreFn)( void* data, World&, const Zone&, int x, int z, int ori_x, int ori_z, int& cost, bool& halt, bool& found_goal );

void explore( void* data, World& world, const Zone& zone, int x, int z, int max, ExploreFn fn, Path* return_path );




}



#endif
