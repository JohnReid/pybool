/**
@file

Copyright John Reid 2006

*/

#ifdef _MSC_VER
# pragma warning( disable : 4244 )
#endif //_MSC_VER

#include "myrrh/chain_iterator.h"

#include <boost/progress.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/iterator/counting_iterator.hpp>

using namespace myrrh;
using namespace boost;
using namespace boost::unit_test;
using namespace boost::assign;
using namespace std;


void
check_chain_iterator()
{
	typedef std::vector< int > vector;
	typedef chain_iterator< 
		boost::range_const_iterator< vector >::type,
		boost::range_const_iterator< vector >::type
	> iterator;

	{
		const vector v1 = list_of(1)(2)(3)(4);
		const vector v2 = list_of(5)(6)(7)(8);

		//BOOST_FOREACH( int i, make_chain_iterator_range( v1, v2 ) ) std::cout << i << "\n";
		BOOST_CHECK( std::equal( make_chain_iterator_begin( v1, v2 ), make_chain_iterator_begin( v1, v2 ), counting_iterator< int >( 1 ) ) );

		iterator i = make_chain_iterator_begin( v1, v2 );
		iterator j = make_chain_iterator_begin( v1, v2 );
		j = next( j, 6 );
		BOOST_CHECK_EQUAL( j - i, 6 );
		BOOST_CHECK_EQUAL( i - j, -6 );
		BOOST_CHECK_EQUAL( *j, 7 );
		BOOST_CHECK_EQUAL( *prior( j, 6 ), 1 );
	}

	typedef std::deque< int > deque;
	typedef chain_iterator< 
		boost::range_const_iterator< vector >::type,
		boost::range_const_iterator< deque >::type
	> hetero_iterator;
	{
		const vector v1 = list_of(1)(2)(3)(4);
		deque l2 = list_of(5)(6)(7)(8);
		BOOST_CHECK( std::equal( make_chain_iterator_begin( v1, l2 ), make_chain_iterator_begin( v1, l2 ), counting_iterator< int >( 1 ) ) );

		hetero_iterator i = make_chain_iterator_begin( v1, l2 );
		hetero_iterator j = make_chain_iterator_begin( v1, l2 );
		j = next( j, 6 );
		BOOST_CHECK_EQUAL( j - i, 6 );
		BOOST_CHECK_EQUAL( i - j, -6 );
		BOOST_CHECK_EQUAL( *j, 7 );
		BOOST_CHECK_EQUAL( *prior( j, 6 ), 1 );
	}
}



test_suite *
init_unit_test_suite( int argc, char * argv [] )
{
	test_suite * test = BOOST_TEST_SUITE( "Myrrh chain iterator test suite" );

	try
	{
		test->add( BOOST_TEST_CASE( &check_chain_iterator ), 0);
	}
	catch (const std::exception & e)
	{
		cerr << "Exception: " << e.what() << endl;
	}

    return test; 
}

