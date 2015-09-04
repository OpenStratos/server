#ifndef CONSTANTS_H_
	#define CONSTANTS_H_

	#define DEBUG true

	#define FLIGHT_LENGTH 4.222 // Hours

	#define BAT_GSM_MAX 4.2
	#define BAT_GSM_MIN 3.7
	#define BAT_MAIN_MAX 8.4*2660/(2660+7420) // Measured Ohms in voltage divider
	#define BAT_MAIN_MIN 7.4*BAT_MAIN_MAX/8.4

	#define VIDEO_WIDTH 1920
	#define VIDEO_HEIGHT 1080
	#define VIDEO_BITRATE 17 //Mbps
	#define VIDEO_FPS 30
	#define VIDEO_CONTRAST 50
	#define VIDEO_BRIGHTNESS 50
	#define VIDEO_EXPOSURE "antishake"

	#define PHOTO_WIDTH 2592
	#define PHOTO_HEIGHT 1944
	#define PHOTO_QUALITY 90
	#define PHOTO_RAW true
	#define PHOTO_CONTRAST 50
	#define PHOTO_BRIGHTNESS 50
	#define PHOTO_EXPOSURE "antishake"

	#define GPS_UART "/dev/ttyAMA0"
	#define GPS_ENABLE_GPIO 2
	#define GPS_BAUDRATE 9600
	#define GPS_ENDL "\r\n"

	#define GSM_LOC_SERV "gprs-service.com"
	#define GSM_UART "/dev/ttyUSB0"
	#define GSM_PWR_GPIO 7
	#define GSM_STATUS_GPIO 21
	#define GSM_BAUDRATE 9600
	#define GSM_ENDL "\r\n"

	#define SMS_PHONE ""

	#define STATE_FILE "data/last_state.txt"
#endif // CONSTANTS_H_
