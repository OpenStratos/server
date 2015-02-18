describe("Battery", [](){

	it("voltage to percentage calculator test", [&](){
		AssertThat(volt_to_percent(8.4), Is().EqualToWithDelta(100, 0.01));
		AssertThat(volt_to_percent(7.4), Is().EqualToWithDelta(0, 0.01));
		AssertThat(volt_to_percent(7.75), Is().EqualToWithDelta(35, 0.01));
		AssertThat(volt_to_percent(8), Is().EqualToWithDelta(60, 0.01));
	});
});
