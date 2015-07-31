#ifndef CAMERA_CAMERA_H_
#define CAMERA_CAMERA_H_

#include <string>

#include "logger/Logger.h"

using namespace std;

namespace os {

	class Camera
	{
	private:
		Logger* logger;

		bool recording = false;
		Camera();
		void record_thread(int time);
	public:
		Camera(Camera& copy) = delete;
		~Camera();
		static Camera& get_instance();

		bool record(int time);
		bool record();
		bool take_picture();
		bool stop();
		bool is_recording() const {return this->recording;}
	};

	int get_file_count(const string& path);
}

#endif // CAMERA_CAMERA_H_
