#ifndef RL_SHARED_GAME_OBJECT_HPP
#define	RL_SHARED_GAME_OBJECT_HPP


#include "WorldObjects/Actor.hpp"
#include "WorldObjects/Types.hpp"

#include "World-2DTiles/World.hpp"

namespace RL_shared
{

class Zone;
class GameModel;


//TODO Break inheritance from Actor, since only a few game objects use it
class GameObject : public Actor
{
public:

	explicit GameObject( WorldObjectType type ) 
	: m_type(type)
	{
	}
	WorldObjectType type(void) const	{ return m_type; }

	virtual void notifyTimeStep( GameModel&, GameTimeCoordinate dt );
	virtual bool removeMe(GameModel&) const;
	virtual bool isFastMovingObject(void) const;
	virtual bool alwaysProcess(void) const; //always get timestep notifications, even when in an inactive zone?

	virtual bool canOverlap(const WorldObject&) const = 0;
	virtual bool canOverlap(TerrainType) const = 0;

	bool canOverlap( const GameModel&, const WorldObject::WorldLocation& ) const;
	bool canOverlap( const World&, const Zone&, int x, int z ) const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & boost::serialization::base_object< Actor >(*this);
		ar & m_type;
	}

private:

	WorldObjectType m_type;

};


}


#endif
