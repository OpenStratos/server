bin_PROGRAMS = openstratos
openstratos_SOURCES = openstratos.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc battery/Battery.cc temperature/Temperature.cc adc/MCP3424.cc adc/ADC.cc
openstratos_LDADD = -lwiringPi
openstratos_CPPFLAGS = -std=c++11 -Wno-unused-result

EXTRA_PROGRAMS = openstratosRoot utesting
openstratosRoot_SOURCES = openstratos-root.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc battery/Battery.cc temperature/Temperature.cc adc/MCP3424.cc adc/ADC.cc
openstratosRoot_CPPFLAGS = -std=c++11 -Wno-unused-result

utesting_SOURCES = testing/testing.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc battery/Battery.cc temperature/Temperature.cc adc/MCP3424.cc adc/ADC.cc
utesting_CPPFLAGS = -std=c++11 -Itesting/bandit -Wno-unused-result -DOS_TESTING
