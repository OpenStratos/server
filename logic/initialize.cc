check_or_create("data/video", &logger);
check_or_create("data/img", &logger);

float available_disk_space = get_available_disk_space();

logger.log("Available disk space: " + to_string(available_disk_space/1073741824) + " GiB");
if (available_disk_space < (FLIGHT_LENGTH*1.25+0.5)*7549747200)
{
	logger.log("Error: Not enough disk space.");

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}

logger.log("Disk space enough for about " + to_string(available_disk_space/7549747200) +
	" hours of fullHD video.");

logger.log("Initializing WiringPi...");
wiringPiSetup();
logger.log("WiringPi initialized.");

logger.log("Initializing GPS...");
if ( ! GPS::get_instance().initialize())
{
	logger.log("GPS initialization error.");

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}
logger.log("GPS initialized.");

logger.log("Initializing GSM...");
if ( ! GSM::get_instance().initialize())
{
	logger.log("GSM initialization error.");
	logger.log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
		logger.log("GPS off.");
	else
		logger.log("Error turning GPS off.");

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}
logger.log("GSM initialized.");

logger.log("Checking batteries...");
double main_battery, gsm_battery;
if ( ! GSM::get_instance().get_battery_status(main_battery, gsm_battery) &&
	 ! GSM::get_instance().get_battery_status(main_battery, gsm_battery))
{
	logger.log("Error checking batteries.");

	logger.log("Turning GSM off...");
	if (GSM::get_instance().turn_off())
		logger.log("GSM off.");
	else
		logger.log("Error turning GSM off.");

	logger.log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
		logger.log("GPS off.");
	else
		logger.log("Error turning GPS off.");

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}

logger.log("Batteries checked => Main battery: "+ (main_battery > -1 ? to_string(main_battery*100)+"%" : "disconnected") +
	" - GSM battery: "+ to_string(gsm_battery*100) +"%");

if ((main_battery < 0.95  && main_battery > -1) || gsm_battery < 0.95)
{
	logger.log("Error: Not enough battery.");

	logger.log("Turning GSM off...");
	if (GSM::get_instance().turn_off())
		logger.log("GSM off.");
	else
		logger.log("Error turning GSM off.");

	logger.log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
		logger.log("GPS off.");
	else
		logger.log("Error turning GPS off.");

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}

logger.log("Waiting for GSM connectivity...");
while ( ! GSM::get_instance().has_connectivity())
	this_thread::sleep_for(1s);

logger.log("GSM connected.");

logger.log("Testing camera recording...");
#ifndef RASPICAM
	logger.log("Error: No raspivid found. Is this a Raspberry?");
	exit(1);
#endif
logger.log("Recording 10 seconds as test...");
if ( ! Camera::get_instance().record(10000))
{
	logger.log("Error starting recording");

	logger.log("Turning GSM off...");
	if (GSM::get_instance().turn_off())
		logger.log("GSM off.");
	else
		logger.log("Error turning GSM off.");

	logger.log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
		logger.log("GPS off.");
	else
		logger.log("Error turning GPS off.");

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}
this_thread::sleep_for(11s);
if (file_exists("data/video/test.h264"))
{
	logger.log("Camera test OK.");

	logger.log("Removing test file...");
	if (remove("data/video/test.h264"))
		logger.log("Error removing test file.");
	else
		logger.log("Test file removed.");
}
else
{
	logger.log("Test recording error.");
	logger.log("Turning GSM off...");
	if (GSM::get_instance().turn_off())
		logger.log("GSM off.");
	else
		logger.log("Error turning GSM off.");

	logger.log("Turning GPS off...");
	if (GPS::get_instance().turn_off())
		logger.log("GPS off.");
	else
		logger.log("Error turning GPS off.");

	#ifndef NO_POWER_OFF
		sync();
		reboot(RB_POWER_OFF);
	#else
		exit(1);
	#endif
}
