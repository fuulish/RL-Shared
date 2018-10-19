#ifndef RL_SHARED_FAST_ACTION_HPP
#define	RL_SHARED_FAST_ACTION_HPP


#include "GameModel.hpp"
#include "ActionEngine/ActionEngine.hpp"


namespace RL_shared
{


class FastAction : public BaseAction
{
public:

	explicit FastAction( GameModel& model )
		: m_model( &model )
	{
		m_model->incFastActions();
	}

	~FastAction()
	{
		try
		{
			m_model->decFastActions();
		}
		catch(...)
		{
		}
	}


    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & boost::serialization::base_object< BaseAction >(*this);
		ar & m_model;
	}

	//Default constructor intended only for serialization use.
	FastAction(void) : m_model(0)
	{
	}

private:

	GameModel* m_model;
};


}


#endif
