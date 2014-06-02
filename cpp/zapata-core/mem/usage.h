#pragma once

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {
	template<typename T>
	struct is_pointer { static const bool value = false; };

	template<typename T>
	struct is_pointer<T*> { static const bool value = true; };

	void process_mem_usage(double& vm_usage, double& resident_set);
	void log_mem_usage();
}

