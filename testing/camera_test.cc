describe("Camera", [](){

	it("recording test", [&](){
		AssertThat(Camera::get_instance().record(2000), Equals(true));
		AssertThat(Camera::get_instance().is_recording(), Equals(true));

		this_thread::sleep_for(2.5s);
		AssertThat(Camera::get_instance().is_recording(), Equals(false));

		#ifdef RASPICAM
			remove("data/video/test.h264");
		#endif
	});

	it("recording and stopping test", [&](){
		AssertThat(Camera::get_instance().record(), Equals(true));
		AssertThat(Camera::get_instance().is_recording(), Equals(true));

		this_thread::sleep_for(2s);
		AssertThat(Camera::get_instance().stop(), Equals(true));
		AssertThat(Camera::get_instance().is_recording(), Equals(false));

		#ifdef RASPICAM
			remove("data/video/test.h264");
		#endif
	});

	it("picture taking test", [&](){
		AssertThat(Camera::get_instance().take_picture(), Equals(true));

		#ifdef RASPICAM
			remove("data/img/test.jpg");
		#endif
	});

	#ifdef RASPICAM
		it("video file creation test", [&](){
			AssertThat(Camera::get_instance().record(10000), Equals(true));
			this_thread::sleep_for(chrono::seconds(11));

			struct stat buf;
			int result = stat("data/video/test.h264", &buf);

			AssertThat(result, Equals(0));
			remove("data/video/test.h264");
		});
	#else
		it_skip("video file creation test", [&](){});
	#endif

	#ifdef RASPICAM
		it("picture file creation test", [&](){
			int start_file_count = get_file_count("data/img/");
			AssertThat(Camera::get_instance().take_picture(), Equals(true));
			int end_file_count = get_file_count("data/img/");

			AssertThat(end_file_count, Equals(start_file_count+1));
			remove(("data/img/img-"+ to_string(get_file_count("data/img/")-1) +".jpg").c_str());
		});
	#else
		it_skip("picture file creation test", [&](){});
	#endif
});
