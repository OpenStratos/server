describe("GPS", [](){

	it("Knots to m/s conversion test", [&](){
		AssertThat(kt_to_mps(150), Is().EqualToWithDelta(77.1666667, 0.005));
		AssertThat(kt_to_mps(75), Is().EqualToWithDelta(38.58333, 0.005));
		AssertThat(kt_to_mps(0), Equals(0));
	});

	it("GGA frame parser test", [&](){
		GPS::get_instance().parse("$GPGGA,151025,2011.3454,N,12020.2464,W,1,05,1.53,20134.13,M,20103.45,M,,*56");

		AssertThat(GPS::get_instance().is_active(), Equals(true));

		tm* gps_time = GPS::get_instance().get_time();
		AssertThat(gps_time->tm_hour, Equals(15));
		AssertThat(gps_time->tm_min, Equals(10));
		AssertThat(gps_time->tm_sec, Equals(25));

		AssertThat(GPS::get_instance().get_satellites(), Equals(5));
		AssertThat(GPS::get_instance().get_latitude(), Is().EqualToWithDelta(20.18909, 0.00001));
		AssertThat(GPS::get_instance().get_longitude(), Is().EqualToWithDelta(-120.33744, 0.00001));

		AssertThat(GPS::get_instance().get_HDOP(), Is().EqualToWithDelta(1.53, 0.0005));
		AssertThat(GPS::get_instance().get_altitude(), Is().EqualToWithDelta(20134.13, 0.0005));
	});

	it("GGA frame parser pass test", [&](){

		int hour = GPS::get_instance().get_time()->tm_hour;
		int min = GPS::get_instance().get_time()->tm_min;
		int sec = GPS::get_instance().get_time()->tm_sec;

		int satellites = GPS::get_instance().get_satellites();
		double latitude = GPS::get_instance().get_latitude();
		double longitude = GPS::get_instance().get_longitude();
		double altitude = GPS::get_instance().get_altitude();
		float hdop = GPS::get_instance().get_HDOP();

		GPS::get_instance().parse("$GPGGA,170810,2316.3654,S,12225.2464,E,0,07,1.89,18647.15,M,18640.35,M,,*53");

		AssertThat(GPS::get_instance().is_active(), Equals(false));

		AssertThat(GPS::get_instance().get_time()->tm_hour, Equals(hour));
		AssertThat(GPS::get_instance().get_time()->tm_min, Equals(min));
		AssertThat(GPS::get_instance().get_time()->tm_sec, Equals(sec));

		AssertThat(GPS::get_instance().get_satellites(), Equals(satellites));
		AssertThat(GPS::get_instance().get_latitude(), Equals(latitude));
		AssertThat(GPS::get_instance().get_longitude(), Equals(longitude));
		AssertThat(GPS::get_instance().get_altitude(), Equals(altitude));
		AssertThat(GPS::get_instance().get_HDOP(), Equals(hdop));
	});

	it("GSA frame parser test", [&](){
		GPS::get_instance().parse("$GPGSA,A,3,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C");

		AssertThat(GPS::get_instance().is_active(), Equals(true));
		AssertThat(GPS::get_instance().get_HDOP(), Is().EqualToWithDelta(2.1, 0.0005));
		AssertThat(GPS::get_instance().get_VDOP(), Is().EqualToWithDelta(2.2, 0.0005));
	});

	it("GSA frame parser pass test", [&](){
		float hdop = GPS::get_instance().get_HDOP();
		float vdop = GPS::get_instance().get_VDOP();

		GPS::get_instance().parse("$GPGSA,A,1,19,28,14,18,27,22,31,39,,,,,1.7,1.0,1.3*36");

		AssertThat(GPS::get_instance().is_active(), Equals(false));
		AssertThat(GPS::get_instance().get_HDOP(), Equals(hdop));
		AssertThat(GPS::get_instance().get_VDOP(), Equals(vdop));
	});

	it("RMC frame parser test", [&](){
		GPS::get_instance().parse("$GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68");

		AssertThat(GPS::get_instance().is_active(), Equals(true));

		tm* gps_time = GPS::get_instance().get_time();

		AssertThat(gps_time->tm_mday, Equals(19));
		AssertThat(gps_time->tm_mon, Equals(11));
		AssertThat(gps_time->tm_year, Equals(194));
		AssertThat(gps_time->tm_hour, Equals(22));
		AssertThat(gps_time->tm_min, Equals(54));
		AssertThat(gps_time->tm_sec, Equals(46));

		AssertThat(GPS::get_instance().get_latitude(), Is().EqualToWithDelta(49.27417, 0.00001));
		AssertThat(GPS::get_instance().get_longitude(), Is().EqualToWithDelta(-123.18533, 0.00001));

		AssertThat(GPS::get_instance().get_velocity()->speed, Is().EqualToWithDelta(0.25722, 0.00001));
		AssertThat(GPS::get_instance().get_velocity()->course, Is().EqualToWithDelta(54.7, 0.001));
	});

	it("RMC frame parser pass test", [&](){

		int hour = GPS::get_instance().get_time()->tm_hour;
		int min = GPS::get_instance().get_time()->tm_min;
		int sec = GPS::get_instance().get_time()->tm_sec;
		int mday = GPS::get_instance().get_time()->tm_mday;
		int mon = GPS::get_instance().get_time()->tm_mon;
		int year = GPS::get_instance().get_time()->tm_year;

		double latitude = GPS::get_instance().get_latitude();
		double longitude = GPS::get_instance().get_longitude();
		float speed = GPS::get_instance().get_velocity()->speed;
		float course = GPS::get_instance().get_velocity()->course;

		GPS::get_instance().parse("$GPRMC,081836,V,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*75");

		AssertThat(GPS::get_instance().is_active(), Equals(false));

		AssertThat(GPS::get_instance().get_time()->tm_hour, Equals(hour));
		AssertThat(GPS::get_instance().get_time()->tm_min, Equals(min));
		AssertThat(GPS::get_instance().get_time()->tm_sec, Equals(sec));
		AssertThat(GPS::get_instance().get_time()->tm_mday, Equals(mday));
		AssertThat(GPS::get_instance().get_time()->tm_mon, Equals(mon));
		AssertThat(GPS::get_instance().get_time()->tm_year, Equals(year));

		AssertThat(GPS::get_instance().get_latitude(), Equals(latitude));
		AssertThat(GPS::get_instance().get_longitude(), Equals(longitude));
		AssertThat(GPS::get_instance().get_velocity()->speed, Equals(speed));
		AssertThat(GPS::get_instance().get_velocity()->course, Equals(course));
	});
});
