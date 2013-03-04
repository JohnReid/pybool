/**
@file

Copyright John Reid 2008

*/

#ifdef _MSC_VER
# pragma warning( disable : 4244 )
#endif //_MSC_VER

#include "myrrh/seqan_boost_range_adaptors.h"

#include <boost/progress.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/iterator/counting_iterator.hpp>

using namespace boost;
using namespace boost::unit_test;
using namespace boost::assign;
using namespace std;


void
check_string_set_iterator()
{
	cout << "*********************** check_string_set_iterator()\n";
	typedef seqan::String< seqan::Dna5 >       string_t;            /**< The string type. */
	typedef seqan::StringSet< string_t >       string_set_t;        /**< The string set type. */

	string_set_t string_set;
	seqan::appendValue( string_set, "ACGTGGACTGCGCTAGGCTCGCGC" );
	BOOST_CHECK_EQUAL( 1, seqan::length( string_set ) );

	BOOST_FOREACH( string_t & s, string_set ) {
		cout << seqan::length( s ) << "\n";
	}

	BOOST_FOREACH( const string_t & s, const_cast< const string_set_t & >( string_set ) ) {
		cout << seqan::length( s ) << "\n";
	}
}



test_suite *
init_unit_test_suite( int argc, char * argv [] )
{
	test_suite * test = BOOST_TEST_SUITE( "Myrrh seqan boost range adaptors test suite" );

	try
	{
		test->add( BOOST_TEST_CASE( &check_string_set_iterator ), 0 );
	}
	catch (const std::exception & e)
	{
		cerr << "Exception: " << e.what() << endl;
	}

    return test; 
}

