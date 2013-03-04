/** Copyright John Reid 2009
 *
 * \file
 * \brief Exposes seqan strings to python.
 */

#ifndef _MYRRH_PYTHON_SEQAN_STRING_H_
#define _MYRRH_PYTHON_SEQAN_STRING_H_

#include <myrrh/python/seqan/defs.h>

namespace myrrh {
namespace python {
namespace seqan {


/**
 * Expose a string to python.
 */
template< typename String >
struct PyString {


	typedef typename S::Value< String >::Type                      value_t;           ///< Type of the items in the container.
	typedef typename S::Spec< String >::Type                       spec_t;            ///< The spec of a SeqAn type is the class that is used in template subclassing to specify the specialization.


	/** Convert to a C++ string. */
	static
	std::string
	as_string( const String & s ) {
		std::string result;
		S::assign( result, s );
		return result;
	}


	static
	void
	expose() {
		py::class_<
			String,
			boost::shared_ptr< String >
		> _class(
			"String",
			"A Seqan string.",
			py::init< char const * >( "Initialise a string." )
		);
		size_t (* len_fn)(String const &) = S::length;
		_class.def( "__len__", len_fn, "Length of the index." );
		_class.def( "__str__", as_string, "Convert to a python string." );
	}
};



/**
 * Expose a string set to python.
 */
template< typename StringSet >
struct PyStringSet {

	typedef typename S::Spec< StringSet >::Type                       spec_t;            ///< The spec of a SeqAn type is the class that is used in template subclassing to specify the specialization.
	typedef typename S::Value< StringSet >::Type                      string_t;          ///<
	typedef typename S::SAValue< StringSet >::Type                    savalue_t;         ///< The default alphabet type of a suffix array, i.e. the type to store a position of a string or string set.
	typedef typename S::DefaultOverflowImplicit< StringSet >::Type    resize_t;          ///< The default overflow strategy for implicit resize.


	static
	void
	append_value( StringSet & string_set, string_t const & string, py::object & overflow_strategy ) {
		if( py::extract< S::Tag< S::TagInsist_ > >( overflow_strategy ).check() ) {
			S::appendValue( string_set, string, S::Insist() );
		} else if( py::extract< S::Tag< S::TagLimit_ > >( overflow_strategy ).check() ) {
			S::appendValue( string_set, string, S::Limit() );
		} else if( py::extract< S::Tag< S::TagExact_ > >( overflow_strategy ).check() ) {
			S::appendValue( string_set, string, S::Exact() );
		} else if( py::extract< S::Tag< S::TagGenerous_ > >( overflow_strategy ).check() ) {
			S::appendValue( string_set, string, S::Generous() );
		} else {
			throw std::logic_error( "Unknown overflow strategy." );
		}
	}


	/** Expose the class using boost::python. */
	static
	void
	expose() {
		py::class_<
			StringSet,
			boost::shared_ptr< StringSet >
		> _class(
			"StringSet",
			"A Seqan string.",
			py::init< >( "Initialise a string set." )
		);
		//void (* append_value_fn)( StringSet &, string_t const &, resize_t const & ) = S::appendValue;
		_class.def(
			"appendValue",
			append_value,
			( py::arg( "string_set" ), py::arg( "string" ), py::arg( "tag" )=typename S::DefaultOverflowImplicit< StringSet >::Type() ),
			"Append a value to the container."
		);
		size_t (* len_fn)(StringSet const &) = S::length;
		_class.def( "__len__", len_fn, "Length of the index." );
	}
};

}
}
}


#endif //_MYRRH_PYTHON_SEQAN_STRING_H_


