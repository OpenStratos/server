describe("GPS", []()
{
	before_each([&]()
	{
		GPS::get_instance().initialize();
	});

	it("Knots to m/s conversion test", [&]()
	{
		AssertThat(kt_to_mps(150), Is().EqualToWithDelta(77.1666667, 0.005));
		AssertThat(kt_to_mps(75), Is().EqualToWithDelta(38.58333, 0.005));
		AssertThat(kt_to_mps(0), Equals(0));
	});

	it("frame validity test", [&]()
	{
		string valid = "$REPORT,0,23,185213184,1421782514,1,4140.7276,-0404.8853,73,52,43*1E";
		string valid2 = "$PMTK226,3,30*4";
		string not_valid = "$REPORT,0,23,185213184,1421782514,1,4140.7276,-0404.8853,73,52,43*14";

		AssertThat(GPS::is_valid(valid), Equals(true));
		AssertThat(GPS::is_valid(valid2), Equals(true));
		AssertThat(GPS::is_valid(not_valid), Equals(false));
	});

	it("GGA frame parser test", [&]()
	{
		GPS::get_instance().parse("$GNGGA,065022.90,4518.48425,N,00113.59390,W,1,05,2.33,10202.2,M,10235.9,M,,*60");

		AssertThat(GPS::get_instance().is_fixed(), Equals(true));

		timeval gps_time = GPS::get_instance().get_time();
		tm* ptm = gmtime(&gps_time.tv_sec);

		AssertThat(ptm->tm_hour, Equals(6));
		AssertThat(ptm->tm_min, Equals(50));
		AssertThat(ptm->tm_sec, Equals(22));
		AssertThat(gps_time.tv_usec, Equals(900000));

		AssertThat(GPS::get_instance().get_satellites(), Equals(5));
		AssertThat(GPS::get_instance().get_latitude(),
			Is().EqualToWithDelta(45.308070833, 0.00001));
		AssertThat(GPS::get_instance().get_longitude(),
			Is().EqualToWithDelta(-1.226565, 0.00001));

		AssertThat(GPS::get_instance().get_HDOP(),
			Is().EqualToWithDelta(2.33, 0.0005));
		AssertThat(GPS::get_instance().get_altitude(),
			Is().EqualToWithDelta(10202.2, 0.0005));
	});

	it("GGA frame parser pass test", [&]()
	{
		int satellites = GPS::get_instance().get_satellites();
		double latitude = GPS::get_instance().get_latitude();
		double longitude = GPS::get_instance().get_longitude();
		double altitude = GPS::get_instance().get_altitude();
		float hdop = GPS::get_instance().get_HDOP();

		GPS::get_instance().parse("$GNGGA,054320.70,4416.46425,N,00512.59490,W,0,05,1.25,9502.63,M,9515.2,M,,*55");

		AssertThat(GPS::get_instance().is_fixed(), Equals(false));

		timeval gps_time = GPS::get_instance().get_time();
		tm* ptm = gmtime(&gps_time.tv_sec);

		AssertThat(ptm->tm_hour, Equals(5));
		AssertThat(ptm->tm_min, Equals(43));
		AssertThat(ptm->tm_sec, Equals(20));
		AssertThat(gps_time.tv_usec, Equals(700000));

		AssertThat(GPS::get_instance().get_satellites(), Equals(satellites));
		AssertThat(GPS::get_instance().get_latitude(), Equals(latitude));
		AssertThat(GPS::get_instance().get_longitude(), Equals(longitude));
		AssertThat(GPS::get_instance().get_altitude(), Equals(altitude));
		AssertThat(GPS::get_instance().get_HDOP(), Equals(hdop));
	});

	it("GSA frame parser test", [&]()
	{
		GPS::get_instance().parse("$GNGSA,A,3,22,01,19,23,17,03,11,31,,,,,1.91,0.99,1.63*1E");

		AssertThat(GPS::get_instance().is_fixed(), Equals(true));
		AssertThat(GPS::get_instance().get_PDOP(),
			Is().EqualToWithDelta(1.91, 0.0005));
		AssertThat(GPS::get_instance().get_HDOP(),
			Is().EqualToWithDelta(0.99, 0.0005));
		AssertThat(GPS::get_instance().get_VDOP(),
			Is().EqualToWithDelta(1.63, 0.0005));
	});

	it("GSA frame parser pass test", [&]()
	{
		float pdop = GPS::get_instance().get_PDOP();
		float hdop = GPS::get_instance().get_HDOP();
		float vdop = GPS::get_instance().get_VDOP();

		GPS::get_instance().parse("$GNGSA,A,1,22,01,19,23,17,03,11,31,,,,,2.30,0.05,1.52*13");

		AssertThat(GPS::get_instance().is_fixed(), Equals(false));
		AssertThat(GPS::get_instance().get_PDOP(), Equals(pdop));
		AssertThat(GPS::get_instance().get_HDOP(), Equals(hdop));
		AssertThat(GPS::get_instance().get_VDOP(), Equals(vdop));
	});

	it("RMC frame parser test", [&]()
	{
		GPS::get_instance().parse("$GNRMC,083250.50,A,4210.48220,N,00306.55496,W,0.072,0.25,070616,,,A*6A");

		AssertThat(GPS::get_instance().is_fixed(), Equals(true));

		timeval gps_time = GPS::get_instance().get_time();
		tm* ptm = gmtime(&gps_time.tv_sec);

		AssertThat(ptm->tm_mday, Equals(7));
		AssertThat(ptm->tm_mon, Equals(5));
		AssertThat(ptm->tm_year, Equals(116));
		AssertThat(gps_time.tv_usec, Equals(500000));

		AssertThat(ptm->tm_hour, Equals(8));
		AssertThat(ptm->tm_min, Equals(32));
		AssertThat(ptm->tm_sec, Equals(50));

		AssertThat(GPS::get_instance().get_latitude(),
			Is().EqualToWithDelta(42.174703333, 0.00001));
		AssertThat(GPS::get_instance().get_longitude(),
			Is().EqualToWithDelta(-3.109249333, 0.00001));

		AssertThat(GPS::get_instance().get_velocity().speed,
			Is().EqualToWithDelta(0.03704, 0.00001));
		AssertThat(GPS::get_instance().get_velocity().course,
			Is().EqualToWithDelta(0.25, 0.001));
	});

	it("RMC frame parser pass test", [&]()
	{
		double latitude = GPS::get_instance().get_latitude();
		double longitude = GPS::get_instance().get_longitude();
		float speed = GPS::get_instance().get_velocity().speed;
		float course = GPS::get_instance().get_velocity().course;

		GPS::get_instance().parse("$GNRMC,093024.50,V,4012.20220,N,00508.50496,W,0.015,0.80,060515,,,A*71");

		AssertThat(GPS::get_instance().is_fixed(), Equals(false));

		timeval gps_time = GPS::get_instance().get_time();
		tm* ptm = gmtime(&gps_time.tv_sec);

		AssertThat(ptm->tm_hour, Equals(9));
		AssertThat(ptm->tm_min, Equals(30));
		AssertThat(ptm->tm_sec, Equals(24));
		AssertThat(gps_time.tv_usec, Equals(500000));

		AssertThat(ptm->tm_mday, Equals(6));
		AssertThat(ptm->tm_mon, Equals(4));
		AssertThat(ptm->tm_year, Equals(115));

		AssertThat(GPS::get_instance().get_latitude(), Equals(latitude));
		AssertThat(GPS::get_instance().get_longitude(), Equals(longitude));
		AssertThat(GPS::get_instance().get_velocity().speed, Equals(speed));
		AssertThat(GPS::get_instance().get_velocity().course, Equals(course));
	});

	it("incomplete frame parser test", [&]()
	{
		GPS::get_instance().parse("$GNGSA,A,2,,N,00256.61767,W,0.124,,140616,,,A*74");
	});
});
