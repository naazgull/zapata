/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <base/smart_ptr.h>
#include <base/str_map.h>
#include <dlfcn.h>
#include <exceptions/ClosedException.h>
#include <exceptions/InterruptedException.h>
#include <json/JSONObj.h>
#include <log/log.h>
#include <resource/FileRemove.h>
#include <resource/FileUpload.h>
#include <sys/sem.h>
#include <text/convert.h>
#include <thread/RESTServer.h>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

zapata::RESTServer::RESTServer(string _key_file_path) : JobServer(_key_file_path), __n_jobs(0) {
	this->__configuration << zapata::pretty;

	if (!!this->configuration()["zapata"]["core"]["log"]["level"]) {
		zapata::log_lvl = (int) this->configuration()["zapata"]["core"]["log"]["level"];
	}
	if (!!this->configuration()["zapata"]["core"]["log"]["file"]) {
		zapata::log_fd = new ofstream();
		((ofstream*) zapata::log_fd)->open(((string) this->configuration()["zapata"]["core"]["log"]["file"]).data());
	}

	this->__pool.configuration(&this->__configuration);

	for (JSONObjIterator _i = this->configuration()->begin(); _i != this->configuration()->end(); _i++) {
		if ((*_i)->first != "zapata") {
			JSONObj _att((zapata::JSONObjRef*) (*_i)->second->get());
			string _lib_file("lib");
			_lib_file.append(_att["lib"]);
			_lib_file.append(".so");

			if (_lib_file.length() > 6) {
				void *hndl = dlopen(_lib_file.data(), RTLD_NOW);
				if (hndl == NULL) {
					zapata::log(string(dlerror()), zapata::error);
				}
				else {
					void (*_populate)(RESTPool&);
					_populate = (void (*)(RESTPool&)) dlsym(hndl, "populate");
					_populate(this->__pool);
				}
			}
		}
	}

	if (!!this->configuration()["zapata"]["rest"]["uploads"]["upload_controller"]) {
		zapata::FileUpload* _file_upload = new zapata::FileUpload();
		this->__pool.add(_file_upload);
		zapata::FileRemove* _file_remove = new zapata::FileRemove();
		this->__pool.add(_file_remove);
	}

	unsigned int _port =  (unsigned int) this->configuration()["zapata"]["rest"]["port"];
	string _text("starting RESTful server on port ");
	zapata::tostr(_text, _port);
	zapata::log(_text, zapata::sys);
	this->__ss.bind(_port);
}

zapata::RESTServer::~RESTServer(){
	for (vector<RESTJob*>::iterator i = this->__jobs.begin(); i != this->__jobs.end(); i++) {
		delete (*i);
	}
}

void zapata::RESTServer::wait() {
	try {
		int _cs_fd = -1;
		this->__ss.accept(&_cs_fd);

		if (this->__n_jobs < this->max()) {
			RESTJob* _job = new RESTJob(this->__skey);
			_job->max(this->max());
			_job->idx(this->next());
			_job->pool(&this->__pool);
			_job->start();

			this->__jobs.push_back(_job);
			this->__n_jobs++;
		}

		this->__jobs.at(this->next())->assign(_cs_fd);
	}
	catch(zapata::ClosedException& e) {
		throw zapata::InterruptedException(e.what());
	}
}

void zapata::RESTServer::notify() {
	struct sembuf ops[1] = { { (short unsigned int) this->next(), 1 } };
	semop(this->semid(), ops, 1);
}

zapata::RESTPool& zapata::RESTServer::pool() {
	return this->__pool;
}
