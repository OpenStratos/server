describe("Battery", [](){

	it("voltage to percentage calculator test", [&](){
		AssertThat(volt_to_percent(8.4), Is().EqualToWithDelta(100, 0.01));
		AssertThat(volt_to_percent(7.4), Is().EqualToWithDelta(0, 0.01));
		AssertThat(volt_to_percent(7.75), Is().EqualToWithDelta(35, 0.01));
		AssertThat(volt_to_percent(8), Is().EqualToWithDelta(60, 0.01));
	});

	it("reading test", [&](){
		Battery bat(80);

		bat.start_reading();
		this_thread::sleep_for(75ms);

		AssertThat(bat.get_battery(), Is().EqualToWithDelta(50, 0.05));
		bat.stop_reading();
	});
});
