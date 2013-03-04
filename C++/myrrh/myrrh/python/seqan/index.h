/** Copyright John Reid 2009
 *
 * \file
 * \brief Exposes seqan iterators to python.
 */

#ifndef _MYRRH_PYTHON_SEQAN_INDEX_H_
#define _MYRRH_PYTHON_SEQAN_INDEX_H_

#include <myrrh/python/seqan/defs.h>

namespace myrrh {
namespace python {
namespace seqan {


/**
 * Expose an index to python.
 */
template< typename Index >
struct PyIndex {

	typedef typename S::Host< Index >::Type                             host_t;          ///< Type of the object a given object depends on.
	typedef typename S::Size< Index >::Type                             size_t;          ///< The number of items/characters.
	typedef typename S::SAValue< Index >::Type                          savalue_t;       ///< The default alphabet type of a suffix array, i.e. the type to store a position of a string or string set.


	static
	void
	expose() {
		py::class_<
			Index,
			boost::shared_ptr< Index >
		> _class(
			"Index",
			"Seqan index.",
			py::init< host_t const & >( "Initialise an index." )
		);
		size_t (* len_fn)(Index const &) = S::length;
		_class.def( "__len__", len_fn, "Length of the index." );
	}
};

}
}
}


#endif //_MYRRH_PYTHON_SEQAN_INDEX_H_


