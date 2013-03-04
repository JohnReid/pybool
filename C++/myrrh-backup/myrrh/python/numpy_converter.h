/**
@file

Copyright John Reid 2007-2009

*/

#ifndef MYRRH_PYTHON_NUMPY_CONVERTER_H_
#define MYRRH_PYTHON_NUMPY_CONVERTER_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <boost/python.hpp>

#include <deque>

#include <myrrh/defs.h>
#include <myrrh/type_names.h>


namespace myrrh {
namespace python {
namespace impl {

//to specialise for various containers
template< typename CContainer >
struct _converter
{
	static void
	from_numpy( boost::python::object numpy_array, CContainer & c_array )
	{
		throw std::logic_error( MYRRH_MAKE_STRING( "convert_from_numpy not specialised for this type: "<<get_type_name(c_array) ) );
	}

	static boost::python::object
	to_numpy( boost::python::object numpy, const CContainer & c_array, boost::python::object array_function )
	{
		throw std::logic_error( MYRRH_MAKE_STRING( "convert_to_numpy not specialised for this type: "<<get_type_name(c_array) ) );
	}
};

} //namespace impl




/** Converts between numpy arrays and different types of C++ matrices. */
struct numpy_converter
{
public:
	typedef boost::python::object		object;
	typedef boost::python::tuple		tuple;

	/** Constructor. dtype_name determines type of matrices created. */
	numpy_converter()
	{
		load_numpy_module();
	}

	template< typename CContainer >
	void
	from_numpy(
		object numpy_array,
		CContainer & c_array )
	{
		impl::_converter< CContainer >::from_numpy( numpy_array, c_array );
	}

	template< typename CContainer >
	object
	to_numpy(
		const CContainer & c_array )
	{
		return impl::_converter< CContainer >::to_numpy( numpy, c_array, array_function );
	}

protected:
	object numpy;
	object array_function;

	void load_numpy_module()
	{
		using namespace boost::python;

		numpy = object( handle<>( ::PyImport_Import( object( "numpy" ).ptr() ) ) );
		if( ! numpy  ) throw std::logic_error( "Could not import numpy" );
		array_function = numpy.attr( "zeros" );
		if( ! array_function  ) throw std::logic_error( "Could not find array function" );
	}
};




namespace impl {




template< typename T >
struct get_dtype
{
	static const char * name() { throw std::logic_error( MYRRH_MAKE_STRING( "get_dtype not specialised for this type: "<<type_name< T >::name() ) ); }
};

#define DECLARE_DTYPE_FOR( type, dtype ) template< > struct get_dtype< type > { static const char * name() { return dtype; } };
DECLARE_DTYPE_FOR( double, "float64" )
DECLARE_DTYPE_FOR( float, "float32" )
DECLARE_DTYPE_FOR( int, "int32" )
DECLARE_DTYPE_FOR( unsigned, "uint32" )
DECLARE_DTYPE_FOR( long, "int64" )
DECLARE_DTYPE_FOR( unsigned long, "uint64" )

/** Convert a numpy matrix to a ublas matrix and back again. */
template< typename T >
struct _converter< boost::numeric::ublas::matrix< T > >
{
	typedef typename boost::numeric::ublas::matrix< T > matrix;
	typedef boost::python::object object;
	typedef boost::python::tuple tuple;

	inline static void
	from_numpy( object numpy_array, matrix & c_array )
	{
		using namespace boost::python;

		tuple shape( numpy_array.attr( "shape" ) );
		if( len( shape ) != 2 ) throw std::logic_error( "numpy::array must have 2 dimensions for conversion to ublas matrix" );
		c_array.resize(
			extract< unsigned >( shape[0] ),
			extract< unsigned >( shape[1] ) );
		for( unsigned i = 0; i < c_array.size1(); ++i )
		{
			for( unsigned j = 0; j < c_array.size2(); ++j )
			{
				c_array( i, j ) = extract< T >( numpy_array[ boost::python::make_tuple( i, j ) ] );
			}
		}
	}

	inline static object
	to_numpy( object numpy, const matrix & c_array, object array_function )
	{
		using namespace boost::python;

		//create a numpy array to put it in
		object result(
			array_function(
				boost::python::make_tuple( c_array.size1(), c_array.size2() ),
				numpy.attr( "dtype" )( get_dtype< T >::name() ) ) );

		//copy the elements
		for( unsigned i = 0; c_array.size1() != i; ++i )
		{
			for( unsigned j = 0; c_array.size2() != j; ++j )
			{
				result[ boost::python::make_tuple( i, j ) ] = c_array( i, j );
			}
		}

		return result;
	}
};



template < typename Container >
struct _random_access_container_converter
{
	typedef Container container;
	typedef typename container::value_type value_type;
	typedef boost::python::object object;
	typedef boost::python::tuple tuple;

	inline static void
	from_numpy( object numpy_array, container & c_array )
	{
		using namespace boost::python;

		tuple shape( numpy_array.attr( "shape" ) );
		if( len( shape ) != 1 ) throw std::logic_error( "numpy::array must have 1 dimension for conversion to this container" );
		c_array.resize( extract< unsigned >( shape[0] ) );
		for( unsigned i = 0; i < c_array.size(); ++i )
		{
			c_array[ i ] = extract< value_type >( numpy_array[ boost::python::make_tuple( i ) ] );
		}
	}

	inline static object
	to_numpy( object numpy, const container & c_array, object array_function )
	{
		using namespace boost::python;

		//create a numpy array to put it in
		object result(
			array_function(
				boost::python::make_tuple( c_array.size() ),
				numpy.attr( "dtype" )( get_dtype< value_type >::name() ) ) );

		//copy the elements
		for( unsigned i = 0; c_array.size() != i; ++i )
		{
			result[ boost::python::make_tuple( i ) ] = c_array[ i ];
		}

		return result;
	}
};


/** Convert a numpy matrix to a std::vector and back again. */
template< typename T >
struct _converter< std::vector< T > > : _random_access_container_converter< std::vector< T > >
{
};


/** Convert a numpy matrix to a std::deque and back again. */
template< typename T >
struct _converter< std::deque< T > > : _random_access_container_converter< std::deque< T > >
{
};




/** Convert a numpy matrix to a boost::array and back again. */
template< typename T, unsigned N >
struct _converter< boost::array< T, N > >
{
	typedef typename boost::array< T, N > array;
	typedef boost::python::object object;
	typedef boost::python::tuple tuple;

	inline static void
	from_numpy( object numpy_array, array & c_array )
	{
		using namespace boost::python;

		tuple shape( numpy_array.attr( "shape" ) );
		if( len( shape ) != 1 ) throw std::logic_error( "numpy::array must have 1 dimension for conversion to boost::array" );
		if( shape[ 0 ] != N ) throw std::logic_error( "numpy::array must have correct number elements for conversion to boost::array" );
		for( unsigned i = 0; i < c_array.size(); ++i )
		{
			c_array[ i ] = extract< T >( numpy_array[ boost::python::make_tuple( i ) ] );
		}
	}

	inline static object
	to_numpy( object numpy, const array & c_array, object array_function )
	{
		using namespace boost::python;

		//create a numpy array to put it in
		object result(
			array_function(
				boost::python::make_tuple( c_array.size() ),
				numpy.attr( "dtype" )( get_dtype< T >::name() ) ) );

		//copy the elements
		for( unsigned i = 0; c_array.size() != i; ++i )
		{
			result[ boost::python::make_tuple( i ) ] = c_array[ i ];
		}

		return result;
	}
};




/** Convert a numpy matrix to a boost::multi_array and back again. */
template< typename T, std::size_t N >
struct _converter< boost::multi_array< T, N > >
{
	typedef typename boost::multi_array< T, N > multi_array;
	typedef std::vector< typename multi_array::index > index;
	typedef boost::python::object object;
	typedef boost::python::tuple tuple;

	inline static bool increment_index( index & i, const multi_array & c_array )
	{
		for( unsigned dim = 0; i.size() != dim; ++dim )
		{
			++i[dim];
			if( int( i[dim] ) != int( c_array.shape()[dim] ) )
			{
				return true;
			}
			else
			{
				i[dim] = 0;
			}
		}
		return false;
	}

	inline static void
	from_numpy( object numpy_array, multi_array & c_array )
	{
		using namespace boost::python;

		//std::cout << "from_numpy\n";

		//resize c_array
		object shape( numpy_array.attr( "shape" ) );
		if( len( shape ) != N ) throw std::logic_error( "numpy::array must have correct number of dimensions for conversion to boost::multi_array" );
		boost::multi_array_types::extent_gen e;
		std::vector< unsigned > extents;
		for( unsigned dim = 0; N != dim; ++dim ) extents.push_back( extract< unsigned >( shape[ dim ] ) );
		c_array.resize( extents );
		//for( unsigned dim = 0; N != dim; ++dim ) { std::cout << "," << c_array.shape()[dim]; } std::cout << "\n";

		//extract each element from numpy array and put in c array
		if( c_array.num_elements() )
		{
			index i( N, 0 );
			do
			{
				//for( unsigned dim = 0; N != dim; ++dim ) { std::cout << "," << i[dim]; } std::cout << "\n";
				boost::python::list numpy_index;
				for( unsigned dim = 0; N != dim; ++dim ) { numpy_index.append( i[dim] ); }
				c_array( i ) = extract< T >( numpy_array[ tuple( numpy_index ) ] );
			}
			while( increment_index( i, c_array ) );
		}
	}

	inline static object
	to_numpy( object numpy, const multi_array & c_array, object array_function )
	{
		using namespace boost::python;

		//std::cout << "to_numpy\n";

		//create a numpy array to put it in
		boost::python::list extents;
		for( unsigned dim = 0; N != dim; ++dim ) extents.append( c_array.shape()[ dim ] );
		//for( unsigned dim = 0; N != dim; ++dim ) { std::cout << "," << c_array.shape()[ dim ]; } std::cout << "\n";
		object result(
			array_function(
				extents,
				numpy.attr( "dtype" )( get_dtype< T >::name() ) ) );

		//copy the elements
		if( c_array.num_elements() )
		{
			index i( N, 0 );
			do
			{
				//for( unsigned dim = 0; N != dim; ++dim ) { std::cout << "," << i[dim]; } std::cout << "\n";
				boost::python::list numpy_index;
				for( unsigned dim = 0; N != dim; ++dim ) { numpy_index.append( i[dim] ); }
				result[ tuple( numpy_index ) ] = c_array( i );
			}
			while( increment_index( i, c_array ) );
		}

		return result;
	}
};





} //namespace impl
} //namespace python
} //namespace myrrh


#endif //MYRRH_PYTHON_NUMPY_CONVERTER_H_

