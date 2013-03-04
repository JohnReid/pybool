/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_PYTHON_TUPLE_H_
#define MYRRH_PYTHON_TUPLE_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include <boost/python/tuple.hpp>
#include <boost/python/module.hpp>
#include <boost/python/refcount.hpp>

namespace myrrh {
namespace python {
namespace impl {




template< typename H, typename T >
struct _tuple_to_python
{
	boost::python::object 
	operator()( const boost::tuples::cons< H, T > & x ) const
	{
		return boost::python::make_tuple( x.head ) + tuple_to_python( x.tail );
	}
};


template< typename H >
struct _tuple_to_python< H, boost::tuples::null_type >
{
	boost::python::tuple 
	operator()( const boost::tuples::cons< H, boost::tuples::null_type > & x ) const
	{
		return boost::python::make_tuple( x.head );
	}
};

} //namespace impl



template< typename Tuple >
boost::python::object
tuple_to_python( const Tuple & x )
{
	return impl::_tuple_to_python< typename Tuple::head_type, typename Tuple::tail_type >()( x );
}

template< typename Tuple >
struct tuple_converter
{
    static PyObject* convert( const Tuple & x )
    {
        return boost::python::incref( tuple_to_python( x ).ptr() );
    }
};

template< typename Tuple >
void 
register_tuple_converter()
{
	boost::python::to_python_converter< Tuple, tuple_converter< Tuple > >();
}



} //namespace myrrh
} //namespace python

#endif //MYRRH_PYTHON_TUPLE_H_
