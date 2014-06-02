#include <mem/usage.h>

#include <log/log.h>
#include <text/convert.h>

void zapata::process_mem_usage(double& vm_usage, double& resident_set) {
	using std::ios_base;
	using std::ifstream;
	using std::string;

	vm_usage = 0.0;
	resident_set = 0.0;

	ifstream stat_stream("/proc/self/stat", ios_base::in);

	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	string utime, stime, cutime, cstime, priority, nice;
	string O, itrealvalue, starttime;

	unsigned long vsize;
	long rss;

	stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
			>> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
			>> utime >> stime >> cutime >> cstime >> priority >> nice
			>> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

	stat_stream.close();

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
	vm_usage = vsize / 1024.0;
	resident_set = rss * page_size_kb;
}

void zapata::log_mem_usage() {
	double _vm;
	double _resident;
	zapata::process_mem_usage(_vm, _resident);
	string _text("virtual_memory: ");
	zapata::tostr(_text, _vm);
	_text.insert(_text.length(), "kb | resident_memory: ");
	zapata::tostr(_text, _resident);
	_text.insert(_text.length(), "kb");
	zapata::log(_text, zapata::debug);
}
