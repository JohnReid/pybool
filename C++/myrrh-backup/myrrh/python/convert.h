/**
@file

Copyright John Reid 2006

*/

#ifndef MYRRH_PYTHON_CONVERT_H_
#define MYRRH_PYTHON_CONVERT_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/python/numpy_converter.h"
#include "myrrh/types.h"

#include <boost/multi_array.hpp>

#include <vector>
#include <deque>





namespace myrrh {
namespace python {

extern numpy_converter converter;


//forward decl
/** Convert given C++ Class boost::python::object, o, to python boost::python::object. */
template< typename Class >
boost::python::object
convert_to_python( const Class & o );

//forward decl
template< typename Class >
void
convert_from_python( boost::python::object py_obj, Class & o );

namespace impl {

template< typename T >
struct _convert_to_python
{
	boost::python::object operator()( const T & o ) const
	{
		throw std::logic_error( "_convert_to_python not specialised for this type" );
		//BOOST_STATIC_ASSERT( ! "_convert_to_python not specialised for this type" );
	}
};

template< >
struct _convert_to_python< double_array >
{
	boost::python::object operator()( const double_array & o ) const
	{
		boost::python::object result = converter.to_numpy( o );
		result.attr( "setflags" )( false ); //make not writeable
		return result;
	}
};

template< typename T >
struct _convert_to_python< std::vector< T > >
{
	boost::python::object operator()( const std::vector< T > & o ) const
	{
		boost::python::object result = converter.to_numpy( o );
		result.attr( "setflags" )( false ); //make not writeable
		return result;
	}
};

template< typename T >
struct _convert_to_python< std::deque< T > >
{
	boost::python::object operator()( const std::deque< T > & o ) const
	{
		boost::python::object result = converter.to_numpy( o );
		result.attr( "setflags" )( false ); //make not writeable
		return result;
	}
};

template< unsigned N >
struct _convert_to_python< boost::array< double, N > >
{
	boost::python::object operator()( const boost::array< double, N > & o ) const
	{
		boost::python::object result = converter.to_numpy( o );
		result.attr( "setflags" )( false ); //make not writeable
		return result;
	}
};

template< typename T >
struct _convert_to_python< std::vector< std::vector< T > > >
{
	boost::python::object operator()( const std::vector< std::vector< T > > & o ) const
	{
		boost::python::list l;
		for( unsigned i = 0; o.size() != i; ++i )
		{
			l.append( convert_to_python( o[i] ) );
		}
		return l;
	}
};

template< >
struct _convert_to_python< double_vec_vec_vec >
{
	boost::python::object operator()( const double_vec_vec_vec & o ) const
	{
		boost::python::list l;
		for( unsigned i = 0; o.size() != i; ++i )
		{
			l.append( convert_to_python( o[i] ) );
		}
		return l;
	}
};

template< typename T >
struct _convert_from_python
{
	void operator()( boost::python::object py_obj, T & o ) const
	{
		throw std::logic_error( "_convert_from_python not specialised for this type" );
		//BOOST_STATIC_ASSERT( ! "_convert_from_python not specialised for this type" );
	}
};

template< >
struct _convert_from_python< double_array >
{
	void operator()( boost::python::object py_obj, double_array & o ) const
	{
		converter.from_numpy( py_obj, o );
	}
};

template< typename T >
struct _convert_from_python< std::vector< T > >
{
	void operator()( boost::python::object py_obj, std::vector< T > & o ) const
	{
		converter.from_numpy( py_obj, o );
	}
};

template< typename T, unsigned N >
struct _convert_from_python< boost::array< T, N > >
{
	void operator()( boost::python::object py_obj, boost::array< T, N > & o ) const
	{
		converter.from_numpy( py_obj, o );
	}
};

template< typename T >
struct _convert_from_python< std::vector< std::vector< T > > >
{
	void operator()( boost::python::object py_obj, std::vector< std::vector< T > > & o ) const
	{
		o.resize( boost::python::len( py_obj ) );
		for( unsigned i = 0; o.size() != i; ++i )
		{
			convert_from_python( py_obj[ i ], o[i] );
		}
	}
};

template< typename T >
struct _convert_from_python< std::vector< std::vector< std::vector< T > > > >
{
	void operator()( boost::python::object py_obj, std::vector< std::vector< std::vector< T > > > & o ) const
	{
		o.resize( boost::python::len( py_obj ) );
		for( unsigned i = 0; o.size() != i; ++i )
		{
			convert_from_python( py_obj[ i ], o[i] );
		}
	}
};


} //namespace impl



template< typename Class >
boost::python::object
convert_to_python( const Class & o )
{
	return impl::_convert_to_python< Class >()( o );
}

template< 
	typename Class,
	typename Container,
	Container Class:: * Member
>
boost::python::object
access_and_convert( const Class & obj )
{
	return convert_to_python( obj.*Member );
}

template< typename Class >
void
convert_from_python( boost::python::object py_obj, Class & o )
{
	impl::_convert_from_python< Class >()( py_obj, o );
}

template< 
	typename Class,
	typename Container,
	Container Class:: * Member
>
void
convert_and_set( Class & obj, boost::python::object py_obj )
{
	convert_from_python( py_obj, obj.*Member );
}

template< typename Class >
struct to_python_converter
{
    static PyObject * convert( const Class & o )
    {
		return boost::python::incref( convert_to_python( o ).ptr() );
    }
};

/** Specialise for boost::shared_ptr. */
template< typename Pointee >
struct to_python_converter< boost::shared_ptr< Pointee > >
{
    static PyObject * convert( boost::shared_ptr< Pointee > o )
    {
		return boost::python::incref( convert_to_python( *o ).ptr() );
    }
};

template< typename Class >
void
register_converter()
{
	boost::python::to_python_converter< 
		Class, 
		myrrh::python::to_python_converter< Class > 
	>();
}


} //namespace myrrh
} //namespace python

#endif //MYRRH_PYTHON_CONVERT_H_
