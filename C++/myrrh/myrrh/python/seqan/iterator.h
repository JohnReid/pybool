/** Copyright John Reid 2009
 *
 * \file
 * \brief Exposes seqan iterators to python.
 */

#ifndef _MYRRH_PYTHON_SEQAN_ITERATOR_H_
#define _MYRRH_PYTHON_SEQAN_ITERATOR_H_

#include <myrrh/python/seqan/defs.h>

namespace myrrh {
namespace python {
namespace seqan {

namespace {


}


template< typename It >
struct PyIterator {
	typedef typename S::Container< It >::Type container_t;
	typedef typename S::Host< It >::Type host_t;
	typedef typename S::Spec< It >::Type spec_t;
	typedef typename S::Size< container_t >::Type size_t;
	typedef typename S::Fibre< container_t, S::Fibre_Text >::Type fibre_t;
	typedef typename S::Infix< fibre_t const >::Type infix_t;

	static
	It
	copy( It const & it ) {
		return it;
	}

	static
	void
	expose() {
		py::class_< It > _class(
			"Iterator",
			"Seqan iterator.",
			py::init< container_t & >( "Initialise an iterator." )
		);

		_class.def( "__copy__", copy, "Copies the iterator." );

		bool (* go_down_fn)(It &) = S::goDown;
		_class.def( "goDown", go_down_fn, "Iterates down one edge or a path in a tree." );

		bool (* go_right_fn)(It &) = S::goRight;
		_class.def( "goRight", go_right_fn, "Iterates to the next sibling in a tree." );

		infix_t (* representative_fn)(It const &) = S::representative;
		_class.add_property( "representative", representative_fn, "A substring representing the path from root to iterator node." );

		size_t (* countOccurrences_fn)(It const &) = S::countOccurrences;
		_class.add_property( "occurrences", countOccurrences_fn, "Returns the number of occurences of representative in the index text." );
	}
};

}
}
}


#endif //_MYRRH_PYTHON_SEQAN_ITERATOR_H_


