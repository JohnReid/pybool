/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_TRIE_H_
#define MYRRH_TRIE_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"

#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <vector>
#include <stdexcept>

namespace myrrh {
namespace trie {


template< 
	typename Char,
	typename Value
>
struct trie
{
protected:
	typedef Char _char;
	typedef std::vector< _char > char_vector;
	typedef Value _value;

	struct node
	{
		typedef node * ptr;
		typedef boost::scoped_ptr< node > scoped_ptr;

		_char split_char;
		scoped_ptr lokid;
		scoped_ptr eqkid;
		scoped_ptr hikid;
		_value value;

		node( _char split_char, const _value & value ) : split_char( split_char ), value( value ) { }

		/** Search for the given string using recursion. */
		template< typename ForwardIterator >
		ptr
		rsearch(
			ForwardIterator s_begin, 
			ForwardIterator s_end )
		{
			if( *s_begin < split_char ) return lokid ? lokid->rsearch( s_begin, s_end ) : 0;
			if( *s_begin > split_char ) return hikid ? hikid->rsearch( s_begin, s_end ) : 0; 
			if( ++s_begin == s_end ) return this;
			return eqkid ? eqkid->rsearch( s_begin, s_end ) : 0;
		}

		/** Insert the given string into the node. */
		template< typename ForwardIterator >
		static
		ptr 
		insert(
			scoped_ptr & p,
			ForwardIterator s_begin,
			ForwardIterator s_end,
			const _value & default_value )
		{
			if( ! p ) p.reset( new node( *s_begin, default_value ) );
			if( *s_begin < p->split_char ) return insert( p->lokid, s_begin, s_end, default_value );
			if( *s_begin > p->split_char ) return insert( p->hikid, s_begin, s_end, default_value );
			if( ++s_begin == s_end ) return p.get();
			return insert( p->eqkid, s_begin, s_end, default_value ); 
		}

		/** Traverse all the prefixes in the trie. */
		template< typename OutputFn >
		OutputFn &
		traverse_prefixes(
			char_vector & prefix,
			OutputFn & output_fn )
		{
			if( lokid ) lokid->traverse_prefixes( prefix, output_fn );
			prefix.push_back( split_char );
			output_fn( this, prefix );
			if( eqkid ) eqkid->traverse_prefixes( prefix, output_fn );
			prefix.pop_back();
			if( hikid ) hikid->traverse_prefixes( prefix, output_fn );
			return output_fn;
		}
	};

public:
	typedef typename node::ptr node_ptr;

	typename node::scoped_ptr root;							/**< The root of the trie. */

	/** Search for the given string using recursion. */
	template< typename ForwardIterator >
	node_ptr
	rsearch(
		ForwardIterator s_begin, 
		ForwardIterator s_end )
	{
		if( s_begin == s_end ) throw std::logic_error( "This trie cannot store empty strings" );
		return root ? root->rsearch( s_begin, s_end ) : 0;
	}

	/** Insert the given string into the trie. */
	template< typename ForwardIterator >
	node_ptr
	insert(
		ForwardIterator s_begin, 
		ForwardIterator s_end,
		const _value & default_value = _value() )
	{
		if( s_begin == s_end ) throw std::logic_error( "This trie cannot store empty strings" );
		else return node::insert( root, s_begin, s_end, default_value );
	}

	/** Traverse all the prefixes in the trie. 
	node_ptr node;
	Range prefix;
	output_fn( node, prefix );
	must be valid.
	*/
	template< typename OutputFn >
	OutputFn &
	traverse_prefixes(
		OutputFn & output_fn )
	{
		char_vector prefix;
		if( root ) return root->traverse_prefixes( prefix, output_fn );
		else return output_fn;
	}
};



} //namespace trie
} //namespace myrrh


#endif //MYRRH_TRIE_H_

