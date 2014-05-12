#include <thread/RESTJob.h>

#include <zapata/net.h>
#include <zapata/http.h>

zapata::RESTJob::RESTJob(string _key_file_path) : Job(_key_file_path), __cur_fd(-1) {
}

zapata::RESTJob::~RESTJob() {
}

void zapata::RESTJob::run() {
	for (; true; ) {
		this->wait();
		socketstream _cs(this->__cur_fd);

		HTTPReq obj;
		zapata::fromstream(_cs, obj);


	}
}

void zapata::RESTJob::assign(int _cs_fd) {
	this->__cur_fd = _cs_fd;
}
