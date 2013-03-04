/** Copyright John Reid 2009
*/

#include <myrrh/python/multi_array_to_numpy.h>


namespace myrrh {
namespace python {
std::string exposed_typechars;
}
}


BOOST_PYTHON_MODULE( _man )
{
	import_array();

	using boost::python::scope;

#ifndef NDEBUG
	scope().attr("__debug__") = 1;
	std::cout << "WARNING: Debug version of _man module loaded. If you did not intend this then check your configuration!" << std::endl;
#else //_DEBUG
	scope().attr("__debug__") = 0;
#endif //_DEBUG

	using namespace myrrh::python;

	//expose_converters< npy_byte >();
	//expose_converters< npy_ubyte >();
	//expose_converters< npy_short >();
	//expose_converters< npy_ushort >();
	//expose_converters< npy_int >();
	//expose_converters< npy_uint >();
	//expose_converters< npy_long >();
	//expose_converters< npy_ulong >();
	//expose_converters< npy_longlong >();
	//expose_converters< npy_ulonglong >();
	//expose_converters< npy_float >();
	expose_converters< npy_double >();
	//expose_converters< npy_longdouble >();
	//expose_converters< npy_cfloat >();
	//expose_converters< npy_cdouble >();
	//expose_converters< npy_clongdouble >();

	expose_man_fns();

#define DEF_TEST_FNS(z, n, T) tester< T, n+1 >::def_test_fns();
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_byte)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_ubyte)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_short)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_ushort)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_int)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_uint)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_long)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_ulong)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_longlong)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_ulonglong)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_float)
BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_double)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_longdouble)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_cfloat)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_cdouble)
//BOOST_PP_REPEAT(NUMPY_MAX_DIMS, DEF_TEST_FNS, npy_clongdouble)
#undef DEF_TEST_FNS
}
