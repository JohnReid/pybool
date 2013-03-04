/**
@file

Copyright John Reid 2009

*/

#include "boost/python/detail/wrap_python.hpp"

#include <myrrh/defs.h>
#include <myrrh/python/numpy_converter.h>
#include <myrrh/python/convert.h>

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include <iostream>

namespace myrrh {
namespace python {

numpy_converter converter;

} //namespace myrrh
} //namespace myrrh



template< typename T >
boost::python::object check_numpy_conversion( boost::python::object py_obj )
{
	T c_obj;
	myrrh::python::convert_from_python( py_obj, c_obj );
	return myrrh::python::convert_to_python( c_obj );
}

BOOST_PYTHON_MODULE( test_python_numpy_converter )
{
	using namespace boost::python;
	using boost::python::arg;

	def(
		"check_numpy_conversion_multi_array",
		check_numpy_conversion< boost::multi_array< double, 2 > >,
		"Check numpy conversion for boost multi_array."
	);

}
