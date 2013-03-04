#include <boost/itl/interval.hpp>
#include <boost/python.hpp>

namespace myrrh {
namespace python {
namespace pyitl {

void
register_boundtypes_enum() {
	using namespace boost::python;
	using namespace boost::itl;

	enum_< BoundTypes >( "BoundTypes" )
        .value( "open_bounded",   open_bounded )
        .value( "left_open",      left_open )
		.value( "right_open",     right_open )
		.value( "closed_bounded", closed_bounded )
        .export_values()
        ;
}

} // namespace pyitl
} // namespace python
} // namespace myrrh


