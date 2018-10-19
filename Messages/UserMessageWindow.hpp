#ifndef RL_SHARED_USER_MESSAGE_WINDOW_HPP_INCLUDED
#define RL_SHARED_USER_MESSAGE_WINDOW_HPP_INCLUDED




namespace RL_shared
{


class UserMessageWindow
{
    unsigned int m_scroll_pos;

public:

    UserMessageWindow(void)
        : m_scroll_pos(0)
    {
    }

    void scrollForward(void)
    {
        if (m_scroll_pos > 0)
        {
            --m_scroll_pos;
        }
    }
	void scrollBackward(void)
	{
        ++m_scroll_pos;
	}

    unsigned int scrollOffset(void) const
    {
        return m_scroll_pos;
    }
};


}




#endif
