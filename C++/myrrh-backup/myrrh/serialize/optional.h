/**
@file

Copyright John Reid 2008

*/

#ifndef MYRRH_SERIALISE_OPTIONAL_H_
#define MYRRH_SERIALISE_OPTIONAL_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

namespace boost {
namespace serialization {



/** Save a optional. */
template<
	typename Archive, 
	typename T
>
inline void save(
	Archive & ar,
	const boost::optional< T > & o,
	const unsigned int /* file_version */
) {
	bool has_value = o;
	ar & has_value;
	if( has_value ) {
		ar & o.get();
	}
}



/** Load a optional. */
template<
	typename Archive, 
	typename T
>
inline void load(
	Archive & ar,
	boost::optional< T > & o,
	const unsigned int /* file_version */
) {
	bool has_value;
	ar & has_value;
	if( has_value ) {
		o = T();
		ar & o.get();
	} else {
		o = boost::optional< T >();
	}
}


/** Split non-intrusive serialize into save/load. */
template<
	typename Archive, 
	typename T
>
inline void serialize(
	Archive & ar,
	boost::optional< T > & o,
	const unsigned int file_version
){
   boost::serialization::split_free( ar, o, file_version );
} 


} // namespace serialization
} // namespace boost



#endif //MYRRH_SERIALISE_OPTIONAL_H_

