/* Copyright John Reid 2008, 2010
*/



#include "myrrh/application.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include <cstdio>

namespace po = boost::program_options;

#include <iostream>
#include <exception>
using namespace std;

#ifdef _WIN32
# include <windows.h>
#else //_WIN32
# include <signal.h>
#endif



namespace myrrh {
namespace detail {


static Application * app;

#ifdef _WIN32
BOOL
CtrlHandler(DWORD fdwCtrlType)
{
	Application::CtrlSignal signal = Application::CTRL_UNKNOWN_SIGNAL;

	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
		cout << endl << "Ctrl-C" << endl << endl;
		signal = Application::CTRL_C_SIGNAL;
		break;

	case CTRL_CLOSE_EVENT:
		cout << endl << "Ctrl-CLOSE" << endl << endl;
		signal = Application::CTRL_CLOSE_SIGNAL;
		break;

	case CTRL_BREAK_EVENT:
		cout << endl << "Ctrl-BREAK" << endl << endl;
		signal = Application::CTRL_BREAK_SIGNAL;
		break;

	case CTRL_LOGOFF_EVENT:
		cout << endl << "Ctrl-LOGOFF" << endl << endl;
		signal = Application::CTRL_LOGOFF_SIGNAL;
		break;

	case CTRL_SHUTDOWN_EVENT:
		cout << endl << "Ctrl-SHUTDOWN" << endl << endl;
		signal = Application::CTRL_SHUTDOWN_SIGNAL;
		break;

	default:
		cout << endl << "Unknown control signal" << endl << endl;
		signal = Application::CTRL_UNKNOWN_SIGNAL;
		break;
	}

	return app->ctrl_handler( signal );
}
#else //_WIN32
void sighandler(int signum)
{
	printf( "Caught signal = %d\n", signum );
	switch( signum )
	{
	case SIGINT: app->ctrl_handler( Application::CTRL_BREAK_SIGNAL );
	case SIGQUIT: app->ctrl_handler( Application::CTRL_C_SIGNAL );
	case SIGABRT: app->ctrl_handler( Application::CTRL_C_SIGNAL );
	case SIGKILL: app->ctrl_handler( Application::CTRL_C_SIGNAL );
	case SIGTERM: app->ctrl_handler( Application::CTRL_CLOSE_SIGNAL );
	case SIGSTOP: app->ctrl_handler( Application::CTRL_BREAK_SIGNAL );
	default: break;
	}
}

#endif

} //namespace detail




Application::Application(const char * options_name)
: options(options_name)
{
}

bool
Application::ctrl_handler(CtrlSignal signal)
{
	return false;
}

void
Application::register_ctrl_handler()
{
	detail::app = this;
#ifdef _WIN32
	if (! SetConsoleCtrlHandler((PHANDLER_ROUTINE) detail::CtrlHandler, TRUE ))
	{
		throw std::logic_error( "ERROR: Could not set control handler" );
	}
#else //_WIN32
	if( SIG_ERR == signal( SIGINT , detail::sighandler ) ) throw std::logic_error( "ERROR: Could not set signal handler" );
	//if( SIG_ERR == signal( SIGQUIT, detail::sighandler ) ) throw std::logic_error( "ERROR: Could not set signal handler" );
	//if( SIG_ERR == signal( SIGABRT, detail::sighandler ) ) throw std::logic_error( "ERROR: Could not set signal handler" );
	//if( SIG_ERR == signal( SIGKILL, detail::sighandler ) ) throw std::logic_error( "ERROR: Could not set signal handler" );
	//if( SIG_ERR == signal( SIGTERM, detail::sighandler ) ) throw std::logic_error( "ERROR: Could not set signal handler" );
	//if( SIG_ERR == signal( SIGSTOP, detail::sighandler ) ) throw std::logic_error( "ERROR: Could not set signal handler" );
#endif
}

void
Application::init()
{
}

bool
Application::parse_command_line( int argc, char * argv [] )
{
	bool help;
	get_options().add_options()
        ("help,h", po::bool_switch(&help), "print usage message")
		;

	//parse the command line
	po::variables_map values;
	po::store(
		po::command_line_parser( argc, argv )
			.options( get_options())
			.run(),
		values);
	po::notify( values );

	if( help )
	{
		cout << get_options() << endl;
		return false;
	}

	return true;
}


int
Application::main( int argc, char * argv [] )
{
	int result = 0;

	try
	{
		init();

		if( parse_command_line( argc, argv ) ) {
			result = task();
		}
	}
	catch (const std::exception & ex)
	{
		cerr
			<< to_simple_string( boost::posix_time::second_clock::local_time() )
			<< ": Error: "
			<< ex.what()
			<< endl;
		result = -1;
	}
	catch (const string & msg)
	{
		cerr
			<< to_simple_string( boost::posix_time::second_clock::local_time() )
			<< ": Error: "
			<< msg
			<< endl;
		result = -2;
	}
	catch (const char * msg)
	{
		cerr
			<< to_simple_string( boost::posix_time::second_clock::local_time() )
			<< ": Error: "
			<< msg
			<< endl;
		result = -3;
	}
	catch (...)
	{
		cerr
			<< to_simple_string( boost::posix_time::second_clock::local_time() )
			<< ": Undefined error"
			<< endl;
		result = -4;
	}

	return result;
}



po::options_description &
Application::get_options()
{
	return options;
}

po::positional_options_description &
Application::get_positional_options()
{
	return positional_options;
}




} //namespace myrrh
