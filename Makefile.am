bin_PROGRAMS = openstratos
openstratos_SOURCES = openstratos.cpp camera/Camera.cpp gps/GPS.cpp serial/Serial.cpp
openstratos_LDADD = -lwiringPi
openstratos_CPPFLAGS = -std=c++11 -Wno-unused-result

EXTRA_PROGRAMS = openstratosRoot utesting
openstratosRoot_SOURCES = openstratos-root.cpp camera/Camera.cpp gps/GPS.cpp serial/Serial.cpp
openstratosRoot_CPPFLAGS = -std=c++11 -Wno-unused-result

utesting_SOURCES = testing/testing.cpp camera/Camera.cpp gps/GPS.cpp serial/Serial.cpp
utesting_CPPFLAGS = -std=c++11 -Itesting/bandit -Wno-unused-result -DOS_TESTING
