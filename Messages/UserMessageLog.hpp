#ifndef RL_SHARED_USERMSGLOG_HPP_INCLUDED
#define RL_SHARED_USERMSGLOG_HPP_INCLUDED



#include <boost/scoped_ptr.hpp>
#include <string>
#include <deque>
#include <vector>



namespace RL_shared
{


class UserMessageLogImpl;



class UserMessageLog
{
public:

    class StoredMsg;
    typedef std::deque< StoredMsg > MsgStore;

    class Msg
    {
        friend class UserMessageLog::StoredMsg;
        const char *m_text;
    public:
        Msg(const char * in_text) : m_text(in_text)
        {
        }
    };

    class StoredMsg
    {
    public:
		typedef std::vector< std::string > Words;

        StoredMsg(const Msg& in_msg);
        const Words& words(void) const;
        Words& words(void);
        unsigned int lines( unsigned int line_width ) const;
        void toLines( unsigned int line_width, std::vector< std::string >& ) const;
    private:
		Words m_words;
		mutable unsigned int m_cache_num_lines;
		mutable unsigned int m_cache_line_width;
    };


	explicit UserMessageLog(unsigned int size);
	~UserMessageLog();

	//This will combine messages which don't end in newline characters.
	void addMessage(const Msg& msg);
    void removeLastMessage(void);
    void beginNewMessage(void);

	typedef MsgStore::const_iterator const_iterator;
	typedef MsgStore::const_reverse_iterator const_reverse_iterator;
	const_iterator begin(void) const;
    const_iterator end(void) const;
	const_reverse_iterator rbegin(void) const;
    const_reverse_iterator rend(void) const;
	unsigned int size(void);

private:


	boost::scoped_ptr<UserMessageLogImpl> m_impl;


    UserMessageLog(const UserMessageLog&);
    UserMessageLog& operator=(const UserMessageLog&);
};


}




#endif
