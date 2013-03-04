/**
@file

Copyright John Reid 2006

*/

#ifdef _MSC_VER
# pragma warning( disable : 4244 )
#endif //_MSC_VER

#include "myrrh/bit_set.h"
#include "myrrh/type_names.h"

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
using namespace myrrh;


std::vector< unsigned > sizes = list_of(163)(255)(256)(257)(0)(1)(3)(15)(16)(31)(32);


void
check_word_size_traits()
{
	cout << "************************ check_word_size_traits ****************************\n";

	using namespace myrrh::detail;

	typedef word_size_traits< sizeof( unsigned long ) > wst;

	BOOST_CHECK_EQUAL( wst::size, 8 );
	BOOST_CHECK_EQUAL( wst::bits_in_word, 64 );
	BOOST_CHECK_EQUAL( wst::num_shifts, 6 );
	BOOST_CHECK_EQUAL( wst::index_word, 63 );

	BOOST_CHECK_EQUAL( wst::word_idx(93), 1 );
	BOOST_CHECK_EQUAL( wst::bit_idx(93), 93-64 );
	BOOST_CHECK_EQUAL( wst::word_idx(32), 0 );
	BOOST_CHECK_EQUAL( wst::bit_idx(32), 32 );
}

template< typename StorageT >
void
check_bit_set( const unsigned num_bits )
{
	cout << "************************ check_bit_set: " << type_name< StorageT >::name() << " : " << num_bits << " ****************************\n";

	typedef myrrh::bit_set< StorageT > bs;
	//std::cout << "sizeof(bs): " << sizeof(bs) << "\n";

	typedef typename bs::wst wst;
	//std::cout << "Index word: " << wst::index_word << "\n";
	for( unsigned i = 0; num_bits != i; ++i ) {
		//std::cout << i << "\n";
		BOOST_REQUIRE_EQUAL( wst::bit_idx( i ), i % wst::bits_in_word );
		BOOST_REQUIRE_EQUAL( wst::word_idx( i ), i / wst::bits_in_word );
	}

	bs bits( num_bits );
	for( unsigned i = 0; num_bits != i; ++i )
	{
		for( unsigned j = 0; num_bits != j; ++j )
		{
			BOOST_REQUIRE_MESSAGE( ! bits.get(j), i << " " << j );
		}
		BOOST_REQUIRE_MESSAGE( ! bits.get(i), i );
		bits.set(i);
		for( unsigned j = 0; num_bits != j; ++j )
		{
			if( i == j ) {
				BOOST_REQUIRE_MESSAGE( bits.get(j), i << " " << j );
			} else {
				BOOST_REQUIRE_MESSAGE( ! bits.get(j), i << " " << j );
			}
		}
		bits.unset(i);
		BOOST_CHECK( ! bits.get(i) );
	}
}



test_suite *
init_unit_test_suite( int argc, char * argv [] )
{
	test_suite * test = BOOST_TEST_SUITE( "Myrrh bit set test suite" );

	try
	{
		test->add( BOOST_TEST_CASE( &check_word_size_traits ), 0);
		test->add( BOOST_PARAM_TEST_CASE( &check_bit_set< unsigned long >, sizes.begin(), sizes.end() ), 0);
		test->add( BOOST_PARAM_TEST_CASE( &check_bit_set< unsigned char >, sizes.begin(), sizes.end() ), 0);
		test->add( BOOST_PARAM_TEST_CASE( &check_bit_set< unsigned short >, sizes.begin(), sizes.end() ), 0);
		test->add( BOOST_PARAM_TEST_CASE( &check_bit_set< unsigned int >, sizes.begin(), sizes.end() ), 0);
	}
	catch (const std::exception & e)
	{
		cerr << "Exception: " << e.what() << endl;
	}

    return test;
}

