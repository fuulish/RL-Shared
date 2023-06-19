#ifndef RL_SHARED_SYSTEM_HPP_INCLUDED
#define	RL_SHARED_SYSTEM_HPP_INCLUDED


#ifdef _DEBUG
#else //_DEBUG

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
#define IS_UNIX
#elif defined(_WIN32)
#define IS_WINDOWS
#endif

#endif //_DEBUG



#endif
