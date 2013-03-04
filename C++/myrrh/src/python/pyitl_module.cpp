/** Copyright John Reid 2010
*/

#include <myrrh/python/pyitl.h>
#include <iostream>

#include <myrrh/python/pyitl_impl.h>


BOOST_PYTHON_MODULE( _pyitl )
{
	using namespace myrrh::python::pyitl;
	namespace bp = boost::python;
	namespace itl = boost::itl;

#ifndef NDEBUG
	bp::scope().attr("__debug__") = 1;
	std::cout << "WARNING: Debug version of _pyitl module loaded. If you did not intend this then check your configuration!" << std::endl;
#else //_DEBUG
	bp::scope().attr("__debug__") = 0;
#endif //_DEBUG


	register_boundtypes_enum();

	//
	// Expose intervals
	//
	interval_exposer< itl::interval< float > >::expose( "FloatInterval", "An interval of floats." );
	interval_exposer< itl::interval< int > >::expose( "IntInterval", "An interval of ints." );


	//
	// Expose (joining) interval sets.
	//
	interval_set_exposer< itl::interval_set< int > >::expose( "IntIntervalSet", "A (joining) interval set of int intervals." );
	interval_set_exposer< itl::interval_set< float > >::expose( "FloatIntervalSet", "A (joining) interval set of float intervals." );


	//
	// Expose (joining) interval maps.
	//
	interval_map_exposer< itl::interval_map< int, bp::object > >::expose( "IntIntervalMap", "A (joining) map from int intervals to objects." );
	interval_map_exposer< itl::interval_map< int, int > >::expose( "IntIntervalMapOfInts", "A (joining) map from int intervals to integers." );
	interval_map_exposer< itl::interval_map< int, float > >::expose( "IntIntervalMapOfFloats", "A (joining) map from int intervals to floats." );
	interval_map_exposer< itl::interval_map< float, bp::object > >::expose( "FloatIntervalMap", "A (joining) map from float intervals to objects." );


	//
	// Expose separate interval sets.
	//
	interval_set_exposer< itl::separate_interval_set< int > >::expose( "SeparateIntIntervalSet", "A separate interval set of int intervals." );
	interval_set_exposer< itl::separate_interval_set< float > >::expose( "SeparateFloatIntervalSet", "A separate interval set of float intervals." );


	//
	// Expose split interval sets.
	//
	interval_set_exposer< itl::split_interval_set< int > >::expose( "SplitIntIntervalSet", "A split interval set of int intervals." );
	interval_set_exposer< itl::split_interval_set< float > >::expose( "SplitFloatIntervalSet", "A split interval set of float intervals." );


	//
	// Expose split interval maps.
	//
	interval_map_exposer< itl::split_interval_map< int, bp::object > >::expose( "SplitIntIntervalMap", "A map from int intervals to objects." );
	interval_map_exposer< itl::split_interval_map< int, int > >::expose( "SplitIntIntervalMapOfInts", "A map from int intervals to integers." );
	interval_map_exposer< itl::split_interval_map< int, float > >::expose( "SplitIntIntervalMapOfFloats", "A map from int intervals to floats." );
	interval_map_exposer< itl::split_interval_map< float, bp::object > >::expose( "SplitFloatIntervalMap", "A map from float intervals to objects." );

}
