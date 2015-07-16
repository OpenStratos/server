#ifndef OPENSTRATOS_H_
#define OPENSTRATOS_H_

#ifndef RASPIVID
#define RASPIVID 0
#endif

#include <iostream>
#include <string>
#include <thread>

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>

#include "config.h"
#include "logger/Logger.h"
#include "gps/GPS.h"
#include "camera/Camera.h"

using namespace std;
using namespace os;

inline bool file_exists(const string& name);
inline float get_available_disk_space();
void gps_thread_fn();

#endif // OPENSTRATOS_H_
