/**
@file

Copyright John Reid 2006

*/

#ifndef MYRRH_PYTHON_NUMPY_H_
#define MYRRH_PYTHON_NUMPY_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/types.h"

#include <boost/multi_array.hpp>
#include <boost/python.hpp>
#include <boost/python/object.hpp>  //len function
#include <boost/python/ssize_t.hpp> //ssize_t type definition
#include <boost/python/detail/none.hpp>

#include <vector>





namespace myrrh {
namespace python {



namespace detail {

template< typename T >
struct get_dtype
{
	static const char * name() { return "object"; } //assume is object
	//static const char * name() { throw std::logic_error( "get_dtype not specialised for this type" ); }
};

#define DECLARE_DTYPE_FOR( type, dtype ) template< > struct get_dtype< type > { static const char * name() { return dtype; } };
DECLARE_DTYPE_FOR( double, "float64" )
DECLARE_DTYPE_FOR( float, "float32" )
DECLARE_DTYPE_FOR( int, "int32" )
DECLARE_DTYPE_FOR( unsigned, "uint32" )
DECLARE_DTYPE_FOR( long, "int64" )
DECLARE_DTYPE_FOR( unsigned long, "uint64" )
}

// namespace impl

template< typename MultiArrayType >
struct numpy_multi_array_converter
{
    typedef MultiArrayType multi_array_t;
    typedef std::vector< std::size_t > shape_t;

	static void register_to_and_from_python()
	{
		register_from_python();
		register_to_python();
	}

	static void register_to_python()
	{
		boost::python::to_python_converter< multi_array_t, numpy_multi_array_converter< multi_array_t > >();
	}

	static void register_from_python()
	{
		boost::python::converter::registry::push_back(  
			&numpy_multi_array_converter< multi_array_t >::convertible,
			&numpy_multi_array_converter< multi_array_t >::construct,
			boost::python::type_id< multi_array_t >()
		);
	}

    static
	void * 
	convertible( PyObject * obj )
	{
		using namespace boost::python;
		try
		{
			shape_t shape;
			get_shape( object( handle<>( borrowed( obj ) ) ), shape );
			if( multi_array_t::dimensionality != shape.size() ) return 0;
		}
		catch( ... )
		{
			return 0;
		}

		return obj;
    }

	template< 
		typename C,
		multi_array_t C::* pm
	>
	static
	void
	set_member_with_resize( C & c, const multi_array_t & a )
	{
		std::vector< unsigned > extents;
		for( unsigned dim = 0; a.num_dimensions() != dim; ++dim ) extents.push_back( a.shape()[ dim ] );
		(c.*pm).resize( extents ); 
		(c.*pm) = a; 
	}

    static 
	void
    construct( 
		PyObject* obj, 
		boost::python::converter::rvalue_from_python_stage1_data* data )
	{
		using namespace boost::python;

		//get the storage
        typedef boost::python::converter::rvalue_from_python_storage< multi_array_t > storage_t;
        storage_t * the_storage = reinterpret_cast< storage_t * >( data );
        void * memory_chunk = the_storage->storage.bytes;

		//new placement
		object py_obj( handle<>( borrowed( obj ) ) );
		shape_t shape;
		get_shape( py_obj, shape );
		multi_array_t * a = new (memory_chunk) multi_array_t( shape );

		//extract each element from numpy array and put in c array
		index i( a->num_dimensions(), 0 );
		do
		{
			using boost::python::list;
			using boost::python::tuple;

			list numpy_index;
			for( unsigned dim = 0; a->num_dimensions() != dim; ++dim ) { numpy_index.append( i[ dim ] ); }
			typename multi_array_t::element el = extract< typename multi_array_t::element >( py_obj[ tuple( numpy_index ) ] );
			( *a )( i ) = el;
		}
		while( increment_index( i, *a ) );

        data->convertible = memory_chunk;
    }

	static 
	PyObject * 
	convert( const multi_array_t & c_array )
	{
		using namespace boost::python;

		object numpy = object( handle<>( ::PyImport_Import( object( "numpy" ).ptr() ) ) );
		if( ! numpy  ) throw std::logic_error( "Could not import numpy" );
		object array_function = numpy.attr( "empty" );
		if( ! array_function  ) throw std::logic_error( "Could not find array function" );

		//create a numpy array to put it in
		boost::python::list extents;
		for( unsigned dim = 0; c_array.num_dimensions() != dim; ++dim ) extents.append( c_array.shape()[ dim ] );

		object result(
			array_function(
				extents,
				numpy.attr( "dtype" )( detail::get_dtype< typename multi_array_t::element >::name() ) ) );

		//copy the elements	
		index i( c_array.num_dimensions(), 0 );
		do
		{
			using boost::python::tuple;
			boost::python::list numpy_index;
			for( unsigned dim = 0; c_array.num_dimensions() != dim; ++dim ) { numpy_index.append( i[dim] ); }
			result[ tuple( numpy_index ) ] = c_array( i );
		}
		while( increment_index( i, c_array ) );

		return incref( result.ptr() );
	}

protected:
	static
	void
	get_shape( boost::python::object obj, shape_t & shape )
	{
		using namespace boost::python;
		shape.clear();
		object py_shape = obj.attr( "shape" );
		const std::size_t N = len( py_shape );
		for( std::size_t i = 0; N != i; ++i ) shape.push_back( extract< std::size_t >( py_shape[ i ] ) );
	}

	typedef std::vector< typename multi_array_t::index > index; /**< To iterate over entries in num_dimensions independent fashion. */

	/**< Iterates over entries in num_dimensions independent fashion. */
	static
	bool
	increment_index( index & i, const multi_array_t & c_array )
	{
		for( unsigned dim = 0; i.size() != dim; ++dim )
		{
			++i[dim];
			if( int( i[dim] ) != int( c_array.shape()[dim] ) ) return true;
			else i[dim] = 0;
		}
		return false;
	}

};


} //namespace python
} //namespace myrrh

#endif //MYRRH_PYTHON_NUMPY_H_
