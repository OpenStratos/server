#ifndef CAMERA_CAMERA_H_
#define CAMERA_CAMERA_H_

#include <string>

using namespace std;

namespace os {

	class Camera
	{
	private:
		Camera() = default;
		~Camera();
		void record_thread(int time);

		bool recording = false;
	public:
		Camera(Camera& copy) = delete;
		static Camera& get_instance();

		void record(int time);
		void record();
		void stop();
		bool is_recording() const {return this->recording;}
	};

	int get_file_count(const string& path);
}

#endif // CAMERA_CAMERA_H_
