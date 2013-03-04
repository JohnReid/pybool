/** Copyright John Reid 2010
*/


/**
 * \file Code to expose the interval template library to python.
 */

#include <boost/python.hpp>
#include <boost/functional/hash.hpp>
#include <boost/itl/interval_set.hpp>
#include <boost/itl/interval_map.hpp>
#include <boost/itl/split_interval_set.hpp>
#include <boost/itl/split_interval_map.hpp>
#include <boost/itl/separate_interval_set.hpp>
#include <boost/test/utils/wrap_stringstream.hpp>


/// Make a string from the streamed arguments.
#define PYITL_MAKE_STRING( x ) ( boost::wrap_stringstream().ref() << x ).str()



//
// Make intervals hashable.
//
namespace boost {
namespace itl {

template<
	class DomainT ,
	ITL_COMPARE Compare
>
std::size_t
hash_value( interval< DomainT, Compare > const & x )
{
	std::size_t seed = 0;
	boost::hash_combine( seed, x.lower() );
	boost::hash_combine( seed, x.upper() );
	return seed;
}

} // namespace itl
} // namespace boost






namespace myrrh {
namespace python {
namespace pyitl {

namespace {

/// If argument is a boost python object, then extract a string from it.
template< typename T >
struct extract_str_if_object_ {
	typedef T return_type;
	T operator()( T t ) { return t; }
};


/// If argument is a boost python object, then extract a string from it. Specialisation for boost::python::object.
template<>
struct extract_str_if_object_< boost::python::object > {
	typedef std::string return_type;
	std::string operator()( boost::python::object t ) {
		using namespace boost::python;
		return extract< std::string >( str( t ) )();
	}
};


/// If argument is a boost python object, then extract a string from it, otherwise return the argument unchanged.
template< typename T >
typename extract_str_if_object_< T >::return_type
extract_str_if_object( T t ) {
	return extract_str_if_object_< T >()( t );
}


/// Is x non-empty?
template< typename base_t >
bool
non_zero( base_t const & x ) {
	return ! x.empty();
}


/// Register those methods that can only be called on discrete intervals.
template< typename base_t, typename Class, bool is_continuous >
struct discrete_interval_methods {
	static void expose( Class & _class ) {
		using namespace boost::python;

		scope(_class).attr("__continuous__") = 1;
	}
};

/// Register those methods that can only be called on discrete intervals. Specialisation when not continuous
template< typename base_t, typename Class >
struct
discrete_interval_methods< base_t, Class, false > {
	static void expose( Class & _class ) {
		using namespace boost::python;

		//DomainT first() const;
		_class.add_property( "first",           &base_t::first, "First (smallest) element of the container." );

		//DomainT last() const;
		_class.add_property( "last",            &base_t::last, "Last (largest) element of the container." );

		{
			scope(_class).attr("__continuous__") = 0;
		}
	}
};


/// Add the object to the container.
template< typename exposed_t >
exposed_t &
add( exposed_t & c, boost::python::object o ) {
	using namespace boost::python;

	{
		extract< typename exposed_t::segment_type > extractor( o );
		if( extractor.check() ) {
			return c.add( extractor() );
		}
	}

	{
		extract< typename exposed_t::element_type > extractor( o );
		if( extractor.check() ) {
			return c.add( extractor() );
		}
	}

	throw std::invalid_argument( "Cannot add objects of this type to this interval container." );
}


template< typename exposed_t >
boost::shared_ptr< exposed_t >
intersection( exposed_t const & e1, exposed_t const & e2 ) {
	boost::shared_ptr< exposed_t > result( new exposed_t );
	*result = e1 & e2;
	return result;
}

/// Expose set like methods
template< typename base_t, typename bp_class_t >
void
expose_set_like_methods( bp_class_t & bp_class ) {

	using namespace boost::python;

	//expose methods just for discrete intervals
	discrete_interval_methods<
		base_t,
		bp_class_t,
		boost::itl::is_continuous< typename base_t::interval_type::domain_type >::value
	>::expose( bp_class );

	//expose iteration
	bp_class.def( "__iter__",          iterator< typename base_t::sub_type >(), "Return an iterator over the container." );

	//size_type cardinality() const;
	bp_class.def( "cardinality",       &base_t::cardinality, "Number of elements in the container (cardinality)." );

	//size_type size() const;
	bp_class.def( "size",              &base_t::size, "An interval container's size is it's cardinality." );

	//difference_type length() const;
	bp_class.def( "__len__",           &base_t::length, "The length of the interval container which is the sum of interval lengths." );

	//void clear() ;
	bp_class.def( "clear",             &base_t::clear, "Clear (empty) the container." );

	//bool empty() const;
	bp_class.add_property( "empty",    &base_t::empty, "Is the container empty?" );
	bp_class.def( "__nonzero__",       &non_zero< typename base_t::sub_type >, "Does the container have any entries." );

	//size_t interval_count() const;
	bp_class.def( "interval_count",    &base_t::interval_count, "Number of intervals which is also the size of the iteration over the container." );

	//size_t iterative_size() const;
	bp_class.def( "iterative_size",    &base_t::iterative_size, "Size of the iteration over this container." );

	//DomainT lower() const;
	bp_class.add_property( "lower",    &base_t::lower, "Lower bound of the first interval." );

	//DomainT upper() const;
	bp_class.add_property( "upper",    &base_t::upper, "Upper bound of the last interval." );

	//T operator  & (T,  const P&)
	//typename base_t::sub_type ( * ptr_intersection )( typename base_t::sub_type, const typename base_t::sub_type & ) = boost::itl::operator&;
	//bp_class.def( "intersection",      ptr_intersection, "Intersection." );
	bp_class.def( "intersection",      &intersection< typename base_t::sub_type >, "Intersection." );


	//void swap(interval_base_set &) ;
	//void clear() ;
	//bool empty() const;
	//template<template< class DomT, ITL_COMPARE Comp, template< class DomT2, ITL_COMPARE >class Interv, ITL_ALLOC Allc > class IntervalSet>
	//  bool contains(const IntervalSet< DomainT, Compare, Interval, Alloc > &) const;
	//template<template< class DomT, ITL_COMPARE Comp, template< class DomT2, ITL_COMPARE >class Interv, ITL_ALLOC Allc > class IntervalSet>
	//  bool contained_in(const IntervalSet< DomainT, Compare, Interval, Alloc > &) const;
	//DomainT first() const;
	//DomainT last() const;
	//const_iterator find(const element_type &) const;

	//SubType & subtract(const element_type &) ;
	//SubType & subtract(const segment_type &) ;
	//SubType & insert(const element_type &) ;
	//SubType & insert(const segment_type &) ;
	//iterator insert(iterator, const segment_type &) ;
	//SubType & add(const element_type &) ;
	//SubType & add(const segment_type &) ;
	//iterator add(iterator, const segment_type &) ;
	bp_class.def( "__iadd__",          &add< typename base_t::sub_type >, "Addition of an element to the container.", return_internal_reference<>() );

	//void add_intersection(interval_base_set &, const element_type &) const;
	//void add_intersection(interval_base_set &, const segment_type &) const;
	//template<template< class DomT, ITL_COMPARE Comp, template< class DomT2, ITL_COMPARE >class Interv, ITL_ALLOC Allc > class IntervalSet>
	//  void add_intersection(interval_base_set &,
	//                        const IntervalSet< DomainT, Compare, Interval, Alloc > &) const;
	//bool intersects(const element_type &) const;
	//bool intersects(const interval_type &) const;
	//SubType & flip(const element_type &) ;
	//SubType & flip(const segment_type &) ;
	//template<class SubType2 >
	//  SubType & flip(const interval_base_set< SubType2, DomainT, Compare, Interval, Alloc > &) ;
}





} // anonymous



/// Register the boundtype enum with boost.python.
void
register_boundtypes_enum();


/**
 * Expose an interval.
 */
template< typename Interval >
struct interval_exposer {

	typedef Interval                            exposed_t;             ///< Interval type to be exposed.
	typedef typename exposed_t::element_type    element_t;              ///< Element type.
	typedef boost::python::class_< exposed_t >  bp_class_t;             ///< Boost.python class type.


	/// Register those methods that can only be called on discrete intervals.
	template< typename Class, bool is_continuous >
	struct discrete_methods {
		static void expose( Class & ) {
		}
	};

	/// Register those methods that can only be called on discrete intervals. Specialisation when not continuous
	template< typename Class >
	struct
	discrete_methods< Class, false > {
		static void expose( Class & bp_class ) {
			using namespace boost::python;

			//interval & as(bound_type) ;
			bp_class.def( "as_",                      &exposed_t::as, "Transforms the interval to the bound-type bounded without changing it's content. Requires Integral<domain_type>.", return_internal_reference<>() );
		}
	};


	/// Hash value.
	static
	std::size_t
	hash( exposed_t const & x ) {
		return boost::hash< exposed_t >()( x );
	}


	/// Check if the object is in the interval.
	static
	bool
	contains( exposed_t const & x, boost::python::object o ) {
		using namespace boost::python;

		{
			extract< typename exposed_t::domain_type > extractor( o );
			if( extractor.check() ) {
				return x.contains( extractor() );
			}
		}

		{
			extract< exposed_t > extractor( o );
			if( extractor.check() ) {
				return x.contains( extractor() );
			}
		}

		throw std::invalid_argument( "Cannot test objects of this type for inclusion in an interval." );
	}


	/// Expose/register the interval.
	static
	void
	expose( char const * name, char const * docstring ) {
		using namespace boost::python;
	    using namespace boost::mpl;

	    bp_class_t bp_class(
			name,
			docstring,
			init< element_t, element_t >( args( "start", "end" ), "Construct a closed interval containing both end points." )
		);

	    // add other constructors
	    bp_class.def( init<>( "Contructs an empty interval." ) );
	    bp_class.def( init< element_t >( "Contructs a singleton interval." ) );
	    bp_class.def( init< element_t, element_t, boost::itl::bound_type >( "Contructs an interval with given bound type." ) );


	    //expose discrete methods if applicable
		discrete_methods<
			bp_class_t,
			boost::itl::is_continuous< typename exposed_t::domain_type >::value
		>::expose( bp_class );

		discrete_interval_methods<
			exposed_t,
			bp_class_t,
			boost::itl::is_continuous< typename exposed_t::domain_type >::value
		>::expose( bp_class );

		//bool empty() const;
		bp_class.add_property( "empty",           &exposed_t::empty, "Is the interval empty?" );
		bp_class.def( "__nonzero__",              &non_zero< exposed_t >, "Does the interval contain anything?" );

		//void clear() ;
		bp_class.def( "clear",                    &exposed_t::clear, "Set the interval empty." );

		//bool contains(const DomainT &) const;
		//bool contains(const interval &) const;
		bp_class.def( "__contains__",             &contains, "Does the interval contain x?" );

		//bool contained_in(const interval &) const;
		bp_class.def( "contained_in",             &exposed_t::contained_in, "Is this is a subset of super?" );

		//bool free_contains(const interval &) const;
		bp_class.def( "free_contains",            &exposed_t::free_contains, "Is this is a proper subset of super and does not touch the borders?" );

		//bool proper_contains(const interval &) const;
		bp_class.def( "proper_contains",          &exposed_t::proper_contains, "Is this is a proper subset of super?" );

		//bool is_disjoint(const interval &) const;
		bp_class.def( "is_disjoint",              &exposed_t::is_disjoint, "Are the 2 intervals disjoint?" );

		//bool intersects(const interval &) const;
		bp_class.def( "intersects",               &exposed_t::intersects, "Is the intersection non-empty?" );

		//size_type cardinality() const;
		bp_class.add_property( "cardinality",     &exposed_t::cardinality, "Cardinality of the interval: The number of elements." );

		//difference_type length() const;
		bp_class.def( "__len__",                  &exposed_t::length, "Arithmetic size of the interval." );

		//size_type size() const;
		bp_class.add_property( "size",            &exposed_t::size, "Size of the interval." );

		//DomainT lower() const;
		bp_class.add_property( "lower",           &exposed_t::lower, "Lower bound of the interval." );

		//DomainT upper() const;
		bp_class.add_property( "upper",           &exposed_t::upper, "Upper bound of the interval." );

		//bound_type boundtype() const;
		bp_class.add_property( "boundtype",       &exposed_t::boundtype, "Type of interval bounds." );

		//interval & extend(const interval &) ;
		exposed_t ( * ptr_hull )( exposed_t, const exposed_t & ) = &boost::itl::hull;
		bp_class.def( "hull",                     ptr_hull, "hull returns the smallest interval containing left and right." );
		bp_class.def( "extend",                   &exposed_t::extend, "Extend *this to x2 yielding an interval from the minimum of lower bounds to the maximum of upper bounds.", return_internal_reference<>() );

		//interval & left_extend(const interval &) ;
		bp_class.def( "left_extend",              &exposed_t::left_extend, "Extend *this interval to the minimum of the lower bounds of this and left_extension.", return_internal_reference<>() );

		//interval & right_extend(const interval &) ;
		bp_class.def( "right_extend",             &exposed_t::right_extend, "Extend *this interval to the maximum of the upper bounds of this and right_extension.", return_internal_reference<>() );

		//interval & left_set(const interval &) ;
		bp_class.def( "left_set",                 &exposed_t::left_set, "Set the lower bound and bound type according to interval left.", return_internal_reference<>() );

		//interval & right_set(const interval &) ;
		bp_class.def( "right_set",                &exposed_t::right_set, "Set the upper bound and bound type according to interval right.", return_internal_reference<>() );

		//interval span(const interval &) const;
		exposed_t ( exposed_t:: * ptr_span )( const exposed_t & rhs ) const = &exposed_t::span;
		bp_class.def( "span",                     ptr_span, "Interval spanning from lower bound of *this interval to the upper bound of rhs. Bordertypes according to the lower bound of *this and the upper bound of rhs." );

		//interval & left_subtract(const interval &) ;
		bp_class.def( "left_subtract",            &exposed_t::left_subtract, "subtract left_minuend from *this interval on it's left side.", return_internal_reference<>() );

		//interval & right_subtract(const interval &) ;
		bp_class.def( "right_subtract",           &exposed_t::right_subtract, "subtract right_minuend from *this interval on it's right side.", return_internal_reference<>() );

		//interval & operator&=(const interval &) ;
		exposed_t ( * ptr_and )( exposed_t, const exposed_t & ) = &boost::itl::operator&;
		bp_class.def( "__and__",                  ptr_and, "Intersection with the interval x2; assign result to isec." );
		bp_class.def( "__iand__",                 &exposed_t::operator&=, "Intersection with the interval x2; assign result to isec.", return_internal_reference<>() );

		//const std::string as_string() const;
		bp_class.def( "__str__",                  &exposed_t::as_string, "String representation." );
		bp_class.def( "__repr__",                 &exposed_t::as_string, "String representation." );

		//bool is(bound_type) const;
		bp_class.def( "type_is",                  &exposed_t::is, "What type is the interval?" );

		//bool is_left(bound_type) const;
		bp_class.def( "is_left",                  &exposed_t::is_left, "What bound type is the left interval border?" );

		//bool is_right(bound_type) const;
		bp_class.def( "is_right",                 &exposed_t::is_right, "What bound type is the right interval border?" );

		//bool touches(const interval &) const;
		bp_class.def( "touches",                  &exposed_t::touches, "There is no gap between *this and x2 but they have no element in common." );

		//bool exclusive_less(const interval &) const;
		bp_class.def( "exclusive_less",           &exposed_t::exclusive_less, "Maximal element of *this is less than the minimal element of x2." );

		//bool distant_less(const interval &) const;
		bp_class.def( "distant_less",             &exposed_t::distant_less, "Maximal element of *this is less than the minimal element of x2 and there is at least one element in between." );

		//interval & set(const DomainT &, const DomainT &, bound_type) ;
		bp_class.def( "set",                      &exposed_t::set, "Set the interval to the given bounds and bound type.", return_internal_reference<>() );

		//bool lower_less(const interval &) const;
		bp_class.def( "lower_less",               &exposed_t::lower_less, "First element of *this is less than first element of x2." );

		//bool upper_less(const interval &) const;
		bp_class.def( "upper_less",               &exposed_t::upper_less, "Last element of *this is less than last element of x2." );

		//bool lower_less_equal(const interval &) const;
		bp_class.def( "lower_less_equal",         &exposed_t::lower_less_equal, "First element of *this is less than or equal to the first element of x2." );

		//bool upper_less_equal(const interval &) const;
		bp_class.def( "upper_less_equal",         &exposed_t::upper_less_equal, "Last element of *this is less than or equal to the last element of x2." );

		//bool lower_equal(const interval &) const;
		bp_class.def( "lower_equal",              &exposed_t::lower_equal, "First element of *this is equal to the first element of x2." );

		//bool upper_equal(const interval &) const;
		bp_class.def( "upper_equal",              &exposed_t::upper_equal, "Last element of *this is equal to the last element of x2." );

		bp_class.def( "__hash__",                 hash, "Hash value." );

		bool ( * ptr_equals )( const exposed_t &, const exposed_t & ) = &boost::itl::operator==;
		bp_class.def( "__eq__",                   ptr_equals, "Interval equality." );

		bool ( * ptr_notequals )( const exposed_t &, const exposed_t & ) = &boost::itl::operator!=;
		bp_class.def( "__ne__",                   ptr_notequals, "Interval inequality." );

		bool ( * ptr_less )( const exposed_t &, const exposed_t & ) = &boost::itl::operator<;
		bp_class.def( "__lt__",                   ptr_less, "Interval less." );

		bool ( * ptr_lessequals )( const exposed_t &, const exposed_t & ) = &boost::itl::operator<=;
		bp_class.def( "__le__",                   ptr_lessequals, "Interval less or equals." );

		bool ( * ptr_greater )( const exposed_t &, const exposed_t & ) = &boost::itl::operator>;
		bp_class.def( "__gt__",                   ptr_greater, "Interval greater." );

		bool ( * ptr_greaterequals )( const exposed_t &, const exposed_t & ) = &boost::itl::operator>=;
		bp_class.def( "__ge__",                   ptr_greaterequals, "Interval greater or equals." );

		//static interval closed(const DomainT &, const DomainT &) ;
		bp_class.def( "closed",                   &exposed_t::closed, "Closed interval [low,up]." );
		bp_class.staticmethod( "closed" );

		//static interval rightopen(const DomainT &, const DomainT &) ;
		bp_class.def( "rightopen",                &exposed_t::rightopen, "Right open interval [low,up)." );
		bp_class.staticmethod( "rightopen" );

		//static interval leftopen(const DomainT &, const DomainT &) ;
		bp_class.def( "leftopen",                 &exposed_t::leftopen, "Left open interval (low,up]." );
		bp_class.staticmethod( "leftopen" );

		//static interval open(const DomainT &, const DomainT &) ;
		bp_class.def( "open",                     &exposed_t::open, "Open interval (low,up)." );
		bp_class.staticmethod( "open" );

		//static interval whole() ;
		bp_class.def( "whole",                    &exposed_t::whole, "Whole range of domain." );
		bp_class.staticmethod( "whole" );

	}

};






/**
 * Expose a interval set.
 */
template< typename IntervalContainerSet >
struct interval_set_exposer {

	typedef IntervalContainerSet                           exposed_t;                 ///< The type to be exposed.
	typedef boost::shared_ptr< exposed_t >                 exposed_ptr;               ///< Pointer to exposed type.
	typedef boost::python::class_<
		exposed_t,
		exposed_ptr,
		//boost::python::bases< typename exposed_t::base_type >,
		boost::noncopyable
	>                                                      bp_class_t;                ///< Boost.python class type.

	/// Check if the object is in the container.
	static
	bool
	contains( exposed_t const & c, boost::python::object o ) {
		using namespace boost::python;

		{
			extract< typename exposed_t::base_type::element_type > extractor( o );
			if( extractor.check() ) {
				return c.contains( extractor() );
			}
		}

		{
			extract< typename exposed_t::base_type::segment_type > extractor( o );
			if( extractor.check() ) {
				return c.contains( extractor() );
			}
		}

		throw std::invalid_argument( "Cannot test objects of this type for inclusion in this interval container." );
	}


	/// Check if the object intersects the container.
	static
	bool
	intersects( exposed_t const & c, boost::python::object o ) {
		using namespace boost::python;

		{
			extract< typename exposed_t::base_type::element_type > extractor( o );
			if( extractor.check() ) {
				return c.intersects( extractor() );
			}
		}

		{
			extract< typename exposed_t::base_type::interval_type > extractor( o );
			if( extractor.check() ) {
				return c.intersects( extractor() );
			}
		}

		throw std::invalid_argument( "Cannot test objects of this type for intersection with this interval container." );
	}


	/// Add an interval
	static
	exposed_t &
	add( exposed_t & c, typename exposed_t::interval_type i ) {
		return c += i;
	}


	/// Erase the object from the container.
	static
	exposed_t &
	erase( exposed_t & c, boost::python::object o ) {
		using namespace boost::python;

		{
			extract< typename exposed_t::element_type > extractor( o );
			if( extractor.check() ) {
				return c.erase( extractor() );
			}
		}

		{
			extract< typename exposed_t::segment_type > extractor( o );
			if( extractor.check() ) {
				return c.erase( extractor() );
			}
		}

		throw std::invalid_argument( "Cannot erase objects of this type from this interval container." );
	}


	/// Expose the exposed_t using boost.python.
	static
	void
	expose( char const * name, char const * docstring ) {
		using namespace boost::python;

		bp_class_t bp_class( name, docstring );

		expose_set_like_methods< typename exposed_t::base_type >( bp_class );

		//bool contains(const element_type &) const;
		//bool contains(const segment_type &) const;
		//bool contains(const interval_base_set &) const;
		bp_class.def( "__in__",         &contains, "Is the argument in the container?" );

		//bool intersects(const element_type &) const;
		//bool intersects(const interval_type &) const;
		bp_class.def( "intersects",     &intersects, "Does the argument intersect the container?" );

		//SubType & erase(const element_type &) ;
		//SubType & erase(const segment_type &) ;
		//void erase(iterator) ;
		//void erase(iterator, iterator) ;
		bp_class.def( "__iminus__",     &erase, "Erase the argument from the container.", return_internal_reference<>() );

		bp_class.def( "add",            &add, "Add the interval.", return_internal_reference<>() );
	}
};







/**
 * Expose a interval map.
 */
template< typename IntervalContainerMap >
struct interval_map_exposer {

	typedef IntervalContainerMap                           exposed_t;                 ///< The container type to be exposed.
	typedef typename exposed_t::base_type                  base_t;                    ///< The base type.
	typedef typename exposed_t::codomain_type              codomain_t;                ///< The codomain type.
	typedef typename exposed_t::element_type               element_t;                 ///< The element type.
	typedef typename exposed_t::interval_type              interval_t;                ///< The interval type.
	typedef typename exposed_t::segment_type               segment_t;                 ///< The segment type.
	typedef std::pair< const interval_t, codomain_t >      entry_t;                   ///< An entry in the map.
	typedef boost::shared_ptr< exposed_t >                 container_ptr_t;           ///< A type for a pointer to the container.
	typedef boost::python::class_<
		exposed_t,
		container_ptr_t,
		//boost::python::bases< typename exposed_t::base_type >,
		boost::noncopyable
	>                                                      bp_class_t;                ///< Boost.python class type.


	/// Check if the object is in the container.
	static
	bool
	contains( exposed_t const & c, boost::python::object o ) {
		using namespace boost::python;

		{
			extract< typename exposed_t::domain_type > extractor( o );
			if( extractor.check() ) {
				return c.contains( extractor() );
			}
		}

		{
			extract< typename exposed_t::interval_type > extractor( o );
			if( extractor.check() ) {
				return c.contains( extractor() );
			}
		}

		{
			extract< typename exposed_t::element_type > extractor( o );
			if( extractor.check() ) {
				return c.contains( extractor() );
			}
		}

		{
			extract< typename exposed_t::segment_type > extractor( o );
			if( extractor.check() ) {
				return c.contains( extractor() );
			}
		}

		throw std::invalid_argument( "Cannot test objects of this type for inclusion in this interval container." );
	}


	/// Erase the object from the container.
	static
	exposed_t &
	erase( exposed_t & c, boost::python::object o ) {
		using namespace boost::python;

		{
			extract< typename exposed_t::domain_type > extractor( o );
			if( extractor.check() ) {
				return c.erase( extractor() );
			}
		}

		{
			extract< typename exposed_t::interval_type > extractor( o );
			if( extractor.check() ) {
				return c.erase( extractor() );
			}
		}

		{
			extract< typename exposed_t::element_type > extractor( o );
			if( extractor.check() ) {
				return c.erase( extractor() );
			}
		}

		{
			extract< typename exposed_t::segment_type > extractor( o );
			if( extractor.check() ) {
				return c.erase( extractor() );
			}
		}

		throw std::invalid_argument( "Cannot erase objects of this type from this interval container." );
	}


	/// Set the values all the values for the given interval in the container.
	static
	exposed_t &
	set( exposed_t & c, boost::python::object o ) {
		using namespace boost::python;

		{
			extract< typename exposed_t::segment_type > extractor( o );
			if( extractor.check() ) {
				return c.set( extractor() );
			}
		}

		{
			extract< typename exposed_t::element_type > extractor( o );
			if( extractor.check() ) {
				return c.set( extractor() );
			}
		}

		throw std::invalid_argument( "Cannot set objects of this type in an interval container." );
	}


	/// Wrapper for find
	static
	entry_t
	find( exposed_t const & c, typename exposed_t::domain_type k ) {
		typename exposed_t::const_iterator i = c.find( k );
		if( c.end() == i ) {
			throw std::range_error( "Could not find key." );
		}
		return *i;
	}


	/// String representation
	template< typename Segment >
	static
	std::string
	segment_str( Segment const & x ) {
		return PYITL_MAKE_STRING(
			x.first.as_string()
			<< "; "
			<< extract_str_if_object( x.second )
		);
	}


	/// Add an interval-value pair
	static
	exposed_t &
	add( exposed_t & c, typename exposed_t::interval_type i, typename exposed_t::codomain_type v ) {
		return c += typename exposed_t::segment_type( i, v );
	}


	/// Expose/register the interval container.
	static
	void
	expose( char const * name, char const * docstring ) {
		using namespace boost::python;

		// the class
		bp_class_t bp_class( name, docstring );

		expose_set_like_methods< typename exposed_t::base_type >( bp_class );

		{
			scope _scope = bp_class;

			// expose the segment type inside the class scope
			class_<
				segment_t
			> segment_class(
				"Segment",
				"A segment in the interval map, that is an interval and an associated value.",
				init< interval_t, codomain_t >( args( "interval", "value" ), "Construct a segment (an interval, value pair)." )
			);
			segment_class.def( "__str__", segment_str< segment_t >, "String representation." );
			segment_class.def_readonly( "interval", &segment_t::first,  "The segment's interval." );
			segment_class.def_readonly( "value",    &segment_t::second, "The segment's value." );

			// and the entry type
			class_<
				entry_t
			> entry_class(
				"Entry",
				"An entry in the interval map, that is a const interval and an associated value.",
				no_init
			);
			entry_class.def( "__str__", segment_str< entry_t >, "String representation." );
			entry_class.def_readonly( "interval", &entry_t::first,  "The entry's interval." );
			entry_class.def_readonly( "value",    &entry_t::second, "The entry's value." );

		}

		// expose the fundamental aspect

		//bool contains(const domain_type &) const;
		//bool contains(const interval_type &) const;
		//bool contains(const element_type &) const;
		//bool contains(const segment_type &) const;
		//template<class SetType >
		//bool contains(const interval_base_set< SetType, DomainT, Compare, Interval, Alloc > &) const;
		//template<class MapType >
		//bool contains(const interval_base_map< MapType, DomainT, CodomainT, Traits, Compare, Combine, Section, Interval, Alloc > &) const;
		bp_class.def( "__contains__",      &contains );

		//template<class MapType >
		//bool contained_in(const interval_base_map< MapType, DomainT, CodomainT, Traits, Compare, Combine, Section, Interval, Alloc > &) const;

		//const_iterator find(const domain_type &) const;
		bp_class.def( "__getitem__",       &find, "Find the interval value pair, that contains key." );

		//SubType & set(const element_type &) ;
		//SubType & set(const segment_type &) ;
		bp_class.def( "set",               &set, "With interval_value_pair = (I,v) set value v for all keys in interval I in the map.", return_internal_reference<>() );

		//codomain_type operator()(const domain_type &) const;
		//SubType & subtract(const element_type &) ;
		//SubType & subtract(const segment_type &) ;
		//SubType & erase(const element_type &) ;
		//SubType & erase(const segment_type &) ;
		//SubType & erase(const domain_type &) ;
		//SubType & erase(const interval_type &) ;
		//template<class SetSubType >
		//SubType & erase(const interval_base_set< SetSubType, DomainT, Compare, Interval, Alloc > &) ;
		//SubType & erase(const interval_base_map &) ;
		//void erase(iterator) ;
		//void erase(iterator, iterator) ;
		bp_class.def( "erase",              &erase, "Erase a object from the map.", return_internal_reference<>() );

		bp_class.def( "add",                &add, "Add the interval-value pair.", return_internal_reference<>() );
	}

};


} // namespace pyitl
} // namespace python
} // namespace myrrh
