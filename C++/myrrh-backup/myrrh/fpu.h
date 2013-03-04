/**
@file

Copyright John Reid 2007

*/

#ifndef MYRRH_FPU_H_
#define MYRRH_FPU_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"

#ifdef WIN32
#include <float.h>
#endif

namespace myrrh {



inline
std::string fpu_control_word_summary()
{
	unsigned status = _controlfp(0, 0);
	return MYRRH_MAKE_STRING(
		"floating-point control word:      " << status << "\r\n"

		"Denormal control:    save:        " << (status & _DN_SAVE ? "on" : "off") << "\r\n"
		"Denormal control:    flush:       " << (status & _DN_FLUSH ? "on" : "off") << "\r\n"
		
		"Interrupt exception: invalid:     " << (status & _EM_INVALID ? "on" : "off") << "\r\n"
		"Interrupt exception: denormal:    " << (status & _EM_DENORMAL ? "on" : "off") << "\r\n"
		"Interrupt exception: zero divide: " << (status & _EM_ZERODIVIDE ? "on" : "off") << "\r\n"
		"Interrupt exception: overflow:    " << (status & _EM_OVERFLOW ? "on" : "off") << "\r\n"
		"Interrupt exception: underflow:   " << (status & _EM_UNDERFLOW ? "on" : "off") << "\r\n"
		"Interrupt exception: inexact:     " << (status & _EM_INEXACT ? "on" : "off") << "\r\n"
		
		"Infinity control:    affine:      " << (status & _IC_AFFINE ? "on" : "off") << "\r\n"
		"Infinity control:    projective:  " << (status & _IC_PROJECTIVE ? "on" : "off") << "\r\n"
		
		"Rounding control:    chop:        " << (status & _RC_CHOP ? "on" : "off") << "\r\n"
		"Rounding control:    up:          " << (status & _RC_UP ? "on" : "off") << "\r\n"
		"Rounding control:    down:        " << (status & _RC_DOWN ? "on" : "off") << "\r\n"
		"Rounding control:    near:        " << (status & _RC_NEAR ? "on" : "off") << "\r\n"
		
		"Precision control:   24:          " << (status & _PC_24 ? "on" : "off") << "\r\n"
		"Precision control:   53:          " << (status & _PC_53 ? "on" : "off") << "\r\n"
		"Precision control:   64:          " << (status & _PC_64 ? "on" : "off") << "\r\n"
		);
}

} //namespace myrrh


#endif //MYRRH_FPU_H_

