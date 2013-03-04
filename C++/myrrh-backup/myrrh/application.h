/**
@file

Copyright John Reid 2008

*/

#ifndef MYRRH_APPLICATION_H_
#define MYRRH_APPLICATION_H_

#ifdef _MSC_VER
# pragma once
#endif //_MSC_VER

#include "myrrh/defs.h"

#include <boost/program_options.hpp>

namespace myrrh {






/**
Parses command line arguments, handles/reports exceptions and handles control signals (on Windows).
*/
class Application
{
public:

	enum CtrlSignal
	{
		CTRL_C_SIGNAL,
		CTRL_CLOSE_SIGNAL,
		CTRL_BREAK_SIGNAL,
		CTRL_LOGOFF_SIGNAL,
		CTRL_SHUTDOWN_SIGNAL,
		CTRL_UNKNOWN_SIGNAL
	};

	Application(const char * options_name = "Program options");

	/** Override this to implement the main activity of your program. 0 is a successful return code. */
	virtual int task() = 0;

	/** Override this to implement functionality that must be done at start up. */
	virtual void init();

	/** Override this and call register_ctrl_handler on windows to call this function when ctrl signals are
	sent to the application. */
	virtual bool ctrl_handler(CtrlSignal signal);

	/** Call this to register a ctrl handler (overriden ctrl_handler()) */
	void register_ctrl_handler();

	/** Called to parse the command line. */
	bool parse_command_line( int argc, char * argv [] );

	/** Call this to run the application. */
	int main(int argc, char * argv []);

	/** Add options to the application in your constructor. */
	boost::program_options::options_description & get_options();

	/** Add positional options to the application in your constructor. */
	boost::program_options::positional_options_description & get_positional_options();

protected:
	boost::program_options::options_description options;
	boost::program_options::positional_options_description positional_options;

};






} //namespace myrrh

#endif //MYRRH_APPLICATION_H_
