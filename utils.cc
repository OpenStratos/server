#include "utils.h"

#include <fstream>
#ifdef DEBUG
	#include <iostream>
#endif

#include <unistd.h>
#include <sys/reboot.h>

#include "constants.h"

using namespace std;
using namespace os;

void os::check_or_create(const string& path, Logger* logger)
{
	if ( ! file_exists(path))
	{
		if (logger != NULL)
		{
			logger->log("No '"+path+"' directory, creating...");
		}
		#ifdef DEBUG
			else
			{
				cout << "[OpenStratos] No '"+path+"' directory, creating..." << endl;
			}
		#endif
		if (mkdir(path.c_str(), 0755) != 0)
		{
			if (logger != NULL)
			{
				logger->log("Error creating '"+path+"' directory.");
			}
			#ifdef DEBUG
				else
				{
					cout << "[OpenStratos] Error creating '"+path+"' directory." << endl;
				}
			#endif

			sync();
			reboot(RB_POWER_OFF);
		}
		else
		{
			if (logger != NULL)
			{
				logger->log("'"+path+"' directory created.");
			}
			#ifdef DEBUG
				else
				{
					cout << "[OpenStratos] '"+path+"' directory created." << endl;
				}
			#endif
		}
	}
}

State os::set_state(State new_state)
{
	ofstream state_file(STATE_FILE);
	state_file << state_to_string(new_state);
	state_file.close();

	return new_state;
}

State os::get_last_state()
{
	ifstream state_file(STATE_FILE);
	string state_str((istreambuf_iterator<char>(state_file)),
				 istreambuf_iterator<char>());
	state_file.close();

	if (state_str == "INITIALIZING")
	{
		return INITIALIZING;
	}
	if (state_str == "ACQUIRING_FIX")
	{
		return ACQUIRING_FIX;
	}
	if (state_str == "FIX_ACQUIRED")
	{
		return FIX_ACQUIRED;
	}
	if (state_str == "WAITING_LAUNCH")
	{
		return WAITING_LAUNCH;
	}
	if (state_str == "GOING_UP")
	{
		return GOING_UP;
	}
	if (state_str == "GOING_DOWN")
	{
		return GOING_DOWN;
	}
	if (state_str == "LANDED")
	{
		return LANDED;
	}
	if (state_str == "SHUT_DOWN")
	{
		return SHUT_DOWN;
	}

	return SAFE_MODE;
}

const string os::state_to_string(State state)
{
	switch (state)
	{
		case INITIALIZING:
			return "INITIALIZING";
		break;
		case ACQUIRING_FIX:
			return "ACQUIRING_FIX";
		break;
		case FIX_ACQUIRED:
			return "FIX_ACQUIRED";
		break;
		case WAITING_LAUNCH:
			return "WAITING_LAUNCH";
		break;
		case GOING_UP:
			return "GOING_UP";
		break;
		case GOING_DOWN:
			return "GOING_DOWN";
		break;
		case LANDED:
			return "LANDED";
		break;
		case SHUT_DOWN:
			return "SHUT_DOWN";
		break;
		case SAFE_MODE:
			return "SAFE_MODE";
	}
	return "";
}
