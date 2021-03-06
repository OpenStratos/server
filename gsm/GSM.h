#ifndef GSM_GSM_H_
#define GSM_GSM_H_

#include <string>
#include <atomic>

#include "serial/Serial.h"
#include "logger/Logger.h"

using namespace std;

namespace os {

	class GSM
	{
	private:
		Serial* serial;
		Logger* logger;
		Logger* command_logger;

		int fh;
		atomic_bool occupied;

		GSM() = default;

		const string send_command_read(const string& command) const;
		bool init_GPRS() const;
		bool tear_down_GPRS() const;
	public:
		GSM(GSM& copy) = delete;
		~GSM();
		static GSM& get_instance();

		bool initialize();
		bool send_SMS(const string& message, const string& number);
		bool get_location(double& latitude, double& longitude);
		bool is_on() const;
		bool get_battery_status(double& main_bat_percentage, double& gsm_bat_percentage);
		bool has_connectivity();
		bool turn_on() const;
		bool turn_off() const;
	};
}
#endif // GSM_GSM_H_
