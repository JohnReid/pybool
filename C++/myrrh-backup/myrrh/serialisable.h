/**
@file

Copyright John Reid 2008

*/

#ifndef MYRRH_SERIALISABLE_H_
#define MYRRH_SERIALISABLE_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/timer.hpp>

namespace myrrh {




namespace detail {

/** Metafunction to choose archive type. */
template< bool binary >
struct archive {
	typedef boost::archive::binary_iarchive iarchive_t;
	typedef boost::archive::binary_oarchive oarchive_t;
	static std::ios_base::openmode get_open_mode() { return std::ios::binary; }
};

/** Specialisation of metafunction to choose archive type for text archives. */
template<>
struct archive< false > {
	typedef boost::archive::text_iarchive iarchive_t;
	typedef boost::archive::text_oarchive oarchive_t;
	static std::ios_base::openmode get_open_mode() { return std::ios_base::openmode( 0 ); }
};

} //namespace detail




/** Serialise object t to the given file. If binary is true uses a binary binary archive format. */
template< bool binary, typename T >
void
serialise(
	const T & t,
	const boost::filesystem::path & archive_filename
) {
	typedef detail::archive< binary > archive_t;
	boost::filesystem::ofstream stream( archive_filename, archive_t::get_open_mode() );
	typename archive_t::oarchive_t( stream ) << t;
}



/** Deserialises object t from the given file. If binary is true uses a binary archive format. */
template< bool binary, typename T >
void
deserialise(
	T & t,
	const boost::filesystem::path & archive_filename
) {
	typedef detail::archive< binary > archive_t;
	boost::filesystem::ifstream stream( archive_filename, archive_t::get_open_mode() );
	typename archive_t::iarchive_t( stream ) >> t;
}


/** Deserialise object from the file into a shared pointer. */
template< bool binary, typename T >
boost::shared_ptr< T >
deserialise_into_shared_ptr(
	const boost::filesystem::path & archive_filename
) {
	boost::shared_ptr< T > result(new T);
	deserialise< binary, T >( *result, archive_filename );
	return result;
}



/**
Try to deserialise from disk. If not print error and return false.
*/
template<
	bool binary,				// binary serialisation format?
	typename T					// the type of object we are serialising
>
bool 
try_to_deserialise(
	T & t,
	const boost::filesystem::path & archive_filename,
	std::ostream & print_stream = std::cout
) {
	//try to deserialise
	bool deserialised = false;
	try
	{
		if( ! boost::filesystem::exists( archive_filename ) )
		{
			print_stream << "Cannot deserialise: \"" << archive_filename.native_file_string() << "\" does not exist\n";
		}
		else
		{
			boost::timer timer;
			myrrh::deserialise< binary, T >( t, archive_filename );
			print_stream << "Deserialised \"" << archive_filename.native_file_string() << "\" - " << timer.elapsed() << "s\n";
			deserialised = true;
		}
	}
	catch( const std::exception & exception )
	{
		print_stream
			<< "Could not deserialise \""
			<< archive_filename.native_file_string()
			<< "\" : " << exception.what() << "\n";
	}
	catch(...)
	{
		print_stream
			<< "Could not deserialise \""
			<< archive_filename.native_file_string()
			<< "\" : unknown error\n";
	}

	return deserialised;
}


/**
Deserialise from disk. If not possible initialise using Init functor. Returns true iff was deserialised rather than initialised.
*/
template<
	bool binary,				// binary serialisation format?
	typename T,					// the type of object we are serialising
	typename Init				// function object to initialise object if deserialising fails
>
bool
deserialise_or_init(
	T & t,
	const boost::filesystem::path & archive_filename,
	Init init,
	std::ostream & print_stream = std::cout
) {
	bool deserialised = try_to_deserialise< binary >( t, archive_filename );
	if( ! deserialised )
	{
		boost::timer timer;

		//if we couldn't deserialise, construct from scratch
		init( t );
		print_stream << "Initialised \"" << archive_filename.native_file_string() << "\" : " << timer.elapsed() << "s\n";

		//save for next time
		timer.restart();
		myrrh::serialise< binary, T >( t, archive_filename );
		print_stream << "Serialised \"" << archive_filename.native_file_string() << "\" : " << timer.elapsed() << "s\n";
	}

	return deserialised;
}




} //namespace myrrh

#endif //MYRRH_SERIALISABLE_H_

