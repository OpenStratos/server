#ifndef CAM_H
	#define CAM_H

	namespace os {

		class Camera
		{
		private:
			Camera();
			void recordThread(int time);

			bool recording = false;
		public:
			static Camera& getCamera();

			void record(int time);
			void record();
			void stop();
		};
	}
#endif