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

std::map<std::string, zpt::gen::datum> zpt::Generator::datums;
std::map<std::string, zpt::gen::resource> zpt::Generator::resources;

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
				_resource << "namespace" << _spec["namespace"];
				zpt::Generator::resources.insert(std::make_pair(std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]), zpt::gen::resource(new zpt::GenResource(_resource))));
			}
		}
		if (_spec["datums"]->type() == zpt::JSArray) {
			for (auto _datum : _spec["datums"]->arr()) {
				_datum << "namespace" << _spec["namespace"];
				zpt::Generator::datums.insert(std::make_pair(std::string(_datum["namespace"]) + std::string("::") + std::string(_datum["name"]), zpt::gen::datum(new zpt::GenDatum(_datum))));
				if (_datum["dbms"]->type() == zpt::JSArray) {
					for (auto _dbms : _datum["dbms"]->arr()) {
						_spec["dbms"] << std::string(_dbms) << true;
					}
				}
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

		for (auto _datum : _spec["datums"]->arr()) {
			std::string _h_file = std::string(this->__options["data-out-h"][0]) + std::string("/") + std::string(_datum["name"]) + std::string(".h");
			std::string _cxx_file = std::string(this->__options["data-out-cxx"][0]) + std::string("/") + std::string(_datum["name"]) + std::string(".cpp");
			std::string _datum_h;
			zpt::load_path("/usr/share/zapata/gen/Datum.h", _datum_h);
			std::string _datum_cxx;
			zpt::load_path("/usr/share/zapata/gen/Datum.cpp", _datum_cxx);

			auto _found = zpt::Generator::datums.find(std::string(_datum["namespace"]) + std::string("::") + std::string(_datum["name"]));
			if (_found != zpt::Generator::datums.end()) {
				if (_datum["extends"]->type() == zpt::JSObject) {
					zpt::replace(_datum_cxx, "$[datum.extends.get]", _found->second->build_extends_get());
					zpt::replace(_datum_cxx, "$[datum.extends.query]", _found->second->build_extends_query());
					zpt::replace(_datum_cxx, "$[datum.extends.insert]", _found->second->build_extends_insert());
					zpt::replace(_datum_cxx, "$[datum.extends.save]", _found->second->build_extends_save());
					zpt::replace(_datum_cxx, "$[datum.extends.set.topic]", _found->second->build_extends_set_topic());
					zpt::replace(_datum_cxx, "$[datum.extends.set.pattern]", _found->second->build_extends_set_pattern());
					zpt::replace(_datum_cxx, "$[datum.extends.remove.topic]", _found->second->build_extends_remove_topic());
					zpt::replace(_datum_cxx, "$[datum.extends.remove.pattern]", _found->second->build_extends_remove_pattern());
				}
				else {
					zpt::replace(_datum_cxx, "$[datum.extends.get]", "zpt::connector _c = _emitter->connector(\"$[datum.method.get.client]\");\nzpt::json _r_data = _c->get(\"$[datum.collection]\", _topic);\n");
					zpt::replace(_datum_cxx, "$[datum.extends.query]", "zpt::connector _c = _emitter->connector(\"$[datum.method.query.client]\");\nzpt::json _r_data = _c->query(\"$[datum.collection]\", _filter, _filter);\n");
					zpt::replace(_datum_cxx, "$[datum.extends.insert]", "zpt::connector _c = _emitter->connector(\"$[datum.method.insert.client]\");\n\n_document <<\n\"created\" << zpt::json::date() <<\n\"updated\" << zpt::json::date();\n\nzpt::json _r_data = _c->insert(\"$[datum.collection]\", _topic, _document);\n");
					zpt::replace(_datum_cxx, "$[datum.extends.save]", "zpt::connector _c = _emitter->connector(\"$[datum.method.save.client]\");\n\n_document <<\n\"updated\" << zpt::json::date();\n\nzpt::json _r_data = _c->save(\"$[datum.collection]\", _topic, _document);\n");
					zpt::replace(_datum_cxx, "$[datum.extends.set.topic]", "zpt::connector _c = _emitter->connector(\"$[datum.method.set.client]\");\n\n_document <<\n\"updated\" << zpt::json::date();\n\nzpt::json _r_data = _c->set(\"$[datum.collection]\", _topic, _document);\n");
					zpt::replace(_datum_cxx, "$[datum.extends.set.pattern]", "zpt::connector _c = _emitter->connector(\"$[datum.method.set.client]\");\n\n_document <<\n\"updated\" << zpt::json::date();\n\nzpt::json _r_data = _c->set(\"$[datum.collection]\", _filter, _document);\n");
					zpt::replace(_datum_cxx, "$[datum.extends.remove.topic]", "zpt::connector _c = _emitter->connector(\"$[datum.method.remove.client]\");\nzpt::json _r_data = _c->remove(\"$[datum.collection]\", _topic);\n");
					zpt::replace(_datum_cxx, "$[datum.extends.remove.pattern]", "zpt::connector _c = _emitter->connector(\"$[datum.method.remove.client]\");\nzpt::json _r_data = _c->remove(\"$[datum.collection]\", _filter, _filter);\n");
				}
			}
			
			std::string _namespace = std::string(_spec["namespace"]);
			std::string _collection = std::string(_datum["name"]);
			zpt::replace(_datum_h, "$[datum.collection]", _collection);
			zpt::replace(_datum_cxx, "$[datum.collection]", _collection);

			zpt::replace(_datum_h, "$[datum.name]", std::string(_datum["name"]));
			zpt::replace(_datum_cxx, "$[datum.name]", std::string(_datum["name"]));
		
			zpt::json _namespaces = zpt::split(_namespace, "::");
			std::string _namespaces_begin;
			std::string _namespaces_end;
			for (auto _part : _namespaces->arr()) {
				_namespaces_begin += std::string("namespace ") + std::string(_part) + std::string(" {\n");
				_namespaces_end += "}\n";
			}
			zpt::replace(_datum_h, "$[namespaces.begin]", _namespaces_begin);
			zpt::replace(_datum_h, "$[namepsaces.end]", _namespaces_end);
			zpt::replace(_datum_cxx, "$[namespaces.begin]", _namespaces_begin);
			zpt::replace(_datum_cxx, "$[namepsaces.end]", _namespaces_end);

			std::string _include_path(_h_file.data());
			zpt::replace(_include_path, std::string(this->__options["prefix-h"][0]), "");
			if (_include_path.front() == '/') {
				_include_path.erase(0, 1);
			}
			_include_path.erase(_include_path.length() - 1, 2);
			zpt::replace(_datum_cxx, "$[resource.path.h]", std::string("#include <") + _include_path + std::string(">"));

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

			std::ofstream _h_ofs(_h_file.data());
			_h_ofs << _datum_h << endl << flush;
			_h_ofs.close();
			std::ofstream _cxx_ofs(_cxx_file.data());
			_cxx_ofs << _datum_cxx << endl << flush;
			_cxx_ofs.close();
			zlog(_h_file, zpt::debug);
			zlog(_cxx_file, zpt::debug);
			// zlog(_datum_h, zpt::debug);
			// zlog(_datum_cxx, zpt::debug);
		}
	}
}

auto zpt::Generator::build_container() -> void {
	std::string _container_cxx;
	zpt::load_path("/usr/share/zapata/gen/Container.cpp", _container_cxx);

	for (auto _pair : this->__specs->obj()) {
		zpt::json _spec = _pair.second;
		
		std::string _connectors_initialize("{ ");
		for (auto _dbms : _spec["dbms"]->obj()) {
			_connectors_initialize += zpt::GenDatum::build_initialization(_dbms.first, std::string(_spec["namespace"]));
		}
		_connectors_initialize += "}";
		zpt::replace(_container_cxx, "$[datum.connectors.initialize]", _connectors_initialize);

		zpt::replace(_container_cxx, "$[namespace]", std::string(_spec["namespace"]));

		std::string _includes;
		std::string _registry;
		if (_spec["resources"]->type() == zpt::JSArray) {
			for (auto _resource : _spec["resources"]->arr()) {
				std::string _key = std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]);

				std::string _include(std::string(this->__options["resource-out-h"][0]) + std::string("/") + std::string(_resource["type"]) + std::string("s/") + std::string(_resource["name"]));
				zpt::replace(_include, std::string(this->__options["prefix-h"][0]), "");
				if (_include.front() == '/') {
					_include.erase(0, 1);
				}
				_includes += std::string("#include <") + _include + std::string(">\n");				
				_registry += zpt::Generator::resources.find(_key)->second->build_handlers();
			}
		}
		
		zpt::replace(_container_cxx, "$[resource.path.h]", _includes);
		zpt::replace(_container_cxx, "$[resource.handlers.delegate]", _registry);
	}
	
	for (auto _pair : this->__specs->obj()) {
		zpt::json _spec = _pair.second;
		
	}
	//zlog(_handler_cxx, zpt::debug);
}

auto zpt::Generator::build_mutations() -> void {
}

zpt::GenDatum::GenDatum(zpt::json _spec) : __spec(_spec) {
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

auto zpt::GenDatum::build_get(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::query(_topic, _envelope[\"params\"], _emitter, _identity, _envelope);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::get(_topic, _emitter);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_post(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "collection") {
		_return += std::string("_r_body = ") + _class + std::string("::insert(_topic, _envelope[\"payload\"], _emitter, _identity, _envelope);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_put(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
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
	std::string _name = std::string(this->__spec["name"]);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
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
	std::string _name = std::string(this->__spec["name"]);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
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
	std::string _name = std::string(this->__spec["name"]);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::query(_topic, _envelope[\"params\"], _emitter, _identity, _envelope);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::get(_topic, _emitter, _identity, _envelope);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_mutations() -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_associations_get() -> std::string{
	std::string _return;
	if (this->__spec["fields"]->type() == zpt::JSObject) {
		for (auto _field : this->__spec["fields"]->obj()) {
			std::string _type(_field.second["type"]);
			zpt::json _opts = _field.second["opts"];

			if (_type == "object") {
				if (
					_opts->type() == zpt::JSArray &&
					_field.second["ref"]->ok() &&
					std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("on-demand")) != std::end(_opts->arr())
				) {
					std::string _topic;
					zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
					zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
					for (auto _part : _splited->arr()) {
						_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
					}

					std::string _remote_invoke;
					if (_rel->obj()->size() != 0) {
						std::string _params = this->build_params(_rel, false);
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n)[\"payload\"];\n");
					}
					else {
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
					}
					
					_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
					_return += std::string("_r_data << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->type() == zpt::JSArray ? _d_") + _field.first + std::string("[\"elements\"][0] : _d_") + _field.first + std::string(");\n");
				}
			}
			else if (_type == "array") {
				if (
					_opts->type() == zpt::JSArray &&
					_field.second["ref"]->type() == zpt::JSString &&
					std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("on-demand")) != std::end(_opts->arr())
				) {

					std::string _topic;
					zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
					zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
					for (auto _part : _splited->arr()) {
						_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
					}

					std::string _remote_invoke;
					if (_rel->obj()->size() != 0) {
						std::string _params = this->build_params(_rel, false);
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n)[\"payload\"];\n");
					}
					else {
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
					}

					_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
					_return += std::string("_r_data << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->type() == zpt::JSArray ? _d_") + _field.first + std::string("[\"elements\"] : { zpt::array, _d_") + _field.first + std::string(" });\n");
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
			zpt::json _opts = _field.second["opts"];

			if (_type == "object") {
				if (
					_opts->type() == zpt::JSArray &&
					_field.second["ref"]->ok() &&
					std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("on-demand")) != std::end(_opts->arr())
				) {
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
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n)[\"payload\"];\n");
					}
					else {
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
					}
					
					_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
					_return += std::string("_d_element << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->type() == zpt::JSArray ? _d_") + _field.first + std::string("[\"elements\"][0] : _d_") + _field.first + std::string(");\n");

					if (!_found) {
						_return += std::string("}\n");
						_return += std::string("}\n");
					}
					_found = true;
				}
			}
			else if (_type == "array") {
				if (
					_opts->type() == zpt::JSArray &&
					_field.second["ref"]->type() == zpt::JSString &&
					std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("on-demand")) != std::end(_opts->arr())
				) {
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
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n)[\"payload\"];\n");
					}
					else {
						_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
					}
					
					_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
					_return += std::string("_d_element << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->type() == zpt::JSArray ? _d_") + _field.first + std::string("[\"elements\"] : { zpt::array, _d_") + _field.first + std::string("});\n");

					if (!_found) {
						_return += std::string("}\n");
						_return += std::string("}\n");
					}
					_found = true;
				}
			}
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
	std::string _return;
	
	if (this->__spec["extends"]["ref"]->ok()) {
		zpt::json _opts = this->__spec["extends"]["opts"];
		if (_opts->type() == zpt::JSArray && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("on-demand")) != std::end(_opts->arr())) {
			std::string _topic;
			zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["extends"]["rel"]));
			zpt::json _splited = zpt::split(std::string(this->__spec["extends"]["ref"]), "/");
			for (auto _part : _splited->arr()) {
				_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
			}

			if (_rel->obj()->size() != 0) {
				std::string _params = this->build_params(_rel, false);
				_return += std::string("_r_data = _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n)[\"payload\"];\n");
			}
			else {
				_return += std::string("_r_data = _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
			}
		}
		else {
			_return += "zpt::connector _c = _emitter->connector(\"$[datum.method.get.client]\");\nzpt::json _r_data = _c->get(\"$[datum.collection]\", _topic);\n";
		}
	}
	return _return;
}

auto zpt::GenDatum::build_extends_query() -> std::string{
	std::string _return;
	
	if (this->__spec["extends"]["ref"]->ok()) {
		zpt::json _opts = this->__spec["extends"]["opts"];
		if (_opts->type() == zpt::JSArray && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("on-demand")) != std::end(_opts->arr())) {
			std::string _topic;
			zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["extends"]["rel"]));
			zpt::json _splited = zpt::split(std::string(this->__spec["extends"]["ref"]), "/");
			for (auto _part : _splited->arr()) {
				_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
			}

			if (_rel->obj()->size() != 0) {
				std::string _params = this->build_params(_rel, true);
				_return += std::string("_r_data = _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n)[\"payload\"];\n");
			}
			else {
				_return += std::string("_r_data = _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
			}
			_return += std::string("_r_data << \"payload\" << (_r_data[\"payload\"][\"elements\"]->ok() ? _r_data[\"payload\"] : { \"size\", 1, \"elements\", _r_data[\"payload\"] });\n");
		}
		else {
			_return += std::string("zpt::connector _c = _emitter->connector(\"$[datum.method.query.client]\");\nzpt::json _r_data = _c->query(\"$[datum.collection]\", _filter, _filter);\n");
		}
	}
	return _return;
}

auto zpt::GenDatum::build_extends_insert() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_extends_save() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_extends_set_topic() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_extends_set_pattern() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_extends_remove_topic() -> std::string{
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_extends_remove_pattern() -> std::string{
	std::string _return;
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
			return std::string("dbms.") + _db->str() + std::string(".") + zpt::r_replace(_namespace, "::", ".");
		}
	}
	return "";
}

zpt::GenResource::GenResource(zpt::json _spec) : __spec(_spec) {
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

auto zpt::GenResource::build_handlers() -> std::string {
	std::string _handler_h;
	zpt::load_path("/usr/share/zapata/gen/Handlers.h", _handler_h);
	std::string _handler_cxx;
	zpt::load_path("/usr/share/zapata/gen/Handlers.cpp", _handler_cxx);

	std::string _handler_cxx;
	zpt::load_path("/usr/share/zapata/gen/Container.cpp", _handler_cxx);

	size_t _begin = _handler_cxx.find("$[resource.registry.begin]") + 26;
	size_t _end = _handler_cxx.find("$[resource.registry.end]");

	std::string _return(_handler_cxx.substr(_begin, _end - _begin));
	zpt::replace(_return, "$[resource.topic.regex]", zpt::gen::url_pattern_to_regexp(this->__spec["topic"]));
	zpt::replace(_return, "$[resource.handler.get]", this->build_get());
	zpt::replace(_return, "$[resource.handler.post]", this->build_post());
	zpt::replace(_return, "$[resource.handler.put]", this->build_put());
	zpt::replace(_return, "$[resource.handler.patch]", this->build_patch());
	zpt::replace(_return, "$[resource.handler.delete]", this->build_delete());
	zpt::replace(_return, "$[resource.handler.head]", this->build_head());

	zpt::json _resource_opts = zpt::json::object();
	if (this->__spec["protocols"]->type() == zpt::JSArray) {
		for (auto _proto : this->__spec["protocols"]->arr()) {
			_resource_opts << _proto->str() << true;
		}
	}
	zpt::replace(_return, "$[resource.opts]", zpt::r_replace(std::string(_resource_opts), ":", ", "));
	
	return _return;
}

auto zpt::GenResource::build_validation(bool _mandatory) -> std::string {
	if (!this->__spec["datum"]["name"]->ok()) {
		return "";
	}
	std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
	std::string _return;
	if (_found != zpt::Generator::datums.end()) {
		for (auto _field : _found->second->spec()["fields"]->obj()) {
			std::string _type(_field.second["type"]);
			zpt::json _opts = _field.second["opts"];

			if (_mandatory
				&& _opts->type() == zpt::JSArray
				&& std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("mandatory")) != std::end(_opts->arr())
				&& std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("read-only")) == std::end(_opts->arr())
			) {
				_return += std::string("assertz_mandatory(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
			}

			if (!_opts->ok() || std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("read-only")) == std::end(_opts->arr())) {
				if ( _type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri") {
					_return += std::string("assertz_") + _type + std::string("(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
				}
				else if ( _type == "int") {
					_return += std::string("assertz_integer(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
				}
				else if ( _type == "double") {
					_return += std::string("assertz_double(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
				}
				else if ( _type == "timestamp") {
					_return += std::string("assertz_timestamp(_envelope[\"payload\"], \"") + _field.first + std::string("\", 412);\n");
				}
			}
			else if (_opts->ok()) {
				_return += std::string("_envelope[\"payload\"] >> \"") + _field.first + std::string("\";\n");
			}

			if (_mandatory && _opts->ok() && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("auto")) != std::end(_opts->arr())) {
				if ( _type == "token") {
					_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::generate_key(24);\n");
				}
			}
			if (_opts->ok() && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("auto")) != std::end(_opts->arr())) {
				if ( _type == "timestamp") {
					_return += std::string("_envelope[\"payload\"] << \"") + _field.first + std::string("\" << zpt::json::date();\n");
				}
			}
		}
	}
	return _return;
}

auto zpt::GenResource::build_handler_header(zpt::ev::performative _perf) -> std::string {
	std::string _return;
	bool _mandatory =
	(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
	(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
	(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put);
	
	_return += this->build_validation(_mandatory);
	_return += std::string("\nzpt::json _t_split = zpt::split(_topic, \"/\");\n");
	_return += zpt::gen::url_pattern_to_vars(std::string(this->__spec["topic"]));
	_return += std::string("zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);\n");
	_return += std::string("\nzpt::json _r_body;\n");
	_return += std::string("/* ---> YOUR CODE HERE <---*/\n");
	if (_perf != zpt::ev::Delete && this->__spec["links"]->type() == zpt::JSArray) {
		_return += std::string("if (_r_body[\"payload\"]->ok() && int(_r_body[\"status\"]) < 300) {\n");
		_return += std::string("_r_body[\"payload\"] << \"links\" << { ");
		
		for (auto _link : this->__spec["links"]->arr()) {
			std::string _topic;
			zpt::json _url_params = zpt::gen::url_pattern_to_params(std::string(_link["ref"]));		
			zpt::json _splited = zpt::split(std::string(_link["ref"]), "/");
			for (auto _part : _splited->arr()) {
				_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
			}
			_return += std::string("\"") + std::string(_link["rel"]) + std::string("\", zpt::path::join({ ") + _topic + std::string(" })");
		}
		_return += std::string("};\n");
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
	_return += std::string("\n}\n},\n");

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
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n);\n");
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
	_return += std::string("\n}\n},\n");

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
	_return += std::string("\n}\n},\n");

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
	_return += std::string("\n}\n},\n");

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
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Patch,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"], \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n);\n");
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
	_return += std::string("\n}\n},\n");

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
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n);\n");
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
	_return += std::string("\n}\n},\n");

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
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Head,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + { ") + _params + std::string(" }) }\n);\n");
		}
		else {
			_remote_invoke += std::string("return _emitter->route(\nzpt::ev::Head,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n);\n");
		}
		zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _remote_invoke);
	}
	
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
		_return += std::string("/") + (_part->str().find("{") != string::npos ? "([^/]+)" : _part->str());
	}

	_return += std::string("$");
	return _return;
}

auto zpt::gen::url_pattern_to_vars(std::string _url) -> std::string {
	zpt::json _splited = zpt::split(_url, "/");
	std::string _return;

	short _i = 0;
	for (auto _part : _splited->arr()) {
		if (_part->str().find("{") != string::npos) {
			_return += std::string("zpt::json _tv_") + _part->str().substr(1, _part->str().length() - 2) + std::string(" = _t_split[") + std::to_string(_i) + std::string("];\n");
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
		if (_part->str().find("{") != string::npos) {
			_return << _part->str() << (std::string("_tv_") + _part->str().substr(1, _part->str().length() - 2));
		}
		_i++;
	}

	return _return;
}

auto zpt::conf::gen::init(int argc, char* argv[]) -> zpt::json {
	return zpt::conf::getopt(argc, argv);
}
