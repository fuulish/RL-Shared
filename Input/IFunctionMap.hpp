#ifndef RL_SHARED_IFUNCTIONMAP_HPP
#define	RL_SHARED_IFUNCTIONMAP_HPP



#include "Abstract/AUserInputItem.hpp"
#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <vector>



namespace RL_shared
{

class IFunctionMap
{
public:

	virtual ~IFunctionMap()
	{
	}

	virtual bool isFunction( const AUserInputItem&, const std::string& ) const = 0;

	typedef std::map< std::string, std::vector<std::string> > FunctionToControlsMap;
	virtual boost::shared_ptr< const FunctionToControlsMap > getFunctions(void) const = 0;
};

}




#endif
