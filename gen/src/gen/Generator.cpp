/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

#include <zapata/exceptions/ClosedException.h>
#include <zapata/exceptions/InterruptedException.h>
#include <zapata/json.h>
#include <zapata/log/log.h>
#include <sys/sem.h>
#include <zapata/text/convert.h>
#include <zapata/file/manip.h>
#include <zapata/gen/Generator.h>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <string>
#include <vector>

std::string zpt::Generator::datum_includes;
std::map<std::string, zpt::gen::datum> zpt::Generator::datums;
std::map<std::string, zpt::gen::resource> zpt::Generator::resources;
std::map<std::string, std::string> zpt::Generator::alias;

zpt::GeneratorPtr::GeneratorPtr(zpt::json _options) : std::shared_ptr<zpt::Generator>(new zpt::Generator(_options)) {
}

zpt::GeneratorPtr::GeneratorPtr(zpt::Generator* _ptr) : std::shared_ptr<zpt::Generator>(_ptr) {
}

zpt::GeneratorPtr::~GeneratorPtr() {
}

auto zpt::GeneratorPtr::launch(int argc, char* argv[]) -> int {
	zpt::json _options = zpt::conf::gen::init(argc, argv);
	std::ifstream _ifs;
	if (_options["c"]->type() == zpt::JSString) {
		_ifs.open(_options["c"]->str().data());
	}
	else {
		_ifs.open(".zpt_rc");
	}
	if (_ifs.is_open()) {
		zpt::json _conf_options;
		_ifs >> _conf_options;
		_options = _conf_options + _options;
	}
	zpt::conf::setup(_options);
	zpt::gen::worker _gen(_options);
	_gen->build();
	return 0;
}

zpt::Generator::Generator(zpt::json _options) : __options(_options), __specs(zpt::json::object()) {	
}

zpt::Generator::~Generator() {
}

auto zpt::Generator::options() -> zpt::json {
	return this->__options;
}

auto zpt::Generator::load() -> void {
	if (!this->__options["files"]->ok()) {
		return;
	}
	for (auto _file : this->__options["files"]->arr()) {
		std::ifstream _ifs(_file->str().data());
		zpt::json _spec;
		_ifs >> _spec;

		if (this->__specs[std::string(_spec["lib"])]->ok()) {
			this->__specs << std::string(_spec["lib"]) << (this->__specs[std::string(_spec["lib"])] + _spec);
		}
		else {
			this->__specs << std::string(_spec["lib"]) << _spec;
		}
		_spec << "dbms" << zpt::json::object();

		if (_spec["resources"]->type() == zpt::JSArray) {
			for (auto _resource : _spec["resources"]->arr()) {
				_resource << "namespace" << _spec["namespace"] << "lib" << _spec["lib"] << "spec_name" << _spec["name"];
				zpt::Generator::resources.insert(std::make_pair(std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]), zpt::gen::resource(new zpt::GenResource(_resource, this->__options))));
				if (_resource["datum"]["href"]->ok()) {
					zpt::Generator::alias.insert(std::make_pair(zpt::gen::url_pattern_to_regexp(std::string(_resource["topic"])), zpt::gen::url_pattern_to_regexp(std::string(_resource["datum"]["href"]))));
				}
			}
		}
		if (_spec["datums"]->type() == zpt::JSArray) {
			for (auto _datum : _spec["datums"]->arr()) {
				_datum << "namespace" << _spec["namespace"] << "lib" << _spec["lib"] << "spec_name" << _spec["name"];
				zpt::Generator::datums.insert(std::make_pair(std::string(_datum["namespace"]) + std::string("::") + std::string(_datum["name"]), zpt::gen::datum(new zpt::GenDatum(_datum, this->__options))));
				if (_datum["dbms"]->type() == zpt::JSArray) {
					for (auto _dbms : _datum["dbms"]->arr()) {
						_spec["dbms"] << std::string(_dbms) << true;
					}
				}

				std::string _h_file = std::string(this->__options["data-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/") + std::string(_datum["name"]) + std::string(".h");
				std::string _include = zpt::r_replace(_h_file, std::string(this->__options["prefix-h"][0]), "");
				if (_include.front() == '/') {
					_include.erase(0, 1);
				}
				zpt::Generator::datum_includes += std::string("#include <") + _include + std::string(">\n");
			}
		}
	}
}

auto zpt::Generator::build() -> void {
	if (!this->__options["files"]->ok()) {
		return;
	}
	this->load();
	if (this->__options["data-out-cxx"]->ok() && this->__options["data-out-h"]->ok()) {
		this->build_data_layer();
	}
	if (this->__options["resource-out-cxx"]->ok() && this->__options["resource-out-h"]->ok()) {
		this->build_container();
	}
	if (this->__options["mutations-out-cxx"]->ok() && this->__options["mutations-out-h"]->ok()) {
		this->build_mutations();
	}	
}

auto zpt::Generator::build_data_layer() -> void {
	for (auto _pair : this->__specs->obj()) {
		zpt::json _spec = _pair.second;
		
		zpt::mkdir_recursive(std::string(this->__options["data-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/"));
		zpt::mkdir_recursive(std::string(this->__options["data-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/"));

		if (_spec["datums"]->type() == zpt::JSArray) {		
			for (auto _datum : _spec["datums"]->arr()) {
				std::string _h_file = std::string(this->__options["data-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/") + std::string(_datum["name"]) + std::string(".h");
				std::string _cxx_file = std::string(this->__options["data-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/") + std::string(_datum["name"]) + std::string(".cpp");
				std::string _sql_file = std::string(this->__options["data-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/") + std::string(_datum["name"]) + std::string(".sql");
				std::string _datum_h;
				zpt::load_path("/usr/share/zapata/gen/Datum.h", _datum_h);
				std::string _datum_cxx;
				zpt::load_path("/usr/share/zapata/gen/Datum.cpp", _datum_cxx);
				std::string _datum_sql;
				zpt::load_path("/usr/share/zapata/gen/Datum.sql", _datum_sql);

				zpt::replace(_datum_h, "$[data.path.h]", zpt::Generator::datum_includes);

				if (_datum["fields"]->type() == zpt::JSObject){
					std::string _fields;
					_fields += std::string("id char(36) not null primary key,\n");
					_fields += std::string("created timestamp not null,\n");
					_fields += std::string("updated timestamp not null,\n");
					_fields += std::string("href varchar(1024) not null");
					for (auto _f : _datum["fields"]->obj()) {
						if (_f.second["ref"]->ok()) {
							continue;
						}
						_fields += std::string(",\n") + std::string(_f.first) + std::string(" ") + zpt::GenDatum::get_type(_f.second) + std::string(" ") + zpt::GenDatum::get_restrictions(_f.second);
					}
					zpt::replace(_datum_sql, "$[datum.fields]", _fields);
				}
				
				auto _found = zpt::Generator::datums.find(std::string(_datum["namespace"]) + std::string("::") + std::string(zpt::r_replace(_datum["name"]->str(), "-", "_")));
				if (_found != zpt::Generator::datums.end()) {
					zpt::replace(_datum_cxx, "$[datum.extends.get]", _found->second->build_extends_get());
					zpt::replace(_datum_cxx, "$[datum.extends.query]", _found->second->build_extends_query());
					zpt::replace(_datum_cxx, "$[datum.extends.insert]", _found->second->build_extends_insert());
					zpt::replace(_datum_cxx, "$[datum.extends.save]", _found->second->build_extends_save());
					zpt::replace(_datum_cxx, "$[datum.extends.set.topic]", _found->second->build_extends_set_topic());
					zpt::replace(_datum_cxx, "$[datum.extends.set.pattern]", _found->second->build_extends_set_pattern());
					zpt::replace(_datum_cxx, "$[datum.extends.remove.topic]", _found->second->build_extends_remove_topic());
					zpt::replace(_datum_cxx, "$[datum.extends.remove.pattern]", _found->second->build_extends_remove_pattern());
				}
			
				std::string _namespace = std::string(_spec["namespace"]);
				std::string _collection = std::string(_datum["name"]);
				zpt::replace(_datum_h, "$[datum.collection]", _collection);
				zpt::replace(_datum_cxx, "$[datum.collection]", _collection);

				zpt::replace(_datum_h, "$[datum.name]", std::string(zpt::r_replace(_datum["name"]->str(), "-", "_")));
				zpt::replace(_datum_cxx, "$[datum.name]", std::string(zpt::r_replace(_datum["name"]->str(), "-", "_")));
				zpt::replace(_datum_sql, "$[datum.name]", std::string(zpt::r_replace(_datum["name"]->str(), "-", "_")));
		
				zpt::json _namespaces = zpt::split(_namespace, "::");
				std::string _namespaces_begin;
				std::string _namespaces_end;
				for (auto _part : _namespaces->arr()) {
					_namespaces_begin += std::string("namespace ") + std::string(zpt::r_replace(_part, "-", "_")) + std::string(" {\n");
					_namespaces_end += "}\n";
				}
				zpt::replace(_datum_h, "$[namespaces.begin]", _namespaces_begin);
				zpt::replace(_datum_h, "$[namepsaces.end]", _namespaces_end);
				zpt::replace(_datum_cxx, "$[namespaces.begin]", _namespaces_begin);
				zpt::replace(_datum_cxx, "$[namepsaces.end]", _namespaces_end);

				std::string _include_path = zpt::r_replace(_h_file.data(), std::string(this->__options["prefix-h"][0]), "");
				if (_include_path.front() == '/') {
					_include_path.erase(0, 1);
				}
				zpt::replace(_datum_cxx, "$[data.path.self.h]", std::string("#include <") + _include_path + std::string(">"));

				zpt::replace(_datum_h, "$[namespace]", _namespace);
				zpt::replace(_datum_cxx, "$[namespace]", _namespace);
			
				std::string _get_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "redis", "mongodb", "postgresql", "mariadb" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.get.client]", _get_client);
				zpt::replace(_datum_cxx, "$[datum.method.get.client]", _get_client);
		
				std::string _query_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "mongodb", "postgresql", "mariadb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.query.client]", _query_client);
				zpt::replace(_datum_cxx, "$[datum.method.query.client]", _query_client);

				std::string _insert_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "mongodb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.insert.client]", _insert_client);
				zpt::replace(_datum_cxx, "$[datum.method.insert.client]", _insert_client);
		
				std::string _save_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "mongodb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.save.client]", _save_client);
				zpt::replace(_datum_cxx, "$[datum.method.save.client]", _save_client);

				std::string _set_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "mongodb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.set.client]", _set_client);
				zpt::replace(_datum_cxx, "$[datum.method.set.client]", _set_client);

				std::string _remove_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "mongodb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.remove.client]", _remove_client);
				zpt::replace(_datum_cxx, "$[datum.method.remove.client]", _remove_client);
			
				if (_found != zpt::Generator::datums.end()) {
					zpt::replace(_datum_cxx, "$[datum.relations.get]", _found->second->build_associations_get());
					zpt::replace(_datum_cxx, "$[datum.relations.query]", _found->second->build_associations_query());
					zpt::replace(_datum_cxx, "$[datum.relations.insert]", _found->second->build_associations_insert());
					zpt::replace(_datum_cxx, "$[datum.relations.save]", _found->second->build_associations_save());
					zpt::replace(_datum_cxx, "$[datum.relations.set]", _found->second->build_associations_set());
					zpt::replace(_datum_cxx, "$[datum.relations.remove]", _found->second->build_associations_remove());
				}

				struct stat _buffer;
				bool _cxx_exists = stat(_cxx_file.c_str(), &_buffer) == 0;
				bool _h_exists = stat(_h_file.c_str(), &_buffer) == 0;
				bool _sql_exists = stat(_sql_file.c_str(), &_buffer) == 0;
				if (bool(this->__options["force-data"][0]) || (!bool(this->__options["force-data"][0]) && !_h_exists)) {
					std::ofstream _h_ofs(_h_file.data());
					_h_ofs << _datum_h << endl << flush;
					_h_ofs.close();
					zlog(std::string("processed ") + _h_file, zpt::trace);
				}
				if (bool(this->__options["force-data"][0]) || (!bool(this->__options["force-data"][0]) && !_cxx_exists)) {
					std::ofstream _cxx_ofs(_cxx_file.data());
					_cxx_ofs << _datum_cxx << endl << flush;
					_cxx_ofs.close();
					zlog(std::string("processed ") + _cxx_file, zpt::trace);
				}
				if (bool(this->__options["force-data"][0]) || (!bool(this->__options["force-data"][0]) && !_sql_exists)) {
					std::ofstream _sql_ofs(_sql_file.data());
					_sql_ofs << _datum_sql << endl << flush;
					_sql_ofs.close();
					zlog(std::string("processed ") + _sql_file, zpt::trace);
				}
			}
		}
	}
}

auto zpt::Generator::build_container() -> void {
	for (auto _pair : this->__specs->obj()) {
		zpt::json _spec = _pair.second;
		bool _handlers_built = false;

		if (!this->__options["resource-out-lang"]->ok() || std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("c++")) != std::end(this->__options["resource-out-lang"]->arr())) {
			std::string _container_cxx;
			zpt::load_path("/usr/share/zapata/gen/Container.cpp", _container_cxx);

			zpt::mkdir_recursive(std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_spec["name"]));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/collections/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/stores/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/documents/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/controllers/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/mutations/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/collections/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/stores/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/documents/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/controllers/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/mutations/"));
		
			std::string _cxx_file = std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/api.cpp");
			std::string _h_am_file = std::string(this->__options["prefix-h"][0]) + std::string("/add_to_am_from_") + zpt::r_replace(std::string(_spec["name"]), "/", "_") + std::string(".mk");
			std::string _am_file = std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/Makefile.am");
		
			std::string _connectors_initialize("{ ");
			for (auto _dbms : _spec["dbms"]->obj()) {
				_connectors_initialize += zpt::GenDatum::build_initialization(_dbms.first, std::string(_spec["namespace"]));
			}
			_connectors_initialize += "}";
			zpt::replace(_container_cxx, "$[datum.connectors.initialize]", _connectors_initialize);

			zpt::replace(_container_cxx, "$[namespace]", std::string(_spec["namespace"]));

			std::string _h_make_files;
			std::string _make_files;
			std::string _child_includes;
			std::string _registry;
			std::string _dyn_link;
			std::string _dyn_dir;
			if (_spec["datums"]->type() == zpt::JSArray) {
				for (auto _datum : _spec["datums"]->arr()) {
					if (_datum["name"]->ok()) {
						std::string _include(std::string(this->__options["data-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/") + std::string(_datum["name"]) + std::string(".h"));
						zpt::replace(_include, std::string(this->__options["prefix-h"][0]), "");
						if (_include.front() == '/') {
							_include.erase(0, 1);
						}
						_child_includes += std::string("#include <") + _include + std::string(">\n");
						_h_make_files += std::string("./") + _include + std::string(" \\\n");

						_include.assign(std::string(this->__options["data-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/mutations/") + std::string(_datum["name"]) + std::string(".h"));
						zpt::replace(_include, std::string(this->__options["prefix-h"][0]), "");
						if (_include.front() == '/') {
							_include.erase(0, 1);
						}
						_child_includes += std::string("#include <") + _include + std::string(">\n");
						_h_make_files += std::string("./") + _include + std::string(" \\\n");

						std::string _make_file(std::string("./datums/") + std::string(_datum["name"]) + std::string(".cpp \\\n"));
						_make_files += _make_file;
						_make_file.assign(std::string("./mutations/") + std::string(_datum["name"]) + std::string(".cpp \\\n"));
						_make_files += _make_file;
						if (_datum["dynlink"]->ok()) {
							_dyn_link += std::string(_datum["dynlink"]);
						}
						if (_datum["dyndir"]->ok()) {
							_dyn_dir += std::string(_datum["dyndir"]);
						}
					}
				}
				for (auto _datum : _spec["datums"]->arr()) {
					if (_datum["name"]->ok()) {
						std::string _key = std::string(_datum["namespace"]) + std::string("::") + std::string(_datum["name"]);
						_registry += zpt::Generator::datums.find(_key)->second->build_mutations(std::string(_spec["name"]), _child_includes);
					}
				}
			}
		
			std::string _includes;
			if (_spec["resources"]->type() == zpt::JSArray) {
				_handlers_built = true;
				for (auto _resource : _spec["resources"]->arr()) {
					std::string _key = std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]);

					std::string _include(std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/") + std::string(_resource["type"]) + std::string("s/") + std::string(_resource["name"]) + std::string(".h"));
					zpt::replace(_include, std::string(this->__options["prefix-h"][0]), "");
					if (_include.front() == '/') {
						_include.erase(0, 1);
					}
					_includes += std::string("#include <") + _include + std::string(">\n");
					_registry += zpt::Generator::resources.find(_key)->second->build_handlers(std::string(_spec["name"]), _child_includes);

					_h_make_files += std::string("./") + _include + std::string(" \\\n");
					std::string _make_file(std::string("./") + std::string(_resource["type"]) + std::string("s/") + std::string(_resource["name"]) + std::string(".cpp \\\n"));
					_make_files += _make_file;
				}
			}		
			zpt::replace(_container_cxx, "$[resource.path.h]", _includes);
			zpt::replace(_container_cxx, "$[resource.handlers.delegate]", _registry);
			zpt::replace(_container_cxx, "_emitter->connector({ });", "");

			struct stat _buffer;
			bool _cxx_exists = stat(_cxx_file.c_str(), &_buffer) == 0;
			bool _am_exists = stat(_am_file.c_str(), &_buffer) == 0;
			if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_cxx_exists)) {
				std::ofstream _cxx_ofs(_cxx_file.data());
				_cxx_ofs << _container_cxx << endl << flush;
				_cxx_ofs.close();
				zlog(std::string("processed ") + _cxx_file, zpt::trace);	
			}

			size_t _cxx_out_split = zpt::split(std::string(this->__options["resource-out-cxx"][0]), "/")->arr()->size() + zpt::split(std::string(_spec["name"]), "/")->arr()->size();
			std::string _parent_dir;
			for (size_t _i = 0; _i != _cxx_out_split; _i++) _parent_dir += "../";
			std::string _make;
			std::string _lib_escaped = zpt::r_replace(std::string(_spec["lib"]), "-", "_");
			_make += std::string("lib_LTLIBRARIES = lib") + std::string(_spec["lib"]) + std::string(".la\n\n");
			_make += std::string("lib") + _lib_escaped + std::string("_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest -lzapata-postgresql -lzapata-mariadb -lzapata-mongodb -lzapata-redis") + _dyn_link + std::string("\n");
			_make += std::string("lib") + _lib_escaped + std::string("_la_LDFLAGS = -version-info 0:1:0") + _dyn_dir + std::string("\n");
			_make += std::string("lib") + _lib_escaped + std::string("_la_CPPFLAGS = -O3 -std=c++14 -I") + _parent_dir + std::string("include\n\n");
			_make += std::string("lib") + _lib_escaped + std::string("_la_SOURCES = \\\n");
			_make += _make_files;
			_make += std::string("./api.cpp\n");
			if (bool(this->__options["force-makefile"][0]) || (!bool(this->__options["force-makefile"][0]) && !_am_exists)) {
				std::ofstream _am_ofs(_am_file.data());
				_am_ofs << _make << endl << flush;
				_am_ofs.close();
				zlog(std::string("processed ") + _am_file, zpt::trace);
				std::ofstream _h_am_ofs(_h_am_file.data());
				_h_am_ofs << _h_make_files << endl << flush;
				_h_am_ofs.close();
			}
		}
		if (this->__options["resource-out-lang"]->ok() && std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("lisp")) != std::end(this->__options["resource-out-lang"]->arr())) {
			std::string _container_lisp;
			zpt::load_path("/usr/share/zapata/gen/Container.lisp", _container_lisp);

			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/collections/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/stores/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/documents/"));
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/controllers/"));
		
			std::string _lisp_file = std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/api.lisp");
				
			std::string _includes;
			if (_spec["resources"]->type() == zpt::JSArray) {
				for (auto _resource : _spec["resources"]->arr()) {
					std::string _key = std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]);

					std::string _include = std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/") + std::string(_resource["type"]) + std::string("s/") + std::string(_resource["name"]) + std::string(".lisp");
					zpt::replace(_include, std::string(this->__options["prefix-scripts"][0]), "");
					if (_include.front() == '/') {
						_include.erase(0, 1);
					}
					_includes += std::string("(load \"/usr/share/") + _include + std::string("\")\n");
					if (!_handlers_built) {
						zpt::Generator::resources.find(_key)->second->build_handlers(std::string(_spec["name"]), "");
					}
				}
				_handlers_built = true;
			}		
			zpt::replace(_container_lisp, "$[resource.path.h]", _includes);

			struct stat _buffer;
			bool _lisp_exists = stat(_lisp_file.c_str(), &_buffer) == 0;
			if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_lisp_exists)) {
				std::ofstream _lisp_ofs(_lisp_file.data());
				_lisp_ofs << _container_lisp << endl << flush;
				_lisp_ofs.close();
				zlog(std::string("processed ") + _lisp_file, zpt::trace);	
			}
		}
		if (this->__options["resource-out-lang"]->ok() && std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("python")) != std::end(this->__options["resource-out-lang"]->arr())) {
			std::string _container_py;
			zpt::load_path("/usr/share/zapata/gen/Container.py", _container_py);

			std::ofstream _py_ofs;
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]));
			_py_ofs.open((std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/__init__.py")).data());
			_py_ofs << endl << flush;
			_py_ofs.close();
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/collections/"));
			_py_ofs.open((std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/collections/__init__.py")).data());
			_py_ofs << endl << flush;
			_py_ofs.close();
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/stores/"));
			_py_ofs.open((std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/stores/__init__.py")).data());
			_py_ofs << endl << flush;
			_py_ofs.close();
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/documents/"));
			_py_ofs.open((std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/documents/__init__.py")).data());
			_py_ofs << endl << flush;
			_py_ofs.close();
			zpt::mkdir_recursive(std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/controllers/"));
			_py_ofs.open((std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/controllers/__init__.py")).data());
			_py_ofs << endl << flush;
			_py_ofs.close();
		
			std::string _py_file = std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/api.py");
				
			std::string _includes;
			if (_spec["resources"]->type() == zpt::JSArray) {
				for (auto _resource : _spec["resources"]->arr()) {
					std::string _key = std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]);

					std::string _include = std::string(zpt::r_replace(_spec["name"]->str(), "-", "_")) + std::string(" import ") + std::string(_resource["type"]) + std::string("s.") + std::string(zpt::r_replace(_resource["name"]->str(), "-", "_")) + std::string("");
					if (_include.front() == '/') {
						_include.erase(0, 1);
					}
					_includes += std::string("from ") + _include + std::string("\n");
					if (!_handlers_built) {
						zpt::Generator::resources.find(_key)->second->build_handlers(std::string(_spec["name"]), "");
					}
				}
				_handlers_built = true;
			}		
			zpt::replace(_container_py, "$[resource.path.h]", _includes);

			struct stat _buffer;
			bool _py_exists = stat(_py_file.c_str(), &_buffer) == 0;
			if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_py_exists)) {
				_py_ofs.open(_py_file.data());
				_py_ofs << _container_py << endl << flush;
				_py_ofs.close();
				zlog(std::string("processed ") + _py_file, zpt::trace);	
			}
		}
	}
	
	//zlog(_handler_cxx, zpt::trace);
}

auto zpt::Generator::build_mutations() -> void {
}

zpt::GenDatum::GenDatum(zpt::json _spec, zpt::json _options) : __spec(_spec), __options(_options) {
}

zpt::GenDatum::~GenDatum(){
}

auto zpt::GenDatum::spec() -> zpt::json {
	return this->__spec;
}

auto zpt::GenDatum::build() -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_data_layer() -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_params(zpt::json _rel, bool _multi) -> std::string {
	std::string _params;
	for (auto _param : _rel->obj()) {
		if (_params.length() != 0) {
			_params += std::string(", ");
		}
		if (_multi) {
			_params += std::string("\"") + _param.first + std::string("\", ") + (_param.second->str().front() == '{' ? std::string("std::string(_d_element[\"") + _param.second->str().substr(1, _param.second->str().length() - 2) + std::string("\"])") : std::string("\"") + _param.second->str() + std::string("\""));
		}
		else {
			_params += std::string("\"") + _param.first + std::string("\", ") + (_param.second->str().front() == '{' ? std::string("std::string(_r_data[\"") + _param.second->str().substr(1, _param.second->str().length() - 2) + std::string("\"])") : std::string("\"") + _param.second->str() + std::string("\""));
		}
	}
	return _params;
}

auto zpt::GenDatum::build_inverted_params(zpt::json _rel) -> std::string {
	std::string _params;
	for (auto _param : _rel->obj()) {
		if (_param.second->str().front() == '{') {
			if (_params.length() != 0) {
				_params += std::string(", ");
			}
			_params += std::string("\"") + _param.second->str().substr(1, _param.second->str().length() - 2) + std::string("\", ") + std::string("std::string(_r_element[\"") + _param.first + std::string("\"])");
		}
	}
	return _params;
}

auto zpt::GenDatum::build_topic(zpt::json _topic) -> std::string {
	std::string _parts;
	for (auto _part : _topic->arr()) {
		if (_parts.length() != 0) {
			_parts += std::string(", ");
		}
		_parts += (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
	}
	return _parts;
}

auto zpt::GenDatum::build_dbms() -> std::string {
	std::string _dbmss;
	if (this->__spec["dbms"]->type() == zpt::JSArray) {
		for (auto _dbms : this->__spec["dbms"]->arr()) {
			if (_dbms == zpt::json::string("postgresql") || _dbms == zpt::json::string("mariadb")) {
				continue;
			}
			if (_dbmss.length() != 0) {
				_dbmss += ", ";
			}
			_dbmss += std::string("\"") + zpt::GenDatum::build_data_client(this->__spec["dbms"], { zpt::array, _dbms->str() }, std::string(this->__spec["namespace"])) + std::string("\"");
		}
	}
	return _dbmss;
}

auto zpt::GenDatum::build_get(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_"));
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::datums::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::query(_topic, _envelope[\"params\"], _emitter, _identity, _envelope);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::get(_topic, _emitter, _identity, _envelope);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_post(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_"));
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::datums::") + _name;
	if (std::string(_resource["type"]) == "collection") {
		_return += std::string("_r_body = ") + _class + std::string("::insert(_topic, _envelope[\"payload\"], _emitter, _identity, _envelope);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_put(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_"));
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::datums::") + _name;
	if (std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::insert(_topic, _envelope[\"payload\"], _emitter, _identity, _envelope);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::save(_topic, _envelope[\"payload\"], _emitter, _identity, _envelope);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_patch(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_"));
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::datums::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::set(_topic, _envelope[\"payload\"], _envelope[\"params\"], _emitter, _identity, _envelope);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::set(_topic, _envelope[\"payload\"], _emitter, _identity, _envelope);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_delete(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_"));
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::datums::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::remove(_topic, _envelope[\"params\"], _emitter, _identity, _envelope);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::remove(_topic, _emitter, _identity, _envelope);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_head(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_"));
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::datums::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::query(_topic, _envelope[\"params\"], _emitter, _identity, _envelope);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::get(_topic, _emitter, _identity, _envelope);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_mutations(std::string _parent_name, std::string _child_includes) -> std::string {
	std::string _return;
	if (!this->__options["resource-out-lang"]->ok() || std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("c++")) != std::end(this->__options["resource-out-lang"]->arr())) {
		std::string _h_file = std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_parent_name) + std::string("/mutations/") + std::string(this->__spec["name"]) + std::string(".h");
 		std::string _cxx_file = std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_parent_name) + std::string("/mutations/") + std::string(this->__spec["name"]) + std::string(".cpp");

		std::string _mutation_h;
		zpt::load_path("/usr/share/zapata/gen/Mutations.h", _mutation_h);
		std::string _mutation_cxx;
		zpt::load_path("/usr/share/zapata/gen/Mutations.cpp", _mutation_cxx);

		std::string _include = zpt::r_replace(_h_file, std::string(this->__options["prefix-h"][0]), "");
		if (_include.front() == '/') {
			_include.erase(0, 1);
		}
		zpt::replace(_mutation_h, "$[data.path.h]", _child_includes);
		zpt::replace(_mutation_cxx, "$[mutation.path.self.h]", std::string("#include <") + _include + std::string(">"));
	
		zpt::replace(_mutation_h, "$[namespace]", std::string(this->__spec["namespace"]));
		zpt::replace(_mutation_cxx, "$[namespace]", std::string(this->__spec["namespace"]));

		zpt::json _namespaces = zpt::split(std::string(this->__spec["namespace"]), "::");
		std::string _namespaces_begin;
		std::string _namespaces_end;
		for (auto _part : _namespaces->arr()) {
			_namespaces_begin += std::string("namespace ") + std::string(_part) + std::string(" {\n");
			_namespaces_end += "}\n";
		}
		_namespaces_begin += std::string("namespace mutations {\n");
		_namespaces_end += "}\n";
		_namespaces_begin += std::string("namespace ") + std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")) + std::string(" {\n");
		_namespaces_end += "}\n";
	
		zpt::replace(_mutation_h, "$[namespaces.begin]", _namespaces_begin);
		zpt::replace(_mutation_h, "$[namepsaces.end]", _namespaces_end);
		zpt::replace(_mutation_cxx, "$[namespaces.begin]", _namespaces_begin);
		zpt::replace(_mutation_cxx, "$[namepsaces.end]", _namespaces_end);

		zpt::replace(_mutation_h, "$[mutation.name]", std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")));
		zpt::replace(_mutation_cxx, "$[mutation.name]", std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")));

		zpt::replace(_mutation_cxx, "$[mutation.topic.self.regex]", zpt::gen::url_pattern_to_regexp(this->__spec["name"]->str()));
		std::string _mutation;
		bool _first = true;
		_mutation.assign(this->build_insert());
		zpt::replace(_mutation_cxx, "$[mutation.handler.self.insert]", _mutation);
		_first = _mutation.length() == 0;
		_mutation.assign(this->build_update());
		if (_mutation.length() != 0 && !_first) {
			_first = false;
			_mutation = std::string(",\n") + _mutation;
		}
		zpt::replace(_mutation_cxx, "$[mutation.handler.self.update]", _mutation);
		_mutation.assign(this->build_remove());
		if (_mutation.length() != 0 && !_first) {
			_first = false;
			_mutation = std::string(",\n") + _mutation;
		}		
		zpt::replace(_mutation_cxx, "$[mutation.handler.self.remove]", _mutation);
		_mutation.assign(this->build_replace());
		if (_mutation.length() != 0 && !_first) {
			_first = false;
			_mutation = std::string(",\n") + _mutation;
		}		
		zpt::replace(_mutation_cxx, "$[mutation.handler.self.replace]", _mutation);
		
		size_t _begin = _mutation_cxx.find("$[mutation.handler.begin]") + 25;
		size_t _end = _mutation_cxx.find("$[mutation.handler.end]");
		std::string _on = _mutation_cxx.substr(_begin, _end - _begin);
		_mutation_cxx.erase(_begin, _end - _begin + 23);

		std::string _mutation_ons;
		for (auto _field : this->__spec["fields"]->obj()) {
			zpt::json _opts = zpt::gen::get_opts(_field.second);
			if (!_field.second["ref"]->ok() || !_opts["publish"]->ok()) {
				continue;
			}

			std::string _mutation_on(_on.data());
			std::string _topic = zpt::gen::url_pattern_to_regexp(_field.second["ref"]->str());
			auto _found = zpt::Generator::alias.find(_topic);
			_topic = (_found != zpt::Generator::alias.end() ? _found->second : _topic);
			zpt::replace(_mutation_on, "$[mutation.topic.regex]", _topic);
			std::string _mutation;
			bool _first = true;
			_mutation.assign(this->build_associations_insert(_field.first, _field.second));
			zpt::replace(_mutation_on, "$[mutation.handler.insert]", _mutation);
			_first = _mutation.length() == 0;
			_mutation.assign(this->build_associations_update(_field.first, _field.second));
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string(",\n") + _mutation;
			}
			zpt::replace(_mutation_on, "$[mutation.handler.update]", _mutation);
			_mutation.assign(this->build_associations_remove(_field.first, _field.second));
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string(",\n") + _mutation;
			}		
			zpt::replace(_mutation_on, "$[mutation.handler.remove]", _mutation);
			_mutation.assign(this->build_associations_replace(_field.first, _field.second));
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string(",\n") + _mutation;
			}		
			zpt::replace(_mutation_on, "$[mutation.handler.replace]", _mutation);

			zpt::json _resource_opts = zpt::json::object();
			if (this->__spec["protocols"]->type() == zpt::JSArray) {
				for (auto _proto : this->__spec["protocols"]->arr()) {
					_resource_opts << _proto->str() << true;
				}
			}
			zpt::replace(_mutation_on, "$[mutation.opts]", zpt::r_replace(std::string(_resource_opts), ":", ", "));
			_mutation_ons += _mutation_on + std::string("\n");
		}
		zpt::replace(_mutation_cxx, "$[mutation.handler.begin]", _mutation_ons);
		
		struct stat _buffer;
		bool _cxx_exists = stat(_cxx_file.c_str(), &_buffer) == 0;
		bool _h_exists = stat(_h_file.c_str(), &_buffer) == 0;
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_h_exists)) {
			std::ofstream _h_ofs(_h_file.data());
			_h_ofs << _mutation_h << endl << flush;
			_h_ofs.close();
			zlog(std::string("processed ") + _h_file, zpt::trace);
		}
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_cxx_exists)) {
			std::ofstream _cxx_ofs(_cxx_file.data());
			_cxx_ofs << _mutation_cxx << endl << flush;
			_cxx_ofs.close();
			zlog(std::string("processed ") + _cxx_file, zpt::trace);
		}
		return std::string(this->__spec["namespace"]) + std::string("::mutations::") + std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")) + std::string("::mutify(_emitter->mutations());\n");
	}
	return _return;
}

auto zpt::GenDatum::build_insert() -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		_return += std::string("{\nzpt::mutation::Insert,\n[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {\n");	
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += this->build_dbms();
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("zpt::json _r_data = _envelope[\"payload\"][\"new\"];\n");

		for (auto _r : this->__spec["fields"]->obj()) {
			zpt::json _field = _r.second;
			zpt::json _opts = zpt::gen::get_opts(_field);
			std::string _name = _r.first;
			if (_field["ref"]->ok() && !bool(_opts["on-demand"])) {
				zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
				_return += std::string("zpt::json _r_") + _name + std::string(" = _emitter->events()->route(zpt::ev::Get, zpt::path::join({ zpt::array, ");
				_return += this->build_topic(zpt::split(std::string(_field["ref"]), "/"));
				if (_rel->ok() && _rel->obj()->size() != 0) {
					_return += std::string(" }), { \"params\", { ");
					_return += this->build_params(_rel, false);
					_return += std::string(" } }");
				}
				else {
					_return += std::string(" }), zpt::undefined");
				}
				_return += std::string(")[\"payload\"];\n");
				if (_field["type"] == zpt::json::string("array")) {
					_return += std::string("_r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->type() == zpt::JSArray ? _r_") + _name + std::string("[\"elements\"] : zpt::json({ zpt::array, _r_") + _name + std::string(" }));\n");
				}
				else if (_field["type"] == zpt::json::string("object")) {
					_return += std::string("_r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->type() == zpt::JSArray ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(");\n");
				}
			}
		}

		_return += std::string("_c->insert(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"href\"]->str(), _r_data, { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("}\n}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_update() -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		_return += std::string("{\nzpt::mutation::Update,\n[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {\n");	
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += this->build_dbms();
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("if (_envelope[\"payload\"][\"filter\"]->ok()) {\n");
		_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"filter\"], _envelope[\"payload\"][\"changes\"], { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("else {\n");
		_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"href\"]->str(), _envelope[\"payload\"][\"changes\"], { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("}\n");	
		_return += std::string("}\n}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_remove() -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		_return += std::string("{\nzpt::mutation::Remove,\n[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {\n");	
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += this->build_dbms();
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("if (_envelope[\"payload\"][\"filter\"]->ok()) {\n");
		_return += std::string("_c->remove(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"filter\"], { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("else {\n");
		_return += std::string("_c->remove(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"href\"]->str(), { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("}\n");	
		_return += std::string("}\n}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_replace() -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		_return += std::string("{\nzpt::mutation::Replace,\n[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {\n");	
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += this->build_dbms();
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("zpt::json _r_data = _envelope[\"payload\"][\"new\"];\n");

		for (auto _r : this->__spec["fields"]->obj()) {
			zpt::json _field = _r.second;
			zpt::json _opts = zpt::gen::get_opts(_field);
			std::string _name = _r.first;
			if (_field["ref"]->ok() && !bool(_opts["on-demand"])) {
				zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
				_return += std::string("zpt::json _r_") + _name + std::string(" = _emitter->events()->route(zpt::ev::Get, zpt::path::join({ zpt::array, ");
				_return += this->build_topic(zpt::split(std::string(_field["ref"]), "/"));
				if (_rel->ok() && _rel->obj()->size() != 0) {
					_return += std::string(" }), { \"params\", { ");
					_return += this->build_params(_rel, false);
					_return += std::string(" } }");
				}
				else {
					_return += std::string(" }), zpt::undefined");
				}
				_return += std::string(")[\"payload\"];\n");
				if (_field["type"] == zpt::json::string("array")) {
					_return += std::string("_r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->type() == zpt::JSArray ? _r_") + _name + std::string("[\"elements\"] : zpt::json({ zpt::array, _r_") + _name + std::string(" }));\n");
				}
				else if (_field["type"] == zpt::json::string("object")) {
					_return += std::string("_r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->type() == zpt::JSArray ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(");\n");
				}
			}
		}

		_return += std::string("_c->save(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"href\"]->str(), _r_data, { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("}\n}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_insert(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
		_return += std::string("{\nzpt::mutation::Insert,\n(_h_on_change = [] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {\n");	
		_return += std::string("zpt::json _r_base = _emitter->events()->route(zpt::ev::Get, _envelope[\"payload\"][\"href\"]->str(), (_envelope[\"payload\"][\"filter\"]->ok() ? zpt::json({ \"params\", _envelope[\"payload\"][\"filter\"] }) : zpt::undefined))[\"payload\"];\n");
		_return += std::string("if (_r_base[\"elements\"]->type() != zpt::JSArray) {\n_r_base = { \"elements\", { zpt::array, _r_base } };\n}\n");
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += this->build_dbms();
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("for (auto _r_element : _r_base[\"elements\"]->arr()) {\n");
		_return += std::string("zpt::json _r_targets = _c->query(\"") + std::string(this->__spec["name"]) + std::string("\", { ");
		std::string _inverted_params = this->build_inverted_params(_rel);
		if (_inverted_params.length() != 0) {
			_return += _inverted_params;
		}
		else {
			_return += std::string("\"") + _name + std::string("\", { \"href\", _r_element[\"href\"] }");
		}
		_return += std::string(" });\n");
		_return += std::string("for (auto _r_data : _r_targets[\"elements\"]->arr()) {\n");
		_return += std::string("zpt::json _r_") + _name + std::string(" = _emitter->events()->route(zpt::ev::Get, zpt::path::join({ zpt::array, ");
		_return += this->build_topic(zpt::split(std::string(_field["ref"]), "/"));
		_return += std::string(" }), ");
		std::string _params = this->build_params(_rel, false);
		if (_params.length() != 0) {
			_return += std::string("{ \"params\", { ");
			_return += _params;
			_return += std::string(" } }");
		}
		else {
			_return += std::string("zpt::undefined");
		}
		_return += std::string(")[\"payload\"];\n");
		
		if (_field["type"] == zpt::json::string("array")) {
			_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _r_data[\"href\"]->str(), { \"") + _name + std::string("\", (_r_") + _name + std::string("[\"elements\"]->type() == zpt::JSArray ? _r_") + _name + std::string("[\"elements\"] : zpt::json({ zpt::array, _r_") + _name + std::string(" })) }, { \"href\", _r_data[\"href\"] });\n");
		}
		else if (_field["type"] == zpt::json::string("object")) {
			_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _r_data[\"href\"]->str(), { \"") + _name + std::string("\", (_r_") + _name + std::string("[\"elements\"]->type() == zpt::JSArray ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(") }, { \"href\", _r_data[\"href\"] });\n");
		}
		
		_return += std::string("}\n");
		_return += std::string("}\n");
		_return += std::string("}\n");	
		_return += std::string("})\n}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_update(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		_return += std::string("{ zpt::mutation::Update, _h_on_change }\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_remove(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
		_return += std::string("{\nzpt::mutation::Remove,\n[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {\n");	
		_return += std::string("if (_envelope[\"payload\"][\"filter\"]->ok()) return;\n");
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += this->build_dbms();
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("zpt::json _r_targets = _c->query(\"") + std::string(this->__spec["name"]) + std::string("\", { \"") + _name + std::string("\", { \"href\", _envelope[\"payload\"][\"href\"] } });\n");
		_return += std::string("for (auto _r_data : _r_targets[\"elements\"]->arr()) {\n");
		_return += std::string("zpt::json _r_") + _name + std::string(" = _emitter->events()->route(zpt::ev::Get, zpt::path::join({ zpt::array, ");
		_return += this->build_topic(zpt::split(std::string(_field["ref"]), "/"));
		_return += std::string(" }), ");
		std::string _params = this->build_params(_rel, false);
		if (_params.length() != 0) {
			_return += std::string("{ \"params\", { ");
			_return += _params;
			_return += std::string(" } }");
		}
		else {
			_return += std::string("zpt::undefined");
		}
		_return += std::string(")[\"payload\"];\n");
		
		if (_field["type"] == zpt::json::string("array")) {
			_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _r_data[\"href\"]->str(), { \"") + _name + std::string("\", (_r_") + _name + std::string("[\"elements\"]->type() == zpt::JSArray ? _r_") + _name + std::string("[\"elements\"] : zpt::json({ zpt::array, _r_") + _name + std::string(" })) }, { \"href\", _r_data[\"href\"] });\n");
		}
		else if (_field["type"] == zpt::json::string("object")) {
			_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _r_data[\"href\"]->str(), { \"") + _name + std::string("\", (_r_") + _name + std::string("[\"elements\"]->type() == zpt::JSArray ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(") }, { \"href\", _r_data[\"href\"] });\n");
		}
		
		_return += std::string("}\n");
		_return += std::string("}\n");
		_return += std::string("}\n}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_replace(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		_return += std::string("{ zpt::mutation::Replace, _h_on_change }\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_get() -> std::string{
	std::string _return;
	if (this->__spec["fields"]->type() == zpt::JSObject) {
		for (auto _field : this->__spec["fields"]->obj()) {
			std::string _type(_field.second["type"]);
			zpt::json _opts = zpt::gen::get_opts(_field.second);

			if (_type == "object") {
				if (_field.second["ref"]->ok() && _opts["on-demand"]->ok()) {
					std::string _topic;
					zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
					zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
					for (auto _part : _splited->arr()) {
						_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
					}

					std::string _remote_invoke;
					if (_rel->obj()->size() != 0) {
						std::string _params = this->build_params(_rel, false);
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n)[\"payload\"];\n");
					}
					else {
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
					}
					
					_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
					_return += std::string("_r_data << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->type() == zpt::JSArray ? _d_") + _field.first + std::string("[\"elements\"][0] : _d_") + _field.first + std::string(");\n");
				}
			}
			else if (_type == "array") {
				if (_field.second["ref"]->type() == zpt::JSString && _opts["on-demand"]->ok()) {
					std::string _topic;
					zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
					zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
					for (auto _part : _splited->arr()) {
						_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
					}

					std::string _remote_invoke;
					if (_rel->obj()->size() != 0) {
						std::string _params = this->build_params(_rel, false);
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n)[\"payload\"];\n");
					}
					else {
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
					}

					_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
					_return += std::string("_r_data << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->type() == zpt::JSArray ? _d_") + _field.first + std::string("[\"elements\"] : zpt::json({ zpt::array, _d_") + _field.first + std::string(" }));\n");
				}
			}
		}
	}
	return _return;
}

auto zpt::GenDatum::build_associations_query() -> std::string{
	std::string _return;
	if (this->__spec["fields"]->type() == zpt::JSObject) {
		bool _found = false;
		for (auto _field : this->__spec["fields"]->obj()) {
			std::string _type(_field.second["type"]);
			zpt::json _opts = zpt::gen::get_opts(_field.second);

			if (_type == "object") {
				if (_field.second["ref"]->ok() && _opts["on-demand"]->ok()) {
					if (!_found) {
						_return += std::string("if (_r_data[\"elements\"]->type() == zpt::JSArray) {\n");
						_return += std::string("for (auto _d_element : _r_data[\"elements\"]->arr()) {\n");
					}
					std::string _topic;
					zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
					zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
					for (auto _part : _splited->arr()) {
						_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_d_element[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
					}

					std::string _remote_invoke;
					if (_rel->obj()->size() != 0) {
						std::string _params = this->build_params(_rel, true);
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n)[\"payload\"];\n");
					}
					else {
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
					}
					
					_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
					_return += std::string("_d_element << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->type() == zpt::JSArray ? _d_") + _field.first + std::string("[\"elements\"][0] : _d_") + _field.first + std::string(");\n");

					_found = true;
				}
			}
			else if (_type == "array") {
				if (_field.second["ref"]->ok() && _opts["on-demand"]->ok()) {
					if (!_found) {
						_return += std::string("if (_r_data[\"elements\"]->type() == zpt::JSArray) {\n");
						_return += std::string("for (auto _d_element : _r_data[\"elements\"]->arr()) {\n");
					}

					std::string _topic;
					zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
					zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
					for (auto _part : _splited->arr()) {
						_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_d_element[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
					}

					std::string _remote_invoke;
					if (_rel->obj()->size() != 0) {
						std::string _params = this->build_params(_rel, true);
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n)[\"payload\"];\n");
					}
					else {
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
					}
					
					_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
					_return += std::string("_d_element << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->type() == zpt::JSArray ? _d_") + _field.first + std::string("[\"elements\"] : zpt::json({ zpt::array, _d_") + _field.first + std::string(" }));\n");

					_found = true;
				}
			}
		}
		if (_found) {
			_return += std::string("}\n");
			_return += std::string("}\n");
		}
	}
	return _return;
}

auto zpt::GenDatum::build_associations_insert() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_associations_save() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_associations_set() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_associations_remove() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_extends_get() -> std::string{
	std::string _return("zpt::connector _c = _emitter->connector(\"$[datum.method.get.client]\");\n_r_data = _c->get(\"$[datum.collection]\", _topic, { \"href\", _topic });\n");
	return _return;
}

auto zpt::GenDatum::build_extends_query() -> std::string{
	std::string _return("zpt::connector _c = _emitter->connector(\"$[datum.method.query.client]\");\n_r_data = _c->query(\"$[datum.collection]\", _filter, _filter + zpt::json({ \"href\", _topic }));\n");
	return _return;
}

auto zpt::GenDatum::build_extends_insert() -> std::string{
	std::string _return("zpt::connector _c = _emitter->connector(\"$[datum.method.insert.client]\");\n\n_document <<\n\"created\" << zpt::json::date() <<\n\"updated\" << zpt::json::date();\n");
	if (this->__spec["extends"]["name"]->ok()) {
		auto _found = zpt::Generator::datums.find(zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_"));
		if (_found != zpt::Generator::datums.end()) {
			if (this->__spec["lib"] != _found->second->spec()["lib"]) {
				if (this->__spec["dynlink"]->ok()) {
					this->__spec << "dynlink" << (this->__spec["dynlink"]->str() + std::string(" -l") + _found->second->spec()["lib"]->str());
				}
				else {
					this->__spec << "dynlink" << (std::string(" -l") + _found->second->spec()["lib"]->str());
				}
				if (this->__spec["dyndir"]->ok()) {
					this->__spec << "dyndir" << (this->__spec["dyndir"]->str() + std::string(" -L../") + _found->second->spec()["spec_name"]->str() + std::string("/.libs"));
				}
				else {
					this->__spec << "dyndir" << (std::string(" -L../") + _found->second->spec()["spec_name"]->str() + std::string("/.libs"));
				}
			}
			
			std::string _name = std::string(zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_"));
			std::string _class = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + _name;

			_return += std::string("\nzpt::json _r_parent = zpt::json::object();\n");
			for (auto _field : _found->second->spec()["fields"]->obj()) {
				_return += std::string("if (_document[\"") + _field.first + std::string("\"]->ok()) _r_parent << \"") + _field.first + std::string("\" << _document[\"") + _field.first + std::string("\"];\n");
			}
			_return += std::string("\nstd::string _r_id = _c->insert(\"$[datum.collection]\", _topic, _document - _r_parent, { \"href\", _topic });\n");
			_return += std::string("_r_data = { \"href\", (_topic + std::string(\"/\") + _r_id) };\n");
			_return += std::string("_r_parent << \"id\" << _r_id;\n");
			_return += _class + std::string("::insert(_topic, _r_parent, _emitter, _identity, _envelope);\n");
			_return += std::string("\n_r_extends = true;\n");
		}
	}
	else {
		_return += std::string("\n_r_data = { \"href\", (_topic + std::string(\"/\") + _c->insert(\"$[datum.collection]\", _topic, _document, { \"href\", _topic })) };\n");
	}
	return _return;
}

auto zpt::GenDatum::build_extends_save() -> std::string {
	std::string _return("zpt::connector _c = _emitter->connector(\"$[datum.method.save.client]\");\n\n_document <<\n\"updated\" << zpt::json::date();\n");
	if (this->__spec["extends"]["name"]->ok()) {
		auto _found = zpt::Generator::datums.find(zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_"));
		if (_found != zpt::Generator::datums.end()) {
			std::string _name = std::string(zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_"));
			std::string _class = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + _name;

			_return += std::string("\nzpt::json _r_parent = zpt::json::object();\n");
			for (auto _field : _found->second->spec()["fields"]->obj()) {
				_return += std::string("if (_document[\"") + _field.first + std::string("\"]->ok()) _r_parent << \"") + _field.first + std::string("\" << _document[\"") + _field.first + std::string("\"];\n");
			}
			_return += std::string("_r_data = { \"href\", _topic, \"n_updated\", _c->save(\"$[datum.collection]\", _topic, _document - _r_parent, { \"href\", _topic }) };\n");
			_return += _class + std::string("::save(_topic, _r_parent, _emitter, _identity, _envelope);\n");
			_return += std::string("\n_r_extends = true;\n");
		}
	}
	else {
		_return += std::string("\n_r_data = { \"href\", _topic, \"n_updated\", _c->save(\"$[datum.collection]\", _topic, _document, { \"href\", _topic }) };\n");
	}
	return _return;
}

auto zpt::GenDatum::build_extends_set_topic() -> std::string {
	std::string _return("zpt::connector _c = _emitter->connector(\"$[datum.method.set.client]\");\n\n_document <<\n\"updated\" << zpt::json::date();\n");
	if (this->__spec["extends"]["name"]->ok()) {
		auto _found = zpt::Generator::datums.find(zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_"));
		if (_found != zpt::Generator::datums.end()) {
			std::string _name = std::string(zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_"));
			std::string _class = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + _name;

			_return += std::string("\nzpt::json _r_parent = zpt::json::object();\n");
			for (auto _field : _found->second->spec()["fields"]->obj()) {
				_return += std::string("if (_document[\"") + _field.first + std::string("\"]->ok()) _r_parent << \"") + _field.first + std::string("\" << _document[\"") + _field.first + std::string("\"];\n");
			}
			_return += std::string("_r_data = { \"href\", _topic, \"n_updated\", _c->set(\"$[datum.collection]\", _topic, _document - _r_parent, { \"href\", _topic }) };\n");
			_return += _class + std::string("::set(_topic, _r_parent, _emitter, _identity, _envelope);\n");
			_return += std::string("\n_r_extends = true;\n");
		}
	}
	else {
		_return += std::string("\n_r_data = { \"href\", _topic, \"n_updated\", _c->set(\"$[datum.collection]\", _topic, _document, { \"href\", _topic }) };\n");
	}
	return _return;
}

auto zpt::GenDatum::build_extends_set_pattern() -> std::string {
	std::string _return("zpt::connector _c = _emitter->connector(\"$[datum.method.set.client]\");\n\n_document <<\n\"updated\" << zpt::json::date();\n");
	if (this->__spec["extends"]["name"]->ok()) {
		std::string _name = std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_"));
		std::string _class = std::string(this->__spec["namespace"]) + std::string("::datums::") + _name;
		
		_return += std::string("zpt::json _r_elements = ") + _class + std::string("::query(_topic, _filter, _emitter, _identity, _envelope);\n");
		_return += std::string("for (auto _r_element : _r_elements[\"elements\"]->arr()) {\n");
		_return += _class + std::string("::set(_r_element[\"href\"]->str(), _document, _emitter, _identity, _envelope);\n");
		_return += std::string("}\n");
		_return += std::string("_r_data = { \"href\", _topic, \"n_updated\", _r_elements[\"size\"] };\n");
		_return += std::string("\n_r_extends = true;\n");
	}
	else {
		_return += std::string("\n_r_data = { \"href\", _topic, \"n_updated\", _c->set(\"$[datum.collection]\", _filter, _document, { \"href\", _topic }) };\n");
	}
	return _return;
}

auto zpt::GenDatum::build_extends_remove_topic() -> std::string {
	std::string _return("zpt::connector _c = _emitter->connector(\"$[datum.method.remove.client]\");\n");
	if (this->__spec["extends"]["name"]->ok()) {
		auto _found = zpt::Generator::datums.find(zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_"));
		if (_found != zpt::Generator::datums.end()) {
			std::string _name = std::string(zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_"));
			std::string _class = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + _name;
			
			_return += std::string("_r_data = { \"href\", _topic, \"n_updated\", _c->remove(\"$[datum.collection]\", _topic, { \"href\", _topic }) };\n");
			_return += _class + std::string("::remove(_topic, _emitter, _identity, _envelope);\n");
			_return += std::string("\n_r_extends = true;\n");
		}
	}
	else {
		_return += std::string("\n_r_data = { \"href\", _topic, \"n_deleted\", _c->remove(\"$[datum.collection]\", _topic, { \"href\", _topic }) };\n");
	}
	return _return;
}

auto zpt::GenDatum::build_extends_remove_pattern() -> std::string {
	std::string _return("zpt::connector _c = _emitter->connector(\"$[datum.method.remove.client]\");\n");
	if (this->__spec["extends"]["name"]->ok()) {
		std::string _name = std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_"));
		std::string _class = std::string(this->__spec["namespace"]) + std::string("::datums::") + _name;
		
		_return += std::string("zpt::json _r_elements = ") + _class + std::string("::query(_topic, _filter, _emitter, _identity, _envelope);\n");
		_return += std::string("for (auto _r_element : _r_elements[\"elements\"]->arr()) {\n");
		_return += _class + std::string("::remove(_r_element[\"href\"]->str(), _emitter, _identity, _envelope);\n");
		_return += std::string("}\n");
		_return += std::string("_r_data = { \"href\", _topic, \"n_deleted\", _r_elements[\"size\"] };\n");
		_return += std::string("\n_r_extends = true;\n");
	}
	else {
		_return += std::string("\n_r_data = { \"href\", _topic, \"n_deleted\", _c->remove(\"$[datum.collection]\", _filter, _filter + zpt::json({ \"href\", _topic })) };\n");
	}
	return _return;
}

auto zpt::GenDatum::build_initialization(std::string _dbms, std::string _namespace) -> std::string {
	if (_dbms == "postgresql") {
		return "{ \"dbms.pgsql." + zpt::r_replace(_namespace, "::", ".") + "\", zpt::connector(new zpt::pgsql::Client(_emitter->options(), \"pgsql." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	else if (_dbms == "mariadb") {
		return "{ \"dbms.mariadb." + zpt::r_replace(_namespace, "::", ".") + "\", zpt::connector(new zpt::mariadb::Client(_emitter->options(), \"mariadb." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	else if (_dbms == "mongodb") {
		return "{ \"dbms.mongodb." + zpt::r_replace(_namespace, "::", ".") + "\", zpt::connector(new zpt::mongodb::Client(_emitter->options(), \"mongodb." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	else if (_dbms == "redis") {
		return "{ \"dbms.redis." + zpt::r_replace(_namespace, "::", ".") + "\", zpt::connector(new zpt::redis::Client(_emitter->options(), \"redis." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	return "";
}

auto zpt::GenDatum::build_data_client(zpt::json _dbms, zpt::json _ordered, std::string _namespace) -> std::string {
	for (auto _db : _ordered->arr()) {
		if (std::find(std::begin(_dbms->arr()), std::end(_dbms->arr()), _db) != std::end(_dbms->arr())) {
			std::string _db_client(_db->str());
			if (_db->str() == "postgresql") {
				_db_client.assign("pgsql");
			}
			return std::string("dbms.") + _db_client + std::string(".") + zpt::r_replace(_namespace, "::", ".");
		}
	}
	return "";
}

auto zpt::GenDatum::get_type(zpt::json _field) -> std::string {
	std::string _type(_field["type"]);
	std::string _return;
	if (_type == "utf8" || _type == "ascii" || _type == "text") {
		_return += std::string("text");
	}
	else if (_type == "string") {
		_return += std::string("varchar(1024)");
	}
	else if (_type == "token" || _type == "uri" || _type == "email") {
		_return += std::string("varchar(512)");
	}
	else if (_type == "uuid") {
		_return += std::string("char(36)");
	}
	else if (_type == "int") {
		_return += std::string("bigint");
	}
	else if (_type == "boolean") {
		_return += std::string("boolean");
	}
	else if (_type == "double") {
		_return += std::string("decimal");
	}
	else if (_type == "timestamp") {
		_return += std::string("timestamp");
	}
	else if (_type == "json") {
		_return += std::string("json");
	}
	return _return;
}

auto zpt::GenDatum::get_restrictions(zpt::json _field) -> std::string {
	std::string _return;
	zpt::json _opts = zpt::gen::get_opts(_field);
	if (_opts["mandatory"]->ok()) {
		if (_return.length() != 0) {
			_return += std::string(" ");
		}
		_return += std::string("not null");
	}
	if (_opts["index"]->ok()) {
		if (_return.length() != 0) {
			_return += std::string(" ");
		}
		_return += std::string("index");
	}
	if (_opts["foreign"]->ok()) {
		if (_return.length() != 0) {
			_return += std::string(" ");
		}
		zpt::json _splited = zpt::split(std::string(_opts["foreign"]), ":");
		_return += std::string("references ") + _splited->arr()->back()->str();
	}
	return _return;
}

zpt::GenResource::GenResource(zpt::json _spec, zpt::json _options) : __spec(_spec), __options(_options) {
}

zpt::GenResource::~GenResource(){
}

auto zpt::GenResource::spec() -> zpt::json {
	return this->__spec;
}

auto zpt::GenResource::build() -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenResource::build_data_layer() -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenResource::build_handlers(std::string _parent_name, std::string _child_includes) -> std::string {
	if (!this->__options["resource-out-lang"]->ok() || std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("c++")) != std::end(this->__options["resource-out-lang"]->arr())) {
		std::string _h_file = std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_parent_name) + std::string("/") + std::string(this->__spec["type"]) + std::string("s/") + std::string(this->__spec["name"]) + std::string(".h");
		std::string _cxx_file = std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + std::string(_parent_name) + std::string("/") + std::string(this->__spec["type"]) + std::string("s/") + std::string(this->__spec["name"]) + std::string(".cpp");

		std::string _handler_h;
		zpt::load_path("/usr/share/zapata/gen/Handlers.h", _handler_h);
		std::string _handler_cxx;
		zpt::load_path("/usr/share/zapata/gen/Handlers.cpp", _handler_cxx);

		std::string _include = zpt::r_replace(_h_file, std::string(this->__options["prefix-h"][0]), "");
		if (_include.front() == '/') {
			_include.erase(0, 1);
		}
		zpt::replace(_handler_h, "$[data.path.h]", _child_includes);
		zpt::replace(_handler_cxx, "$[resource.path.self.h]", std::string("#include <") + _include + std::string(">"));
	
		zpt::replace(_handler_h, "$[namespace]", std::string(this->__spec["namespace"]));
		zpt::replace(_handler_cxx, "$[namespace]", std::string(this->__spec["namespace"]));

		zpt::json _namespaces = zpt::split(std::string(this->__spec["namespace"]), "::");
		std::string _namespaces_begin;
		std::string _namespaces_end;
		for (auto _part : _namespaces->arr()) {
			_namespaces_begin += std::string("namespace ") + std::string(_part) + std::string(" {\n");
			_namespaces_end += "}\n";
		}
		_namespaces_begin += std::string("namespace ") + std::string(this->__spec["type"]) + std::string("s {\n");
		_namespaces_end += "}\n";
		_namespaces_begin += std::string("namespace ") + std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")) + std::string(" {\n");
		_namespaces_end += "}\n";
	
		zpt::replace(_handler_h, "$[namespaces.begin]", _namespaces_begin);
		zpt::replace(_handler_h, "$[namepsaces.end]", _namespaces_end);
		zpt::replace(_handler_cxx, "$[namespaces.begin]", _namespaces_begin);
		zpt::replace(_handler_cxx, "$[namepsaces.end]", _namespaces_end);

		zpt::replace(_handler_h, "$[resource.type]", std::string(this->__spec["type"]));
		zpt::replace(_handler_cxx, "$[resource.type]", std::string(this->__spec["type"]));
		zpt::replace(_handler_h, "$[resource.name]", std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")));
		zpt::replace(_handler_cxx, "$[resource.name]", std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")));
	
		zpt::replace(_handler_cxx, "$[resource.topic.regex]", zpt::gen::url_pattern_to_regexp(this->__spec["topic"]));
		std::string _handler;
		bool _first = true;
		_handler.assign(this->build_get());
		zpt::replace(_handler_cxx, "$[resource.handler.get]", _handler);
		_first = _handler.length() == 0;
		_handler.assign(this->build_post());
		if (_handler.length() != 0 && !_first) {
			_first = false;
			_handler = std::string(",\n") + _handler;
		}
		zpt::replace(_handler_cxx, "$[resource.handler.post]", _handler);
		_handler.assign(this->build_put());
		if (_handler.length() != 0 && !_first) {
			_first = false;
			_handler = std::string(",\n") + _handler;
		}		
		zpt::replace(_handler_cxx, "$[resource.handler.put]", _handler);
		_handler.assign(this->build_patch());
		if (_handler.length() != 0 && !_first) {
			_first = false;
			_handler = std::string(",\n") + _handler;
		}		
		zpt::replace(_handler_cxx, "$[resource.handler.patch]", _handler);
		_handler.assign(this->build_delete());
		if (_handler.length() != 0 && !_first) {
			_first = false;
			_handler = std::string(",\n") + _handler;
		}		
		zpt::replace(_handler_cxx, "$[resource.handler.delete]", _handler);
		_handler.assign(this->build_head());
		if (_handler.length() != 0 && !_first) {
			_first = false;
			_handler = std::string(",\n") + _handler;
		}		
		zpt::replace(_handler_cxx, "$[resource.handler.head]", _handler);
		_handler.assign(this->build_reply());
		if (_handler.length() != 0 && !_first) {
			_first = false;
			_handler = std::string(",\n") + _handler;
		}		
		zpt::replace(_handler_cxx, "$[resource.handler.reply]", _handler);

		zpt::json _resource_opts = zpt::json::object();
		if (this->__spec["protocols"]->type() == zpt::JSArray) {
			for (auto _proto : this->__spec["protocols"]->arr()) {
				_resource_opts << _proto->str() << true;
			}
		}
		zpt::replace(_handler_cxx, "$[resource.opts]", zpt::r_replace(std::string(_resource_opts), ":", ", "));
	
		struct stat _buffer;
		bool _cxx_exists = stat(_cxx_file.c_str(), &_buffer) == 0;
		bool _h_exists = stat(_h_file.c_str(), &_buffer) == 0;
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_h_exists)) {
			std::ofstream _h_ofs(_h_file.data());
			_h_ofs << _handler_h << endl << flush;
			_h_ofs.close();
			zlog(std::string("processed ") + _h_file, zpt::trace);
		}
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_cxx_exists)) {
			std::ofstream _cxx_ofs(_cxx_file.data());
			_cxx_ofs << _handler_cxx << endl << flush;
			_cxx_ofs.close();
			zlog(std::string("processed ") + _cxx_file, zpt::trace);
		}
	}		
	if (this->__options["resource-out-lang"]->ok() && std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("lisp")) != std::end(this->__options["resource-out-lang"]->arr())) {
		std::string _lisp_file = std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_parent_name) + std::string("/") + std::string(this->__spec["type"]) + std::string("s/") + std::string(this->__spec["name"]) + std::string(".lisp");

		std::string _handler_lisp;
		zpt::load_path("/usr/share/zapata/gen/Handlers.lisp", _handler_lisp);

		zpt::json _performatives = { zpt::array, "get", "post", "put", "patch", "delete", "head" };
		for (auto _perf : _performatives->arr()) {
			if (this->__spec["performatives"]->ok() && std::find(std::begin(this->__spec["performatives"]->arr()), std::end(this->__spec["performatives"]->arr()), zpt::json::string(_perf->str())) == std::end(this->__spec["performatives"]->arr())) {
				zpt::replace(_handler_lisp, std::string("$[resource.handler.") + _perf->str() + std::string("]\n"), std::string(""));
				zpt::replace(_handler_lisp, std::string("      $[resource.handler.") + _perf->str() + std::string(".name]\n"), std::string(""));
				continue;
			}

			std::string _f_name = std::string(this->__spec["name"]) + std::string("-") + std::string(this->__spec["type"]) + std::string("-") + _perf->str();
			std::string _function;
			if (_perf->str() == "get") {
				_function.assign(this->build_lisp_get());
			}
			else if (_perf->str() == "post") {
				_function.assign(this->build_lisp_post());
			}
			else if (_perf->str() == "put") {
				_function.assign(this->build_lisp_put());
			}
			else if (_perf->str() == "patch") {
				_function.assign(this->build_lisp_patch());
			}
			else if (_perf->str() == "delete") {
				_function.assign(this->build_lisp_delete());
			}
			else if (_perf->str() == "head") {
				_function.assign(this->build_lisp_head());
			}

			zpt::replace(_handler_lisp, std::string("$[resource.handler.") + _perf->str() + std::string("]"), _function);
			zpt::replace(_handler_lisp, std::string("$[resource.handler.") + _perf->str() + std::string(".name]"), std::string("\"") + _perf->str() + std::string("\" \"") + _f_name + std::string("\""));
		}
		zpt::replace(_handler_lisp, "$[resource.topic.regex]", zpt::gen::url_pattern_to_regexp(this->__spec["topic"]));

		std::string _resource_opts;
		if (this->__spec["protocols"]->type() == zpt::JSArray) {
			for (auto _proto : this->__spec["protocols"]->arr()) {
				_resource_opts += std::string(" \"") + _proto->str() + std::string("\" t");
			}
		}
		zpt::replace(_handler_lisp, "$[resource.opts]", _resource_opts);
		
		struct stat _buffer;
		bool _lisp_exists = stat(_lisp_file.c_str(), &_buffer) == 0;
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_lisp_exists)) {
			std::ofstream _lisp_ofs(_lisp_file.data());
			_lisp_ofs << _handler_lisp << endl << flush;
			_lisp_ofs.close();
			zlog(std::string("processed ") + _lisp_file, zpt::trace);
		}
	}
	if (this->__options["resource-out-lang"]->ok() && std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("python")) != std::end(this->__options["resource-out-lang"]->arr())) {
		std::string _handler_py;
		zpt::load_path("/usr/share/zapata/gen/Handler.py", _handler_py);
		std::string _py_file = std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_parent_name) + std::string("/") + std::string(this->__spec["type"]) + std::string("s/") + std::string(this->__spec["name"]) + std::string(".py");

		struct stat _buffer;
		bool _py_exists = stat(_py_file.c_str(), &_buffer) == 0;
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_py_exists)) {
			std::ofstream _py_ofs(_py_file.data());
			_py_ofs << _handler_py << endl << flush;
			_py_ofs.close();
			zlog(std::string("processed ") + _py_file, zpt::trace);
		}
	}

	return std::string(this->__spec["namespace"]) + std::string("::") + std::string(this->__spec["type"]) + std::string("s::") + std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")) + std::string("::restify(_emitter);\n");
}

auto zpt::GenResource::build_validation(zpt::ev::performative _perf) -> std::string {
	if (!this->__spec["datum"]["name"]->ok() && !this->__spec["datum"]["fields"]->ok()) {
		return "";
	}
	if (_perf == zpt::ev::Reply) {
		return "";
	}
	zpt::json _fields;
	std::string _return;
	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			_fields = _found->second->spec()["fields"];
		}
	}
	else if (this->__spec["datum"]["fields"]->ok()) {
		_fields = this->__spec["datum"]["fields"];
	}
	if (_fields->type() == zpt::JSObject) {
		for (auto _field : _fields->obj()) {
			std::string _type(_field.second["type"]);
			zpt::json _opts = zpt::gen::get_opts(_field.second);
			
			if (_opts["mandatory"]->ok() && 
				(
					(std::string(this->__spec["type"]) == "controller") ||
					(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
					(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
					(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
				)
				&& !_opts["read-only"]->ok() && !_opts["default"]->ok() && !_opts["auto"]->ok()
			) {
				_return += std::string("assertz_mandatory(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
			}

			if (_perf != zpt::ev::Get && _perf != zpt::ev::Head && _perf != zpt::ev::Delete) {
				if (!_opts["read-only"]->ok()) {
					if (_type == "json") {
						_return += std::string("assertz_complex(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
					else {
						_return += std::string("assertz_") + _type + std::string("(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
				}
				else {
					_return += std::string("_envelope[\"payload\"] >> \"") + _field.first + std::string("\";\n");
				}
			
				if (_opts["default"]->ok() && 
					(
						(std::string(this->__spec["type"]) == "controller") ||
						(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
						(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
						(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
					)
				) {
					if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::string(\"") + std::string(_opts["default"]) + std::string("\");\n");
						_return += std::string("}\n");
					}
					else if (_type == "int") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::integer(") + std::string(_opts["default"]) + std::string(");\n");
						_return += std::string("}\n");
					}
					else if (_type == "boolean") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::boolean(") + std::string(_opts["default"]) + std::string(");\n");
						_return += std::string("}\n");
					}
					else if (_type == "double") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::floating(") + std::string(_opts["default"]) + std::string(");\n");
						_return += std::string("}\n");
					}
					else if (_type == "timestamp") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date(") + std::string(_opts["default"]) + std::string(");\n");
						_return += std::string("}\n");
					}
					else if (_type == "object") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json(\"") + std::string(_opts["default"]) + std::string("\");\n");
						_return += std::string("}\n");
					}
				}
			
				if (_opts["auto"]->ok()) {
					if (_opts["every-time"]->ok()) {
						if (_type == "token") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(24);\n");
						}
						else if (_type == "uuid") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_uuid();\n");
						}
						else if (_type == "hash") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(64);\n");
						}
						else if (_type == "timestamp") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date();\n");
						}
					}
					else if ((std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) || (std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put)) {
						if (_type == "token") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(24);\n");
						}
						else if (_type == "uuid") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_uuid();\n");
						}
						else if (_type == "hash") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(64);\n");
						}
						else if (_type == "timestamp") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date();\n");
						}
					}
					else if (std::string(this->__spec["type"]) == "document" && (_perf == zpt::ev::Patch || _perf == zpt::ev::Put)) {
						if (_type == "token") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(24);\n");
						}
						else if (_type == "uuid") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_uuid();\n");
						}
						else if (_type == "hash") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(64);\n");
						}
						else if (_type == "timestamp") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date();\n");
						}
					}
					else if (_opts["mandatory"]->ok() && 
						(
							(std::string(this->__spec["type"]) == "controller") ||
							(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
							(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
							(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
						)
						&& !_opts["read-only"]->ok() && !_opts["default"]->ok()
					) {
						_return += std::string("assertz_mandatory(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
				}
			}
		}
	}
	return _return;
}

auto zpt::GenResource::build_handler_header(zpt::ev::performative _perf) -> std::string {
	std::string _return;
	
	_return += this->build_validation(_perf);
	_return += std::string("\nzpt::json _t_split = zpt::split(_topic, \"/\");\n");
	_return += zpt::gen::url_pattern_to_vars(std::string(this->__spec["topic"]));
	_return += std::string("zpt::json _identity = zpt::rest::authorization::validate(\"") + std::string(this->__spec["topic"]) + std::string("\", _envelope, _emitter);\n");
	_return += std::string("\nzpt::json _r_body;\n");
	_return += std::string("/* ---> YOUR CODE HERE <---*/\n");
	if (_perf != zpt::ev::Reply && _perf != zpt::ev::Delete && this->__spec["links"]->type() == zpt::JSArray) {
		_return += std::string("if (_r_body[\"payload\"]->ok() && int(_r_body[\"status\"]) < 300) {\n");
		_return += std::string("_r_body[\"payload\"] << \"links\" << zpt::json({ ");
		
		for (auto _link : this->__spec["links"]->arr()) {
			std::string _topic;
			zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(_link["ref"]));		
			zpt::json _splited = zpt::split(std::string(_link["ref"]), "/");
			for (auto _part : _splited->arr()) {
				_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
			}
			_return += std::string("\"") + std::string(_link["rel"]) + std::string("\", zpt::join({ zpt::array") + _topic + std::string(" }, \"/\"), ");
		}
		_return += std::string(" });\n");
		_return += std::string("}\n");
	}
	return _return;
}

auto zpt::GenResource::build_get() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("get")) == std::end(_performatives->arr())) {
		return "";
	}
	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Get,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Get);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body  };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body  };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::join({ zpt::array") + _topic + std::string(" }, \"/\");\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_get(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("zpt::undefined"));
			}
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_post() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("post")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "document") {
		return "";
	}
	if (std::string(this->__spec["type"]) == "store") { 
		return "";
	}

	std::string _return("{\nzpt::ev::Post,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Post);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"status\", 201, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "controller") {
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::join({ zpt::array") + _topic + std::string(" }, \"/\");\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_post(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Post,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"] }\n);\n");
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_put() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("put")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "collection") { 
		return "";
	}
	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Put,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Put);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"status\", 201, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::join({ zpt::array") + _topic + std::string(" }, \"/\");\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_put(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Put,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"] }\n);\n");
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_patch() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("patch")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Patch,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Patch);
	
	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::join({ zpt::array") + _topic + std::string(" }, \"/\");\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_patch(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("zpt::undefined"));
			}
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Patch,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"], \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Patch,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"], \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_delete() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("delete")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Delete,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Delete);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::join({ zpt::array") + _topic + std::string(" }, \"/\");\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_delete(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("zpt::undefined"));
			}
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_head() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("head")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Head,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Head);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"headers\",\n{ \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"headers\",\n{ \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"headers\",\n{ \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::join({ zpt::array") + _topic + std::string(" }, \"/\");\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_head(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("zpt::undefined"));
			}
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Head,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Head,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_reply() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("reply")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Reply,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Head);
	_return += std::string("return zpt::undefined;\n");
	_return += std::string("\n}\n}");
	return _return;
}

auto zpt::GenResource::build_lisp_validation(zpt::ev::performative _perf) -> std::string {
	if (!this->__spec["datum"]["name"]->ok() && !this->__spec["datum"]["fields"]->ok()) {
		return "";
	}
	if (_perf == zpt::ev::Reply) {
		return "";
	}
	zpt::json _fields;
	std::string _return;
	if (this->__spec["datum"]["name"]->ok()) {
		return "";
	}
	else if (this->__spec["datum"]["fields"]->ok()) {
		_fields = this->__spec["datum"]["fields"];
	}
	if (_fields->type() == zpt::JSObject) {
		for (auto _field : _fields->obj()) {
			std::string _type(_field.second["type"]);
			zpt::json _opts = zpt::gen::get_opts(_field.second);
			
			if (_opts["mandatory"]->ok() && 
				(
					(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
					(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
					(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
				)
				&& !_opts["read-only"]->ok() && !_opts["default"]->ok() && !_opts["auto"]->ok()
			) {
				_return += std::string("   (zpt:assertz-mandatory (gethash \"payload\" envelope) \"") + _field.first + std::string("\" 412)\n");
			}

			if (_perf != zpt::ev::Get && _perf != zpt::ev::Head && _perf != zpt::ev::Delete) {
				if (!_opts["read-only"]->ok()) {
					if (_type == "json") {
						_return += std::string("  (zpt:assertz-complex (gethash \"payload\" envelope) \"") + _field.first + std::string("\" 412)\n");
					}
					else {
						_return += std::string("  (zpt:assertz-") + _type + std::string(" (gethash \"payload\" envelope) \"") + _field.first + std::string("\" 412)\n");
					}
				}
			
				if (_opts["default"]->ok() && 
					(
						(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
						(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
						(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
					)
				) {
					if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email") {
						_return += std::string("  (if (null (gethash \"") + _field.first + std::string("\" (gethash \"payload\" envelope))) (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" \"") + std::string(_opts["default"]) + std::string("\"))");
					}
					else if (_type == "int") {
						_return += std::string("  (if (null (gethash \"") + _field.first + std::string("\" (gethash \"payload\" envelope))) (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" ") + std::string(_opts["default"]) + std::string("))");
					}
					else if (_type == "boolean") {
						_return += std::string("  (if (null (gethash \"") + _field.first + std::string("\" (gethash \"payload\" envelope))) (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" ") + std::string(_opts["default"]) + std::string("))");
					}
					else if (_type == "double") {
						_return += std::string("  (if (null (gethash \"") + _field.first + std::string("\" (gethash \"payload\" envelope))) (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" ") + std::string(_opts["default"]) + std::string("))");
					}
					else if (_type == "timestamp") {
						_return += std::string("  (if (null (gethash \"") + _field.first + std::string("\" (gethash \"payload\" envelope))) (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" \"") + std::string(_opts["default"]) + std::string("\"))");
					}
					else if (_type == "object") {
						_return += std::string("  (if (null (gethash \"") + _field.first + std::string("\" (gethash \"payload\" envelope))) (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" \"") + std::string(_opts["default"]) + std::string("\"))");
					}
				}
			
				if (_opts["auto"]->ok()) {
					if (
						_opts["every-time"]->ok() ||
						((std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) || (std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put)) ||
						(std::string(this->__spec["type"]) == "document" && (_perf == zpt::ev::Patch || _perf == zpt::ev::Put))
					) {
						if (_type == "token") {
							_return += std::string("  (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" (zpt:generate-key 24))\n");
						}
						else if (_type == "uuid") {
							_return += std::string("  (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" (zpt:generate-uuid))\n");
						}
						else if (_type == "hash") {
							_return += std::string("  (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" (zpt:generate-key 64))\n");
						}
						else if (_type == "timestamp") {
							_return += std::string("  (add-to-object (gethash \"payload\" envelope) \"") + _field.first + std::string("\" (zpt:json-date))\n");
						}
					}
					else if (_opts["mandatory"]->ok() && 
						(
							(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
							(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
							(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
						)
						&& !_opts["read-only"]->ok() && !_opts["default"]->ok()
					) {
						_return += std::string("  (zpt:assertz-mandatory (gethash \"payload\" envelope) \"") + _field.first + std::string("\" 412)\n");
					}
				}
			}
		}
	}
	return _return;
}

auto zpt::GenResource::build_lisp_handler_header(zpt::ev::performative _perf) -> std::string {
	std::string _return;	
	_return += this->build_lisp_validation(_perf);
	_return += std::string("  (let* ((indentity (zpt:authorize \"") + std::string(this->__spec["topic"]) + std::string("\" envelope))\n         (t-split (zpt:split topic \"/\"))");
	_return += zpt::gen::url_pattern_to_vars_lisp(std::string(this->__spec["topic"]));
	_return += std::string(")\n");
	_return += std::string("    ;; ---> YOUR CODE HERE <--- ;;\n");
	return _return;
}

auto zpt::GenResource::build_lisp_get() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("get")) == std::end(_performatives->arr())) {
		return "";
	}
	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _f_name = std::string(this->__spec["name"]) + std::string("-") + std::string(this->__spec["type"]) + std::string("-get");
	std::string _return = std::string("(defun ") + _f_name + std::string(" (performative topic envelope)\n");
	
	_return += this->build_lisp_handler_header(zpt::ev::Get);

	if (!this->__spec["datum"]["ref"]->ok()) {
		_return += std::string("    (json \"status\" 204)))\n");
	}

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("nil"));
			}
			_remote_invoke += std::string("        (zpt:route \"get\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"get\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope))) ");
		}
		zpt::replace(_return, ";; ---> YOUR CODE HERE <--- ;;", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_lisp_post() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("post")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "document") {
		return "";
	}
	if (std::string(this->__spec["type"]) == "store") { 
		return "";
	}

	std::string _f_name = std::string(this->__spec["name"]) + std::string("-") + std::string(this->__spec["type"]) + std::string("-post");
	std::string _return = std::string("(defun ") + _f_name + std::string(" (performative topic envelope)\n");
	
	_return += this->build_lisp_handler_header(zpt::ev::Post);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("    (json \"status\" 201)))\n");
		}
		if (std::string(this->__spec["type"]) == "controller") {
			_return += std::string("    (json \"status\" 200)))\n");
		}
	}

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("nil"));
			}
			_remote_invoke += std::string("        (zpt:route \"post\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"post\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope))) ");
		}
		zpt::replace(_return, ";; ---> YOUR CODE HERE <--- ;;", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_lisp_put() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("put")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "collection") { 
		return "";
	}
	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _f_name = std::string(this->__spec["name"]) + std::string("-") + std::string(this->__spec["type"]) + std::string("-put");
	std::string _return = std::string("(defun ") + _f_name + std::string(" (performative topic envelope)\n");
	
	_return += this->build_lisp_handler_header(zpt::ev::Put);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("    (json \"status\" 201)))\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("    (json \"status\" 200)))\n");
		}
	}

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("nil"));
			}
			_remote_invoke += std::string("        (zpt:route \"put\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"put\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope))) ");
		}
		zpt::replace(_return, ";; ---> YOUR CODE HERE <--- ;;", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_lisp_patch() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("patch")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _f_name = std::string(this->__spec["name"]) + std::string("-") + std::string(this->__spec["type"]) + std::string("-patch");
	std::string _return = std::string("(defun ") + _f_name + std::string(" (performative topic envelope)\n");
	
	_return += this->build_lisp_handler_header(zpt::ev::Patch);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("    (json \"status\" 200)))\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("    (json \"status\" 200)))\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("    (json \"status\" 200)))\n");
		}
	}

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("nil"));
			}
			_remote_invoke += std::string("        (zpt:route \"patch\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"patch\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope))) ");
		}
		zpt::replace(_return, ";; ---> YOUR CODE HERE <--- ;;", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_lisp_delete() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("delete")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _f_name = std::string(this->__spec["name"]) + std::string("-") + std::string(this->__spec["type"]) + std::string("-delete");
	std::string _return = std::string("(defun ") + _f_name + std::string(" (performative topic envelope)\n");
	
	_return += this->build_lisp_handler_header(zpt::ev::Delete);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("    (json \"status\" 200)))\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("    (json \"status\" 200)))\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("    (json \"status\" 200)))\n");
		}
	}

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("nil"));
			}
			_remote_invoke += std::string("        (zpt:route \"delete\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"delete\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope))) ");
		}
		zpt::replace(_return, ";; ---> YOUR CODE HERE <--- ;;", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_lisp_head() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("head")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _f_name = std::string(this->__spec["name"]) + std::string("-") + std::string(this->__spec["type"]) + std::string("-head");
	std::string _return = std::string("(defun ") + _f_name + std::string(" (performative topic envelope)\n");
	
	_return += this->build_lisp_handler_header(zpt::ev::Head);

	if (!this->__spec["datum"]["ref"]->ok()) {
		_return += std::string("    (json \"status\" 200 \"headers\" (json \"Content-Length\" body-length) )))\n");
	}

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("nil"));
			}
			_remote_invoke += std::string("        (zpt:route \"head\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"head\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope))) ");
		}
		zpt::replace(_return, ";; ---> YOUR CODE HERE <--- ;;", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_lisp_reply() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("reply")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Reply,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Head);
	_return += std::string("return zpt::undefined;\n");
	_return += std::string("\n}\n}");
	return _return;
}

auto zpt::GenResource::build_python_validation(zpt::ev::performative _perf) -> std::string {
	if (!this->__spec["datum"]["name"]->ok() && !this->__spec["datum"]["fields"]->ok()) {
		return "";
	}
	if (_perf == zpt::ev::Reply) {
		return "";
	}
	zpt::json _fields;
	std::string _return;
	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			_fields = _found->second->spec()["fields"];
		}
	}
	else if (this->__spec["datum"]["fields"]->ok()) {
		_fields = this->__spec["datum"]["fields"];
	}
	if (_fields->type() == zpt::JSObject) {
		for (auto _field : _fields->obj()) {
			std::string _type(_field.second["type"]);
			zpt::json _opts = zpt::gen::get_opts(_field.second);
			
			if (_opts["mandatory"]->ok() && 
				(
					(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
					(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
					(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
				)
				&& !_opts["read-only"]->ok() && !_opts["default"]->ok() && !_opts["auto"]->ok()
			) {
				_return += std::string("assertz_mandatory(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
			}

			if (_perf != zpt::ev::Get && _perf != zpt::ev::Head && _perf != zpt::ev::Delete) {
				if (!_opts["read-only"]->ok()) {
					if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email") {
						_return += std::string("assertz_") + _type + std::string("(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
					else if (_type == "int") {
						_return += std::string("assertz_integer(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
					else if (_type == "boolean") {
						_return += std::string("assertz_boolean(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
					else if (_type == "double") {
						_return += std::string("assertz_double(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
					else if (_type == "timestamp") {
						_return += std::string("assertz_timestamp(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
					else if (_type == "json") {
						_return += std::string("assertz_complex(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
				}
				else {
					_return += std::string("_envelope[\"payload\"] >> \"") + _field.first + std::string("\";\n");
				}
			
				if (_opts["default"]->ok() && 
					(
						(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
						(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
						(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
					)
				) {
					if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::string(\"") + std::string(_opts["default"]) + std::string("\");\n");
						_return += std::string("}\n");
					}
					else if (_type == "int") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::integer(") + std::string(_opts["default"]) + std::string(");\n");
						_return += std::string("}\n");
					}
					else if (_type == "boolean") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::boolean(") + std::string(_opts["default"]) + std::string(");\n");
						_return += std::string("}\n");
					}
					else if (_type == "double") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::floating(") + std::string(_opts["default"]) + std::string(");\n");
						_return += std::string("}\n");
					}
					else if (_type == "timestamp") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date(") + std::string(_opts["default"]) + std::string(");\n");
						_return += std::string("}\n");
					}
					else if (_type == "object") {
						_return += std::string("if (!_envelope[\"payload\"][\"") + _field.first + std::string("\"]->ok()) {\n");
						_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json(\"") + std::string(_opts["default"]) + std::string("\");\n");
						_return += std::string("}\n");
					}
				}
			
				if (_opts["auto"]->ok()) {
					if (_opts["every-time"]->ok()) {
						if (_type == "token") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(24);\n");
						}
						else if (_type == "uuid") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_uuid();\n");
						}
						else if (_type == "hash") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(64);\n");
						}
						else if (_type == "timestamp") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date();\n");
						}
					}
					else if ((std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) || (std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put)) {
						if (_type == "token") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(24);\n");
						}
						else if (_type == "uuid") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_uuid();\n");
						}
						else if (_type == "hash") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(64);\n");
						}
						else if (_type == "timestamp") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date();\n");
						}
					}
					else if (std::string(this->__spec["type"]) == "document" && (_perf == zpt::ev::Patch || _perf == zpt::ev::Put)) {
						if (_type == "token") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(24);\n");
						}
						else if (_type == "uuid") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_uuid();\n");
						}
						else if (_type == "hash") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate::r_key(64);\n");
						}
						else if (_type == "timestamp") {
							_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date();\n");
						}
					}
					else if (_opts["mandatory"]->ok() && 
						(
							(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
							(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
							(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
						)
						&& !_opts["read-only"]->ok() && !_opts["default"]->ok()
					) {
						_return += std::string("assertz_mandatory(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
					}
				}
			}
		}
	}
	return _return;
}

auto zpt::GenResource::build_python_handler_header(zpt::ev::performative _perf) -> std::string {
	std::string _return;
	
	_return += this->build_validation(_perf);
	_return += std::string("\nzpt::json _t_split = zpt::split(_topic, \"/\");\n");
	_return += zpt::gen::url_pattern_to_vars(std::string(this->__spec["topic"]));
	_return += std::string("zpt::json _identity = zpt::rest::authorization::validate(\"") + std::string(this->__spec["topic"]) + std::string("\", _envelope, _emitter);\n");
	_return += std::string("\nzpt::json _r_body;\n");
	_return += std::string("/* ---> YOUR CODE HERE <---*/\n");
	if (_perf != zpt::ev::Reply && _perf != zpt::ev::Delete && this->__spec["links"]->type() == zpt::JSArray) {
		_return += std::string("if (_r_body[\"payload\"]->ok() && int(_r_body[\"status\"]) < 300) {\n");
		_return += std::string("_r_body[\"payload\"] << \"links\" << zpt::json({ ");
		
		for (auto _link : this->__spec["links"]->arr()) {
			std::string _topic;
			zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(_link["ref"]));		
			zpt::json _splited = zpt::split(std::string(_link["ref"]), "/");
			for (auto _part : _splited->arr()) {
				_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
			}
			_return += std::string("\"") + std::string(_link["rel"]) + std::string("\", zpt::join({ zpt::array") + _topic + std::string(" }, \"/\"), ");
		}
		_return += std::string(" });\n");
		_return += std::string("}\n");
	}
	return _return;
}

auto zpt::GenResource::build_python_get() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("get")) == std::end(_performatives->arr())) {
		return "";
	}
	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Get,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Get);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("        (json \"status\" 204)))\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body  };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body  };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_get(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("zpt::undefined"));
			}
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_python_post() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("post")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "document") {
		return "";
	}
	if (std::string(this->__spec["type"]) == "store") { 
		return "";
	}

	std::string _return("{\nzpt::ev::Post,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Post);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"status\", 201, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "controller") {
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_post(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Post,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"] }\n);\n");
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_python_put() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("put")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "collection") { 
		return "";
	}
	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Put,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Put);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"status\", 201, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_put(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Put,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"] }\n);\n");
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_python_patch() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("patch")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Patch,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Patch);
	
	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_patch(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("zpt::undefined"));
			}
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Patch,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"], \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Patch,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"], \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_python_delete() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("delete")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Delete,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Delete);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_delete(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("zpt::undefined"));
			}
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_python_head() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("head")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Head,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Head);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("return { \"headers\",\n{ \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("return { \"headers\",\n{ \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("return { \"headers\",\n{ \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_head(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(this->__spec["topic"]));
		
		zpt::json _splited = zpt::split(std::string(this->__spec["topic"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("zpt::undefined"));
			}
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Head,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Head,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_python_reply() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("reply")) == std::end(_performatives->arr())) {
		return "";
	}

	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _return("{\nzpt::ev::Reply,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {\n");
	_return += this->build_handler_header(zpt::ev::Head);
	_return += std::string("return zpt::undefined;\n");
	_return += std::string("\n}\n}");
	return _return;
}

auto zpt::GenResource::build_mutations() -> std::string {
	std::string _return;
	return _return;
}

auto zpt::gen::url_pattern_to_regexp(std::string _url) -> std::string {
	zpt::json _splited = zpt::split(_url, "/");
	std::string _return("^");

	for (auto _part : _splited->arr()) {
		_return += std::string("/") + (_part->str().find("{") != std::string::npos ? "([^/]+)" : _part->str());
	}

	_return += std::string("$");
	return _return;
}

auto zpt::gen::url_pattern_to_vars(std::string _url) -> std::string {
	zpt::json _splited = zpt::split(_url, "/");
	std::string _return;

	short _i = 0;
	for (auto _part : _splited->arr()) {
		if (_part->str().find("{") != std::string::npos) {
			_return += std::string("zpt::json _tv_") + _part->str().substr(1, _part->str().length() - 2) + std::string(" = _t_split[") + std::to_string(_i) + std::string("];\n");
		}
		_i++;
	}

	return _return;
}

auto zpt::gen::url_pattern_to_vars_lisp(std::string _url) -> std::string {
	zpt::json _splited = zpt::split(_url, "/");
	std::string _return;

	short _i = 0;
	for (auto _part : _splited->arr()) {
		if (_part->str().find("{") != std::string::npos) {
			_return += std::string("\n	  (tv-") + zpt::r_replace(_part->str().substr(1, _part->str().length() - 2), "_", "-") + std::string(" (zpt:topic-var t-split ") + std::to_string(_i) + std::string("))");
		}
		_i++;
	}
	return _return;
}

auto zpt::gen::url_pattern_to_params(std::string _url) -> zpt::json {
	zpt::json _splited = zpt::split(_url, "/");
	zpt::json _return = zpt::json::object();

	short _i = 0;
	for (auto _part : _splited->arr()) {
		if (_part->str().find("{") != std::string::npos) {
			_return << _part->str() << (std::string("_tv_") + _part->str().substr(1, _part->str().length() - 2));
		}
		_i++;
	}

	return _return;
}

auto zpt::gen::url_pattern_to_params_lisp(std::string _url) -> zpt::json {
	zpt::json _splited = zpt::split(_url, "/");
	zpt::json _return = zpt::json::object();

	short _i = 0;
	for (auto _part : _splited->arr()) {
		if (_part->str().find("{") != std::string::npos) {
			_return << _part->str() << (std::string("tv-") + zpt::r_replace(_part->str().substr(1, _part->str().length() - 2), "_", "-"));
		}
		_i++;
	}

	return _return;
}

auto zpt::conf::gen::init(int argc, char* argv[]) -> zpt::json {
	return zpt::conf::getopt(argc, argv);
}

auto zpt::gen::get_opts(zpt::json _field) -> zpt::json {
	if (!_field["opts"]->ok()) {
		return zpt::undefined;
	}
	zpt::json _opts = zpt::json::object();
	for (auto _opt : _field["opts"]->arr()) {
		if (_opt->str().find("=") != std::string::npos) {
			zpt::json _pair = zpt::split(_opt->str(), "=");
			_opts << _pair[0]->str() << _pair[1];
		}
		else {
			_opts << _opt->str() << true;
		}
	}
	return _opts;
}

