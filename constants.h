#ifndef CONSTANTS_H_
	#define CONSTANTS_H_

	#define FLIGHT_LENGTH 4 // Hours
	#define FLIGHT_MAX_HEIGHT 34000 // Meters
	#define ASCENT_VELOCITY 2.85 // m/s
	#define DESCENT_VELOCITY FLIGHT_MAX_HEIGHT/(FLIGHT_LENGTH*3600-FLIGHT_MAX_HEIGHT/ASCENT_VELOCITY) // m/s

	#define MIN_MAIN_BAT 0.95
	#define MIN_GSM_BAT 0.95

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

	#define PHOTO_WIDTH 3280
	#define PHOTO_HEIGHT 2464
	#define PHOTO_QUALITY 98
	#define PHOTO_RAW true
	#define PHOTO_CONTRAST 50
	#define PHOTO_BRIGHTNESS 50
	#define PHOTO_EXPOSURE "antishake"

	#define GPS_UART "/dev/ttyAMA0"
	#define GPS_ENABLE_GPIO 2
	#define GPS_BAUDRATE 9600
	#define GPS_ENDL "\r\n"

	#define IDEAL_DOP 1
	#define EXCELENT_DOP 2
	#define GOOD_DOP 5
	#define MODERATE_DOP 10
	#define FAIR_DOP 20
	#define MAX_DOP MODERATE_DOP

	#define GSM_LOC_SERV "gprs-service.com"
	#define GSM_UART "/dev/ttyUSB0"
	#define GSM_PWR_GPIO 7
	#define GSM_STATUS_GPIO 21
	#define GSM_BAUDRATE 9600
	#define GSM_ENDL "\r\n"

	#define SMS_PHONE ""

	#define STATE_FILE "data/last_state.txt"
#endif // CONSTANTS_H_
