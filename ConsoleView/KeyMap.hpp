#ifndef RL_SHARED_KEYMAP_HPP
#define	RL_SHARED_KEYMAP_HPP



#include "Input/IFunctionMap.hpp"
#include "KeyPress.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>
#include <string>



namespace RL_shared
{



class KeyMap : public IFunctionMap, public boost::noncopyable
{
public:

	KeyMap(void);

	virtual bool isFunction( const AUserInputItem&, const std::string& ) const;
	virtual boost::shared_ptr< const FunctionToControlsMap > getFunctions(void) const;

	void setFunction( const KeyPress&, const std::string&, const std::string& );

	void readFromFile( const boost::filesystem::path& );

private:

	typedef std::map< std::string, std::vector< KeyCode > > KeyFunctionMap;
	KeyFunctionMap m_map;

	boost::shared_ptr< FunctionToControlsMap > m_display_map;
};



class KeyMapFile : public KeyMap
{
public:

	explicit KeyMapFile( const boost::filesystem::path& );
	~KeyMapFile();

private:

	boost::filesystem::path m_filename;
};



}



#endif
