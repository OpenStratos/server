bin_PROGRAMS=openstratos
openstratos_SOURCES=openstratos.cpp camera/Camera.cpp gps/GPS.cpp temperature/Temperature.cpp
openstratos_LDADD = -lwiringPi -lpthread
openstratos_CPPFLAGS = -std=c++11 -Wno-unused-result

EXTRA_PROGRAMS=utesting
utesting_SOURCES=testing/testing.cpp camera/Camera.cpp gps/GPS.cpp
utesting_CPPFLAGS = -std=c++11 -Itesting/bandit -Wno-unused-result
