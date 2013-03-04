/**
 * Copyright John Reid 2010
 */

#include <myrrh/python/multi_array_to_numpy.h>
#include <myrrh/python/boost_range.h>



namespace myrrh {
namespace python {
std::string exposed_typechars;
}
}



typedef npy_int64                                 numpy_int;   ///< The type we use for numpy integers.
typedef boost::multi_array< numpy_int, 1 >        vector;      ///< A vector of ints.
typedef boost::multi_array< numpy_int, 2 >        matrix;      ///< A matrix of ints.
typedef boost::multi_array_ref< numpy_int, 1 >    vector_ref;  ///< A reference to a vector of ints.
typedef boost::multi_array_ref< numpy_int, 2 >    matrix_ref;  ///< A reference to a matrix of ints.


/// Execute one time step. Use the current expression levels, X, to calculate the next expression levels, output.
template<
	typename X_t,
	typename J_t,
	typename theta_t,
	typename output_t
>
void
execute_time_step(
	X_t                     X,                 ///< current expression levels
	J_t                     J,                 ///< activation matrix
	theta_t                 theta,             ///< default (constitutive) expression levels
	int                     t,                 ///< current time point
	boost::python::list     external_inputs,   ///< list mapping genes to external inputs
	boost::python::list     input_parameters,  ///< input parameters for external inputs
	output_t                output             ///< output (expression levels at t+1)
) {
	typename boost::range_iterator< output_t >::type o = boost::begin( output );
	typename boost::range_iterator< const theta_t >::type th = boost::begin( theta );
	typename boost::range_iterator< J_t >::type j_row = boost::begin( J );
	myrrh::python::py_seq_iterator external_iter = myrrh::python::py_seq_begin( external_inputs );
	myrrh::python::py_seq_iterator input_param_iter = myrrh::python::py_seq_begin( input_parameters );
	while( boost::end( theta ) != th ) {

		// is there an external input for this gene?
		boost::python::object external_input_fn = *external_iter;
		if( external_input_fn != boost::python::object() ) {
			// yes we have an external input function
			*o = boost::python::extract< typename output_t::element >( external_input_fn( t+1, *input_param_iter ) );
		}
		else
		{
			// no external input - calculate activation
			numpy_int activation = 0;
			typename boost::range_iterator< const typename J_t::const_reference >::type j = boost::begin( *j_row );
			typename boost::range_iterator< X_t >::type x = boost::begin( X );
			while( boost::end( X ) != x ) {
				activation += *j * *x;
				++x;
				++j;
			}

			// check activation levels and external inputs and assign outputs
			if ( 0 == activation ) {
				*o = *th;
			} else if( 0 < activation ) {
				*o = 1; // gene was activated
			} else {
				*o = 0;
			}
		}

		++th;
		++o;
		++j_row;
		++external_iter;
		++input_param_iter;
	}
}





/// Inspect a realisation of a network to calculate when genes turn on and off."
template< typename X_t >
boost::python::list
calculate_change_points( X_t X ) {
	boost::python::list result;

	typename boost::range_iterator< const X_t >::type x_row = boost::begin( X );
	while( boost::end( X ) != x_row ) {
		boost::python::list gene_result;

		typename boost::range_iterator< const typename X_t::const_reference >::type x = boost::begin( *x_row );
		numpy_int t = 0;
		bool on = false;
		while( boost::end( *x_row ) != x ) {
			if( bool( *x ) != on ) {
				gene_result.append( t );
				on = ! on;
			}
			++x;
			++t;
		}
		if( on ) {
			gene_result.append( t );
		}

		result.append( gene_result );
		++x_row;
	}

	return result;
}




BOOST_PYTHON_MODULE( cNetwork )
{
	import_array();

	using boost::python::scope;
	namespace bp = boost::python;

#ifndef NDEBUG
	scope().attr("__debug__") = 1;
	std::cout << "WARNING: Debug version of cNetwork module loaded. If you did not intend this then check your configuration!" << std::endl;
#else //_DEBUG
	scope().attr("__debug__") = 0;
#endif //_DEBUG

	using namespace myrrh::python;
	expose_converters< numpy_int >();

	bp::def(
		"execute_time_step",
		execute_time_step<
			vector_ref,
			matrix_ref,
			vector_ref,
			vector_ref
		>,
		"Execute one time step. Use the current expression levels, X, to calculate the next expression levels, output."
	);

	bp::def(
		"calculate_change_points",
		calculate_change_points< matrix_ref >,
		"Inspect a realisation of a network to calculate when genes are on and off."
	);
}

