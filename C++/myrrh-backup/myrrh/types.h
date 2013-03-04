/**
@file

Copyright John Reid 2006

*/

#ifndef MYRRH_TYPES_H_
#define MYRRH_TYPES_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include <boost/multi_array.hpp>

#include <vector>





namespace myrrh {

typedef std::vector< double > double_vec; /**< A vector of doubles. */
typedef std::vector< double_vec > double_vec_vec; /**< A vector of vector of doubles. */
typedef std::vector< double_vec_vec > double_vec_vec_vec; /**< A vector of vector of vector of doubles. */

typedef std::vector< unsigned > unsigned_vec; /**< A vector of unsigned. */
typedef std::vector< unsigned_vec > unsigned_vec_vec; /**< A vector of vector of unsigned. */
typedef std::vector< unsigned_vec_vec > unsigned_vec_vec_vec; /**< A vector of vector of vector of unsigned. */

typedef boost::multi_array< double, 2 > double_array; /** A 2d array of doubles. */

typedef boost::multi_array< unsigned, 2 > unsigned_array; /** A 2d array of unsigned. */


} //namespace python

#endif //MYRRH_TYPES_H_
