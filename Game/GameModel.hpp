#ifndef RL_SHARED_GAME_MODEL_HPP_INCLUDED
#define	RL_SHARED_GAME_MODEL_HPP_INCLUDED



#include "World-2DTiles/World.hpp"
#include "World-2DTiles/VisionField.hpp"
#include "World-2DTiles/HearingField.hpp"
#include "ActionEngine/ActionEngine.hpp"
#include "Abstract/IGameModel.hpp"




namespace RL_shared
{



class GameModel : public IGameModel
{
public:

	GameModel(void);

	virtual void clear(void);

	virtual bool advance_turn( GameTimeCoordinate& dt );
	virtual void advance_step( GameTimeCoordinate time_step );

	World& world(void)				{ return m_world; }
	const World& world(void) const	{ return m_world; }

	ActionEngine& actionEngine(void)				{ return m_action_engine; }
	const ActionEngine& actionEngine(void) const	{ return m_action_engine; }

	DBKeyValue avatar(void) const		{ return m_avatar_key; }
	void setAvatar( DBKeyValue val )	{ m_avatar_key = val; }

	//TODO: should perhaps move to ownership by player/character object
	VisionField& visionField(void)				{ return m_vision_field; }
	const VisionField& visionField(void) const	{ return m_vision_field; }
	virtual void updateVision(void);
	virtual bool isVisible( DBKeyValue zone_key, int zone_x, int zone_z ) const;
	HearingField& hearingField(void)				{ return m_hearing_field; }
	const HearingField& hearingField(void) const	{ return m_hearing_field; }
	void dirtyHearingField(void)					{ m_hearing_field_dirty = true; }
	virtual void updateHearing(void);
	void updateHearingIfDirty(void);
	virtual boost::uint32_t hearingValue( DBKeyValue zone_key, int zone_x, int zone_z ) const;

	//TODO this makeshift reference counting system is ugly.
	//The root problem is that the game model needs to know of the existence of a fast object 
	//before it is actually created, in order not to step past that point. 
	//The correct solution is probably for the model to observe creation and destruction of 
	//both objects and actions, and to explicitly destroy any fast moving objects that are 
	//not being updated (e.g. by being in an inactive Zone). The latter is already being done. 
	void incFastMovingObjects(void)	{ ++m_fast_moving_objects; }
protected:
	void decFastMovingObjects(void);

	friend class FastAction;
	void incFastActions(void)		{ ++m_fast_actions; }
	void decFastActions(void);

public:

	GameTimeCoordinate gameTime(void) const	{ return m_total_game_time; }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & m_world;
        ar & m_action_engine;
        ar & m_vision_field;
        ar & m_hearing_field;
        //ar & m_hearing_field_dirty; //no need?
        ar & m_avatar_key;
	    ar & m_total_game_time;
	    ar & m_fast_moving_objects;
	    ar & m_fast_actions;
    }

protected:

	World m_world;
	ActionEngine m_action_engine;
	VisionField m_vision_field;
	HearingField m_hearing_field;

	bool m_hearing_field_dirty;

	DBKeyValue m_avatar_key;

	GameTimeCoordinate m_total_game_time;

	int m_fast_moving_objects;
	int m_fast_actions;


	GameModel(GameModel&);
	GameModel& operator=(GameModel&);


	virtual void updateObjects( GameTimeCoordinate time_step, const World::WorldObjects& objects_to_update );
	virtual void updateObject( WorldObjectPtr, DBKeyValue, GameTimeCoordinate, std::vector< DBKeyValue >& remove_objs );
	virtual void advanceGameTime( GameTimeCoordinate time_step );
};



}



#endif
