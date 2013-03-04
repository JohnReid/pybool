/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_MATH_H_
#define MYRRH_MATH_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"

#include <boost/iterator.hpp>
#include <boost/range.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <gsl/gsl_sf_log.h>
#include <gsl/gsl_sf_exp.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_machine.h>

#include <numeric>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <math.h>

#include <boost/version.hpp>
#include <boost/test/floating_point_comparison.hpp>
#if BOOST_VERSION >= 104400
#define MYRRH_FPC_NS ::boost::math::fpc ///< Floating point comparison namespace
#else
#define MYRRH_FPC_NS ::boost::test_tools:: ///< Floating point comparison namespace
#endif

#ifdef _MSC_VER
# define MYRRH_ISNAN( x ) _isnan( x )
# define MYRRH_ISFINITE( x ) _finite( x )
#else
# define MYRRH_ISNAN( x ) isnan( x )
# define MYRRH_ISFINITE( x ) finite( x )
#endif

#ifdef max
#undef max
#endif //max

#ifdef min
#undef min
#endif //min



namespace myrrh {



template< typename T >
T int_power( T x, unsigned exponent )
{
	T result = T( 1.0 );
	for( unsigned i = 0; exponent != i; ++i ) result *= x;
	return result;
}

inline
int
nearest_integer( double a )
{
	return static_cast<int>( a < 0.0 ? a - 0.5 : a + 0.5 );
}

inline bool within_closed_range( double x, double lower, double higher )
{
	return MYRRH_ISFINITE( x ) && lower <= x && x <= higher;
}

inline bool is_probability( double x )
{
	return within_closed_range( x, 0.0, 1.0 );
}

template< typename T >
bool
is_close( T t1, T t2, T percent_tolerance )
{
	return boost::test_tools::check_is_close( t1, t2, MYRRH_FPC_NS::percent_tolerance( percent_tolerance ) );
}


inline
double
safe_exp( double x )
{
	return ( x <= GSL_LOG_DBL_MIN ) ? 0.0 : gsl_sf_exp( x );
}

inline
double
safe_log( double x )
{
	if( 0.0 == x ) return - std::numeric_limits< double >::max();
	if( 0.0 > x ) return std::numeric_limits< double >::quiet_NaN();
	return gsl_sf_log( x );
}


/**
Returns the log of the sum of 2 values, when the values are passed as their logarithms.

I.e. we have log(p) and log(q), we want to calculate log(p+q).
*/
inline
double
log_sum_of_logs(
	double log_p,
	double log_q )
{
	//make sure log_p is the larger of the two, so that exp( log(q) - log(p) ) is small
	if( log_q > log_p ) std::swap( log_q, log_p );
	BOOST_ASSERT( log_p >= log_q );

	return log_p + gsl_sf_log_1plusx( safe_exp( log_q - log_p ) );
}

/** Ensures that the sum of the elements in the range adds to the given value. Returns the scale factor used. */
template< typename Range >
double
scale(
	Range & range,
	typename boost::range_value< Range >::type desired_sum = 1.0 )
{
	using boost::begin;
	using boost::end;
	using boost::range_value;

	typedef typename range_value< Range >::type value_type;

	const value_type current_sum =
		std::accumulate(
			begin( range ),
			end( range ),
			0.0 );

	const double scale_factor = desired_sum / current_sum;
	BOOST_FOREACH( value_type & v, range ) v *= scale_factor;

	return scale_factor;
}

namespace gsl {
namespace impl {

inline
void
error_handler(
	const char * reason,
	const char * file,
	int line,
	int gsl_errno)
{
	using namespace std;

	const string msg = MYRRH_MAKE_STRING( "GSL error(" << gsl_errno << "): " << file << "(" << line << "): " << reason );
	//cout << msg << endl;
	throw logic_error( msg );
}

} //namespace impl


inline
void init()
{
	static bool inited = false;
	if( ! inited )
	{
		gsl_set_error_handler( impl::error_handler );
		inited = true;
	}
}

inline
gsl_rng *
get_rng()
{
	static const gsl_rng_type * T = 0;
	static gsl_rng * r = 0;

	if( 0 == T || 0 == r )
	{
		init();

		gsl_rng_env_setup();

		T = gsl_rng_default;
		if( ! T ) throw std::logic_error( "Could not find default GSL random number generator type" );

		r = gsl_rng_alloc( T );
		if( ! r ) throw std::logic_error( "Could not allocate GSL random number generator" );
	}
	return r;
}



template< typename Range >
unsigned
one_sample_from_multinomial(
	const Range & dist,
	double weight = 1.0,
	gsl_rng * rng = gsl::get_rng() )
{
	double uniform_sample = gsl_rng_uniform( rng ) * weight;
	unsigned result = 0;
	BOOST_FOREACH( double p, dist )
	{
		uniform_sample -= p;
		if( uniform_sample < 0.0 ) return result;
		++result;
	}
	throw std::logic_error( "Multinomial parameters did not sum to 1 (or the supplied weight)" );
}


template<
	typename ParameterRange,
	typename OutputRange
>
void
draw_from_dirichlet(
	const ParameterRange & alpha,
	OutputRange & output,
	gsl_rng * rng = gsl::get_rng() )
{
	using boost::size;
	using boost::begin;

	BOOST_ASSERT( size( alpha ) == size( output ) );

	gsl_ran_dirichlet(
		rng,
		size( alpha ),
		&*begin( alpha ),
		&*begin( output ) );
}


template<
	typename ParameterRange,
	typename ValueRange
>
double
dirichlet_ln_pdf(
	const ParameterRange & alpha,
	const ValueRange & values )
{
	using namespace boost;

	typedef typename range_value< ParameterRange >::type param_t;
	typedef typename range_value< ValueRange >::type value_t;
	typedef boost::tuple< param_t, value_t > tuple;

	BOOST_ASSERT( size( alpha ) == size( values ) );

	double result = 0.0;
	param_t alpha_sum = 0.0;

	BOOST_FOREACH(
		const tuple & t,
		make_iterator_range(
			make_zip_iterator(
				make_tuple(
					const_begin( alpha ),
					const_begin( values ) ) ),
			make_zip_iterator(
				make_tuple(
					const_end( alpha ),
					const_end( values )) ) ) )
	{
		const param_t a = t.get< 0 >();
		const value_t v = t.get< 1 >();
		alpha_sum += a;
		result += ( a - param_t( 1.0 ) ) * gsl_sf_log( v );
		std::cout << a << "\n";
		result -= gsl_sf_lngamma( a );
	}

	return result + gsl_sf_lngamma( alpha_sum );
}



/**
Pre-processed sampling from general discrete distributions.

See http://www.gnu.org/software/gsl/manual/html_node/General-Discrete-Distributions.html
*/
struct ran_discrete : boost::noncopyable
{
	typedef boost::shared_ptr< ran_discrete > ptr;

	gsl_ran_discrete_t * _pre_processed;

	inline ran_discrete(size_t K, const double * P)
		: _pre_processed( gsl_ran_discrete_preproc( K, P ) )
	{
	}

	template< typename Range >
	ran_discrete( const Range & P_range )
	{
		using namespace boost;
		const size_t K = size( P_range );
		std::vector< double > P( K );
		std::copy( begin( P_range ), end( P_range ), P.begin() );
		_pre_processed = gsl_ran_discrete_preproc( K, &(*(P.begin())) );
	}

	inline ~ran_discrete()
	{
		gsl_ran_discrete_free( _pre_processed );
	}

	size_t sample(gsl_rng * rng = get_rng()) const
	{
		return gsl_ran_discrete( rng, _pre_processed );
	}
};






} //namespace gsl
} //namespace myrrh


#endif //MYRRH_MATH_H_

