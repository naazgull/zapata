#include <thread/JobServer.h>

#include <exceptions/InterruptedException.h>

zapata::JobServer::JobServer(string _key_file_path) : __next(0), __max_idx(-1), __sem(-1), __skey(_key_file_path){
	ifstream _key_file;
	_key_file.open(_key_file_path.data());
	zapata::fromfile(_key_file, this->__configuration);

	if (!!this->configuration()["zapata"]["core"]["max_jobs"] && ((int) this->configuration()["zapata"]["core"]["max_jobs"]) != -1) {
		this->max((size_t) this->configuration()["zapata"]["core"]["max_jobs"]);
	}
	else {
		this->max(1);
	}
	this->__next = 0;

	key_t key = ftok(this->__skey.data(), this->__max_idx);
	this->__sem = semget(key, this->__max_idx, IPC_CREAT | 0777);
	if (this->__sem == 0) {
	}
}

zapata::JobServer::~JobServer() {
}

void zapata::JobServer::start() {
	try {
		for (; true; ) {
			if (this->max() != 0 && this->__next == this->max()) {
				this->__next = 0;
			}

			this->wait();
			this->notify();

			this->__next++;
		}
	}
	catch (zapata::InterruptedException& e) {
		return;
	}
}

size_t zapata::JobServer::max() {
	return this->__max_idx;
}

void zapata::JobServer::max(size_t _max) {
	this->__max_idx = _max;
}

size_t zapata::JobServer::next() {
	return this->__next;
}

semid_t zapata::JobServer::semid() {
	return this->__sem;
}

zapata::JSONObj& zapata::JobServer::configuration() {
	return this->__configuration;
}