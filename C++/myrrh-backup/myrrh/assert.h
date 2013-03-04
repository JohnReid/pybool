/**
@file

Copyright John Reid 2007

*/

#include "myrrh/defs.h"

#include <boost/current_function.hpp>

#include <exception>
#include <stdexcept>
#include <cassert>


#ifdef MYRRH_DISABLE_ASSERTS

#define MYRRH_ASSERT( x )

#else //MYRRH_DISABLE_ASSERTS

#ifdef MYRRH_ASSERT_THROWS
#define MYRRH_ASSERT( x ) if( ! ( x ) ) ::myrrh::assertion_throw( #x, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__ )

#else //MYRRH_ASSERT_THROWS

# ifdef MYRRH_ENABLE_ASSERT_HANDLER
# define MYRRH_ASSERT( x ) if( ! ( x ) ) ::myrrh::assertion_failed( #x, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__ )

# else //MYRRH_ENABLE_ASSERT_HANDLER
# define MYRRH_ASSERT( x ) assert( ( x ) )

# endif //MYRRH_ENABLE_ASSERT_HANDLER

#endif //MYRRH_ASSERT_THROWS

#endif //MYRRH_DISABLE_ASSERTS



namespace myrrh {



/**
Not implemented in myrrh.

Implement this function yourself and define MYRRH_ENABLE_ASSERT_HANDLER to have your functionality called when MYRRH_ASSERT( x ) fails.

If MYRRH_ASSERT_THROWS is defined it will override MYRRH_ENABLE_ASSERT_HANDLER.
*/
void assertion_failed( char const * expr, char const * function, char const * file, long line );



inline void assertion_throw( char const * expr, char const * function, char const * file, long line )
{
	throw std::logic_error( MYRRH_MAKE_STRING( file<<"("<<line<<"): error in \""<<function<<"\": \""<<expr<<"\" is false" ) );
}



} //namespace myrrh



