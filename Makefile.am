bin_PROGRAMS = openstratos
openstratos_SOURCES = openstratos.cc logic/main_while.cc logic/shut_down.cc utils.cc threads.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc logger/Logger.cc gsm/GSM.cc
openstratos_CPPFLAGS = -Wall -Wextra -Werror -std=c++14

EXTRA_PROGRAMS = utesting
utesting_SOURCES = testing/testing.cc camera/Camera.cc gps/GPS.cc serial/Serial.cc logger/Logger.cc
utesting_CPPFLAGS = -Wall -Wextra -Werror -std=c++14 -Itesting/bandit -Wno-unused-parameter -DOS_TESTING
