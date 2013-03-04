/**
@file

Copyright John Reid 2008

*/

#include "boost/python/detail/wrap_python.hpp"

#include <myrrh/defs.h>
#include <myrrh/python/boost_range.h>

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include <iostream>

struct test_struct {
	unsigned _i;
	test_struct( unsigned i ) : _i( i ) { }
	typedef boost::shared_ptr< test_struct > ptr;
};

test_struct::ptr make_test_data_type( unsigned i ) { 
	return test_struct::ptr( new test_struct( i ) ); 
}

void check_test_data_type_sequence( boost::python::object py_seq, unsigned first_value )
{
	BOOST_FOREACH( test_struct const& t, myrrh::python::make_boost_range< test_struct const& >( py_seq ) )
	{
		if( first_value != t._i )
			throw std::logic_error(
				MYRRH_MAKE_STRING(
					"Value not as expected: (value) "<<t._i<< "!= "<<first_value<<" (expected)" ) );
		++first_value;
	}
}

BOOST_PYTHON_MODULE( test_python_boost_range )
{
	using namespace boost::python;
	using boost::python::arg;

	class_<
		test_struct,
		test_struct::ptr
	>( 
		"TestDataType",
		"Dummy data type used for testing.",
		no_init
	);

	def(
		"make_test_data_type",
		make_test_data_type,
		"Creates a new test data type."
	);

	def(
		"check_test_data_type_sequence",
		check_test_data_type_sequence,
		"Checks the values of the test data type in the sequence."
	);
}
