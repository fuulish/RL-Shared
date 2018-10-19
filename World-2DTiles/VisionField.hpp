#ifndef RL_SHARED_VISION_FIELD_HPP
#define	RL_SHARED_VISION_FIELD_HPP



#include "WorldObjects/Types.hpp"
#include "DBKeyValue.hpp"
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>



namespace RL_shared
{


class World;
class Zone;


class IIsVisibilityBlocker
{
public:

	~IIsVisibilityBlocker()
	{
	}

	///Returns true if the supplied terrain is visibility blocking.
	virtual bool terrain( TerrainType ) = 0;

	///Returns true if the supplied object is visibility blocking.
	virtual bool object( DBKeyValue ) = 0;
};



class VisionField
{
public:

	VisionField( boost::uint16_t x_size, boost::uint16_t z_size );

	void clear(void);

	boost::uint16_t sizeX(void) const	{return m_x_size;}
	boost::uint16_t sizeZ(void) const	{return m_z_size;}

	bool sample( boost::uint16_t x, boost::uint16_t z ) const;

	void update( 
		const World&, 
		DBKeyValue zone, 
		int eye_x, int eye_z, 
		int look_heading, //degrees anticlockwise from east
		int fov, //fov in degrees
		int range, 
		IIsVisibilityBlocker& 
		);

	void getFieldOrigin( int& x, int& z ) const
	{
		x = m_origin_in_zone_x;
		z = m_origin_in_zone_z;
	}

	class SampleTile;
	class SetTileVisible;


    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & m_flags_field;
		ar & m_x_size;	
		ar & m_z_size;
		ar & m_origin_in_zone_x;
		ar & m_origin_in_zone_z;
	}

private:

	VisionField( const VisionField& );
	VisionField& operator= ( const VisionField& );

	enum {VISIBLE = 1, BLOCKED = 2, BLOCKAGE_PROCESSED = 4};

	std::vector< boost::uint8_t > m_flags_field; //was using boost::scoped_array, but it is not supported by boost::serialization. 
	boost::uint16_t m_x_size;	
	boost::uint16_t m_z_size;
	boost::int32_t m_origin_in_zone_x;
	boost::int32_t m_origin_in_zone_z;



	void resetField(void);
	bool processTile( 
		const Zone& zone, 
		IIsVisibilityBlocker& vis_func, 
		boost::uint16_t x, boost::uint16_t z, 
		boost::int32_t origin_x, boost::int32_t origin_z
		);
	void setTileVisible( boost::uint16_t x, boost::uint16_t z );

	friend class SampleTile;
	friend class SetTileVisible;
};



}



#endif
