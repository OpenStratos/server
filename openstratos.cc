#include "openstratos.h"

int main(void)
{
	#ifdef DEBUG
		#ifdef SIM
			cout << "[OpenStratos] Simulation." << endl;
		#endif

		#ifdef REAL_SIM
			cout << "[OpenStratos] Realistic simulation." << endl;
		#endif

		cout << "[OpenStratos] Starting " << PACKAGE_STRING << "..." << endl;
	#endif

	if ( ! file_exists(STATE_FILE))
	{
		#ifdef DEBUG
			cout << "[OpenStratos] No state file. Starting main logic..." << endl;
		#endif
		#include "logic/main_logic.cc"
	}
	else
	{
		#ifdef DEBUG
			cout << "[OpenStratos] State file found. Starting safe mode..." << endl;
		#endif
		#include "logic/safe_mode.cc"
	}

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#endif

	return 0;
}
