describe("Camera", [](){

	it("recording test", [&](){
		Camera::get_instance().record(200);
		AssertThat(Camera::get_instance().is_recording(), Equals(true));

		this_thread::sleep_for(250ms);
		AssertThat(Camera::get_instance().is_recording(), Equals(false));
	});

	it("recording and stopping test", [&](){
		Camera::get_instance().record();
		AssertThat(Camera::get_instance().is_recording(), Equals(true));

		this_thread::sleep_for(200ms);
		Camera::get_instance().stop();
		AssertThat(Camera::get_instance().is_recording(), Equals(false));

		cout << "checked second test" << endl;
	});

	#ifdef RASPIVID
		it("file creation test", [&](){
			Camera::get_instance().record(10);
			this_thread::sleep_for(chrono::seconds(11));

			struct stat buf;
			int result = stat("os_video.h264", &buf);

			AssertThat(result, Equals(0));
		});
	#else
		it_skip("file creation test", [&](){});
	#endif
});
