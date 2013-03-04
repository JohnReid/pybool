/** Copyright John Reid 2009
 *
 * \file
 * \brief Exposes seqan containers to python.
 */

#ifndef _MYRRH_PYTHON_SEQAN_CONTAINER_H_
#define _MYRRH_PYTHON_SEQAN_CONTAINER_H_

#include <myrrh/python/seqan/defs.h>

namespace myrrh {
namespace python {
namespace seqan {


/**
 * Expose a container to python.
 */
template< typename String >
struct PyContainer {


	static
	void
	expose() {
	}
};


inline
void
expose_overflow_strategies() {

	py::class_<
		S::Tag< S::TagInsist_ >
	> insist_class(
		"Insist",
		"Overflow strategy: No capacity check. The user has to ensure that the container's capacity is large enough.",
		py::init< >( "Initialise overflow strategy." )
	);

	py::class_<
		S::Tag< S::TagLimit_ >
	> limit_class(
		"Limit",
		"Overflow strategy: Limit the contents to current capacity. All entries that exceed the capacity are lost.",
		py::init< >()
	);

	py::class_<
		S::Tag< S::TagExact_ >
	> exact_class(
		"Exact",
		"Overflow strategy: Expand as far as needed. The capacity is only changed if the current capacity is not large enough. If the capacity can only be expanded up to a certain ammount, it will be increased as far as possible and the contents are limited to the new capacity.",
		py::init< >()
	);

	py::class_<
		S::Tag< S::TagGenerous_ >
	> generous_class(
		"Generous",
		"Overflow strategy: Expand if needed, get precautionary extra space. Whenever the capacity has to be increased, the new capacity is choosen somewhat large than actually needed. This strategy limits the number of capacity changes, so that resizing takes armotized constant time. Use this strategy if the total amount of storage is unkown at first.",
		py::init< >()
	);
}

} // namespace seqan
} // namespace python
} // namespace myrrh


#endif //_MYRRH_PYTHON_SEQAN_CONTAINER_H_


