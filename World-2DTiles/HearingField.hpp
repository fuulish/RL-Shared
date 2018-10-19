#ifndef RL_SHARED_HEARING_FIELD_HPP
#define	RL_SHARED_HEARING_FIELD_HPP



#include "WorldObjects/Types.hpp"
#include "DBKeyValue.hpp"
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>



namespace RL_shared
{


class World;
class Zone;


typedef int (*CostMultiplierFn)(TerrainType);


class HearingField
{
public:

	HearingField( boost::uint16_t x_size, boost::uint16_t z_size );

	void clear(void);

	boost::uint16_t sizeX(void) const	{return m_state.m_x_size;}
	boost::uint16_t sizeZ(void) const	{return m_state.m_z_size;}

	boost::uint32_t sample( boost::uint16_t x, boost::uint16_t z ) const;

	void update( 
		World&, 
		DBKeyValue zone, 
		int source_x, int source_z, 
		int range, 
		CostMultiplierFn 
		);

	void getFieldOrigin( int& x, int& z ) const
	{
		x = m_state.m_origin_in_zone_x;
		z = m_state.m_origin_in_zone_z;
	}


    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & m_state;
	}

	struct State
	{
		typedef std::vector< boost::uint32_t > Field;
		Field m_field;
		boost::uint16_t m_x_size;	
		boost::uint16_t m_z_size;
		boost::int32_t m_origin_in_zone_x;
		boost::int32_t m_origin_in_zone_z;

		State(void) : m_x_size(0), m_z_size(0), m_origin_in_zone_x(0), m_origin_in_zone_z(0)
		{
		}
	    template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & m_field;
			ar & m_x_size;	
			ar & m_z_size;
			ar & m_origin_in_zone_x;
			ar & m_origin_in_zone_z;
		}
	};

	struct Data
	{
		CostMultiplierFn func;
		HearingField::State& state;

		Data( CostMultiplierFn in_func, HearingField::State& in_state )
			: func( in_func ), state( in_state )
			{
			}
	};

private:

	HearingField( const HearingField& );
	HearingField& operator= ( const HearingField& );

	State m_state;

	void resetField(void);
};



}



#endif
