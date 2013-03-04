/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_DEFS_H_
#define MYRRH_DEFS_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include <boost/test/utils/wrap_stringstream.hpp>
#define MYRRH_MAKE_STRING( x ) ( boost::wrap_stringstream().ref() << x ).str()


namespace myrrh {







} //namespace myrrh

#endif //MYRRH_DEFS_H_

