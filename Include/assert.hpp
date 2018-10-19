#ifndef RL_SHARED_ASSERT_HPP_INCLUDED
#define	RL_SHARED_ASSERT_HPP_INCLUDED


#ifdef _DEBUG

template <typename T_>
inline void my_assert( T_ b )
{
	if (!b)
	{
		__asm int 3;
	}
}
#define ASSERT(_X_)	my_assert( (_X_) );

#else //_DEBUG

#define ASSERT(_X_)	

#endif //_DEBUG



#endif
