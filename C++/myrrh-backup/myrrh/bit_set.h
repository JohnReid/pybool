/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_BIT_SET_H_
#define MYRRH_BIT_SET_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"

#include <boost/scoped_array.hpp>
#include <boost/static_assert.hpp>

namespace myrrh {

namespace detail {
	template< size_t size >
	struct word_size_traits {
		//BOOST_STATIC_ASSERT( size || (! "Only specialisations should be instantiated") );
	};

	template< typename Traits >
	struct word_size_traits_base {
		static size_t num_words_for_num_bits( size_t num_bits ) {
			if( 0 == num_bits ) return 0;
			else return ((num_bits - 1) / Traits::bits_in_word) + 1;
		}
		static size_t word_idx( size_t bit ) { return bit >> Traits::num_shifts; }
		static size_t bit_idx( size_t bit ) { return bit & Traits::index_word; }
	};

#define MYRRH_BIT_SET_SPECIALISE_NUM_SHIFTS(s, v) \
	template<> struct word_size_traits<s> : word_size_traits_base< word_size_traits<s> > { \
		static const size_t size = s; \
		static const size_t bits_in_word = 8 * s; \
		static const size_t num_shifts = 3 + v; \
		static const size_t index_word = (1 << (3 + v)) - 1; \
	}; \
	const size_t word_size_traits< s >::size; \
	const size_t word_size_traits< s >::bits_in_word; \
	const size_t word_size_traits< s >::num_shifts; \
	const size_t word_size_traits< s >::index_word;

	MYRRH_BIT_SET_SPECIALISE_NUM_SHIFTS(  1, 0);
	MYRRH_BIT_SET_SPECIALISE_NUM_SHIFTS(  2, 1);
	MYRRH_BIT_SET_SPECIALISE_NUM_SHIFTS(  4, 2);
	MYRRH_BIT_SET_SPECIALISE_NUM_SHIFTS(  8, 3);
	MYRRH_BIT_SET_SPECIALISE_NUM_SHIFTS( 16, 4);
	MYRRH_BIT_SET_SPECIALISE_NUM_SHIFTS( 32, 5);
	MYRRH_BIT_SET_SPECIALISE_NUM_SHIFTS( 64, 6);
} //namespace detail


/**
A space efficient bit_set class - much more so than boost::dynamic_bitset
*/
template< typename StorageT = unsigned >
class bit_set
{
public:
	typedef bit_set< StorageT > self_t;
	typedef std::size_t size_t;
	typedef StorageT storage_t;
	typedef detail::word_size_traits< sizeof( storage_t ) > wst;
	typedef boost::scoped_array< storage_t > storage_array;

private:
	storage_array _storage;
	size_t _num_bits;

public:
	bit_set( size_t num_bits ) {
		const unsigned num_words = allocate_storage( num_bits );
		std::fill( _storage.get(), _storage.get() + num_words, storage_t( 0 ) ); //initialise to 0
	}
	bit_set( const bit_set< storage_t > & rhs ) {
		const unsigned num_words = allocate_storage( rhs._num_bits );
		std::copy( rhs._storage.get(), rhs._storage.get() + num_words, _storage.get() );
	}

	void set( size_t bit ) {
		const size_t w_idx = word_idx(bit);
		const size_t b_idx = bit_idx(bit);
		const storage_t mask = storage_t( 1 ) << b_idx;
		_storage[ w_idx ] |= mask;
	}
	void unset( size_t bit ) {
		const size_t w_idx = word_idx(bit);
		const size_t b_idx = bit_idx(bit);
		const storage_t mask = ~(storage_t( 1 ) << b_idx);
		_storage[ w_idx ] &= mask;
	}
	storage_t get( size_t bit ) const {
		const size_t w_idx = word_idx(bit);
		const size_t b_idx = bit_idx(bit);
		const storage_t mask = storage_t( 1 ) << b_idx;
		return _storage[ w_idx ] & mask;
	}
	self_t & operator|=( const self_t & rhs ) {
		size_t num_words = wst::num_words_for_num_bits( _num_bits );
		while( num_words ) {
			--num_words;
			_storage[num_words] |= rhs._storage[num_words];
		}
		return *this;
	}

	/** The total number of bits set. */
	size_t total_set() const {
		size_t total = 0;
		for(size_t i = 0; i != _num_bits; ++i ) {
			if( get( i ) ) {
				total += 1;
			}
		}
		return total;
	}

private:
	unsigned allocate_storage( size_t num_bits ) {
		_num_bits = num_bits;
		const size_t num_words = wst::num_words_for_num_bits( num_bits );
		_storage.reset( new storage_t[ num_words ] );
		return num_words;
	}

	static size_t word_idx( size_t bit ) { return wst::word_idx( bit ); }
	static size_t bit_idx( size_t bit ) { return wst::bit_idx( bit ); }

};


} //namespace myrrh


#endif //MYRRH_BIT_SET_H_

