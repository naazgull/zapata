/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <dlfcn.h>
#include <zapata/exceptions/ClosedException.h>
#include <zapata/exceptions/InterruptedException.h>
#include <zapata/parsers/json.h>
#include <zapata/log/log.h>
#include <sys/sem.h>
#include <zapata/text/convert.h>
#include <zapata/file/manip.h>
#include <zapata/thread/RESTServer.h>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <zapata/api/codes_rest.h>

zapata::RESTServer::RESTServer(string _config_file) {
	ifstream _key_file;
	_key_file.open(_config_file.data());
	this->__conf_file.assign(_config_file);
	this->__configuration = zapata::fromfile(_key_file);

	this->__n_jobs = 0;
	this->__next = 0;

	if (!!this->configuration()["zapata"]["core"]["max_jobs"] && ((int) this->configuration()["zapata"]["core"]["max_jobs"]) != -1) {
		this->max((size_t) this->configuration()["zapata"]["core"]["max_jobs"]);
	}
	else {
		this->max(1);
	}

	if (!this->configuration()["zapata"]["core"]["max_connections_per_job"]->ok() || ((int) this->configuration()["zapata"]["core"]["max_connections_per_job"]) <= 0) {
		((zapata::JSONObj&) this->configuration()["zapata"]["core"]) << "max_connections_per_job" << 1024;
	}

	if (!!this->configuration()["zapata"]["core"]["log"]["level"]) {
		zapata::log_lvl = (int) this->configuration()["zapata"]["core"]["log"]["level"];
	}
	if (!!this->configuration()["zapata"]["core"]["log"]["file"]) {
		zapata::log_fd = new ofstream();
		((ofstream*) zapata::log_fd)->open(((string) this->configuration()["zapata"]["core"]["log"]["file"]).data());
	}

	this->__pool.configuration(& this->__configuration);

	for (auto _i : * this->__configuration) {
		string _key = _i.first;
		JSONElement _value = _i.second;
		if (_key != "zapata") {
			
			string _lib_file("lib");
			_lib_file.append((string) _value["lib"]);
			_lib_file.append(".so");

			if (_lib_file.length() > 6) {
				void *hndl = dlopen(_lib_file.data(), RTLD_NOW);
				if (hndl == nullptr) {
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
		/*
		 *  definition of handlers for the file upload controller
		 *  registered as a Controller
		 */
		this->__pool.on(zapata::HTTPPost, "^/file/upload$", [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
			string _body = _req->body();
			assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

			assertz(_req->header("Content-Type").find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

			zapata::JSONObj& _params = zapata::fromstr(_body);

			assertz(!!_params["uploaded_file"], "The 'uploaded_file' parameter must be provided.", zapata::HTTP412, zapata::ERRRequiredField);

			string _from((string) _params["uploaded_file"]);
			string _to((string) _config["zapata"]["rest"]["uploads"]["upload_path"]);
			zapata::normalize_path(_to, true);

			string _originalname(_req->header("X-Original-Filename"));
			string _path;
			string _name;
			string _mime;
			do {
				string _dir(_to);
				_path.assign("");
				zapata::generate_hash(_path);
				_dir.insert(_dir.length(), _path);
				zapata::mkdir_recursive(_dir, 0777);
				_path.insert(_path.length(), "/");

				if (_originalname.length() != 0) {
					_path.insert(_path.length(), _originalname);

					_mime.assign(_req->header("X-Original-Mimetype"));
					if (_mime.length() != 0) {
						_mime.assign(_mime.substr(0, _mime.find(";")));
					}
				}
				else {
					zapata::MIMEType _m = zapata::get_mime((string) _params["uploaded_file"]);
					_path.insert(_path.length(), "_uploaded");
					_path.insert(_path.length(), zapata::mimetype_extensions[_m]);

					_mime.assign(zapata::mimetype_names[_m]);
				}

				_name.assign(_path);
				_path.insert(0, _to);
			}
			while (zapata::path_exists(_path));

			string _encoding(_req->header("X-Content-Transfer-Encoding"));
			transform(_encoding.begin(), _encoding.end(), _encoding.begin(), ::toupper);
			if (_encoding == "BASE64") {
				fstream _ifs;
				_ifs.open(_from.data());
				fstream _ofs;
				_ofs.open(_path.data());

				zapata::base64::decode(_ifs, _ofs);

				_ifs.close();
				_ofs.flush();
				_ofs.close();
			}
			else {
				assertz(zapata::copy_path((string ) _params["uploaded_file"], _path), "There was an error copying the temporary file to the 'upload_path' directory.", zapata::HTTP500, zapata::ERRFilePermissions);
			}

			string _location((string) _config["zapata"]["rest"]["uploads"]["upload_url"]);
			zapata::normalize_path(_location, true);
			_location.insert(_location.length(), _name);

			_rep->status(zapata::HTTP201);
			_rep->header( "X-File-Mimetype", _mime);
			_rep->header("Location", _location);
		});

		/*
		 *  definition of handlers for the file upload removal controller
		 *  registered as a Controller
		 */
		this->__pool.on(zapata::HTTPPost, "^/file/remove$", [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
			string _body = _req->body();
			assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

			assertz(_req->header("Content-Type").find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

			zapata::JSONObj& _params = zapata::fromstr(_body);

			assertz(!!_params["file_url"], "The 'file_url' parameter must be provided.", zapata::HTTP412, zapata::ERRRequiredField);

			string _url((string) _params["file_url"]);
			string _url_root(_config["zapata"]["rest"]["uploads"]["upload_url"]);
			string _path_root(_config["zapata"]["rest"]["uploads"]["upload_path"]);

			zapata::replace(_url, _url_root, _path_root);

			assertz(zapata::path_exists(_url), "Couldn't find the designated file", zapata::HTTP404, zapata::ERRFileNotFound);

			_url.assign(_url.substr(0, _url.rfind("/")));

			string _cmd("rm -rf ");
			_cmd.insert(_cmd.length(), _url);
			_cmd.insert(_cmd.length(), " > /dev/null");
			assertz(system(_cmd.data()) == 0, "There was an error removing the uploaded file.", zapata::HTTP500, zapata::ERRFilePermissions);

			_rep->status(zapata::HTTP200);
		});
	}

	unsigned int _port =  (unsigned int) this->configuration()["zapata"]["rest"]["port"];
	string _text("starting RESTful server on port ");
	zapata::tostr(_text, _port);
	zapata::log(_text, zapata::sys);
	this->__ss.bind(_port);

	for (int i = 0; i != this->max(); i++) {
		zapata::RESTJob * _job = new RESTJob(_config_file);
		this->__jobs.push_back(_job);
		(* _job).pool(& this->__pool);
		(* _job)->start();
	}
}

zapata::RESTServer::~RESTServer(){
}

void zapata::RESTServer::wait() {
	int _cs_fd = -1;
	this->__ss.accept(&_cs_fd);

	size_t _next = this->next();
	(* this->__jobs.at(_next)).assign(_cs_fd);
}

void zapata::RESTServer::start() {
	try {
		for (; true; ) {
			if (this->max() != 0 && this->__next == this->max()) {
				this->__next = 0;
			}
			this->wait();
		}
	}
	catch (zapata::InterruptedException& e) {
		return;
	}
}

zapata::RESTPool& zapata::RESTServer::pool() {
	return this->__pool;
}

zapata::JSONObj& zapata::RESTServer::configuration() {
	return this->__configuration;
}

void zapata::RESTServer::notify() {

}

size_t zapata::RESTServer::max() {
	return this->__max_idx;
}

size_t zapata::RESTServer::next() {
	return this->__next++;
}

void zapata::RESTServer::max(size_t _max) {
	this->__max_idx = _max;
}