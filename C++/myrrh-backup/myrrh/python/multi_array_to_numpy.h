/** Copyright John Reid 2009
*/

#include <boost/python.hpp>
#include <boost/range.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>
#include <boost/array.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include <boost/preprocessor/array/elem.hpp>
#include <numpy/arrayobject.h>
#include <iostream>
#include <myrrh/python/boost_range.h>
#include <myrrh/defs.h>

#define NUMPY_MAX_DIMS 11 // or NPY_MAXDIMS

namespace myrrh {
namespace python {

unsigned long
max_numpy_dimensions();

/** The type of numpy arrays. */
const PyTypeObject *
get_PyArray_Type();

void
show_array_info( boost::python::object array );

void
expose_man_fns();




namespace {

namespace py = boost::python;


/** Specialised by types. */
template< typename T >
struct numpy_type {
	NPY_TYPES typenum() const {
		throw std::runtime_error( std::string( "Do not know numpy typenum for type " ) + typeid( T ).name() );
	}
	NPY_TYPECHAR typechar() const {
		throw std::runtime_error( std::string( "Do not know numpy typechar for type " ) + typeid( T ).name() );
	}
	py::object dtype() const {
		py::object numpy = py::import( "numpy" );
		return numpy.attr( "dtype" )( std::string( 1, typechar() ) );
	}
	std::string name() const {
		return py::extract< std::string >( dtype().attr( "name" ) );
	}
};

/** Specialised to convert type_nums to types. */
template< NPY_TYPES type_num >
struct numpy_typenum {
};

/* Specialise the numpy type. */
#define SPECIALISE_NUMPY_TYPE(T, v) \
	template<> NPY_TYPES numpy_type< T >::typenum() const { return NPY_ ## v; } \
	template<> NPY_TYPECHAR numpy_type< T >::typechar() const { return NPY_ ## v ## LTR; } \
	template<> struct numpy_typenum< NPY_ ## v > { typedef T type; }; \


//SPECIALISE_NUMPY_TYPE( bool                        , BOOL )
//SPECIALISE_NUMPY_TYPE( npy_bool                    , BOOL )
SPECIALISE_NUMPY_TYPE( npy_byte                    , BYTE )
SPECIALISE_NUMPY_TYPE( npy_ubyte                   , UBYTE )
SPECIALISE_NUMPY_TYPE( npy_short                   , SHORT )
SPECIALISE_NUMPY_TYPE( npy_ushort                  , USHORT )
SPECIALISE_NUMPY_TYPE( npy_int                     , INT )
SPECIALISE_NUMPY_TYPE( npy_uint                    , UINT )
SPECIALISE_NUMPY_TYPE( npy_long                    , LONG )
SPECIALISE_NUMPY_TYPE( npy_ulong                   , ULONG )
SPECIALISE_NUMPY_TYPE( npy_longlong                , LONGLONG )
SPECIALISE_NUMPY_TYPE( npy_ulonglong               , ULONGLONG )
SPECIALISE_NUMPY_TYPE( npy_float                   , FLOAT )
SPECIALISE_NUMPY_TYPE( npy_double                  , DOUBLE )
SPECIALISE_NUMPY_TYPE( npy_longdouble              , LONGDOUBLE )
//SPECIALISE_NUMPY_TYPE( npy_cfloat                  , CFLOAT )
//SPECIALISE_NUMPY_TYPE( npy_cdouble                 , CDOUBLE )
//SPECIALISE_NUMPY_TYPE( npy_clongdouble             , CLONGDOUBLE )
//SPECIALISE_NUMPY_TYPE( std::complex< float >       , CFLOAT )
//SPECIALISE_NUMPY_TYPE( std::complex< double >      , CDOUBLE )
//SPECIALISE_NUMPY_TYPE( std::complex< long double > , CLONGDOUBLE )
//SPECIALISE_NUMPY_TYPE( boost::python::object       , OBJECT )
/* NPY_STRING, NPY_UNICODE unsupported for now */


template< typename T >
const PyTypeObject *
get_array_scalar_typeobj()
{
	const NPY_TYPES _type = numpy_type< T >().typenum();
	return (PyTypeObject *) PyArray_TypeObjectFromType( _type );
}


template< typename T >
void *
check_array_scalar( PyObject *obj )
{
	if( obj->ob_type == get_array_scalar_typeobj<T>() )
		return obj;
	else
		return 0;
}




template<class T>
static
void
convert_array_scalar(
	PyObject * obj,
	py::converter::rvalue_from_python_stage1_data* data
)
{
	void * storage = ((py::converter::rvalue_from_python_storage<T>*) data)->storage.bytes;

	// no constructor needed, only dealing with POD types
	PyArray_ScalarAsCtype(obj, reinterpret_cast<T *> (storage));

	// record successful construction
	data->convertible = storage;
}


template< std::size_t NumDims >
struct boost_extents {
	typedef typename boost::detail::multi_array::extent_gen< NumDims >::range range;
	template< typename T > range operator()( T * shape );
};

template< std::size_t NumDims >
template< typename T >
typename boost_extents< NumDims >::range
boost_extents< NumDims >::operator()( T * shape ) {
	return boost_extents< NumDims-1 >()( shape )[shape[NumDims-1]];
}

template< >
template< typename T >
typename boost_extents< 0 >::range
boost_extents< 0 >::operator()( T * shape ) {
	return boost::extents[shape[0]];
}


/** Converts numpy arrays to multi_array_refs and multi_arrays to numpy arrays. */
template<
	typename T,
	std::size_t NumDims //only go up to NPY MAXDIMS
>
struct multi_array_converter
{
	typedef boost::multi_array< T, NumDims >                           array_t;             /**< The array type. */
    typedef typename array_t::element                                  element_t;           /**< The type of the elements in the array. */
    typedef typename array_t::size_type                                size_type;           /**< The size type of the array. */
	typedef boost::shared_ptr< array_t >                               array_ptr;           /**< A shared pointer to an array. */
	typedef boost::multi_array_ref< T, NumDims >                       array_ref;           /**< The array reference type. */
	typedef typename array_ref::template array_view< NumDims >::type   array_ref_view_t;    /**< The type of a view into an array reference. */
    typedef typename array_ref::storage_order_type                     storage_order_t;     /**< The storage order type of the array. */

    /** Check we can convert obj to C++. */
    static
    void *
    check_basics( PyObject * obj )
    {
		// must be an array
		if( ! PyArray_Check( obj ) )
			return 0;

		PyArrayObject * py_array = (PyArrayObject *) obj;

		// must have correct number of dimensions
		if( NumDims != py_array->nd )
			return 0;

		// must be of correct type
		numpy_type< T > npy_type;
		if( npy_type.typenum() != py_array->descr->type_num )
			return 0;

		return obj;
    }

    /** Check we can convert obj to C++. */
    static
    void *
    check( PyObject * obj )
    {
    	if( ! check_basics( obj ) )
    		return 0;

		// must be C or fortran ordered contiguous
		if( ! ( PyArray_ISFORTRAN( obj ) || PyArray_ISCONTIGUOUS( obj ) ) )
			return 0;

		return obj;
	}

    /** Construct a C++ multi_array_ref from python numpy array. */
    static
    void
    construct(
        PyObject * obj,
        py::converter::rvalue_from_python_stage1_data * data )
    {
    	void * storage = ( ( py::converter::rvalue_from_python_storage< array_ref > * ) data )->storage.bytes;

    	PyArrayObject * py_array = (PyArrayObject *) obj;
    	std::vector< size_type > shape( py_array->dimensions, py_array->dimensions+NumDims );
    	if( PyArray_ISCONTIGUOUS( py_array ) ) {
    		new (storage) array_ref( (element_t *) py_array->data, shape, boost::c_storage_order() );
    	} else {
    		new (storage) array_ref( (element_t *) py_array->data, shape, boost::fortran_storage_order() );
    	}

		// record successful construction
		data->convertible = storage;
    }

#ifdef MAN_EXPOSING_VIEWS
    /** Check we can convert obj to C++. */
    static
    void *
    check_view( PyObject * obj )
    {
    	if( ! check_basics( obj ) )
    		return 0;

		return obj;
	}

    /** Construct a C++ multi_array_ref::view from python numpy array. */
    static
    void
    construct_view(
        PyObject * obj,
        py::converter::rvalue_from_python_stage1_data * data )
    {
    	void * storage = ( ( py::converter::rvalue_from_python_storage< array_ref_view_t > * ) data )->storage.bytes;

    	PyArrayObject * py_array = ( PyArrayObject * )obj;
    	std::vector< size_type > shape( py_array->dimensions, py_array->dimensions+NumDims );
    	if( PyArray_ISCARRAY( py_array ) ) {
    		array_ref ref( (element_t *) py_array->data, shape, boost::c_storage_order() );
    		//new (storage) array_ref_view_t( ref );
    	} else {
    		throw std::runtime_error( "Not implemented!" );
    		new (storage) array_ref( (element_t *) py_array->data, shape, boost::fortran_storage_order() );
    	}

		// record successful construction
		data->convertible = storage;
    }
#endif //MAN_EXPOSING_VIEWS

    /** Handles conversion to python. Can deal with multi_arrays and multi_array_refs. */
    template< typename multi_array_t >
    struct to_python {

		static
		PyObject *
		convert( const multi_array_t & x )
		{
			boost::array< npy_intp, NumDims > shape;
			for( std::size_t i = 0; NumDims != i; ++i) {
				shape[i] = x.shape()[i];
			}
			return PyArray_SimpleNewFromData(
				NumDims,
				shape.c_array(),
				numpy_type< T >().typenum(),
				const_cast< multi_array_t & >( x ).data()
			);
		}
    };

    /** Register conversions with boost::python. */
    static
    void
    _register()
    {
		// register converter from python to array_ref
		py::converter::registry::push_back(
			&check
			, &construct
			, py::type_id< array_ref >()
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
			, &get_PyArray_Type
#endif
		);

#ifdef MAN_EXPOSING_VIEWS
		// register converter from python to array_ref_view
		py::converter::registry::push_back(
			&check_view
			, &construct_view
			, py::type_id< array_ref_view_t >()
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
			, &get_PyArray_Type
#endif
		);
#endif //MAN_EXPOSING_VIEWS

		// register converter to python for multi_arrays and multi_array_refs
	    py::to_python_converter< array_t,   to_python< array_t   > >();
	    py::to_python_converter< array_ref, to_python< array_ref > >();
	    py::register_ptr_to_python< array_ptr >();
    }
};


} //anon namespace



/** The typechars of the types exposed through the interface. */
extern std::string exposed_typechars;



template< typename T >
void expose_converters()
{
	// conversion of array scalars
	py::converter::registry::push_back(
		check_array_scalar< T >
		, convert_array_scalar< T >
		, py::type_id< T >()
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
		, get_array_scalar_typeobj< T >
#endif
	);

	// register multi_array conversion
#define REGISTER_CONVERTERS(z, n, T) multi_array_converter< T, n+1 >::_register();
	BOOST_PP_REPEAT(NUMPY_MAX_DIMS, REGISTER_CONVERTERS, T)
#undef REGISTER_CONVERTERS

	// add this type to exposed typechars
	exposed_typechars += std::string( 1, numpy_type< T >().typechar() );
}



template< typename T, std::size_t NumDims >
struct tester {
	typedef boost::multi_array_ref< T, NumDims >            ref;
	typedef typename ref::element                           element;
	typedef tester< T, NumDims >                            self_t;

	static void test_from_python_shape( ref a, py::object shape ) {
		std::size_t d = 0;
		BOOST_FOREACH( std::size_t s, myrrh::python::make_boost_range< std::size_t >( shape ) ) {
			if( a.shape()[d] != s )
				throw std::runtime_error( "multi_array not of correct shape" );
			++d;
		}
	}

	static std::vector< typename ref::index > make_index( py::object py_index ) {
		std::vector< typename ref::index > idx;
		BOOST_FOREACH( typename ref::index i, myrrh::python::make_boost_range< std::size_t >( py_index ) ) {
			idx.push_back( i );
		}
		return idx;
	}

	static void set_element( ref a, py::object py_index, element v ) {
		a( make_index( py_index ) ) = v;
	}

	static element get_element( ref a, py::object py_index ) {
		return a( make_index( py_index ) );
	}

	static void def_test_fns() {
		const std::string tag = MYRRH_MAKE_STRING( numpy_type< T >().name() << "_" << NumDims );
		py::def( MYRRH_MAKE_STRING( std::string( "test_from_python_shape_" ) << tag ).c_str(), &self_t::test_from_python_shape );
		py::def( MYRRH_MAKE_STRING( std::string( "test_set_element_" ) << tag ).c_str(), &self_t::set_element );
		py::def( MYRRH_MAKE_STRING( std::string( "test_get_element_" ) << tag ).c_str(), &self_t::get_element );
	}
};

} //namespace python
} //namespace myrrh




