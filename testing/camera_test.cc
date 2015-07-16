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
});
