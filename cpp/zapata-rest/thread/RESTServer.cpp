#include <thread/RESTServer.h>

#include <dlfcn.h>

zapata::RESTServer::RESTServer(string _key_file_path) : JobServer(_key_file_path), __n_jobs(1) {
	for (JSONObjIterator _i = this->configuration()->begin(); _i != this->configuration()->end(); _i++) {
		if ((*_i)->first != "zapata") {
			JSONObj _att((zapata::JSONObjRef*) (*_i)->second->get());
			string _lib_file("lib");
			_lib_file.append(_att["lib"]);
			_lib_file.append(".so");

			if (_lib_file.length() > 6) {
				void *hndl = dlopen(_lib_file.data(), RTLD_NOW);
				if (hndl == NULL) {
					cerr << dlerror() << endl;
				}
				else {
					void (*_populate)(RESTPool&);
					_populate = (void (*)(RESTPool&)) dlsym(hndl, "populate");
					_populate(this->__pool);
				}
			}
		}
	}
	this->__ss.bind((unsigned int) this->configuration()["zapata"]["rest"]["port"]);
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
		_job->pool(&this->__pool);
		_job->start();

		this->__jobs.push_back(_job);
	}

	this->__jobs.at(this->next() - 1)->assign(_cs_fd);
}

void zapata::RESTServer::notify() {
	struct sembuf ops[1] = { { (short unsigned int) this->next(), 1 } };
	semop(this->semid(), ops, 1);
}

zapata::RESTPool& zapata::RESTServer::pool() {
	return this->__pool;
}
