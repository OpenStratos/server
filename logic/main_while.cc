#include "logic/logic.h"

void os::main_while(Logger* logger, State* state)
{
	double launch_altitude = 0;

	while (*state != SHUT_DOWN)
	{
		if (*state == ACQUIRING_FIX)
		{
			while ( ! GPS::get_instance().is_fixed())
			{
				this_thread::sleep_for(1s);
			}
			*state = set_state(FIX_ACQUIRED);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == FIX_ACQUIRED)
		{
			logger->log("GPS fix acquired, waiting 10 seconds for stabilization and date change.");
			this_thread::sleep_for(10s);

			#include "logic/start_recording.cc"
			#include "logic/send_init_sms.cc"
			*state = set_state(WAITING_LAUNCH);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == WAITING_LAUNCH)
		{
			#include "logic/wait_launch.cc"
			*state = set_state(GOING_UP);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == GOING_UP)
		{
			#include "logic/go_up.cc"
		}
		else if (*state == GOING_DOWN)
		{
			#include "logic/go_down.cc"
			*state = set_state(LANDED);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else if (*state == LANDED)
		{
			#include "logic/land.cc"
			*state = set_state(SHUT_DOWN);
			logger->log("State changed to "+ state_to_string(*state) +".");
		}
		else
		{
			#ifndef NO_POWER_OFF
				sync();
				reboot(RB_AUTOBOOT);
			#else
				exit(1);
			#endif
		}
	}
}
