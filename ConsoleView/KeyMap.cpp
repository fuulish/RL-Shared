#include "KeyMap.hpp"
#include <boost/assign.hpp>
#include <fstream>
#include <algorithm>
#include <exception>
#include "Include/system.hpp"
#if defined(IS_UNIX)
#include <curses.h>
#define KEY_ENTER 10
#elif defined(IS_WINDOWS)
#define KEY_UP    72
#define KEY_DOWN  80
#define KEY_LEFT  75
#define KEY_RIGHT 77
#define KEY_ENTER 13
#endif



namespace RL_shared
{



KeyMap::KeyMap(void)
: m_display_map( new FunctionToControlsMap() )
{
}

bool KeyMap::isFunction( const AUserInputItem& akey, const std::string& function ) const
{
	const KeyPress& keyp( dynamic_cast< const KeyPress& >( akey ) );
	KeyCode key( keyp.value );

	KeyFunctionMap::const_iterator entry( m_map.find( function ) );
	if (m_map.end() != entry)
		return (entry->second.end() != std::find(entry->second.begin(), entry->second.end(), key));

	return false;
}

boost::shared_ptr< const KeyMap::FunctionToControlsMap > KeyMap::getFunctions(void) const
{
	return m_display_map;
}

void KeyMap::setFunction( const KeyPress& key, const std::string& function, const std::string& key_name )
{
	m_map[ function ].push_back( key.value );
	(*m_display_map)[ function ].push_back( key_name );
}

namespace 
{
	const KeyCode Esc(27, false );
	const KeyCode Enter( KEY_ENTER, false );
	const KeyCode Up( KEY_UP, true );
	const KeyCode Left( KEY_LEFT, true );
	const KeyCode Down( KEY_DOWN, true );
	const KeyCode Right( KEY_RIGHT, true );
	const KeyCode UpAndLeft( 71, true );
	const KeyCode DownAndLeft( 79, true );
	const KeyCode DownAndRight( 81, true );
	const KeyCode UpAndRight( 73, true );

	const KeyCode BackSpace( 8, false );
	const KeyCode Space( ' ', false );

	std::map< std::string, KeyCode > specials = 
		boost::assign::map_list_of
			( "Esc", Esc )
			( "Enter", Enter )
			( "Up", Up )
			( "UpAndLeft", UpAndLeft )
			( "Left", Left )
			( "DownAndLeft", DownAndLeft )
			( "Down", Down )
			( "DownAndRight", DownAndRight )
			( "Right", Right )
			( "UpAndRight", UpAndRight )
			( "Space", Space )
			( "BackSpace", BackSpace )
			;
}

void KeyMap::readFromFile( const boost::filesystem::path& path )
{
	std::ifstream file( path.string().c_str() );

	if (!file.good())
		throw std::runtime_error("Failed to read keymap file");

	while (!file.eof())
	{
		std::string function, key;
		file >> function;
		file >> key;

		if (key.length() == 1)
		{
			setFunction( KeyCode( key.c_str()[0], false ), function, key );
		}
		else
		{
			std::map< std::string, KeyCode >::const_iterator look( specials.find( key ) );
			if (specials.end() != look)
			{
				setFunction( look->second, function, key );
			}
		}
	}
}





KeyMapFile::KeyMapFile( const boost::filesystem::path& filename )
	: m_filename( filename )
{
	readFromFile( filename );
}

KeyMapFile::~KeyMapFile()
{
}



}
