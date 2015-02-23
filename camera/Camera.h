#ifndef CAMERA_CAMERA_H_
#define CAMERA_CAMERA_H_

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
}

#endif // CAMERA_CAMERA_H_