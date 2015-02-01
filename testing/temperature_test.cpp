describe("Temperature", [&](){

	it("read & stop test", [&]() {
		Temperature temp(20);
		temp.start_reading();
		AssertThat(temp.is_reading(), Equals(true));

	//	this_thread::sleep_for(chrono::milliseconds(100)); if uncommented less segmentation faults

		temp.stop_reading();
		AssertThat(temp.is_reading(), Equals(false));
	});

	it("resistor to temperature conversion test", [&]() {

		AssertThat(r_to_c(1155.4), Is().EqualToWithDelta(40, 0.05));
		AssertThat(r_to_c(1116.9), Is().EqualToWithDelta(30, 0.05));
		AssertThat(r_to_c(1077.9), Is().EqualToWithDelta(20, 0.05));
		AssertThat(r_to_c(1039), Is().EqualToWithDelta(10, 0.05));
		AssertThat(r_to_c(1000), Is().EqualToWithDelta(0, 0.05));
		AssertThat(r_to_c(960.9), Is().EqualToWithDelta(-10, 0.05));
		AssertThat(r_to_c(921.6), Is().EqualToWithDelta(-20, 0.05));
		AssertThat(r_to_c(882.2), Is().EqualToWithDelta(-30, 0.05));
		AssertThat(r_to_c(842.7), Is().EqualToWithDelta(-40, 0.05));
		AssertThat(r_to_c(803.1), Is().EqualToWithDelta(-50, 0.05));
		AssertThat(r_to_c(763.3), Is().EqualToWithDelta(-60, 0.05));
		AssertThat(r_to_c(723.3), Is().EqualToWithDelta(-70, 0.05));
		AssertThat(r_to_c(683.3), Is().EqualToWithDelta(-80, 0.05));
	});
});
