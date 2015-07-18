bin_PROGRAMS = openstratos
openstratos_SOURCES = openstratos.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc battery/Battery.cc temperature/Temperature.cc gsm/GSM.cc logger/Logger.cc
openstratos_LDADD = -lwiringPi
openstratos_CPPFLAGS = -std=c++14

EXTRA_PROGRAMS = utesting
utesting_SOURCES = testing/testing.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc battery/Battery.cc temperature/Temperature.cc gsm/GSM.cc logger/Logger.cc
utesting_CPPFLAGS = -std=c++14 -Itesting/bandit -Wno-unused-result -DOS_TESTING
