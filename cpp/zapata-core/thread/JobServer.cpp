#include <thread/JobServer.h>

zapata::JobServer::JobServer(string _key_file_path) : Job(_key_file_path) {
	if (!!this->configuration()["core"]["max_jobs"] && ((int) this->configuration()["core"]["max_jobs"]) != -1) {
		this->max(1 + (size_t) this->configuration()["core"]["max_jobs"]);
	}
	else {
		this->max(1);
	}
	this->idx(0);
	this->__next = 1;
}

zapata::JobServer::~JobServer() {
}

void zapata::JobServer::run() {
	for (; true; ) {
		if (this->max() != 1 && this->__next == this->max()) {
			this->__next = 1;
		}

		this->wait();
		this->notify();

		this->__next++;
	}
}

size_t zapata::JobServer::next() {
	return this->__next;
}

