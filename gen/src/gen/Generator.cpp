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

		this->__specs << _file->str() << _spec;
		_spec << "dbms" << zpt::json::object();
		
		if (_spec["resources"]->type() == zpt::JSArray) {
			for (auto _resource : _spec["resources"]->arr()) {
				zpt::Generator::resources.insert(std::make_pair(std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]), zpt::gen::resource(new zpt::GenResource(_resource))));
			}
		}
		if (_spec["datums"]->type() == zpt::JSArray) {
			for (auto _datum : _spec["datums"]->arr()) {
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
}

auto zpt::Generator::build_container() -> void {
	std::string _container_cxx;
	zpt::load_path("/usr/share/zapata/gen/Container.cpp", _container_cxx);

	for (auto _pair : this->__specs->obj()) {
		zpt::json _spec = _pair.second;
		
		zpt::json _namespaces = zpt::split(std::string(_spec["namespace"]), "::");
		std::string _namespaces_begin;
		std::string _namespaces_end;
		for (auto _part : _namespaces->arr()) {
			_namespaces_begin += std::string("namespace ") + std::string(_part) + std::string(" {\n");
			_namespaces_end += "}\n";
		}
		zpt::replace(_container_cxx, "$[namespaces.begin]", _namespaces_begin);
		zpt::replace(_container_cxx, "$[namepsaces.end]", _namespaces_end);
		
		zpt::replace(_container_cxx, "$[resource.path.h]", "");

		zpt::replace(_container_cxx, "$[namespace]", std::string(_spec["namespace"]));

		std::string _connectors_initialize("{ ");
		for (auto _dbms : _spec["dbms"]->obj()) {
			_connectors_initialize += zpt::GenDatum::build_initialization(_dbms.first, std::string(_spec["namespace"]));
		}
		_connectors_initialize += "}";
		zpt::replace(_container_cxx, "$[datum.connectors.initialize]", _connectors_initialize);

		size_t _begin = _container_cxx.find("$[resource.registry.begin]") + 26;
		size_t _end = _container_cxx.find("$[resource.registry.end]") + 24;
		if (_begin != string::npos && _end != string::npos) {
			_container_cxx.erase(_begin, _end - _begin);
		}

		std::string _registry;
		if (_spec["resources"]->type() == zpt::JSArray) {
			for (auto _resource : _spec["resources"]->arr()) {
				std::string _key = std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]);
				_registry += zpt::Generator::resources.find(_key)->second->build_handlers();
			}
		}
		
		zpt::replace(_container_cxx, "$[resource.registry.begin]", _registry);
	}
	std::cout << _container_cxx << endl << flush;
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

auto zpt::GenDatum::build_get(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::query(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _envelope[\"params\"], _emitter);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::get(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _emitter);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_post(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "collection") {
		_return += std::string("_r_body = ") + _class + std::string("::insert(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _envelope[\"params\"], _emitter);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_put(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::insert(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _envelope[\"params\"], _emitter);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::save(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _emitter);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_patch(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::set(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _envelope[\"params\"], _emitter);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::set(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _emitter);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_delete(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::remove(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _envelope[\"params\"], _emitter);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::remove(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _emitter);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_head(zpt::json _resource) -> std::string {
	std::string _return;
	std::string _name = std::string(this->__spec["name"]);
	std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
	std::string _class = std::string(this->__spec["namespace"]) + std::string("::") + _name;
	if (std::string(_resource["type"]) == "collection" || std::string(_resource["type"]) == "store") {
		_return += std::string("_r_body = ") + _class + std::string("::query(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _envelope[\"params\"], _emitter);\n");
	}
	else if (std::string(_resource["type"]) == "document") {
		_return += std::string("_r_body = ") + _class + std::string("::get(\"") + _name + std::string("\", _topic, _envelope[\"payload\"], _emitter);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_mutations() -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_initialization(std::string _dbms, std::string _namespace) -> std::string {
	if (_dbms == "postgresql") {
		return "{ \"dbms.pgsql\", zpt::connector(new zpt::pgsql::Client(_emitter->options(), \"pgsql." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	else if (_dbms == "mariadb") {
		return "{ \"dbms.mariadb\", zpt::connector(new zpt::mariadb::Client(_emitter->options(), \"mariadb." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	else if (_dbms == "mongodb") {
		return "{ \"dbms.mongodb\", zpt::connector(new zpt::mongodb::Client(_emitter->options(), \"mongodb." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	else if (_dbms == "redis") {
		return "{ \"dbms.redis\", zpt::connector(new zpt::redis::Client(_emitter->options(), \"redis." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
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
	std::string _container_cxx;
	zpt::load_path("/usr/share/zapata/gen/Container.cpp", _container_cxx);

	size_t _begin = _container_cxx.find("$[resource.registry.begin]") + 26;
	size_t _end = _container_cxx.find("$[resource.registry.end]");

	std::string _return(_container_cxx.substr(_begin, _end - _begin));
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
	std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["ref"]->str());	
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
	_return += std::string("\nzpt::json _r_body;\n/* ---> YOUR CODE HERE <---*/\n");
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

	if (std::string(this->__spec["type"]) == "collection") { 
		_return += std::string("return { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body };\n");
	}
	if (std::string(this->__spec["type"]) == "store") { 
		_return += std::string("return { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body  };\n");
	}
	if (std::string(this->__spec["type"]) == "document") {
		_return += std::string("return { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body  };\n");
	}
	_return += std::string("\n}\n},\n");

	zpt::json _opts = this->__spec["datum"]["opts"];
	if (_opts->ok() && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("generate-data-access")) != std::end(_opts->arr())) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["ref"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_get(this->__spec));
		}
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

	if (std::string(this->__spec["type"]) == "collection") { 
		_return += std::string("return { \"status\", 201, \"payload\", _r_body };\n");
	}
	if (std::string(this->__spec["type"]) == "controller") {
		_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
	}
	_return += std::string("\n}\n},\n");

	zpt::json _opts = this->__spec["datum"]["opts"];
	if (_opts->ok() && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("generate-data-access")) != std::end(_opts->arr())) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["ref"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_post(this->__spec));
		}
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

	if (std::string(this->__spec["type"]) == "store") { 
		_return += std::string("return { \"status\", 201, \"payload\", _r_body };\n");
	}
	if (std::string(this->__spec["type"]) == "document") {
		_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
	}
	_return += std::string("\n}\n},\n");

	zpt::json _opts = this->__spec["datum"]["opts"];
	if (_opts->ok() && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("generate-data-access")) != std::end(_opts->arr())) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["ref"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_put(this->__spec));
		}
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
	
	if (std::string(this->__spec["type"]) == "collection") { 
		_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
	}
	if (std::string(this->__spec["type"]) == "store") { 
		_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
	}
	if (std::string(this->__spec["type"]) == "document") {
		_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
	}
	_return += std::string("\n}\n},\n");

	zpt::json _opts = this->__spec["datum"]["opts"];
	if (_opts->ok() && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("generate-data-access")) != std::end(_opts->arr())) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["ref"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_patch(this->__spec));
		}
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

	if (std::string(this->__spec["type"]) == "collection") { 
		_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
	}
	if (std::string(this->__spec["type"]) == "store") { 
		_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
	}
	if (std::string(this->__spec["type"]) == "document") {
		_return += std::string("return { \"status\", 200, \"payload\", _r_body };\n");
	}
	_return += std::string("\n}\n},\n");

	zpt::json _opts = this->__spec["datum"]["opts"];
	if (_opts->ok() && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("generate-data-access")) != std::end(_opts->arr())) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["ref"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_delete(this->__spec));
		}
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

	if (std::string(this->__spec["type"]) == "collection") { 
		_return += std::string("return { \"headers\", { \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
	}
	if (std::string(this->__spec["type"]) == "store") { 
		_return += std::string("return { \"headers\", { \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
	}
	if (std::string(this->__spec["type"]) == "document") {
		_return += std::string("return { \"headers\", { \"Content-Length\", std::string(_r_body).length() }, \"status\", (_r_body->ok() ? 200 : 204) };\n");
	}
	_return += std::string("\n}\n},\n");

	zpt::json _opts = this->__spec["datum"]["opts"];
	if (_opts->ok() && std::find(std::begin(_opts->arr()), std::end(_opts->arr()), zpt::json::string("generate-data-access")) != std::end(_opts->arr())) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["ref"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _found->second->build_head(this->__spec));
		}
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

auto zpt::conf::gen::init(int argc, char* argv[]) -> zpt::json {
	return zpt::conf::getopt(argc, argv);
}
