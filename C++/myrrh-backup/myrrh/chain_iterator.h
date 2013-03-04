/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_CHAIN_ITERATOR_H_
#define MYRRH_CHAIN_ITERATOR_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range.hpp>

namespace myrrh {

template< typename HeadIterator, typename TailIterator >
struct chain_iterator 
	: boost::iterator_facade< 
		chain_iterator< HeadIterator, TailIterator >,
		typename std::iterator_traits< HeadIterator >::value_type,
		typename std::iterator_traits< HeadIterator >::iterator_category,
		typename std::iterator_traits< HeadIterator >::reference,
		typename std::iterator_traits< HeadIterator >::difference_type >
{
private:
	typedef HeadIterator head_iterator;
	typedef TailIterator tail_iterator;
	typedef chain_iterator< head_iterator, tail_iterator > self_t;

public:
	typedef typename std::iterator_traits< HeadIterator >::reference reference;
	typedef typename std::iterator_traits< HeadIterator >::difference_type difference_type;

private:
	head_iterator head;
	tail_iterator tail;
	head_iterator head_end;
	tail_iterator tail_begin;
	bool in_first;

public:
	chain_iterator(
		head_iterator head_end, 
		tail_iterator tail_begin, 
		head_iterator head = head_iterator(), 
		tail_iterator tail = tail_iterator(), 
		bool in_first = true )
		: head( head )
		, tail( tail )
		, head_end( head_end )
		, tail_begin( tail_begin )
		, in_first( in_first )
	{
	}

private:
    friend class boost::iterator_core_access;

    void increment() {
		if( in_first ) {
			++head;
			if( head == head_end ) tail = tail_begin, in_first = false;
		} else {
			++tail;
		}
	}
    void decrement() { 
		if( in_first ) {
			--head;
		} else {
			if( tail == tail_begin ) in_first = true, head = boost::prior( head_end );
			else --tail;
		}
	}

    bool equal( self_t const& other ) const { 
		if( this->in_first != other.in_first ) return false;
		if( in_first ) return this->head == other.head;
		else return this->tail == other.tail;
	}

    reference dereference() const { 
		return in_first ? *head : *tail;
	}

	void advance( int n ) {
		if( in_first ) {
			const int to_head_end = head_end - head;
			if( n >= to_head_end ) {
				in_first = false;
				tail = boost::next( tail_begin, n - to_head_end );
			} else {
				head = boost::next( head, n );
			}
		} else {
			const int to_tail_begin = tail_begin - tail;
			if( to_tail_begin > n ) {
				in_first = true;
				head = boost::next( head_end, n - to_tail_begin );
			} else {
				tail = boost::next( tail, n );
			}
		}
	}

	difference_type distance_to(self_t const& other) const
	{
		if( this->in_first ) {
			if( other.in_first ) return std::distance( this->head, other.head );
			else return std::distance( this->head, head_end ) + std::distance( other.tail_begin, other.tail );
		} else {
			if( ! other.in_first ) return std::distance( this->tail, other.tail );
			else return std::distance( this->tail, tail_begin ) + std::distance( other.head_end, other.head );
		}
	}
};

template< typename Range1, typename Range2 >
chain_iterator< 
	typename boost::range_const_iterator< Range1 >::type,
	typename boost::range_const_iterator< Range2 >::type 
>
make_chain_iterator_begin( const Range1 & range_1, const Range2 & range_2 )
{
	using namespace boost;
	typedef chain_iterator< 
		typename boost::range_const_iterator< Range1 >::type,
		typename boost::range_const_iterator< Range2 >::type 
	> iterator;
	return iterator( 
		end( range_1 ), 
		begin( range_2 ), 
		begin( range_1 ),
		end( range_2 ),
		true
	);
}

template< typename Range1, typename Range2 >
chain_iterator< 
	typename boost::range_const_iterator< Range1 >::type,
	typename boost::range_const_iterator< Range2 >::type 
>
make_chain_iterator_end( const Range1 & range_1, const Range2 & range_2 )
{
	using namespace boost;
	typedef chain_iterator< 
		typename boost::range_const_iterator< Range1 >::type,
		typename boost::range_const_iterator< Range2 >::type 
	> iterator;
	return iterator( 
		end( range_1 ), 
		begin( range_2 ), 
		begin( range_1 ),
		end( range_2 ),
		false
	);
}

template< typename Range1, typename Range2 >
chain_iterator< 
	typename boost::range_const_iterator< Range1 >::type,
	typename boost::range_const_iterator< Range2 >::type 
>
make_chain_iterator_range( const Range1 & range_1, const Range2 & range_2 )
{
	using namespace boost;
	return make_iterator_range( 
		make_chain_iterator_begin( range_1, range_2 ),
		make_chain_iterator_end( range_1, range_2 ) );
}


} //namespace myrrh


#endif //MYRRH_CHAIN_ITERATOR_H_

