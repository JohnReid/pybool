/**
Copyright John Reid 2009
*/

#ifndef MYRRH_TYPE_NAMES_H_
#define MYRRH_TYPE_NAMES_H_

#include <boost/multi_array.hpp>

#include <typeinfo>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>

#include <myrrh/defs.h>

namespace myrrh {




template< typename T >
struct type_name {
	static std::string name() {
		return typeid( T ).name();
	}
};

template< typename T >
struct type_name< T * > {
	static std::string name() {
		return MYRRH_MAKE_STRING( type_name< T >::name() << " *" );
	}
};

#define SPECIALISE_TYPE_NAME( T, N ) \
template< > \
struct type_name< T > { \
	static std::string name() { \
		return N; \
	} \
};

SPECIALISE_TYPE_NAME( double         , "double" );
SPECIALISE_TYPE_NAME( float          , "float" );
SPECIALISE_TYPE_NAME( char           , "char" );
SPECIALISE_TYPE_NAME( unsigned char  , "unsigned char" );
SPECIALISE_TYPE_NAME( short          , "short" );
SPECIALISE_TYPE_NAME( unsigned short , "unsigned short" );
SPECIALISE_TYPE_NAME( int            , "int" );
SPECIALISE_TYPE_NAME( unsigned int   , "unsigned int" );
SPECIALISE_TYPE_NAME( long           , "long" );
SPECIALISE_TYPE_NAME( unsigned long  , "unsigned long" );


template< typename T >
std::string get_type_name( T ) {
	return type_name< T >::name();
}




//
// std container specialisations
//
template< typename T >
struct type_name< std::vector< T > > {
	static std::string name() {
		return MYRRH_MAKE_STRING( "std::vector< " << type_name< T >::name() << " >" );
	}
};

template< typename T >
struct type_name< std::list< T > > {
	static std::string name() {
		return MYRRH_MAKE_STRING( "std::list< " << type_name< T >::name() << " >" );
	}
};

template< typename T >
struct type_name< std::deque< T > > {
	static std::string name() {
		return MYRRH_MAKE_STRING( "std::deque< " << type_name< T >::name() << " >" );
	}
};

template< typename K, typename V >
struct type_name< std::map< K, V > > {
	static std::string name() {
		return MYRRH_MAKE_STRING( "std::map< " << type_name< K >::name() << ", " << type_name< V >::name() << " >" );
	}
};

template< typename K, typename V >
struct type_name< std::multimap< K, V > > {
	static std::string name() {
		return MYRRH_MAKE_STRING( "std::multimap< " << type_name<  K >::name() << ", " << type_name< V >::name() << " >" );
	}
};

template< typename T >
struct type_name< std::set< T > > {
	static std::string name() {
		return MYRRH_MAKE_STRING( "std::set< " << type_name< T >::name() << " >" );
	}
};



//
// boost containers
//
template <typename V, std::size_t N >
struct type_name< boost::multi_array< V, N > > {
	static std::string name() {
		return MYRRH_MAKE_STRING( "boost::multi_array< " << type_name< V >::name() << ", " << N << " >" );
	}
};

} //namespace myrrh

#endif //MYRRH_TYPE_NAMES_H_*/
