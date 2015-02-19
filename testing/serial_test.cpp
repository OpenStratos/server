describe("Serial", [](){

	it("frame validity test", [&](){
		string valid = "$REPORT,0,23,185213184,1421782514,1,4140.7276,-0404.8853,73,52,43*1E";
		string valid2 = "$PMTK226,3,30*4";
		string not_valid = "$REPORT,0,23,185213184,1421782514,1,4140.7276,-0404.8853,73,52,43*14";

		Serial serial;
		serial.initialize("", 9600, "\n", NULL);

		cout << "Serial initialized" << endl;

		AssertThat(serial.is_valid(valid), Equals(true));
		cout << "First serial test finished" << endl;

		AssertThat(serial.is_valid(valid2), Equals(true));
		cout << "Second serial test finished" << endl;

		AssertThat(serial.is_valid(not_valid), Equals(false));
		cout << "Third serial test finished" << endl;

		cout << "Starting checked, closing" << endl;

		serial.close();

		cout << "Serial closed" << endl;
	});
});
