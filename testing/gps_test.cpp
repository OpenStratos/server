describe("GPS", [](){

	it("Knots to m/s conversion test", [&](){
		AssertThat(kt_to_mps(150), Is().EqualToWithDelta(77.1666667, 0.005));
	});

	it("GGA frame parser test", [&](){
		GPS::get_instance().parse("$GPGGA,151025,2011.3454,N,12020.2464,W,1,05,1.53,20134.13,M,20103.45,M,,*56");

		AssertThat(GPS::get_instance().is_active(), Equals(true));

		time_t gps_time = GPS::get_instance().get_time();
		tm* time = gmtime(&gps_time);
		AssertThat(time->tm_hour, Equals(15));
		AssertThat(time->tm_min, Equals(10));
		AssertThat(time->tm_sec, Equals(25));

		AssertThat(GPS::get_instance().get_satellites(), Equals(5));
		AssertThat(GPS::get_instance().get_latitude(), Is().EqualToWithDelta(20.18909, 0.00001));
		AssertThat(GPS::get_instance().get_longitude(), Is().EqualToWithDelta(-120.33744, 0.00001));

		AssertThat(GPS::get_instance().get_HDOP(), Is().EqualToWithDelta(1.53, 0.0005));
		AssertThat(GPS::get_instance().get_altitude(), Is().EqualToWithDelta(20134.13, 0.0005));
	});

	it("GGA frame parser pass test", [&](){

		time_t time = GPS::get_instance().get_time();
		int satellites = GPS::get_instance().get_satellites();
		double latitude = GPS::get_instance().get_latitude();
		double longitude = GPS::get_instance().get_longitude();
		double altitude = GPS::get_instance().get_altitude();
		float hdop = GPS::get_instance().get_HDOP();

		GPS::get_instance().parse("$GPGGA,151025,2011.3454,N,12020.2464,W,0,05,1.53,20134.13,M,20103.45,M,,*56");

		AssertThat(GPS::get_instance().is_active(), Equals(false));
		AssertThat(GPS::get_instance().get_time(), Equals(time));
		AssertThat(GPS::get_instance().get_satellites(), Equals(satellites));
		AssertThat(GPS::get_instance().get_latitude(), Equals(latitude));
		AssertThat(GPS::get_instance().get_longitude(), Equals(longitude));
		AssertThat(GPS::get_instance().get_altitude(), Equals(altitude));
		AssertThat(GPS::get_instance().get_HDOP(), Equals(hdop));
	});

	it("GSA frame parser test", [&](){

	});

	it("GSA frame parser pass test", [&](){

	});

	it("RMC frame parser test", [&](){

	});

	it("RMC frame parser pass test", [&](){

	});
});
