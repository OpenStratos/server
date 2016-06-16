#ifndef UTILS_H_
#define UTILS_H_

#include <cmath>

#include <string>
#include <thread>
#if defined SIM || defined REAL_SIM
	#include <chrono>
#endif

#include <sys/stat.h>
#include <sys/statvfs.h>

#include "constants.h"
#include "logger/Logger.h"
#include "gps/GPS.h"

namespace os {
	enum State {
		INITIALIZING,
		ACQUIRING_FIX,
		FIX_ACQUIRED,
		WAITING_LAUNCH,
		GOING_UP,
		GOING_DOWN,
		LANDED,
		SHUT_DOWN,
		SAFE_MODE,
	};

	void check_or_create(const string& path, Logger* logger = NULL);

	inline bool file_exists(const string& name)
	{
		struct stat buffer;
		return stat(name.c_str(), &buffer) == 0;
	}

	inline float get_available_disk_space()
	{
		struct statvfs fs;
		statvfs("data", &fs);

		return ((float) fs.f_bsize)*fs.f_bavail;
	}

	State set_state(State new_state);
	State get_last_state();
	const string state_to_string(State state);

	inline State get_real_state()
	{
		for (int i = 0; i < 10 && ( ! GPS::get_instance().is_fixed() ||
						GPS::get_instance().get_VDOP() > MAX_DOP); ++i)
		{
			this_thread::sleep_for(1s);
		}

		double start_alt = GPS::get_instance().get_altitude();
		this_thread::sleep_for(5s);
		double end_alt = GPS::get_instance().get_altitude();

		if (end_alt - start_alt < -10)
		{
			return set_state(GOING_DOWN);
		}
		else if (end_alt - start_alt > 5)
		{
			return set_state(GOING_UP);
		}
		else if (end_alt > 1500)
		{
			return set_state(GOING_DOWN);
		}
		else
		{
			return set_state(LANDED);
		}
	}

	inline bool has_launched(double launch_altitude)
	{
		for (int i = 0; i < 10 && ( ! GPS::get_instance().is_fixed() ||
						GPS::get_instance().get_VDOP() > MAX_DOP); ++i)
		{
			this_thread::sleep_for(500ms);
		}

		if ( ! GPS::get_instance().is_fixed())
		{
			return false;
		}

		double first_altitude = GPS::get_instance().get_altitude();
		if (first_altitude > launch_altitude + 25) return true;

		this_thread::sleep_for(5s);

		#if defined SIM || defined REAL_SIM
			return true;
		#else
			double second_altitude = GPS::get_instance().get_altitude();
			return second_altitude > first_altitude + 8;
		#endif
	}

	inline bool has_bursted(double maximum_altitude)
	{
		for (int i = 0; i < 10 && ( ! GPS::get_instance().is_fixed() ||
						GPS::get_instance().get_VDOP() > MAX_DOP); ++i)
		{
			this_thread::sleep_for(500ms);
		}

		if ( ! GPS::get_instance().is_fixed())
		{
			return false;
		}

		double first_altitude = GPS::get_instance().get_altitude();
		if (first_altitude < maximum_altitude - 1000) return true;

		this_thread::sleep_for(6s);

		#if defined SIM || defined REAL_SIM
			return true;
		#else
			double second_altitude = GPS::get_instance().get_altitude();
			return second_altitude < first_altitude - 10;
		#endif
	}

	inline bool has_landed()
	{
		for (int i = 0; i < 10 && ( ! GPS::get_instance().is_fixed() ||
						GPS::get_instance().get_VDOP() > MAX_DOP); ++i)
		{
			this_thread::sleep_for(500ms);
		}

		if ( ! GPS::get_instance().is_fixed()) return false;

		double first_altitude = GPS::get_instance().get_altitude();
		this_thread::sleep_for(5s);
		double second_altitude = GPS::get_instance().get_altitude();

		return abs(first_altitude-second_altitude) < 5;
	}

	inline bool wait_up_for(double altitude, double& maximum_altitude)
	{
		#if defined SIM && !defined REAL_SIM
			this_thread::sleep_for(2min);

			if (altitude > FLIGHT_MAX_HEIGHT)
			{
				maximum_altitude = FLIGHT_MAX_HEIGHT;
				return true;
			}
			else
			{
				maximum_altitude = altitude;
				return false;
			}
		#elif defined REAL_SIM && !defined SIM
			if (altitude > FLIGHT_MAX_HEIGHT)
			{
				double sleep = (FLIGHT_MAX_HEIGHT - maximum_altitude)/ASCENT_VELOCITY;
				this_thread::sleep_for(std::chrono::duration<double>(sleep));
				maximum_altitude = FLIGHT_MAX_HEIGHT;
				return true;
			}
			else
			{
				double sleep = (altitude - maximum_altitude)/ASCENT_VELOCITY;
				this_thread::sleep_for(std::chrono::duration<double>(sleep));
				maximum_altitude = altitude;
				return false;
			}
		#else
			bool bursted;
			while ( ! (bursted = has_bursted(maximum_altitude)))
			{
				for (int i = 0; i < 10 && ( ! GPS::get_instance().is_fixed() ||
								GPS::get_instance().get_VDOP() > MAX_DOP); ++i)
				{
					this_thread::sleep_for(500ms);
				}

				double current_altitude = GPS::get_instance().get_altitude();
				if (current_altitude > maximum_altitude)
				{
					maximum_altitude = current_altitude;
				}

				if (current_altitude >= altitude)
				{
					break;
				}
			}
			return bursted;
		#endif
	}

	inline bool wait_down_for(double altitude) {
		#if defined SIM || defined REAL_SIM
			if (altitude > FLIGHT_MAX_HEIGHT)
			{
				return false;
			}
		#endif
		#if defined SIM && !defined REAL_SIM
			this_thread::sleep_for(1min);
			return false;
		#elif defined REAL_SIM && !defined SIM
			static double last_altitude;

			if (last_altitude > FLIGHT_MAX_HEIGHT) {
				double sleep = (FLIGHT_MAX_HEIGHT-altitude)/DESCENT_VELOCITY;
				this_thread::sleep_for(std::chrono::duration<double>(sleep));
			} else {
				double sleep = (last_altitude-altitude)/DESCENT_VELOCITY;
				this_thread::sleep_for(std::chrono::duration<double>(sleep));
			}

			last_altitude = altitude;
			return false;
		#else
		for (int i = 0; i < 10 && ( ! GPS::get_instance().is_fixed() ||
						GPS::get_instance().get_VDOP() > MAX_DOP); ++i)
			{
				this_thread::sleep_for(500ms);
			}

			while (GPS::get_instance().get_altitude() > altitude &&
					! has_landed())
			{
				this_thread::sleep_for(3s);
			}

			return has_landed();
		#endif
	}
}

#endif // UTILS_H_
