describe("Temperature", [&](){

	it("Read test", [&]() {
		std::unique_ptr<Temperature> temp(new Temperature(20));
		temp.start_reading();
		AssertThat(temp.get_reading(), Equals(true));

		temp.stopReading();
		AssertThat(temp.get_reading(), Equals(false));
	});

	it("Conversion test", [&]() {
		AssertThat(r_to_c(1000), Equals(0));

		AssertThat(r_to_c(1573), Equals(150));

		AssertThat(r_to_c(803,1), Equals(-50));
	});
});