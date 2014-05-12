#include <thread/RESTServer.h>

zapata::RESTServer::RESTServer(string _key_file_path) : JobServer(_key_file_path), __n_jobs(1) {
	this->__ss.bind((unsigned int) this->configuration()["rest"]["port"]);
}

zapata::RESTServer::~RESTServer(){
	for (vector<RESTJob*>::iterator i = this->__jobs.begin(); i != this->__jobs.end(); i++) {
		delete (*i);
	}
}

void zapata::RESTServer::wait() {
	int _cs_fd;
	this->__ss.accept(&_cs_fd);

	if (this->__n_jobs < this->max()) {
		RESTJob* _job = new RESTJob(this->__skey);
		_job->max(this->max());
		_job->idx(this->next());
		_job->start();

		this->__jobs.push_back(_job);
	}

	this->__jobs.at(this->next())->assign(_cs_fd);
}

void zapata::RESTServer::notify() {
	struct sembuf ops[1] = { { (short unsigned int) this->next(), 1 } };
	semop(this->semid(), ops, 1);
}

