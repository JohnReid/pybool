/**
@file

Copyright John Reid 2008

*/

#ifdef _MSC_VER
# pragma warning( disable : 4244 )
#endif //_MSC_VER

#include "myrrh/type_names.h"

#include <boost/progress.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include <vector>

using namespace boost;
using namespace boost::unit_test;
using namespace boost::assign;
using namespace std;


void
check_type_names()
{
	cout << "*********************** check_type_names() ************************\n";

	BOOST_CHECK_EQUAL( myrrh::get_type_name( (short)0 )                        , "short" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( (const short) 0 )                 , "short" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( (short *)0 )                      , "short *" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( (int)0 )                          , "int" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( std::vector< int >() )            , "std::vector< int >" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( std::deque< int >() )             , "std::deque< int >" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( std::map< int, short >() )        , "std::map< int, short >" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( std::multimap< int, short >() )   , "std::multimap< int, short >" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( std::set< int >() )               , "std::set< int >" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( std::list< int >() )              , "std::list< int >" );
	BOOST_CHECK_EQUAL( myrrh::get_type_name( boost::multi_array< int, 3 >() )  , "boost::multi_array< int, 3 >" );
}



test_suite *
init_unit_test_suite( int argc, char * argv [] )
{
	test_suite * test = BOOST_TEST_SUITE( "Myrrh seqan boost range adaptors test suite" );

	try
	{
		test->add( BOOST_TEST_CASE( &check_type_names ), 0 );
	}
	catch (const std::exception & e)
	{
		cerr << "Exception: " << e.what() << endl;
	}

    return test; 
}

