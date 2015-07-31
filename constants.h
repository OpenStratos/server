#ifndef CONSTANTS_H_
	#define CONSTANTS_H_

	#define ERR_OK 0
	#define ERR_CHK 1
	#define ERR_INT 2
	#define ERR_PWD 3

	#define TEMP_VIN 5
	#define TEMP_R 500

	#define BAT_MAX 8.4
	#define BAT_MIN 7.4
	#define BAT_R1 3300
	#define BAT_R2 4700

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
	// #define GPS_ENABLE_GPIO 6
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
