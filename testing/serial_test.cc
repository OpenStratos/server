describe("Serial", [](){

	it("frame validity test", [&](){
		string valid = "$REPORT,0,23,185213184,1421782514,1,4140.7276,-0404.8853,73,52,43*1E";
		string valid2 = "$PMTK226,3,30*4";
		string not_valid = "$REPORT,0,23,185213184,1421782514,1,4140.7276,-0404.8853,73,52,43*14";

		Serial serial;
		serial.initialize("", 9600, "\n", NULL);

		AssertThat(serial.is_valid(valid), Equals(true));
		AssertThat(serial.is_valid(valid2), Equals(true));
		AssertThat(serial.is_valid(not_valid), Equals(false));

		serial.close();
	});
});
