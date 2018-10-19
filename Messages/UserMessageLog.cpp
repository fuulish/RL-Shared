#include "UserMessageLog.hpp"
#include "assert.hpp"
#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>





namespace RL_shared
{



namespace
{
	bool isWhitespace( char character )
	{
		return (' ' == character) || ('\n' == character);
	}
}

UserMessageLog::StoredMsg::StoredMsg(const Msg& msg)
: m_cache_num_lines(0), m_cache_line_width(0)
{
	const char * in_msg( msg.m_text );
	ASSERT( in_msg );

	std::string word;

	for (; *in_msg; ++in_msg)
	{
		if (isWhitespace(*in_msg))
		{
			if (!word.empty())
			{
				m_words.push_back( word );
				word.clear();
			}
			word += *in_msg;
			m_words.push_back( word );
			word.clear();
		}
		else
		{
			word += *in_msg;
		}
	}

	if (!word.empty())
	{
		m_words.push_back( word );
	}
}

const UserMessageLog::StoredMsg::Words& UserMessageLog::StoredMsg::words(void) const
{
	return m_words;
}
UserMessageLog::StoredMsg::Words& UserMessageLog::StoredMsg::words(void)
{
	return m_words;
}

unsigned int UserMessageLog::StoredMsg::lines( unsigned int line_width ) const
{
	ASSERT( line_width > 0 );

	if (line_width == m_cache_line_width)
		return m_cache_num_lines;

	unsigned int num_lines(0);
	unsigned int line_length(0);

	BOOST_FOREACH( const std::string& word, m_words )
	{
		if ("\n" == word)
		{
			++num_lines;
			line_length = 0;
			continue;
		}

		unsigned int word_len( word.length() );

		if ((line_length > 0) && ((line_length + word.length()) > line_width))
		{
			++num_lines;
			line_length = 0;
		}

		if ((0 == line_length) && (" " == word))
			continue;	//skip whitespace at the beginning of a new line

		while (word_len > line_width)
		{
			++num_lines;
			word_len -= line_width;
		}

		line_length += word_len;
	}

	m_cache_num_lines = num_lines + (line_length > 0 ? 1 : 0);
	m_cache_line_width = line_width;

	return m_cache_num_lines;
}

void UserMessageLog::StoredMsg::toLines( unsigned int line_width, std::vector< std::string >& out_lines ) const
{
	ASSERT( line_width > 0 );

	std::string line;

	BOOST_FOREACH( const std::string& word, m_words )
	{
		if ("\n" == word)
		{
			out_lines.push_back( line );
			line.clear();
			continue;
		}

		unsigned int word_len( word.length() );

		if ((!line.empty()) && ((line.length() + word.length()) > line_width))
		{
			out_lines.push_back( line );
			line.clear();
		}

		if ((line.empty()) && (" " == word))
			continue;	//skip whitespace at the beginning of a new line

		if (word_len < line_width)
		{
			line += word;
		}
		else
		{
			std::string::const_iterator iter( word.begin() );
			std::string::const_iterator end( word.end() );
			for (; static_cast<unsigned int>( std::distance( iter, end ) ) > line_width; 
				iter += line_width)
			{
				std::string part_word( iter, iter + line_width );
				out_lines.push_back( part_word );
			}
			line += std::string( iter, end );
			ASSERT( line.length() < line_width );
		}
	}

	if (!line.empty())
	{
		out_lines.push_back( line );
	}
}




		
class UserMessageLogImpl
{

	typedef UserMessageLog::StoredMsg StoredMsg;
	typedef UserMessageLog::Msg Msg;
	typedef UserMessageLog::MsgStore MsgStore;

    MsgStore m_msg_log;
    unsigned int m_num_added; //used to emulate a size/capacity distinction.

public:

	void addMessage(const Msg& msg);
    void removeLastMessage(void);
    void beginNewMessage(void);

	typedef MsgStore::const_iterator const_iterator;
	typedef MsgStore::const_reverse_iterator const_reverse_iterator;
	const_iterator begin(void) const;
    const_iterator end(void) const;
	const_reverse_iterator rbegin(void) const;
    const_reverse_iterator rend(void) const;
	unsigned int size(void)	{return m_num_added;}

    UserMessageLogImpl(unsigned int in_size)
        : m_msg_log(in_size, StoredMsg(Msg("")) ), m_num_added(0)
    {
    }

private:

    UserMessageLogImpl(void);
    UserMessageLogImpl(const UserMessageLogImpl&);
    UserMessageLogImpl& operator=(const UserMessageLogImpl&);
};



void UserMessageLogImpl::addMessage(const Msg& msg)
{
	StoredMsg add_msg( msg );

	StoredMsg::Words& last_msg_words( m_msg_log.front().words() );

	if ( (0 == m_num_added) 
		|| (  (!last_msg_words.empty()) && ("\n" == last_msg_words.back())  ) 
		)
	{
		if (m_num_added < m_msg_log.size())
			++m_num_added;
		m_msg_log.pop_back();
		m_msg_log.push_front( add_msg );
    }
    else
    {
		if (!last_msg_words.empty())
		{
			last_msg_words.push_back(" ");
		}

		BOOST_FOREACH( const std::string& word, add_msg.words() )
		{
			last_msg_words.push_back( word );
		}
    }
}

void UserMessageLogImpl::removeLastMessage(void)
{
    if (m_num_added > 0)
    {
        --m_num_added;
        m_msg_log.pop_front();
    }
}

void UserMessageLogImpl::beginNewMessage(void)
{
	if (0 < m_num_added)
	{
		StoredMsg::Words& last_msg_words( m_msg_log.front().words() );

		if ( (last_msg_words.empty()) || ("\n" != last_msg_words.back()) )
		{
			last_msg_words.push_back( "\n" );
		}
	}
}

UserMessageLogImpl::const_iterator UserMessageLogImpl::begin(void) const
{
    return m_msg_log.begin();
}
UserMessageLogImpl::const_iterator UserMessageLogImpl::end(void) const
{
    if (m_num_added < m_msg_log.size())
    {
        const_iterator ret = begin();
        advance(ret, m_num_added);
        return ret;
    }
    return m_msg_log.end();
}
UserMessageLog::const_reverse_iterator UserMessageLogImpl::rbegin(void) const
{
	return const_reverse_iterator( end() );
}
UserMessageLog::const_reverse_iterator UserMessageLogImpl::rend(void) const
{
	return const_reverse_iterator( begin() );
}









UserMessageLog::UserMessageLog(unsigned int size)
: m_impl( new UserMessageLogImpl(size) )
{
}

UserMessageLog::~UserMessageLog()
{
}

void UserMessageLog::addMessage(const Msg& msg)
{
	m_impl->addMessage(msg);
}

void UserMessageLog::removeLastMessage(void)
{
	m_impl->removeLastMessage();
}

void UserMessageLog::beginNewMessage(void)
{
	m_impl->beginNewMessage();
}

UserMessageLog::const_iterator UserMessageLog::begin(void) const
{
	return m_impl->begin();
}

UserMessageLog::const_iterator UserMessageLog::end(void) const
{
	return m_impl->end();
}

UserMessageLog::const_reverse_iterator UserMessageLog::rbegin(void) const
{
	return m_impl->rbegin();
}

UserMessageLog::const_reverse_iterator UserMessageLog::rend(void) const
{
	return m_impl->rend();
}

unsigned int UserMessageLog::size(void)
{
	return m_impl->size();
}




}

