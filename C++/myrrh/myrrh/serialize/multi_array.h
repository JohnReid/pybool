/**
@file

Copyright John Reid 2008

*/

#ifndef MYRRH_SERIALISE_MULTI_ARRAY_H_
#define MYRRH_SERIALISE_MULTI_ARRAY_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

namespace boost {
namespace serialization {



/** Save a multi_array. */
template<
	typename Archive, 
	typename T, 
	std::size_t NumDims,
	typename Allocator
>
inline void save(
	Archive & ar,
	const boost::multi_array< T, NumDims, Allocator > & a,
	const unsigned int /* file_version */
) {
	//ar & a.num_elements();
	for( std::size_t d = 0; NumDims != d; ++d ) {
		ar & a.shape()[d];
	}
	for( std::size_t i = 0; a.num_elements() != i; ++i ) {
		ar & a.data()[i];
	}
}



/** Load a multi_array. */
template<
	typename Archive, 
	typename T, 
	std::size_t NumDims,
	typename Allocator
>
inline void load(
	Archive & ar,
	boost::multi_array< T, NumDims, Allocator > & a,
	const unsigned int /* file_version */
) {
	//std::size_t num_elements;
	//ar & num_elements;

	std::vector< std::size_t > shape;
	for( std::size_t d = 0; NumDims != d; ++d ) {
		std::size_t size;
		ar & size;
		shape.push_back( size );
	}
	a.resize( shape );
	for( std::size_t i = 0; a.num_elements() != i; ++i ) {
		ar & a.data()[i];
	}
}


/** Split non-intrusive serialize into save/load. */
template<
	typename Archive, 
	typename T, 
	std::size_t NumDims,
	typename Allocator
>
inline void serialize(
	Archive & ar,
	boost::multi_array< T, NumDims, Allocator > & a,
	const unsigned int file_version
) {
   boost::serialization::split_free( ar, a, file_version );
} 


} // namespace serialization
} // namespace boost



#endif //MYRRH_SERIALISE_MULTI_ARRAY_H_

