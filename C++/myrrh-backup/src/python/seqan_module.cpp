/** Copyright John Reid 2009
 *
 * \file
 * \brief Implements the seqan boost::python module.
 */

#include <myrrh/python/seqan/string.h>
#include <myrrh/python/seqan/index.h>
#include <myrrh/python/seqan/iterator.h>
#include <myrrh/python/seqan/segment.h>
#include <myrrh/python/seqan/container.h>

using namespace myrrh::python::seqan;

typedef S::String< char >                               string_t;
typedef S::StringSet< string_t >                        string_set_t;
typedef S::Index< string_t >                            index_t;
typedef S::Iterator< index_t, S::TopDown<> >::Type      top_down_it;

BOOST_PYTHON_MODULE( _seqan )
{
	using boost::python::scope;

	expose_overflow_strategies();
	PyString< string_t >::expose();
	PyStringSet< string_set_t >::expose();
	PyIndex< index_t >::expose();
	PyIterator< top_down_it >::expose();
	PySegment< PyIterator< top_down_it >::infix_t >::expose();
}
