bin_PROGRAMS = openstratos
openstratos_SOURCES = openstratos.cc utils.cc threads.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc logger/Logger.cc gsm/GSM.cc xbee/XBee.cc
openstratos_CPPFLAGS = -std=c++14

EXTRA_PROGRAMS = utesting
utesting_SOURCES = testing/testing.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc logger/Logger.cc
utesting_CPPFLAGS = -std=c++14 -Itesting/bandit -Wno-unused-result -DOS_TESTING
