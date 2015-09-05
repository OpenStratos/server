#ifndef THREADS_H_
#define THREADS_H_

#include "utils.h"

namespace os {
	void system_thread_fn(State& state);
	void picture_thread_fn(State& state);
	void battery_thread_fn(State& state);
}

#endif // THREADS_H_
