bin_PROGRAMS=openstratos

openstratos_SOURCES=openstratos.cpp camera/Camera.cpp gps/GPS.cpp serial/Serial.cpp temperature/Temperature.cpp

openstratos_LDADD = -lwiringPi -lpthread
openstratos_CPPFLAGS = -std=c++11 -Wno-unused-result

EXTRA_PROGRAMS=utesting
utesting_SOURCES=testing/testing.cpp camera/Camera.cpp gps/GPS.cpp serial/Serial.cpp temperature/Temperature.cpp
utesting_CPPFLAGS = -std=c++11 -DOS_TESTING -Itesting/bandit -Wno-unused-result
