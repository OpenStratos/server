#ifndef GSM_GMS_H_
#define GSM_GSM_H_

#include <string>

#include "serial/Serial.h"
#include "logger/Logger.h"

using namespace std;

namespace os {

	class GSM
	{
	private:
		Serial serial;
		Logger* logger;
		Logger* command_logger;

		int fh;

		GSM() = default;

		const string send_command_read(const string& command) const;
		bool send_command_read_only(const string& command, const string& only) const;
		void send_command(const string& command) const;
		bool init_GPRS() const;
		bool tear_down_GPRS() const;
	public:
		GSM(GSM& copy) = delete;
		~GSM();
		static GSM& get_instance();

		bool initialize();
		bool send_SMS(const string& message, const string& number) const;
		bool get_location(double& latitude, double& longitude) const;
		bool get_status() const;
		bool get_battery_status(double& main_bat_percentage, double& gsm_bat_percentage) const;
		bool is_up() const;
		bool has_connectivity() const;
		bool turn_on() const;
		bool turn_off() const;
	};
}
#endif
