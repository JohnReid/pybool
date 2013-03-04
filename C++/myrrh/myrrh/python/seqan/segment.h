/** Copyright John Reid 2009
 *
 * \file
 * \brief Exposes seqan segments to python.
 */

#ifndef _MYRRH_PYTHON_SEQAN_SEGMENT_H_
#define _MYRRH_PYTHON_SEQAN_SEGMENT_H_

#include <myrrh/python/seqan/defs.h>

namespace myrrh {
namespace python {
namespace seqan {



template< typename Segment >
struct PySegment {

	static
	std::string
	as_string( const Segment & segment ) {
		std::string result;
		S::assign( result, segment );
		return result;
	}

	static
	void
	expose() {

		py::class_< Segment > _class(
			"Segment",
			"Seqan segment.",
			py::no_init
		);

		_class.def( "__str__", as_string, "Convert to a python string." );
	}
};

}
}
}


#endif //_MYRRH_PYTHON_SEQAN_SEGMENT_H_


