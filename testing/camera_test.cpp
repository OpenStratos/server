describe("Camera test", [](){

	it("recording", [&](){
		Camera::getCamera().record(1000);
		this_thread::sleep_for(chrono::milliseconds(200));
		AssertThat(Camera::getCamera().isRecording(), Equals(true));

		this_thread::sleep_for(chrono::milliseconds(1500));
		AssertThat(Camera::getCamera().isRecording(), Equals(false));
	});

	it("recording infinite video and stopping", [&](){
		Camera::getCamera().record();
		this_thread::sleep_for(chrono::milliseconds(200));
		AssertThat(Camera::getCamera().isRecording(), Equals(true));

		this_thread::sleep_for(chrono::seconds(1));
		Camera::getCamera().stop();
		AssertThat(Camera::getCamera().isRecording(), Equals(false));
	});
});
