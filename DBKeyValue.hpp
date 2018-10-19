#ifndef RL_SHARED_DBKEYVALUE_HPP
#define RL_SHARED_DBKEYVALUE_HPP


#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <string>


namespace RL_shared
{

	enum {INVALID_KEY = -1};

	class DBKeyValue
	{
		boost::int32_t m_val;
	public:
		DBKeyValue(void) : m_val(INVALID_KEY)
		{
		}
		DBKeyValue(boost::int32_t val)
		{
			m_val = val;
		}
		operator boost::int32_t() const
		{
			return m_val;
		}
		bool operator == (const DBKeyValue& other) const
		{
			return other.m_val == m_val;
		}
		bool operator != (const DBKeyValue& other) const
		{
			return !(*this == other);
		}
		void operator ++ (void)
		{
			++m_val;
		}

		std::string asString(void) {
			return boost::lexical_cast<std::string>(m_val);
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & m_val;
		}
	};

}


#endif
