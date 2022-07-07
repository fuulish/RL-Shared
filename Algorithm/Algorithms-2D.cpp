#include "Algorithms-2D.hpp"
#include "World-2DTiles/Zone.hpp"
#include <boost/scoped_array.hpp>
#include <list>
#include <utility> //for std::swap
#include <stdlib.h> //for abs




namespace RL_shared
{




//From an implementation at http://www.codecodex.com/wiki/Calculate_an_integer_square_root
unsigned long isqrt(unsigned long x)
{
    register unsigned long op, res, one;

    op = x;
    res = 0;

    /* "one" starts at the highest power of four <= than the argument. */
    one = 1 << 30;  /* second-to-top bit set */
    while (one > op) one >>= 2;

    while (one != 0) {
        if (op >= res + one) {
            op -= res + one;
            res += one << 1;  // <-- faster than 2 * one
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}




//Integer line cast algorithm.
//Based on implementation of Bresenham's line algorithm from wikipedia:
//http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
//***** THE IMPLEMENTATION HAS SINCE BEEN REMOVED FROM THE WIKIPEDIA PAGE *****
bool sample(bool swap, int x, int y, const ISampleCellFunctor& sample)
{
	if (swap)
	{
		return sample(y, x);
	}
	return sample(x, y);
}
void visit(bool swap, int x, int y, const VisitCellFunctor& visit)
{
	if (swap)
		visit(y, x);
	else
		visit(x, y);
}
void lineCast(
	int x0, int y0, int x1, int y1, 
	const ISampleCellFunctor& sample_cell_functor,
	const VisitCellFunctor& visit_cell_functor
	)
{
   int Dx = x1 - x0; 
   int Dy = y1 - y0;
   bool steep = (abs(Dy) >= abs(Dx));
   if (steep) {
       (std::swap)(x0, y0);
       (std::swap)(x1, y1);
       // recompute Dx, Dy after swap
       Dx = x1 - x0;
       Dy = y1 - y0;
   }
   int xstep = 0;
   if (Dx > 0) {
       xstep = 1;
   }
   else if (Dx < 0) {
       xstep = -1;
       Dx = -Dx;
   }
   int ystep = 0;
   if (Dy > 0) {
       ystep = 1;
   }
   else if (Dy < 0) {
       ystep = -1;		
       Dy = -Dy; 
   }
   int TwoDy = 2*Dy; 
   int TwoDyTwoDx = TwoDy - 2*Dx; // 2*Dy - 2*Dx
   int OneDyOneDx = Dy - Dx;
   int E = TwoDy - Dx; //2*Dy - Dx
   int y = y0;
   for (int x = x0; ; x += xstep) {

       visit(steep, x, y, visit_cell_functor);

	   if (sample(steep, x, y, sample_cell_functor))
		   return;

       // next
       if (E > 0) {
           E += TwoDyTwoDx; //E += 2*Dy - 2*Dx;
           y += ystep;
       } else {
           E += TwoDy; //E += 2*Dy;
       }

		if (x == x1)
			break;
   }
}










struct SearchPoint
{
	typedef std::list< SearchPoint > SearchPointList;

	SearchPointList::iterator parent;
	int x, z, cost;
	bool is_goal;

	SearchPoint()
		: x(-1), z(-1), cost(-1), is_goal(false)
	{
	}
	SearchPoint( int in_x, int in_z, int in_cost, SearchPointList::iterator in_parent )
		: parent( in_parent ), x( in_x ), z( in_z ), cost( in_cost ), is_goal(false)
	{
	}
	bool operator<(const SearchPoint& rhs) const
	{
		return cost < rhs.cost;
	}
};

typedef std::list< SearchPoint > SearchPointList;


void explore( void* data, World& world, const Zone& zone, int x, int z, int max, ExploreFn fn, Path* return_path )
{
	if (!zone.isWithin(x, z))
		return;

	//Using 10000 increments in order to measure diagonals reasonably accurately.
	max *= COST_RADIX;

	int map_width( zone.sizeX() );
	int map_height( zone.sizeZ() );
	boost::scoped_array< unsigned int > explored_map( new unsigned int[map_width*map_height] );
	memset( explored_map.get(), 0xff, sizeof(unsigned int)*map_width*map_height );


	SearchPointList search;
	search.push_back( SearchPoint(x, z, 0, search.begin()) );

	SearchPointList::iterator test( search.begin() );

	bool found_goal(false);

	explored_map[ (z*map_width) + x ] = 0;

	for(; test != search.end(); ++test)
	{
		bool done(false);
		bool is_goal(false);
		bool process_neighbours(false);

		process_neighbours = fn(data, world, zone, test->x, test->z, x, z, test->cost, done, is_goal);

		test->is_goal = is_goal;
		found_goal |= is_goal;

		if (process_neighbours)
		{
			static const int dx[8] = {0,  0, 1, -1, -1,  1, -1, 1};
			static const int dz[8] = {1, -1, 0,  0, -1, -1,  1, 1};
			static const int cost[8] = {COST_RADIX, COST_RADIX, COST_RADIX, COST_RADIX, COST_DIAG, COST_DIAG, COST_DIAG, COST_DIAG};

			for (int ti=0; ti<8; ++ti)
			{
				int new_cost = test->cost+cost[ti];
				if (new_cost > max)
					continue;

				int tx = test->x + dx[ti];
				int tz = test->z + dz[ti];

				if ((tx < 0) || (tz < 0) || (tx >= map_width) || (tz >= map_height))
					continue;

				unsigned int& explored_el( explored_map[ (tz*map_width) + tx ] );
				if (explored_el < (unsigned int)test->cost)
					continue;

				explored_el = test->cost;

				//search.push_back( SearchPoint(tx, tz, test->cost+cost[ti], test) );

				SearchPointList::iterator insert = test;
				for (; insert != search.end(); ++insert)
				{
					if (insert->cost > new_cost)
					{
						--insert;
						break;
					}
				}
				search.insert(insert, SearchPoint(tx, tz, test->cost+cost[ti], test));
			}
		}

		if (done)
		{
			break;
		}
	}

	if (return_path && found_goal)
	{
		SearchPointList::const_iterator iter( search.end() );
		while (search.begin() != iter)
		{
			--iter;
			if (iter->is_goal)
				break;
		}
		if (iter->is_goal)
		{
			while (search.begin() != iter)
			{
				return_path->push_back( PathPoint( iter->x, iter->z ) );
				iter = iter->parent;
			}
			return_path->push_back( PathPoint( iter->x, iter->z ) );
		}
	}
}





}
