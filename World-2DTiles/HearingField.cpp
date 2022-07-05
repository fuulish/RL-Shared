#include "HearingField.hpp"
#include "World.hpp"
#include "Zone.hpp"
#include "Algorithm/Algorithms-2D.hpp"
#include "Include/assert.hpp"




namespace RL_shared
{



using namespace boost;
using namespace std;



HearingField::HearingField( uint16_t x_size, uint16_t z_size )
{
	m_state.m_x_size = x_size;
	m_state.m_z_size = z_size;

	ASSERT( (m_state.m_x_size > 2) && (m_state.m_z_size > 2) );

	unsigned int array_size( x_size * z_size);
	m_state.m_field.resize( array_size );

	resetField();
}

void HearingField::clear(void)
{
	resetField();
	m_state.m_origin_in_zone_x = 0;
	m_state.m_origin_in_zone_z = 0;
}

uint32_t HearingField::sample( uint16_t x, uint16_t z ) const
{
	if ((x >= m_state.m_x_size) || (z >= m_state.m_z_size))
		return (uint32_t)-1;

	return m_state.m_field[ (z*m_state.m_x_size) + x ];
}

void HearingField::resetField(void)
{
	unsigned int array_size( m_state.m_x_size * m_state.m_z_size);

	memset( &m_state.m_field[0], 0xff, array_size*sizeof(uint32_t) );
}


namespace
{

typedef HearingField::Data Data;

bool soundExplore( void * data, World& world, const Zone& zone, int x, int z, int ori_x, int ori_z, int& cost, bool&, bool& )
{
	if ((x < 0) || (z < 0) || (x >= zone.sizeX()) || (z >= zone.sizeZ()))
		return false;

	Data& data_obj( *reinterpret_cast< Data* >(data) );

	TerrainType there( zone.terrainAt(x, z) );
	cost += COST_RADIX * (data_obj.func(there)-1); //This does not really count diagonals correctly; but it should not matter.

	HearingField::State& field( data_obj.state );
	int ox = x - field.m_origin_in_zone_x;
	int oz = z - field.m_origin_in_zone_z;
	if ((ox >= 0) && (oz >= 0))
		if ((ox < field.m_x_size) && (oz < field.m_z_size))
		{
			field.m_field[ (oz * field.m_x_size) + ox ] = (cost/COST_RADIX);
		}

	return true;
}

}


void HearingField::update( 
	World& world, 
	DBKeyValue zone_key, 
	int source_x, int source_z, 
	int range, 
	CostMultiplierFn costfunc
	)
{
	resetField();

	const Zone& zone( world.zone( zone_key ) );

	//get tile coordinates of source in field (which is always centred on source)
	uint16_t source_x_pos( m_state.m_x_size/2 );
	uint16_t source_z_pos( m_state.m_z_size/2 );

	//get tile coordinates of field origin in zone
	m_state.m_origin_in_zone_x = ( source_x - source_x_pos );
	m_state.m_origin_in_zone_z = ( source_z - source_z_pos );

	Data data( costfunc, m_state );

	explore( (void*)(&data), world, zone, source_x, source_z, range, soundExplore, 0 );
}






}

