/** Copyright John Reid 2009
*/

#include <myrrh/python/multi_array_to_numpy.h>

namespace myrrh {
namespace python {

unsigned long
max_numpy_dimensions() {
	return NUMPY_MAX_DIMS;
}

/** The type of numpy arrays. */
const PyTypeObject *
get_PyArray_Type()
{
	return &PyArray_Type;
}

void
show_array_info( py::object array )
{
	PyArrayObject * py_array = ( PyArrayObject * )array.ptr();
	std::cout << "Dimensions: " << py_array->nd << "\n";
	std::cout << "Shape: ";
	std::copy( py_array->dimensions, py_array->dimensions + py_array->nd, std::ostream_iterator< npy_intp >( std::cout, " " ) );
	std::cout << "\n";
	std::cout << "Strides: ";
	std::copy( py_array->strides, py_array->strides + py_array->nd, std::ostream_iterator< npy_intp >( std::cout, " " ) );
	std::cout << "\n";
	std::cout << "Data: " << ( void * )( py_array->data ) << "\n";
}

void
expose_man_fns() {
	py::def( "show_array_info", show_array_info, "Print info about shape of array" );
	py::def( "max_numpy_dimensions", max_numpy_dimensions, "The maximum number of dimensions we can convert to/from C++" );
	py::scope().attr( "exposed_typechars" ) = exposed_typechars;
}

} //namespace python
} //namespace myrrh

