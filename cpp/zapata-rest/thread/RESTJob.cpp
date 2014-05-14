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

		HTTPRep _rep;
		HTTPReq _req;
		zapata::fromstream(_cs, _req);

		this->__pool->process(_req, _rep);

		_cs << _rep << flush;
	}
}

void zapata::RESTJob::assign(int _cs_fd) {
	this->__cur_fd = _cs_fd;
}

zapata::RESTPool& zapata::RESTJob::pool() {
	return *this->__pool;
}

void zapata::RESTJob::pool(RESTPool* _pool) {
	this->__pool = _pool;
}
