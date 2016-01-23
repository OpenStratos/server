#include "threads.h"

#include <cstdlib>

#include <vector>
#include <sstream>
#include <string>
#include <thread>

#include <sys/time.h>
#include <sys/sysinfo.h>

#include "logger/Logger.h"
#include "camera/Camera.h"
#include "gsm/GSM.h"

using namespace std;
using namespace os;

void os::system_thread_fn(State& state)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	Logger cpu_logger("data/logs/system/CPU."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon+1) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "CPU");

	Logger ram_logger("data/logs/system/RAM."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon+1) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "RAM");

	Logger temp_logger("data/logs/system/Temp."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon+1) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "Temp");

	FILE *gpu_temp_process, *cpu_command_process;
	char gpu_response[11];
	char cpu_command[100];
	struct sysinfo info;

	while (state != SHUT_DOWN)
	{
		if (get_available_disk_space() < 2000000000)
			Camera::get_instance().stop();

		ifstream cpu_temp_file("/sys/class/thermal/thermal_zone0/temp");
		string cpu_temp_str((istreambuf_iterator<char>(cpu_temp_file)),
			istreambuf_iterator<char>());
		cpu_temp_file.close();

		gpu_temp_process = popen("/opt/vc/bin/vcgencmd measure_temp", "r");
		fgets(gpu_response, 11, gpu_temp_process);
		pclose(gpu_temp_process);

		temp_logger.log("CPU: "+to_string(stoi(cpu_temp_str)/1000.0)+" GPU: "+
			string(gpu_response).substr(5, 4));

		cpu_command_process = popen("grep 'cpu ' /proc/stat", "r");
		fgets(cpu_command, 100, cpu_command_process);
		pclose(cpu_command_process);

		const string cpu_command_str = string(cpu_command);
		stringstream ss(cpu_command_str);
		string data;
		vector<string> s_data;

		// We put all fields in a vector
		while(getline(ss, data, ' ')) s_data.push_back(data);

		// Note that s_data[1] is ""
		cpu_logger.log(to_string((stof(s_data[2])+stof(s_data[4]))/
			(stof(s_data[2])+stof(s_data[4])+stof(s_data[5]))));

		sysinfo(&info);
		ram_logger.log(to_string(((double) info.freeram)/info.totalram));

		this_thread::sleep_for(30s);
	}
}

void os::picture_thread_fn(State& state)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	Logger logger("data/logs/camera/Pictures."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "Pictures");

	logger.log("Waiting for launch...");

	while (state != GOING_UP)
	{
		this_thread::sleep_for(10s);
	}

	logger.log("Launched, waiting 2 minutes for first picture...");
	this_thread::sleep_for(2min);

	while (state == GOING_UP)
	{
		logger.log("Taking picture...");

		if ( ! Camera::get_instance().take_picture(generate_exif_data()))
		{
			logger.log("Error taking picture. Trying again in 30 seconds...");
		}
		else
		{
			logger.log("Picture taken correctly. Next picture in 30 seconds...");
		}

		this_thread::sleep_for(30s);
		logger.log("Taking picture...");

		if ( ! Camera::get_instance().take_picture(generate_exif_data()))
		{
			logger.log("Error taking picture. Next picture in 4 minutes...");
		}
		else
		{
			logger.log("Picture taken correctly. Next picture in 4 minutes...");
		}

		this_thread::sleep_for(4min);
	}
	logger.log("Going down, no more pictures are being taken, picture thread is closing.");
}

void os::battery_thread_fn(State& state)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	struct tm * now = gmtime(&timer.tv_sec);

	Logger logger("data/logs/GSM/Battery."+ to_string(now->tm_year+1900) +"-"+ to_string(now->tm_mon) +"-"+
		to_string(now->tm_mday) +"."+ to_string(now->tm_hour) +"-"+ to_string(now->tm_min) +"-"+
		to_string(now->tm_sec) +".log", "Battery");

	double main_battery, gsm_battery;

	while (state != SHUT_DOWN)
	{
		if (GSM::get_instance().get_status())
		{
			GSM::get_instance().get_battery_status(main_battery, gsm_battery);
			logger.log("Main: "+ to_string(main_battery));
			logger.log("GSM: "+ to_string(gsm_battery));
		}
		else
		{
			this_thread::sleep_for(15min);
			GSM::get_instance().turn_on();

			GSM::get_instance().get_battery_status(main_battery, gsm_battery);
			logger.log("Main: "+ to_string(main_battery));
			logger.log("GSM: "+ to_string(gsm_battery));

			GSM::get_instance().turn_off();
		}

		this_thread::sleep_for(3min);
	}
}
