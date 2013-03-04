/**
@file

Copyright John Reid 2008, 2010

*/

#ifndef MYRRH_SEQAN_BOOST_ADAPTORS_H_
#define MYRRH_SEQAN_BOOST_ADAPTORS_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"

#include <boost/range.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <cstring>

#include <seqan/sequence.h>
#include <seqan/modifier.h>
//#include <seqan/modifier/modifier_iterator.h>
//#include <seqan/modifier/modifier_string.h>


namespace myrrh {





/**
A standard conforming iterator that encapsulates a seqan iterator.
*/
template< typename SeqanIt >
struct seqan_iterator
	: public boost::iterator_facade<
		seqan_iterator< SeqanIt >,
		typename seqan::Value< SeqanIt >::Type,
		boost::random_access_traversal_tag,
		typename seqan::Reference< SeqanIt >::Type,
		typename seqan::Difference< SeqanIt >::Type
    >
{
	typedef SeqanIt                                           seqan_it;               /**< The seqan iterator. */
	typedef seqan_iterator< seqan_it >                        self_t;                 /**< This type. */
	typedef boost::iterator_facade<
		self_t,
		typename seqan::Value< SeqanIt >::Type,
		boost::random_access_traversal_tag,
		typename seqan::Reference< SeqanIt >::Type,
		typename seqan::Difference< SeqanIt >::Type
	> base_t;
	//using typename base_t::difference_type;

	seqan_iterator() { }

	seqan_iterator( seqan_it it ) : it( it ) { }

private:
    friend class boost::iterator_core_access;

    void increment() { ++it; }

	void decrement() { --it; }

	void advance( typename base_t::difference_type n ) { it += n; }

	typename base_t::difference_type distance_to( self_t const & other ) const { return other.it - this->it; }

    bool equal( self_t const & other ) const
    {
        return this->it == other.it;
    }

    typename seqan::Reference< SeqanIt >::Type dereference() const { return *it; }

	seqan_it it;
};









/**
A standard conforming iterator that encapsulates a seqan string iterator.
*/
template< typename TValue, typename TSpec >
struct seqan_string_iterator
	: public boost::iterator_facade<
		seqan_string_iterator< TValue, TSpec >,
		TValue,
		boost::random_access_traversal_tag
    >
{
	typedef seqan_string_iterator< TValue, TSpec >  this_t;
	typedef boost::iterator_facade<
		seqan_string_iterator< TValue, TSpec >,
		TValue,
		boost::random_access_traversal_tag
	> base_t;
	//using typename base_t::difference_type;
	typedef typename seqan::Iterator< seqan::String< TValue, TSpec > >::Type seqan_it;

	seqan_string_iterator() { }

	seqan_string_iterator(seqan_it it) : it( it ) { }

private:
    friend class boost::iterator_core_access;

    void increment() { ++it; }

	void decrement() { --it; }

	void advance( typename base_t::difference_type n ) { it += n; }

	typename base_t::difference_type distance_to( this_t const & other ) const { return other.it - this->it; }

    bool equal( this_t const & other ) const
    {
        return this->it == other.it;
    }

    TValue & dereference() const { return *it; }

	seqan_it it;
};




/**
A standard conforming iterator that iterates over a seqan string set.
*/
template< typename Value, typename string_set_t >
class seqan_string_set_iterator
	: public boost::iterator_facade<
		seqan_string_set_iterator< Value, string_set_t >,
		Value,
		boost::random_access_traversal_tag
	>
{
public:
	typedef seqan_string_set_iterator< Value, string_set_t >   this_t;            /**< The type of this class. */
	typedef unsigned                                           position_t;        /**< The type that represents a position in the string set. */
	typedef boost::iterator_facade<
        seqan_string_set_iterator< Value, string_set_t >,
        Value,
        boost::random_access_traversal_tag
	> base_t;

	/** Constructor. */
	seqan_string_set_iterator()
		: string_set( 0 )
		, position( 0 )
	{ }

	/** Constructor. */
	explicit seqan_string_set_iterator( const string_set_t * string_set, position_t position )
		: string_set( const_cast< string_set_t * >( string_set ) )
		, position( position )
	{ }

private:
	friend class boost::iterator_core_access;

	bool equal( this_t const & other ) const {
		return this->string_set == other.string_set && this->position == other.position;
	}

	void increment() { ++position; }

	void decrement() { --position; }

	void advance( typename base_t::difference_type n ) { position += n; }

	typename base_t::difference_type distance_to( this_t const & other ) const { return other.position - this->position; }

	Value & dereference() const {
		return seqan::getValueById(
			*string_set,
			seqan::positionToId(
				*string_set,
				position
			)
		);
	}

	// data members
	string_set_t *   string_set;       /** The string set we are iterating over. */
	position_t       position;         /** The position we are at. */
};







} //namespace myrrh






//
// Specialize metafunctions. We must include the range.hpp header.
// We must open the boost namespace.
//
namespace boost {





/**
Metafunction that boost range uses to determine the type of a mutable iterator for a seqan string.
*/
template< typename TValue, typename TSpec >
struct range_mutable_iterator< seqan::String< TValue, TSpec > >
{
	typedef myrrh::seqan_string_iterator< TValue, TSpec > type;
};

/**
Metafunction that boost range uses to determine the type of a const iterator for a seqan string.
*/
template< typename TValue, typename TSpec >
struct range_const_iterator< seqan::String< TValue, TSpec > >
{
	typedef myrrh::seqan_string_iterator< const TValue, TSpec > type;
};





/**
Metafunction that boost range uses to determine the type of a mutable iterator for a seqan segment.
*/
template< typename THost, typename TSpec >
struct range_mutable_iterator< seqan::Segment< THost, TSpec > >
{
	typedef myrrh::seqan_iterator< typename seqan::Iterator< seqan::Segment< THost, TSpec > >::Type > type;
};

/**
Metafunction that boost range uses to determine the type of a const iterator for a seqan segment.
*/
template< typename THost, typename TSpec >
struct range_const_iterator< seqan::Segment< THost, TSpec > >
{
	typedef myrrh::seqan_iterator< typename seqan::Iterator< seqan::Segment< const THost, TSpec > >::Type > type;
};





/**
Metafunction that boost range uses to determine the type of a mutable iterator for a seqan string set.
*/
template< typename TString, typename TSpec >
struct range_mutable_iterator< seqan::StringSet< TString, TSpec > >
{
	typedef myrrh::seqan_string_set_iterator< TString, seqan::StringSet< TString, TSpec > > type;
};

/**
Metafunction that boost range uses to determine the type of a const iterator for a seqan string set.
*/
template< typename TString, typename TSpec >
struct range_const_iterator< seqan::StringSet< TString, TSpec > >
{
	typedef myrrh::seqan_string_set_iterator< const TString, seqan::StringSet< TString, TSpec > > type;
};






} //namespace boost




//
// Put mutable and const range_begin and range_end functions in the seqan namespace for ADL.
//
namespace seqan {



/// anonymous namespace
namespace {

/// Selects const versions depending on template parameter
template< typename T, bool const_ >
struct const_selector {
};

/// Specialisation of const_selector for mutable types
template< typename T >
struct const_selector< T, false > {
	typedef typename boost::range_mutable_iterator< T>::type      iterator;     ///< iterator type
};

/// Specialisation of const_selector for const types
template< typename T >
struct const_selector< T, true > {
	typedef typename boost::range_const_iterator< T >::type       iterator;     ///< iterator type
};
} // anonymous namespace


#define MYRRH_IMPLEMENT_RANGE_BEGIN_END( T, NON_CONST_T, IT, BEGIN_END ) \
	template< typename TPARAM1, typename TPARAM2 > \
	inline \
	typename boost::IT< NON_CONST_T< TPARAM1, TPARAM2 > >::type \
	range_ ## BEGIN_END( T< TPARAM1, TPARAM2 > & x ) \
	{ \
		return typename boost::IT< NON_CONST_T< TPARAM1, TPARAM2 > >::type( seqan::BEGIN_END( x ) ); \
	}


#define MYRRH_IMPLEMENT_RANGE( T ) \
	MYRRH_IMPLEMENT_RANGE_BEGIN_END( const T, T, range_const_iterator  , begin ); \
	MYRRH_IMPLEMENT_RANGE_BEGIN_END( const T, T, range_const_iterator  , end ); \
	MYRRH_IMPLEMENT_RANGE_BEGIN_END( T      , T, range_mutable_iterator, begin ); \
	MYRRH_IMPLEMENT_RANGE_BEGIN_END( T      , T, range_mutable_iterator, end );


MYRRH_IMPLEMENT_RANGE( seqan::ModifiedString )

MYRRH_IMPLEMENT_RANGE( seqan::String )

///** range_begin for a mutable seqan string. */
//template< typename TValue, typename TSpec >
//inline
//typename boost::range_mutable_iterator< seqan::String< TValue, TSpec > >::type
//range_begin( seqan::String< TValue, TSpec > & x )
//{
//	return typename boost::range_mutable_iterator< seqan::String< TValue, TSpec > >::type( seqan::begin( x ) );
//}
//
//
//
///** range_begin for a const seqan string. */
//template< typename TValue, typename TSpec >
//inline
//typename boost::range_const_iterator< seqan::String< TValue, TSpec > >::type
//range_begin( const seqan::String< TValue, TSpec > & x )
//{
//	return typename boost::range_const_iterator< seqan::String< TValue, TSpec > >::type( seqan::begin( x ) );
//}
//
//
//
///** range_end for a mutable seqan string. */
//template< typename TValue, typename TSpec >
//inline
//typename boost::range_mutable_iterator< seqan::String< TValue, TSpec > >::type
//range_end( seqan::String< TValue, TSpec > & x )
//{
//	return typename boost::range_mutable_iterator< seqan::String< TValue, TSpec > >::type( seqan::end( x ) );
//}
//
//
//
///** range_end for a const seqan string. */
//template< typename TValue, typename TSpec >
//inline
//typename boost::range_const_iterator< seqan::String< TValue, TSpec > >::type
//range_end( const seqan::String< TValue, TSpec > & x )
//{
//	return typename boost::range_const_iterator< seqan::String< TValue, TSpec > >::type( seqan::end( x ) );
//}








/** range_begin for a mutable seqan segment. */
template< typename THost, typename TSpec >
inline
typename boost::range_mutable_iterator< seqan::Segment< THost, TSpec > >::type
range_begin( seqan::Segment< THost, TSpec > & x )
{
	return typename boost::range_mutable_iterator< seqan::Segment< THost, TSpec > >::type( seqan::begin( x ) );
}



/** range_begin for a const seqan Segment. */
template< typename THost, typename TSpec >
inline
typename boost::range_const_iterator< seqan::Segment< THost, TSpec > >::type
range_begin( const seqan::Segment< THost, TSpec > & x )
{
	return typename boost::range_const_iterator< seqan::Segment< THost, TSpec > >::type( seqan::begin( x ) );
}



/** range_end for a mutable seqan Segment. */
template< typename THost, typename TSpec >
inline
typename boost::range_mutable_iterator< seqan::Segment< THost, TSpec > >::type
range_end( seqan::Segment< THost, TSpec > & x )
{
	return typename boost::range_mutable_iterator< seqan::Segment< THost, TSpec > >::type( seqan::end( x ) );
}



/** range_end for a const seqan Segment. */
template< typename THost, typename TSpec >
inline
typename boost::range_const_iterator< seqan::Segment< THost, TSpec > >::type
range_end( const seqan::Segment< THost, TSpec > & x )
{
	return typename boost::range_const_iterator< seqan::Segment< THost, TSpec > >::type( seqan::end( x ) );
}








/** range_begin for a mutable seqan string set. */
template< typename TString, typename TSpec >
inline
typename boost::range_mutable_iterator< seqan::StringSet< TString, TSpec > >::type
range_begin( seqan::StringSet< TString, TSpec > & x )
{
	return typename boost::range_mutable_iterator< seqan::StringSet< TString, TSpec > >::type( boost::addressof( x ), 0 );
}

/** range_begin for a const seqan string set. */
template< typename TString, typename TSpec >
inline
typename boost::range_const_iterator< seqan::StringSet< TString, TSpec > >::type
range_begin( const seqan::StringSet< TString, TSpec > & x )
{
	return typename boost::range_const_iterator< seqan::StringSet< TString, TSpec > >::type( boost::addressof( x ), 0 );
}

/** range_end for a mutable seqan string set. */
template< typename TString, typename TSpec >
inline
typename boost::range_mutable_iterator< seqan::StringSet< TString, TSpec > >::type
range_end( seqan::StringSet< TString, TSpec > & x )
{
	return typename boost::range_mutable_iterator< seqan::StringSet< TString, TSpec > >::type( boost::addressof( x ), seqan::length( x ) );
}

/** range_end for a const seqan string set. */
template< typename TString, typename TSpec >
inline
typename boost::range_const_iterator< seqan::StringSet< TString, TSpec > >::type
range_end( const seqan::StringSet< TString, TSpec > & x )
{
	return typename boost::range_const_iterator< seqan::StringSet< TString, TSpec > >::type( boost::addressof( x ), seqan::length( x ) );
}











} //namespace seqan

#endif //MYRRH_SEQAN_BOOST_ADAPTORS_H_
