describe("Camera", [](){

	it("recording test", [&](){
		Camera::get_instance().record(2000);
		AssertThat(Camera::get_instance().is_recording(), Equals(true));

		this_thread::sleep_for(2.5s);
		AssertThat(Camera::get_instance().is_recording(), Equals(false));
	});

	it("recording and stopping test", [&](){
		Camera::get_instance().record();
		AssertThat(Camera::get_instance().is_recording(), Equals(true));

		this_thread::sleep_for(2s);
		Camera::get_instance().stop();
		AssertThat(Camera::get_instance().is_recording(), Equals(false));
	});

	#ifdef RASPIVID
		it("file creation test", [&](){
			Camera::get_instance().record(10000);
			this_thread::sleep_for(chrono::seconds(11));

			struct stat buf;
			int result = stat("data/video/test.h264", &buf);

			AssertThat(result, Equals(0));
			remove("data/video/test.h264");
		});
	#else
		it_skip("file creation test", [&](){});
	#endif
});
