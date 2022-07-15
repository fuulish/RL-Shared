#include "VisionField.hpp"
#include "World.hpp"
#include "Zone.hpp"
#include "Include/assert.hpp"
#include "permissive-fov/permissive-fov-cpp.h"
#include <boost/foreach.hpp>




namespace RL_shared
{



using namespace boost;
using namespace std;



VisionField::VisionField( uint16_t x_size, uint16_t z_size )
: m_x_size( x_size ), m_z_size( z_size ), m_origin_in_zone_x(0), m_origin_in_zone_z(0)
{
	ASSERT( (m_x_size > 2) && (m_z_size > 2) );

	unsigned int array_size( x_size * z_size);
	m_flags_field.resize( array_size );

	resetField();
}

void VisionField::clear(void)
{
	resetField();
	m_origin_in_zone_x = 0;
	m_origin_in_zone_z = 0;
}

bool VisionField::sample( boost::uint16_t x, boost::uint16_t z ) const
{
	if ((x >= m_x_size) || (z >= m_z_size))
		return false;

	return (0 != (m_flags_field[ (z*m_x_size) + x ] & VISIBLE));
}

void VisionField::resetField(void)
{
	unsigned int array_size( m_x_size * m_z_size);

	memset( &m_flags_field[0], 0, array_size*sizeof(uint8_t) );
}








bool VisionField::processTile( 
	const Zone& zone, 
	IIsVisibilityBlocker& vis_func, 
	uint16_t x, uint16_t z, 
	int32_t origin_x, int32_t origin_z 
	)
{
	if ((x >= m_x_size) || (z >= m_z_size))
		return true;

	int offset( (z * m_x_size) + x );
	uint8_t* p_flags( &m_flags_field[ offset ] ); 

	if ((*p_flags) & BLOCKAGE_PROCESSED)
	{
		return (0 != ((*p_flags) & BLOCKED));
	}

	(*p_flags) |= BLOCKAGE_PROCESSED;

	bool blocker( false );

	int tile_x( origin_x + x );
	int tile_z( origin_z + z );

	if ((tile_x < 0) || (tile_x >= (int)zone.sizeX()) || 
		(tile_z < 0) || (tile_z >= (int)zone.sizeZ()))
		return false;

	TerrainType terrain( zone.terrainAt( tile_x, tile_z ) );
	blocker |= vis_func.terrain( terrain );

	if (!blocker)
	{
		const ObjectList& objects( zone.objectsAt( tile_x, tile_z ) );

		BOOST_FOREACH( DBKeyValue obj_key, objects )
		{
			blocker |= vis_func.object( obj_key );
			if (blocker)
				break;
		}
	}

	if (blocker)
	{
		(*p_flags) |= BLOCKED;
	}

	return blocker;
}

void VisionField::setTileVisible( boost::uint16_t x, boost::uint16_t z )
{
	if ((x < m_x_size) && (z < m_z_size))
	{
		m_flags_field[ (z * m_x_size) + x ] |= VISIBLE;
	}
}

class VisionField::SampleTile
{
	VisionField& m_parent;
	const Zone& m_tiled_zone;
	IIsVisibilityBlocker& m_vis_func;
	int32_t m_origin_x, m_origin_z;

public:

	SampleTile( 
		VisionField& parent, 
		const Zone& tiled_zone, 
		IIsVisibilityBlocker& vis_func, 
		int32_t origin_x, int32_t origin_z
	) : m_parent( parent ), m_tiled_zone( tiled_zone ), m_vis_func( vis_func ), 
	m_origin_x( origin_x ), m_origin_z( origin_z )
	{
	}

	bool operator() (int x, int z) const
	{
		return m_parent.processTile( m_tiled_zone, m_vis_func, 
			static_cast< uint16_t >(x), static_cast< uint16_t >(z), 
			m_origin_x, m_origin_z );
	}
};

class VisionField::SetTileVisible
{
	VisionField& m_parent;

public:

	SetTileVisible( VisionField& parent )
		: m_parent( parent )
	{
	}

	void operator() (int x, int z) const
	{
		m_parent.setTileVisible( static_cast<uint16_t>(x), static_cast<uint16_t>(z) );
	}
};


namespace
{

bool testAgainstFOV( 
	uint16_t x, uint16_t z, 
	uint16_t eye_x, uint16_t eye_z, 
	double look_x, double look_z,  
	double cos_half_fov 
	)
{
	int eye_x_offs( (int)(x) - (int)eye_x );
	int eye_z_offs( (int)(z) - (int)eye_z );
	double eye_x_offs_sc( (double)(eye_x_offs) );
	double eye_z_offs_sc( (double)(eye_z_offs) );
	int lsq( (eye_x_offs * eye_x_offs) + (eye_z_offs * eye_z_offs) );
	double L( sqrt( (double)(lsq) ) );
	double dot( ((look_x * eye_x_offs_sc) + (look_z * eye_z_offs_sc)) / L );

	return (dot >= (cos_half_fov-0.0001)) || ( (x == eye_x) && (z == eye_z) );
}


class TestAgainstFOV
{
	uint16_t eye_x, eye_z;
	double cos_half_fov;
	double look_x, look_z;	
public:
	TestAgainstFOV(
		uint16_t in_eye_x, uint16_t in_eye_z, 
		double in_cos_half_fov, 
		double in_look_x, double in_look_z 
	) : eye_x( in_eye_x ), eye_z( in_eye_z ), 
		cos_half_fov( in_cos_half_fov ), look_x( in_look_x ), look_z( in_look_z )
	{
	}
	bool operator()( uint16_t x, uint16_t z )
	{
		return testAgainstFOV( x, z, eye_x, eye_z, look_x, look_z, cos_half_fov );
	}
};

class NoTestAgainstFOV
{
public:
	NoTestAgainstFOV()
	{
	}
	bool operator()( uint16_t, uint16_t )
	{
		return true;
	}
};


template < typename FOVT >
struct Adaptor
{
	VisionField::SetTileVisible&	set_tile_visible;
	VisionField::SampleTile&		sampler;
	FOVT&	test_against_fov;
	int	m_range_squared;
	int m_origin_x;
	int m_origin_z;

	Adaptor( VisionField::SetTileVisible& in_set_tile_visible, 
			 VisionField::SampleTile& in_sampler, 
			 FOVT& in_test_against_fov, 
			 int range, int origin_x, int origin_z )
	: set_tile_visible( in_set_tile_visible ), sampler( in_sampler ), test_against_fov( in_test_against_fov ), 
	m_range_squared( range*range ), m_origin_x( origin_x ), m_origin_z( origin_z )
	{
	}
	int isBlocked(short x, short z)
	{
		int diff_x = x - m_origin_x;
		int diff_z = z - m_origin_z;
		if ((diff_x*diff_x) + (diff_z*diff_z) > m_range_squared)
			return 1;
		//Testing against the FOV here helps the algorithm to examine fewer tiles. 
		//It also has the side effect of giving "peripheral vision", and making all the 
		//tiles immediately adjacent to the eye visible. This happens because the tiles 
		//immediately outside the FOV are treated as walls - and like the first wall 
		//encountered in any direction, the algorithm makes them visible.
		//I think this is beneficial.
		return (!test_against_fov(x, z)) || static_cast<int>( sampler(x, z) ); 
	}
	void visit(short x, short z)
	{
		int diff_x = x - m_origin_x;
		int diff_z = z - m_origin_z;
		if ((diff_x*diff_x) + (diff_z*diff_z) <= m_range_squared)
			set_tile_visible(x, z);
	}
};

}



	




void VisionField::update( 
	const World& world, 
	DBKeyValue zone_key, 
	int eye_x, int eye_z, 
	int look_deg, 
	int fov_deg, 
	int range, 
	IIsVisibilityBlocker& vis_func
	)
{
	resetField();

	const Zone& zone( world.zone( zone_key ) );

	//get tile coordinates of eye in field (which is always centred on eye)
	uint16_t eye_x_pos( m_x_size/2 );
	uint16_t eye_z_pos( m_z_size/2 );

	//get tile coordinates of field origin in zone
	m_origin_in_zone_x = ( eye_x - eye_x_pos );
	m_origin_in_zone_z = ( eye_z - eye_z_pos );

	SampleTile sampler( *this, zone, vis_func, m_origin_in_zone_x, m_origin_in_zone_z );
	SetTileVisible set_tile_visible( *this );

	int radius = (std::max)( m_x_size/2, m_z_size/2 );

	if (fov_deg < 360)
	{
		double fov( ((double)fov_deg / 180.0) * 3.1415926535897932384626433832795);
		double half_fov( fov * 0.5 );
		double cos_half_fov( cos( half_fov ) );

		double look( ((double)look_deg / 180.0) * 3.1415926535897932384626433832795);
		double look_x( cos( look ) );
		double look_z( sin( look ) );

		TestAgainstFOV test_against_fov( eye_x_pos, eye_z_pos, cos_half_fov, look_x, look_z );

		Adaptor< TestAgainstFOV > adaptorWithFOV( 
			set_tile_visible, sampler, test_against_fov, range, (int)eye_x_pos, (int)eye_z_pos );

		permissive::squareFov< Adaptor< TestAgainstFOV > >( eye_x_pos, eye_z_pos, radius, adaptorWithFOV );
	}
	else
	{
		NoTestAgainstFOV no_test_against_fov;

		Adaptor< NoTestAgainstFOV > adaptorWithoutFOV( 
			set_tile_visible, sampler, no_test_against_fov, range, (int)eye_x_pos, (int)eye_z_pos );

		permissive::squareFov< Adaptor< NoTestAgainstFOV > >( eye_x_pos, eye_z_pos, radius, adaptorWithoutFOV );
	}
}






}

