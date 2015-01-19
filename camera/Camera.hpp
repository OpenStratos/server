#ifndef CAM_H
	#define CAM_H

	namespace os {

		class Camera
		{
		public:
			void record(int time);
			void record();
			void stop();
		};
	}
#endif