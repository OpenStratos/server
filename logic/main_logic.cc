struct timeval timer;
gettimeofday(&timer, NULL);
struct tm* now = gmtime(&timer.tv_sec);

#ifdef DEBUG
	cout << "[OpenStratos] Current time: " << setfill('0') << setw(2) << now->tm_hour << ":" <<
		setfill('0') << setw(2) << now->tm_min << ":" << setfill('0') << setw(2) << now->tm_sec
		<< " UTC of " << setfill('0') << setw(2) << now->tm_mon << "/" <<
		setfill('0') << setw(2) << now->tm_mday << "/" << (now->tm_year+1900) << endl;
#endif

check_or_create("data");
State state = set_state(INITIALIZING);

check_or_create("data/logs");
check_or_create("data/logs/main");
check_or_create("data/logs/system");
check_or_create("data/logs/camera");
check_or_create("data/logs/GPS");
check_or_create("data/logs/GSM");

#ifdef DEBUG
	cout << "[OpenStratos] Starting logger..." << endl;
#endif

Logger logger("data/logs/main/OpenStratos."+ to_string(now->tm_year+1900) +"-"+
	to_string(now->tm_mon) +"-"+ to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+
	to_string(now->tm_min) +"-"+ to_string(now->tm_sec) +".log", "OpenStratos");

#ifdef DEBUG
	cout << "[OpenStratos] Logger started." << endl;
#endif

logger.log(PACKAGE_STRING);

logger.log("Starting system thread...");
thread system_thread(&system_thread_fn, ref(state));
logger.log("System thread started.");

#include "logic/initialize.cc"

logger.log("Starting battery thread...");
thread battery_thread(&battery_thread_fn, ref(state));
logger.log("Battery thread started.");

logger.log("Starting picture thread...");
thread picture_thread(&picture_thread_fn, ref(state));
logger.log("Picture thread started.");

state = set_state(ACQUIRING_FIX);
logger.log("State changed to "+ state_to_string(state) +".");

main_while(&logger, &state);

logger.log("Joining threads...");
picture_thread.join();
battery_thread.join();
system_thread.join();
logger.log("Threads joined.");

shut_down(&logger);
