#include "GameModel.hpp"
#include "GameObject.hpp"
#include "WorldObjects/Actor.hpp"
#include "World-2DTiles/Zone.hpp"
#include "assert.hpp"
#include <boost/foreach.hpp>
#include <utility>



namespace RL_shared
{


using namespace RL_shared;
using namespace boost;



GameModel::GameModel(void)
: m_avatar_key(INVALID_KEY)
, m_vision_field(33, 33) //<<??
, m_hearing_field(33, 33) //<<??
, m_hearing_field_dirty(true)
, m_total_game_time(0)
, m_fast_moving_objects(0)
, m_fast_actions(0)
{
	clear();
}

void GameModel::clear(void)
{
	m_world.clear();
	m_action_engine.clear();
	m_vision_field.clear();
	m_avatar_key = INVALID_KEY;
	m_total_game_time = 0;
	m_fast_moving_objects = 0;
	m_fast_actions = 0;
}

void GameModel::decFastMovingObjects(void)
{ 
	if (m_fast_moving_objects > 0)
		--m_fast_moving_objects; 
	else
	{
		ASSERT(false);
	}
}
void GameModel::decFastActions(void)
{ 
	if (m_fast_actions > 0)
		--m_fast_actions; 
	else
	{
		ASSERT(false);
	}
}

void GameModel::advanceGameTime( GameTimeCoordinate time_step )
{
	m_total_game_time += time_step;
}

bool GameModel::advance_turn( GameTimeCoordinate& dt )
{
	dt = 0;

	GameTimeCoordinate time_advanced(0);



	//Copy the list of objects as it is before the step. 
	World::WorldObjects world_objects;
	world_objects.reserve( m_world.objects().size() );
	BOOST_FOREACH( World::WorldObjects::value_type element, m_world.objects() )
	{
		world_objects.push_back(element);
	}


	//Never redraw/update world more rapidly than this, unless there are fast objects
	GameTimeCoordinate limit_time(150); //TODO game-dependent

	//The action system tends to disorder so eventually lots of very short updates will be being done. 
	//To avoid slowdown, we want to do as much time advancement as possible before notifying all objects of the time advance. 
	//This does have the side-effect that world objects created in this loop will be "frozen in time" until it exits!
	while (time_advanced < limit_time)
	{
		const GameTimeCoordinate FAST_STEP_TIME = 15;

		//Always do fast steps so that we don't overshoot too far if the player's action is interrupted.
		//Advancing the action engine should (in theory) usually be a cheap operation.

		m_action_engine.advance(FAST_STEP_TIME, FAST_STEP_TIME, *this);

		time_advanced += FAST_STEP_TIME;

		if ( (m_fast_moving_objects > 0) || (m_fast_actions > 0)  )
			break;	//There are fast-changing objects. Exit to do an update/redraw.

		if (m_world.objectExists(m_avatar_key))
		{
			Actor& avatar_obj = dynamic_cast<Actor&>( m_world.object(m_avatar_key) );
			if (avatar_obj.actions().empty())
				break; //Avatar is finished all actions. Exit for more input.
		}
	}

	dt = time_advanced;

	ASSERT( dt > 0 );

	updateObjects(time_advanced, world_objects);
	updateVision();
	updateHearingIfDirty();
	advanceGameTime(time_advanced);

	if (!m_world.objectExists(m_avatar_key))
		return true;

	Actor& avatar_obj = dynamic_cast<Actor&>( m_world.object(m_avatar_key) );
	return (avatar_obj.actions().empty() && (0 == m_fast_moving_objects));
}

void GameModel::advance_step( GameTimeCoordinate dt )
{
	ASSERT( dt > 0 );

	//Copy the list of objects as it is before the step. 
	World::WorldObjects world_objects;
	world_objects.reserve( m_world.objects().size() );
	BOOST_FOREACH( World::WorldObjects::value_type element, m_world.objects() )
	{
		world_objects.push_back(element);
	}

	m_action_engine.advance(dt, dt, *this);
	updateObjects(dt, world_objects);
	updateVision();
	updateHearingIfDirty();
	advanceGameTime(dt);
}

void GameModel::updateObjects( GameTimeCoordinate time_step, const World::WorldObjects& world_objects )
{
	if (!m_world.objectExists(m_avatar_key))
		return;

	WorldObject& avatar_obj = m_world.object(m_avatar_key);
	DBKeyValue zone_key = avatar_obj.location().zone;
	if (m_world.zoneExists(zone_key))
	{
		std::vector< DBKeyValue > remove_objs;

		//TODO Change to only do the objects that are in the avatar's zone - plus certain objects that need to get out-of-zone updates.

		BOOST_FOREACH( WorldObjectPtr obj, world_objects )
		{
			if (obj)
			{
				updateObject(obj, zone_key, time_step, remove_objs);
			}
		}

		BOOST_FOREACH( DBKeyValue obj, remove_objs )
		{
			if (m_world.objectExists(obj))
			{
				const GameObject& game_obj( dynamic_cast< const GameObject& >( m_world.object(obj) ) );
				if (game_obj.isFastMovingObject())
					decFastMovingObjects();
			}

			m_world.removeWorldObject(obj);
		}
	}
}

void GameModel::updateObject( 
	WorldObjectPtr obj, DBKeyValue zone_key, GameTimeCoordinate time_step, 
	std::vector< DBKeyValue >& remove_objs 
	)
{
	shared_ptr< GameObject > game_obj( dynamic_pointer_cast< GameObject >( obj ) );
	if (game_obj)
	{
		if ((obj->location().zone == zone_key) || (game_obj->alwaysProcess()))
		{
			game_obj->notifyTimeStep(*this, time_step);
			if (game_obj->removeMe(*this))
			{
				remove_objs.push_back(obj->key());
			}
		}
		else if (game_obj->isFastMovingObject())
		{
			remove_objs.push_back(obj->key());
		}
	}
}

void GameModel::updateVision(void)
{
	//Game-dependent behaviour
}

bool GameModel::isVisible( DBKeyValue zone_key, int zone_x, int zone_z ) const
{
	if (!m_world.objectExists(m_avatar_key))
		return false;
	DBKeyValue avatar_zone( m_world.object(m_avatar_key).location().zone );
	if (zone_key != avatar_zone)
		return false;

	int fox(0), foz(0);
	m_vision_field.getFieldOrigin(fox, foz);
	return m_vision_field.sample((uint16_t)(zone_x - fox), (uint16_t)(zone_z - foz));
}


void GameModel::updateHearing(void)
{
	//Game-dependent behaviour
}

void GameModel::updateHearingIfDirty(void)
{
	if (m_hearing_field_dirty)
	{
		updateHearing();
		m_hearing_field_dirty = false;
	}
}

boost::uint32_t GameModel::hearingValue( DBKeyValue zone_key, int zone_x, int zone_z ) const
{
	if (!m_world.objectExists(m_avatar_key))
		return (boost::uint32_t)-1;
	DBKeyValue avatar_zone( m_world.object(m_avatar_key).location().zone );
	if (zone_key != avatar_zone)
		return (boost::uint32_t)-1;

	int fox(0), foz(0);
	m_hearing_field.getFieldOrigin(fox, foz);
	return m_hearing_field.sample((uint16_t)(zone_x - fox), (uint16_t)(zone_z - foz));
}



}
