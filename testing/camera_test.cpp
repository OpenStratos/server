describe("Camera", [](){

	it("recording test", [&](){
		Camera::get_instance().record(200);
		AssertThat(Camera::get_instance().is_recording(), Equals(true));

		this_thread::sleep_for(chrono::milliseconds(250));
		AssertThat(Camera::get_instance().is_recording(), Equals(false));
	});

	it("recording and stopping test", [&](){
		Camera::get_instance().record();
		AssertThat(Camera::get_instance().is_recording(), Equals(true));

		this_thread::sleep_for(chrono::milliseconds(200));
		Camera::get_instance().stop();
		AssertThat(Camera::get_instance().is_recording(), Equals(false));
	});
});
