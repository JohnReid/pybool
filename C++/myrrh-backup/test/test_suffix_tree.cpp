/**
@file

Copyright John Reid 2007

*/

#ifdef _DEBUG
#define MYRRH_DEBUG_SUFFIX_TREE
#endif //_DEBUG

#include "myrrh/suffix_tree.h"
#include "myrrh/suffix_tree_graph.h"
#include "myrrh/chain_iterator.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/scoped_array.hpp>
#include <boost/graph/graphviz.hpp>

#include <iostream>
#include <fstream>


using namespace myrrh;
using namespace myrrh::suffix_tree;
using namespace boost;
using namespace boost::unit_test;
using namespace boost::assign;
using namespace std;


const std::vector< std::string > strings = list_of
	( "xyzxzxy" )
	( "mississippi" )
	( "abababc" )
	( "bananas" )
	( "bookkeeper" )
	;

/** Checks all suffixes up to a given length are in the tree. */
template< typename NodeT >
bool
check_all_suffixes_are_in_tree( typename NodeT::ptr root, typename NodeT::iterator begin, typename NodeT::iterator end, int max_suffix_length = 0 )
{
	typedef typename NodeT::iterator iterator;
	typedef typename NodeT::lookup_result lookup_result;
	for( iterator e = begin; true; ++e )
	{
		iterator b = max_suffix_length ? prior( e, min( max_suffix_length, e-begin) ) : begin;
		while( true )
		{
			lookup_result suffix = root->lookup( b, e );
			if( ! suffix )
			{
				std::copy( b, e, std::ostream_iterator< typename NodeT::char_t >( cerr ) );
				std::cerr << " is not in tree\n";
				return false;
			}
			if( e == b ) break;
			++b;
		}
		if( end == e ) break;
	}
	return true;
}

/** Tree visitor that checks that the suffixes in the tree are in the string. Also check no suffix is duplicated. */
template< typename NodeT >
struct check_suffixes_are_in_string
{
	typedef typename NodeT::iterator iterator;
	typedef typename NodeT::string string;
	typedef typename boost::iterator_range< iterator > range;
	typedef std::set< string > already_seen_set;
	range _s;
	already_seen_set & _already_seen;
	check_suffixes_are_in_string( 
		iterator s_begin, 
		iterator s_end,
		already_seen_set & already_seen )
		: _s( s_begin, s_end )
		, _already_seen( already_seen )
	{ }
	void operator()( iterator begin, iterator end ) const
	{
		using namespace boost;
		BOOST_CHECK( find_first( _s, make_iterator_range( begin, end ) ) );
		string suffix( begin, end );
		BOOST_CHECK( _already_seen.end() == _already_seen.find( suffix ) );
		_already_seen.insert( suffix );
	}
};

void
check_suffix_tree( const std::string & s)
{
	typedef sibling_list_node< string::const_iterator > suffix_tree_node;
	//std::cout << sizeof(suffix_tree_node) << "\n";
	typedef suffix_tree_node::shared_ptr shared_ptr;
	shared_ptr root( ukkonen::new_tree( s.begin(), s.end() ) );

	//build graph
	write_dot_file< suffix_tree_node >( root, MYRRH_MAKE_STRING( "graphs/" << s << ".dot" ) );

	//check all suffixes in the string are in the tree
	BOOST_CHECK( check_all_suffixes_are_in_tree< suffix_tree_node >( root.get(), s.begin(), s.end() ) );

	//check all the suffixes in the tree are in the string
	std::set< suffix_tree_node::string > already_seen;
	root->visit_suffixes( check_suffixes_are_in_string< suffix_tree_node >( s.begin(), s.end(), already_seen ) );

	//print the suffixes
	//root->visit_suffixes( suffix_printer< suffix_tree_node >() );
}

void check_multiple_suffix_tree()
{
	using namespace boost;

	const char * s1 = "xabxa$";
	const char * s2 = "babxba$";

	typedef const char * iterator;
	iterator s1_end = s1 + strlen( s1 );
	iterator s2_end = s2 + strlen( s2 );

	typedef multi_sibling_list_node< iterator > suffix_tree_node;
	typedef suffix_tree_node::shared_ptr shared_ptr;
	shared_ptr root( ukkonen::new_tree( s1, s1_end ) );
	write_dot_file< suffix_tree_node >( root, MYRRH_MAKE_STRING( "graphs/xabxa.dot" ) );
	ukkonen::incremental_update< suffix_tree_node >( root.get(), s2, s2_end, suffix_tree_node::node_creator() );
	write_dot_file< suffix_tree_node >( root, MYRRH_MAKE_STRING( "graphs/xabxa_and_babxba.dot" ) );

	//check all suffixes in all the strings are in the tree
	BOOST_CHECK( check_all_suffixes_are_in_tree< suffix_tree_node >( root.get(), s1, s1_end-1 ) );
	//BOOST_CHECK( check_all_suffixes_are_in_tree< suffix_tree_node >( root.get(), s2, s2_end-1 ) );
}

void check_suffix_tree_2( const std::string & s )
{
	using namespace boost;

	boost::scoped_array< char > _s( new char[ s.size() ] );
	std::copy( s.begin(), s.end(), _s.get() );

	typedef const char * base_iterator;
	typedef chain_iterator< base_iterator, base_iterator > iterator;
	typedef sibling_list_node< base_iterator > suffix_tree_node;
	std::cout << "sizeof(base_iterator): " << sizeof(base_iterator) << "\n";
	std::cout << "sizeof(shared_ptr<char>): " << sizeof(shared_ptr<char>) << "\n";
	std::cout << "sizeof(scoped_ptr<char>): " << sizeof(scoped_ptr<char>) << "\n";
	std::cout << "sizeof(auto_ptr<char>): " << sizeof(auto_ptr<char>) << "\n";
	std::cout << "sizeof(char *): " << sizeof(char *) << "\n";
	std::cout << "sizeof(suffix_tree_node::label): " << sizeof(suffix_tree_node::label) << "\n";
	std::cout << "sizeof(suffix_tree_node): " << sizeof(suffix_tree_node) << "\n";

	base_iterator s_begin = _s.get();
	base_iterator s_end = s_begin + s.size();

	typedef suffix_tree_node::shared_ptr shared_ptr;
	shared_ptr root( new suffix_tree_node( s_end, s_end ) );

	base_iterator begin = s_begin;
	base_iterator end = next( begin, min( 4, s_end - begin ) );
	while( s_end != begin )
	{
		ukkonen::update_tree< suffix_tree_node >( root.get(), begin, end, begin, simple_node_creator< suffix_tree_node >() );
		if( s_end != end ) ++end;
		++begin;
	}

	//print the suffixes
	root->visit_suffixes( suffix_printer< suffix_tree_node >() );

	//check all suffixes in the string are in the tree
	BOOST_CHECK( check_all_suffixes_are_in_tree< suffix_tree_node >( root.get(), s_begin, s_end, 4 ) );
}


test_suite *
init_unit_test_suite( int argc, char * argv [] )
{
	test_suite * test = BOOST_TEST_SUITE( "Myrrh suffix tree test suite" );

	try
	{
		test->add( BOOST_PARAM_TEST_CASE( &check_suffix_tree, strings.begin(), strings.end() ), 0);
		//test->add( BOOST_PARAM_TEST_CASE( &check_suffix_tree_2, strings.begin(), strings.end() ), 0);
		//test->add( BOOST_TEST_CASE( &check_multiple_suffix_tree ), 0);
	}
	catch (const std::exception & e)
	{
		cerr << "Exception: " << e.what() << endl;
	}

    return test; 
}

