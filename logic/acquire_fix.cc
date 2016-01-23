while ( ! GPS::get_instance().is_fixed())
	this_thread::sleep_for(1s);

logger->log("GPS fix acquired, waiting for date change.");
this_thread::sleep_for(2s);

struct timezone tz = {0, 0};
tm gps_time = GPS::get_instance().get_time();
struct timeval tv = {timegm(&gps_time), 0};
settimeofday(&tv, &tz);

logger->log("System date change.");
