describe("Temperature", [&](){

	it("Read test", [&]() {
		Temperature temp(20);
		temp.start_reading();
		AssertThat(temp.get_reading(), Equals(true));

		temp.stopReading();
		AssertThat(temp.get_reading(), Equals(false));
	});

	it("Conversion test", [&]() {
		AssertThat(r_to_c(1000), EqualToWithDelta(0, 0.05));

		AssertThat(r_to_c(1573), EqualToWithDelta(150, 0.05));

		AssertThat(r_to_c(803,1), EqualToWithDelta(-50, 0.05));
	});
});
