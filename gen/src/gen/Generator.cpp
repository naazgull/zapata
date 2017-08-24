/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <n@zgul.me>

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
std::map<std::string, zpt::gen::datum> zpt::Generator::included_datums;
std::map<std::string, zpt::gen::resource> zpt::Generator::included_resources;
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
	if (_options["c"]->is_string()) {
		_ifs.open(_options["c"]->str().data());
	}
	else {
		_ifs.open(".zpt_rc");
	}
	if (_ifs.is_open()) {
		zpt::json _conf_options;
		_ifs >> _conf_options;
		_conf_options = _conf_options - _options;
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
	if (!this->__options["version"]->ok()) {
		this->__options << "version" << zpt::json{ zpt::array, 0, 1, 0 };
	}
	if (!this->__options["api_version"]->ok()) {
		this->__options << "api_version" << (std::string("v") + std::string(this->__options["version"][0]));
	}

	for (auto _file : this->__options["files"]->arr()) {
		std::ifstream _ifs(_file->str().data());
		zpt::json _spec;
		_ifs >> _spec;

		if (_spec["ref"]->is_array()) {
			for (auto _include : _spec["ref"]->arr()) {
				std::ifstream _iifs(_include->str().data());
				zpt::json _ispec;
				_iifs >> _ispec;
				
				if (_ispec["resources"]->is_array()) {
					for (auto _resource : _ispec["resources"]->arr()) {
						_resource << "namespace" << _ispec["namespace"] << "lib" << _ispec["lib"] << "spec_name" << _ispec["name"];
						zpt::gen::resource _r = zpt::gen::resource(new zpt::GenResource(_resource, this->__options));
						zpt::Generator::included_resources.insert(std::make_pair(std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]->is_array() ? _resource["topic"][0] : _resource["topic"]), _r));
						zpt::Generator::included_resources.insert(std::make_pair(zpt::gen::url_pattern_to_regexp(_resource["topic"]), _r));
					}
				}
				if (_ispec["datums"]->is_array()) {
					for (auto _datum : _ispec["datums"]->arr()) {
						_datum << "namespace" << _ispec["namespace"] << "lib" << _ispec["lib"] << "spec_name" << _ispec["name"];
						zpt::Generator::included_datums.insert(std::make_pair(std::string(_datum["namespace"]) + std::string("::") + std::string(_datum["name"]), zpt::gen::datum(new zpt::GenDatum(_datum, this->__options))));
					}
				}
			}
		}
	}
	for (auto _datum : zpt::Generator::included_datums) {
		if (_datum.second->spec()["extends"]->ok()) {
			auto _parent = zpt::Generator::included_datums.find(_datum.second->spec()["extends"]["name"]->str());
			if (_parent != zpt::Generator::included_datums.end()) {
				_datum.second->spec() << "fields" << (_parent->second->spec()["fields"] + _datum.second->spec()["fields"]);
			}
		}
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

		if (_spec["resources"]->is_array()) {
			for (auto _resource : _spec["resources"]->arr()) {
				_resource << "namespace" << _spec["namespace"] << "lib" << _spec["lib"] << "spec_name" << _spec["name"];
				zpt::gen::resource _r = zpt::gen::resource(new zpt::GenResource(_resource, this->__options));
				zpt::Generator::resources.insert(std::make_pair(std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]->is_array() ? _resource["topic"][0] : _resource["topic"]), _r));
				zpt::Generator::resources.insert(std::make_pair(zpt::gen::url_pattern_to_regexp(_resource["topic"]), _r));
				if (_resource["datum"]["href"]->ok()) {
					zpt::Generator::alias.insert(std::make_pair(zpt::gen::url_pattern_to_regexp(_resource["topic"]), zpt::gen::url_pattern_to_regexp(_resource["datum"]["href"])));
				}
			}
		}
		if (_spec["datums"]->is_array()) {
			for (auto _datum : _spec["datums"]->arr()) {
				_datum << "namespace" << _spec["namespace"] << "lib" << _spec["lib"] << "spec_name" << _spec["name"];
				zpt::Generator::datums.insert(std::make_pair(std::string(_datum["namespace"]) + std::string("::") + std::string(_datum["name"]), zpt::gen::datum(new zpt::GenDatum(_datum, this->__options))));
				if (_datum["dbms"]->is_array()) {
					for (auto _dbms : _datum["dbms"]->arr()) {
						_spec["dbms"] << std::string(_dbms) << true;
					}
				}

				std::string _h_file = std::string(this->__options["data-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/") + std::string(_datum["name"]) + std::string(".h");
				std::string _include = zpt::r_replace(_h_file, std::string(this->__options["prefix-h"][0]), "");
				if (_include.front() == '/') {
					_include.erase(0, 1);
				}
				if (_datum["dbms"]->is_array()) {
					for (auto _dbms : _datum["dbms"]->arr()) {
						if (zpt::Generator::datum_includes.find(std::string("#include <zapata/") + std::string(_dbms) + std::string(".h>\n")) == std::string::npos) {
							zpt::Generator::datum_includes += std::string("#include <zapata/") + std::string(_dbms) + std::string(".h>\n");
						}
					}
				}
				zpt::Generator::datum_includes += std::string("#include <") + _include + std::string(">\n");
			}
		}
	}
	for (auto _datum : zpt::Generator::datums) {
		if (_datum.second->spec()["extends"]->ok()) {
			auto _parent = zpt::Generator::datums.find(_datum.second->spec()["extends"]["name"]->str());
			if (_parent != zpt::Generator::datums.end()) {
				_datum.second->spec() << "fields" << (_parent->second->spec()["fields"] + _datum.second->spec()["fields"]);
			}
			else {
				_parent = zpt::Generator::included_datums.find(_datum.second->spec()["extends"]["name"]->str());
				if (_parent != zpt::Generator::included_datums.end()) {
					_datum.second->spec() << "fields" << (_parent->second->spec()["fields"] + _datum.second->spec()["fields"]);
				}
			}
		}
	}
}

auto zpt::Generator::get_datum(std::string _ref) -> std::string {
	auto _resource = zpt::Generator::resources.find(zpt::gen::url_pattern_to_regexp(zpt::json::string(_ref)));
	if (_resource != zpt::Generator::resources.end()) {
		zpt::json _splited = zpt::split(std::string(_resource->second->spec()["datum"]["name"]), "::");
		if (_splited->arr()->size() != 0) {
			return std::string(_splited[_splited->arr()->size() - 1]);
		}
	}
	return "";
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
	if (this->__options["resource-out-cxx"]->ok()) {
		this->build_docs();
	}
}

auto zpt::Generator::build_docs() -> void {
	struct stat _buffer;
	std::string _to_pdf_file = std::string("ENDPOINTS.md");
	std::string _apiary_file = std::string("apiary.apib");
	std::string _proj_name = std::string (this->__options["abbr"]);
	std::transform(_proj_name.begin(), _proj_name.end(), _proj_name.begin(), ::toupper);
	std::string _doc = std::string("% ") + zpt::r_replace(zpt::r_replace(_proj_name, "-", " "), "_", " ") + std::string(" CONTAINER\n\n");
	std::string _apiary = std::string("FORMAT: 1A\nHOST: https://api.something.something/\n\n# ") + _proj_name + std::string(" CONTAINER\n\n");
	for (auto _pair : this->__specs->obj()) {
		zpt::json _spec = _pair.second;
		std::string _name = std::string(_spec["name"]);
		std::string _md_file = std::string(this->__options["resource-out-cxx"][0]) + std::string("/") + _name + std::string("/README.md");
		std::transform(_name.begin(), _name.end(), _name.begin(), ::toupper);
		std::string _readme = std::string("# ") + _name + std::string(" INDEX\n\n") + std::string(_spec["description"]) + std::string("\n\n");
		_doc += std::string("# ") + _name + std::string("\n\n") + std::string(_spec["description"]) + std::string("\n\n");
		_apiary += std::string("# Group ") + _name + std::string("\n\n") + std::string(_spec["description"]) + std::string("\n\n");

		if (_spec["datums"]->is_array()) {
			_doc += std::string("## DATUMS\n\n");
			_readme += std::string("\n## DATUMS\n\n");
			for (auto _datum : _spec["datums"]->arr()) {
				_readme += std::string("- **") + std::string(_datum["name"]) + std::string("**: ");
				_readme += std::string(_datum["description"]) + std::string("\n");

				_doc += std::string("### ") + std::string(_datum["name"]) + std::string("\n\n");
				_doc += std::string("---\n\n");				
				_doc += std::string("(in _") + std::string(_datum["namespace"]) + std::string("_)\n\n");
				_doc += std::string(_datum["description"]) + std::string("\n\n");
				_doc += std::string("#### Supported DBMS:\n\n- ") + zpt::r_replace(zpt::r_replace(zpt::r_replace(zpt::r_replace(std::string(_datum["dbms"]), "[", ""), "]", ""), "\"", ""), ",", "\n- ") + std::string("\n\n");
				_doc += std::string(
					"##### Fields\n\n"
					"| FIELD | TYPE | DESCRIPTION |\n"
					"|:------|:----:|:-------|\n"
					"| _**id**_ | uuid | Primary key and index. This field is: _mandatory; index; primary-key_ |\n"
					"| _**href**_ | uri | Record access URI. This field is: _mandatory_ |\n"
				);

				for (auto _field : _datum["fields"]->obj()) {
					std::string _opts = zpt::r_replace(zpt::r_replace(zpt::r_replace(zpt::r_replace(std::string(_field.second["opts"]), "[", ""), "]", ""), "\"", ""), ",", "; ");
					_doc += std::string("| _**") + zpt::r_replace(std::string(_field.first), "_", "\\_") + std::string("**_ | ") + std::string(_field.second["type"]) + std::string(" | ") + std::string(_field.second["description"]) + (_opts.length() != 0 ? std::string(" This field is _") + _opts + std::string("_.") : std::string("")) + std::string(" |\n");
				}

				_doc += std::string(
					"| _**created**_ | timestamp | Record creation timestamp. This field is: _mandatory; auto_ |\n"
					"| _**updated**_ | timstamp | Record update timestamp. This field is: _mandatory; auto_ |\n\n"
				);
			}
			_doc += std::string("\n\n");
		}
		_readme += std::string("\n");

		if (_spec["resources"]->is_array()) {	
			_readme += std::string("## ENDPOINTS\n\n");
			_doc += std::string("## ENDPOINTS\n\n");
			for (auto _resource : _spec["resources"]->arr()) {
				_readme += std::string("- **'") + std::string(_resource["topic"]) + std::string("' ") + std::string(_resource["type"]) + std::string("**: ");
				_readme += zpt::r_replace(std::string(_resource["description"]), "\n", " ") + std::string("\n");

				_doc += std::string("### '") + std::string(_resource["topic"]) + std::string("' ") + std::string(_resource["type"]) + std::string("\n\n");
				_doc += std::string("---\n\n");				
				_doc += std::string("(in _") + std::string(_resource["namespace"]) + std::string("_)\n\n");
				_doc += std::string(_resource["description"]) + std::string("\n\n");
				_doc += std::string("#### Allowed protocols:\n\n- ") + zpt::r_replace(zpt::r_replace(zpt::r_replace(zpt::r_replace(std::string(_resource["protocols"]), "[", ""), "]", ""), "\"", ""), ",", "\n- ") + std::string("\n\n");

				std::string _apiary_title = std::string(_resource["name"]) + std::string(" ") + std::string(_resource["type"]);
				std::transform(_apiary_title.begin(), _apiary_title.end(), _apiary_title.begin(), ::toupper);
				_apiary += std::string("## ") + _apiary_title + std::string(" [") + std::string(_resource["topic"]) + std::string("]\n\n");
				_apiary += std::string(_resource["description"]) + std::string("\n\n");
				
				for (auto _p : _resource["performatives"]->arr()) {
					std::string _perf = std::string(_p);
					std::string _name = std::string(_resource["name"]);
					std::transform(_perf.begin(), _perf.end(), _perf.begin(), ::toupper);
					_doc += std::string("#### ") + _perf + std::string(" ") + std::string(_resource["topic"]) + std::string("\n\n");
					_apiary += std::string("\n### ") + this->generate_title_performative(_resource, _perf) + std::string(" ") + _name + std::string(" [") + _perf + std::string("]\n\n");
					zpt::json _uri_param = zpt::gen::url_pattern_to_params(_resource["topic"]);
					if (_uri_param->is_object() && _uri_param->obj()->size() != 0) {
						_apiary += std::string("+ Parameters\n");
						for (auto _param : _uri_param->obj()) {
							_apiary += std::string("    + ") + zpt::r_replace(zpt::r_replace(_param.first, "{", ""), "}", "") + std::string(" (required, string, ``)\n");
						}
						_apiary += std::string("\n\n");
					}
					
					if (
						_resource["datum"]->ok() &&
						(
							(_resource["type"] == zpt::json::string("collection")) ||
							(_resource["type"] == zpt::json::string("store")) ||
							(_resource["type"] == zpt::json::string("document") && (_perf != "DELETE")) ||
							(_resource["type"] == zpt::json::string("controller"))
						)
					) {
						_doc += std::string(
							"##### Parameters\n\n"
							"| PARAMETER | TYPE | DESCRIPTION |\n"
							"|:------|:----:|:-------|\n"
						);

						int _parent_type = 0;
						zpt::json _fields = this->get_fields(_resource, &_parent_type);
						if (_fields->is_object()) {
							if (_parent_type == 1) {
								if (_perf == "DELETE" || _perf == "HEAD" || _perf == "GET" || (_perf == "PUT" && _resource["type"] == zpt::json::string("store"))) {
									_doc += std::string("| _**id**_ | uuid | Primary key and index.") + std::string(_resource["type"] == zpt::json::string("store") ? ". This field is: _mandatory_" : "") + std::string(" |\n");
								}
								if ((_resource["type"] == zpt::json::string("collection") || _resource["type"] == zpt::json::string("store")) && (_perf == "GET" || _perf == "HEAD")) {
									_doc += std::string(
										"| _**page\\_size**_ | int | Maximum number of elements to be returned |\n"
										"| _**page\\_start_index**_ | int | Index of the first element being returned |\n"
										"| _**order\\_by**_ | string | Comma separated list of fields to order the returned list by. Each must be prefixed by '+' (ascending) or '-' (descending). Example: _order\\_by=+id,-created_ |\n"
										"| _**fields**_ | string | Comma separated list of fields to include as part of the returning object. Example: _fields=id,name,created_ |\n"
									);
								}
							}

							for (auto _field : _fields->obj()) {
								std::string _opts = std::string(_field.second["opts"]);
							
								if (_perf == "PATCH" || _perf == "DELETE" || _perf == "HEAD" || _perf == "GET") {
									_opts = "";
								}
								else if (_opts.find("mandatory") != std::string::npos) {
									_opts = "mandatory";
								}
								else {
									_opts = "";
								}
									
								_doc += std::string("| _**") + zpt::r_replace(std::string(_field.first), "_", "\\_") + std::string("**_ | ") + std::string(_field.second["type"]) + std::string(" | ") + std::string(_field.second["description"]) + (_opts.length() != 0 ? std::string(" This field is _") + _opts + std::string("_.") : std::string("")) + std::string(" |\n");
							}
							_doc += std::string("\n");
						}
						
					}
					
					_doc += std::string("##### Returns\n\n");

					if (_perf == "GET") {
						if (_resource["type"] == zpt::json::string("collection") || _resource["type"] == zpt::json::string("store")) {
							_doc += std::string("- **200 OK**: if the requested resource produces data. The body content is be composed of:\n    - _size_: the total number of elements\n    - _elements_: an array of instances of **") + std::string(_resource["datum"]["name"]) + std::string("**\n");
						}
						else if (_resource["type"] == zpt::json::string("document")) {
							_doc += std::string("- **200 OK**: if the requested resource produces data. The body content is an instance of **") + std::string(_resource["datum"]["name"]) + std::string("**\n");
						}
					}
					else if (_perf == "POST") {
						if (_resource["type"] == zpt::json::string("collection")) {
							_doc += std::string("- **201 Created**: if the resource is created. The body content is composed of:\n    - _id_: the unique identifier for the created resource\n    - _href_: the URL path to access the created resource\n");
						}
						else if (_resource["type"] == zpt::json::string("controller")) {
							_doc += std::string("- **200 OK**: if the requested resource produces data. The body content is [PLACE YOUR CONTENT HERE]\n");
						}
					}
					else if (_perf == "PUT") {
						if (_resource["type"] == zpt::json::string("document")) {
							_doc += std::string("- **200 OK**: if the resource is updated. The body content is composed of:\n    - _href_: the URL path for the invoked resource\n    - _n_updated_: the number of updated records\n");
						}
						else if (_resource["type"] == zpt::json::string("store")) {
							_doc += std::string("- **201 Created**: if the resource is created. The body content is composed of:\n    - _id_: the unique identifier for the created resource\n    - _href_: the URL path to access the created resource\n");
						}
					}
					else if (_perf == "PATCH") {
						_doc += std::string("- **200 OK**: if the resource is updated. The body content is composed of:\n    - _href_: the URL path for the invoked resource\n    - _n_updated_: the number of updated records\n");
					}
					else if (_perf == "DELETE") {
						_doc += std::string("- **200 OK**: if the resource is removed. The body content is composed of:\n    - _href_: the URL path for the invoked resource\n    - _n_deleted_: the number of deleted records\n");
					}
					else if (_perf == "HEAD") {
						_doc += std::string("- **200 OK**: if the requested resource produces data\n");
					}

					_doc += std::string(
						"- **204 No Content**: if the requested resource doesn't produces data\n"
						"- **401 Unauthorized**: if the client credentials couldn't be authorized\n"
						"- **403 Forbidden**: if the client doesn't have the necessary permissions\n"
						"- **404 Not Found**: if the requested resource can't be found\n"
						"- **412 Precondition Failed**: if the request doesn't conform with the endpoint specification\n"
						"- **500 Internal Server Error**: if the backend process encounters an unexpected error\n\n"
						"##### Example\n\n"
						"```\n"
					);

					
					if (_perf == "GET") {
						std::string _uuid = zpt::generate::r_uuid();
						if (_resource["type"] == zpt::json::string("collection") || _resource["type"] == zpt::json::string("store")) {
							_doc += std::string("$ curl -i 'https://api.platform.muzzley.com") + std::string(_resource["topic"]) + std::string("?page_size=2' \\\n       -X GET \\\n       -H \"Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\"\n\n");
							_apiary += std::string("+ Request (application/json)\n\n    + Headers\n\n            Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\n\n");

							_doc += std::string(
								"HTTP/1.1 200 OK\n"
								"Content-Length: 1923\n"
								"Content-Type: application/json\n"
								"Date: Tue, 08 Aug 2017 14:56:32 WEST\n"
								"Server: zapata RESTful server\n"
								"Vary: Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag\n"
								"X-Cid: 629a44d6-7c41-11e7-bdc7-d78cb1f90564\n"
								"X-Status: 200\n\n"
								"{\n"
								"    \"size\" : 43,\n"
								"    \"elements\" : [\n"
							);
							_apiary += std::string(
								"+ Response 200 (application/json)\n\n"
								"        {\n"
								"        \"size\" : 43,\n"
								"        \"elements\" : [\n"
							);
							zpt::json _fields = this->get_fields(_resource);
							if (_fields->is_object()) {
								std::string _element = "            {\n";
								_element += std::string("                  \"id\" : \"") + _uuid + std::string("\",\n");
								_element += std::string("                  \"href\" : \"") + std::string(_resource["topic"]) + std::string("/") + _uuid + std::string("\",\n");
								_element += std::string("                  \"created\" : \"") + std::string(zpt::json::date()) + std::string("\",\n");
								_element += std::string("                  \"updated\" : \"") + std::string(zpt::json::date()) + std::string("\"");
								for (auto _field : _fields->obj()) {
									_element += std::string(",\n                  \"") + _field.first + std::string("\" : ") + this->generate_value(_field.second, _field.first) + std::string("");
								}
								_element += std::string("\n            }");
								_doc += _element + std::string(",\n") + _element + std::string("\n");
								_apiary += _element + std::string(",\n") + _element + std::string("\n");
							}
							_apiary += std::string(
								"        ]\n"
								"        }\n"
							);
							_doc += std::string(
								"    ]\n"
								"}\n"
							);
						}
						else if (_resource["type"] == zpt::json::string("document")) {
							_doc += std::string("$ curl -i 'https://api.platform.muzzley.com") + std::string(_resource["topic"]) + std::string("' \\\n       -X GET \\\n       -H \"Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\"\n\n");
							_apiary += std::string("+ Request (application/json)\n\n    + Headers\n\n            Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\n\n");

							_doc += std::string(
								"HTTP/1.1 200 OK\n"
								"Content-Length: 128\n"
								"Content-Type: application/json\n"
								"Date: Tue, 08 Aug 2017 14:56:32 WEST\n"
								"Server: zapata RESTful server\n"
								"Vary: Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag\n"
								"X-Cid: 629a44d6-7c41-11e7-bdc7-d78cb1f90564\n"
								"X-Status: 200\n\n"
								"{\n"
							);
							_apiary += std::string(
								"+ Response 200 (application/json)\n\n"
								"        {\n"
							);
							zpt::json _fields = this->get_fields(_resource);
							if (_fields->is_object()) {
								_doc += std::string("        \"id\" : \"") + _uuid + std::string("\",\n");
								_doc += std::string("        \"href\" : \"") + std::string(_resource["topic"]) + std::string("/") + _uuid + std::string("\",\n");
								_doc += std::string("        \"created\" : \"") + std::string(zpt::json::date()) + std::string("\",\n");
								_doc += std::string("        \"updated\" : \"") + std::string(zpt::json::date()) + std::string("\"");
								_apiary += std::string("        \"id\" : \"") + _uuid + std::string("\",\n");
								_apiary += std::string("        \"href\" : \"") + std::string(_resource["topic"]) + std::string("/") + _uuid + std::string("\",\n");
								_apiary += std::string("        \"created\" : \"") + std::string(zpt::json::date()) + std::string("\",\n");
								_apiary += std::string("        \"updated\" : \"") + std::string(zpt::json::date()) + std::string("\"");
								for (auto _field : _fields->obj()) {
									_doc += std::string(",\n        \"") + _field.first + std::string("\" : ") + this->generate_value(_field.second, _field.first) + std::string("");
									_apiary += std::string(",\n        \"") + _field.first + std::string("\" : ") + this->generate_value(_field.second, _field.first) + std::string("");
								}
							}
							_apiary += std::string(
								"\n        }\n"
							);
							_doc += std::string(
								"\n}\n"
							);
						}
					}
					else if (_perf == "POST") {
						_doc += std::string("$ curl -i 'https://api.platform.muzzley.com") + std::string(_resource["topic"]) + std::string("' \\\n       -X POST \\\n       -H \"Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\" \\\n       -d '{");
						_apiary += std::string("+ Request (application/json)\n\n    + Headers\n\n            Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\n\n    + Body\n\n            {\n");
						zpt::json _fields = this->get_fields(_resource);
						if (_fields->is_object()) {
							bool _first = true;
							for (auto _field : _fields->obj()) {
								if (!_first) {
									_doc += std::string(",\\\n            ");
									_apiary += std::string(",\n");
								}
								_first = false;
								_doc += std::string("\"") + _field.first + std::string("\" : ") + this->generate_value(_field.second, _field.first);
								_apiary += std::string("                    \"") + _field.first + std::string("\":") + this->generate_value(_field.second, _field.first);
							}
						}
						
						_doc += std::string("}'\n\n");
						_apiary += std::string("\n            }\n\n");
						
						_doc += std::string(
							"HTTP/1.1 201 Created\n"
							"Content-Length: 128\n"
							"Content-Type: application/json\n"
							"Date: Tue, 08 Aug 2017 14:56:32 WEST\n"
							"Server: zapata RESTful server\n"
							"Vary: Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag\n"
							"X-Cid: 629a44d6-7c41-11e7-bdc7-d78cb1f90564\n"
							"X-Status: 200\n\n"
							"{\n"
							"        \"id\":\"4208d1c2-7c61-11e7-8eb3-9bf41b6564f5\",\n"
							"        \"href\":\"") + std::string(_resource["topic"]) + std::string("/4208d1c2-7c61-11e7-8eb3-9bf41b6564f5\"\n"
							"}\n\n"
						);
						_apiary += std::string(
							"+ Response 201 (application/json)\n\n"
							"        {\n"
							"                \"id\":\"{id}\",\n"
							"                \"href\":\"") + std::string(_resource["topic"]) + std::string("\"\n"
							"        }\n\n"
						);
					}
					else if (_perf == "PUT" || _perf == "PATCH") {
						_doc += std::string("$ curl -i 'https://api.platform.muzzley.com") + std::string(_resource["topic"]) + std::string("' \\\n       -X ") + _perf + std::string(" \\\n       -H \"Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\" \\\n       -d '{");
						_apiary += std::string("+ Request (application/json)\n\n    + Headers\n\n            Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\n\n    + Body\n\n            {\n");
						
						zpt::json _fields = this->get_fields(_resource);
						if (_fields->is_object()) {
							bool _first = true;
							for (auto _field : _fields->obj()) {
								if (!_first) {
									_doc += std::string(",\\\n            ");
									_apiary += std::string(",\n");
								}
								_first = false;
								_doc += std::string("\"") + _field.first + std::string("\" : ") + this->generate_value(_field.second, _field.first);
								_apiary += std::string("                     \"") + _field.first + std::string("\":") + this->generate_value(_field.second, _field.first);
							}
						}
						
						_doc += std::string("}'\n\n");
						_apiary += std::string("\n            }\n\n");
						
						_doc += std::string(
							"HTTP/1.1 ") + (_resource["type"] == zpt::json::string("document") ? std::string("200 OK") : std::string("201 Created")) + std::string("\n"
							"Content-Length: 128\n"
							"Content-Type: application/json\n"
							"Date: Tue, 08 Aug 2017 14:56:32 WEST\n"
							"Server: zapata RESTful server\n"
							"Vary: Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag\n"
							"X-Cid: 629a44d6-7c41-11e7-bdc7-d78cb1f90564\n"
							"X-Status: 200\n\n"
							"{\n"
							"        \"n_updated\":1,\n"
							"        \"href\":\"") + std::string(_resource["topic"]) + std::string("\"\n"
							"}\n\n"
						);
						_apiary += std::string(
							"+ Response ") + (_resource["type"] == zpt::json::string("document") ? std::string("200") : std::string("201")) + std::string(" (application/json)\n\n"
							"        {\n"
							"                \"n_updated\":1,\n"
							"                \"href\":\"") + std::string(_resource["topic"]) + std::string("\",\n"
							"        }\n\n"
						);
						
					}
					else if (_perf == "DELETE") {
						_doc += std::string("$ curl -i 'https://api.platform.muzzley.com") + std::string(_resource["topic"]) + std::string("' \\\n       -X ") + _perf + std::string(" \\\n       -H \"Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\" \n\n");
						_apiary += std::string("+ Request (application/json)\n\n    + Headers\n\n            Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\n\n");
						
						_doc += std::string(
							"HTTP/1.1 200 OK\n"
							"Content-Length: 128\n"
							"Content-Type: application/json\n"
							"Date: Tue, 08 Aug 2017 14:56:32 WEST\n"
							"Server: zapata RESTful server\n"
							"Vary: Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag\n"
							"X-Cid: 629a44d6-7c41-11e7-bdc7-d78cb1f90564\n"
							"X-Status: 200\n\n"
							"{\n"
							"        \"n_deleted\":") + (_resource["type"] == zpt::json::string("document") ? std::string("1") : std::string("43")) + std::string(",\n"
							"        \"href\":\"") + std::string(_resource["topic"]) + std::string("\"\n"
							"}\n\n"
						);
						_apiary += std::string(
							"+ Response 200 (application/json)\n\n"
							"        {\n"
							"                \"n_deleted\":") + (_resource["type"] == zpt::json::string("document") ? std::string("1") : std::string("43")) + std::string(",\n"
							"                \"href\":\"") + std::string(_resource["topic"]) + std::string("\",\n"
							"        }\n\n"
						);
					}
					else if (_perf == "HEAD") {
						if (_resource["type"] == zpt::json::string("collection") || _resource["type"] == zpt::json::string("store")) {
							_doc += std::string("$ curl -i 'https://api.platform.muzzley.com") + std::string(_resource["topic"]) + std::string("?page_size=2' \\\n       -X GET \\\n       -H \"Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\"\n\n");
						}
						else if (_resource["type"] == zpt::json::string("document")) {
							_doc += std::string("$ curl -i 'https://api.platform.muzzley.com") + std::string(_resource["topic"]) + std::string("' \\\n-X GET \\\n-H \"Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\"\n\n");
						}
						_apiary += std::string("+ Request (application/json)\n\n    + Headers\n\n            Authorization: Bearer asdnofiudifunaoisduf2839sdfd23\n\n");
						
						_doc += std::string(
							"HTTP/1.1 200 OK\n"
							"Content-Length: 0\n"
							"Content-Type: application/json\n"
							"Date: Tue, 08 Aug 2017 14:56:32 WEST\n"
							"Server: zapata RESTful server\n"
							"Vary: Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag\n"
							"X-Cid: 629a44d6-7c41-11e7-bdc7-d78cb1f90564\n"
							"X-Status: 200\n\n"
						);
						_apiary += std::string(
							"+ Response 200 (application/json)\n\n"
						);
					}
											
					_doc += std::string("```\n\n");
				}

				_doc += std::string("\n");
			}
		}
		_readme += std::string("\n");

		bool _readme_exists = stat(_md_file.c_str(), &_buffer) == 0;
		if (bool(this->__options["force-doc"][0]) || (!bool(this->__options["force-doc"][0]) && !_readme_exists)) {
			std::ofstream _ofs(_md_file.data());
			_ofs << _readme << endl << flush;
			_ofs.close();
			ztrace(std::string("processed ") + _md_file);
		}
	}
	bool _pdf_exists = stat(_to_pdf_file.c_str(), &_buffer) == 0;
	if (bool(this->__options["force-doc"][0]) || (!bool(this->__options["force-doc"][0]) && !_pdf_exists)) {
		std::ofstream _ofs(_to_pdf_file.data());
		_ofs << _doc << endl << flush;
		_ofs.close();
		ztrace(std::string("processed ") + _to_pdf_file);
	}
	bool _apiary_exists = stat(_apiary_file.c_str(), &_buffer) == 0;
	if (bool(this->__options["force-doc"][0]) || (!bool(this->__options["force-doc"][0]) && !_apiary_exists)) {
		std::ofstream _ofs(_apiary_file.data());
		_ofs << _apiary << endl << flush;
		_ofs.close();
		ztrace(std::string("processed ") + _apiary_file);
	}
}

auto zpt::Generator::generate_title_performative(zpt::json _resource, std::string _performative) -> std::string {
	if (_performative == "GET") {
		return "Retrieve ";
	}
	if (_performative == "POST") {
		if (_resource["type"] == zpt::json::string("controller")) {
			return "Invoke ";
		}
		return "Create new ";
	}
	if (_performative == "PUT") {
		if (_resource["type"] == zpt::json::string("store")) {
			return "Create new ";
		}
		return "Replace ";		
	}
	if (_performative == "PATCH") {
		return "Update ";		
	}
	if (_performative == "DELETE") {
		return "Remove ";
	}
	if (_performative == "HEAD") {
		return "Test existance of ";
	}
	return "";
}

auto zpt::Generator::generate_value(zpt::json _field, std::string _name) -> std::string {
	std::string _type = std::string(_field["type"]);
	if (_type == "utf8" || _type == "ascii" || _type == "string") {
		std::string _echo(_name.data());
		std::transform(_echo.begin(), _echo.end(), _echo.begin(), ::toupper);
		return std::string("\"") + _echo + std::string("\"");
	}
	else if (_type == "uri") {
		return "\"https://api.something.something/else\"";
	}
	else if (_type == "email") {
		return "\"you@something.something\"";
	}
	else if (_type == "phone") {
		return "\"(+1) 555 199 199\"";
	}
	else if (_type == "token") {
		return std::string("\"") + zpt::generate::r_key(24) + std::string("\"");
	}
	else if (_type == "uuid") {
		return std::string("\"") + zpt::generate::r_uuid() + std::string("\"");
	}
	else if (_type == "hash") {
		return std::string("\"") + zpt::generate::r_key(64) + std::string("\"");
	}
	else if (_type == "int") {
		return "19";
	}
	else if (_type == "boolean") {
		return "true";
	}
	else if (_type == "double") {
		return "0.1";
	}
	else if (_type == "timestamp") {
		return std::string("\"") + std::string(zpt::json::date()) + std::string("\"");
	}
	else if (_type == "object") {
		return "{ }";
	}	
	else if (_type == "array") {
		return "[ ]";
	}
	else if (_type == "json") {
		return "{ }";
	}
	else if (_type == "location") {
		return "[ 38.709167, -9.139659 ]";
	}

	return "\"\"";
}

auto zpt::Generator::get_fields(zpt::json _resource, int* _parent_type) -> zpt::json {
	if (_resource["datum"]["name"]->ok()) {
		auto _datum = zpt::Generator::datums.find(std::string(_resource["datum"]["name"]));
		if (_datum != zpt::Generator::datums.end()) {
			if (_parent_type != nullptr) {
				*_parent_type = 1;
			}
			return _datum->second->spec()["fields"]->clone();
		}
		_datum = zpt::Generator::included_datums.find(std::string(_resource["datum"]["name"]));
		if (_datum != zpt::Generator::included_datums.end()) {
			if (_parent_type != nullptr) {
				*_parent_type = 1;
			}
			return _datum->second->spec()["fields"]->clone();
		}
	}
	else if (_resource["datum"]["fields"]->ok()) {	
		if (_parent_type != nullptr) {
			*_parent_type = 2;
		}
		return _resource["datum"]["fields"]->clone();
	}						
	else if (_resource["datum"]["ref"]->ok()) {
		auto _iresource = zpt::Generator::included_resources.find(zpt::gen::url_pattern_to_regexp(_resource["datum"]["ref"]));
		if (_iresource != zpt::Generator::included_resources.end()) {
			return this->get_fields(_iresource->second->spec(), _parent_type);
		}
	}
	return zpt::undefined;	
}

auto zpt::Generator::build_data_layer() -> void {
	for (auto _pair : this->__specs->obj()) {
		zpt::json _spec = _pair.second;
		
		zpt::mkdir_recursive(std::string(this->__options["data-out-h"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/"));
		zpt::mkdir_recursive(std::string(this->__options["data-out-cxx"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/datums/"));

		if (_spec["datums"]->is_array()) {		
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

				if (_datum["fields"]->is_object()){
					std::string _fields;
					std::string _pk_constraint(",\n\tprimary key(id");
					int _pks = 0;
					_fields += std::string("\tid ") + (_datum["index_type"]->is_string() ? zpt::GenDatum::get_type({ "type", _datum["index_type"] }) : std::string("char(36)"))  + std::string(" not null,\n");
					_fields += std::string("\tcreated timestamp not null,\n");
					_fields += std::string("\tupdated timestamp not null,\n");
					_fields += std::string("\thref varchar(1024) not null");
					for (auto _f : _datum["fields"]->obj()) {
						if (_f.second["ref"]->ok()) {
							continue;
						}
						_fields += std::string(",\n\t") + std::string(_f.first) + std::string(" ") + zpt::GenDatum::get_type(_f.second) + std::string(" ") + zpt::GenDatum::get_restrictions(_f.second);
						zpt::json _opts = zpt::gen::get_opts(_f.second);
						if (_opts["primary-key"]->ok()) {
							_pk_constraint += std::string(", ") + _f.first;
							_pks++;
						}
					}
					_pk_constraint += std::string(")");
					_fields += _pk_constraint;
					zpt::replace(_datum_sql, "$[datum.fields]", _fields);

					std::string _constraints;
					std::string _unq_constraint;
					std::string _idx_constraint;
					_pk_constraint.assign(std::string("create unique index on ") + std::string(_datum["name"]) + std::string("("));
					_pks = 0;
					for (auto _f : _datum["fields"]->obj()) {
						if (_f.second["ref"]->ok()) {
							continue;
						}
						zpt::json _opts = zpt::gen::get_opts(_f.second);
						if (_opts["primary-key"]->ok()) {
							if (_pks != 0) {
								_pk_constraint += std::string(", ");
							}
							_pk_constraint += _f.first;
							_pks++;
						}
						if (_opts["unique"]->ok()) {
							_unq_constraint += std::string("create unique index on ") + std::string(_datum["name"]) + std::string("(") + _f.first + std::string(");\n");
						}
						else if (_opts["index"]->ok() && !_opts["primary-key"]->ok()) {
							_idx_constraint += std::string("create index on ") + std::string(_datum["name"]) + std::string("(") + _f.first + std::string(");\n");
						}
					}
					_pk_constraint += std::string(");\n");
					_constraints += (_pks > 1 ? _pk_constraint : std::string("")) + _unq_constraint + _idx_constraint;
					zpt::replace(_datum_sql, "$[datum.constraints]", _constraints);
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
			
				std::string _get_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "redis", "couchdb", "mongodb", "postgresql", "mariadb" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.get.client]", _get_client);
				zpt::replace(_datum_cxx, "$[datum.method.get.client]", _get_client);
		
				std::string _query_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "couchdb", "mongodb", "postgresql", "mariadb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.query.client]", _query_client);
				zpt::replace(_datum_cxx, "$[datum.method.query.client]", _query_client);

				std::string _insert_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "couchdb", "mongodb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.insert.client]", _insert_client);
				zpt::replace(_datum_cxx, "$[datum.method.insert.client]", _insert_client);
		
				std::string _save_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "couchdb", "mongodb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.save.client]", _save_client);
				zpt::replace(_datum_cxx, "$[datum.method.save.client]", _save_client);

				std::string _set_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "couchdb", "mongodb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.set.client]", _set_client);
				zpt::replace(_datum_cxx, "$[datum.method.set.client]", _set_client);

				std::string _remove_client = zpt::GenDatum::build_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "couchdb", "mongodb", "redis" }, _namespace);
				zpt::replace(_datum_h, "$[datum.method.remove.client]", _remove_client);
				zpt::replace(_datum_cxx, "$[datum.method.remove.client]", _remove_client);

				std::string _relation_insert;
				std::string _relation_replace;
				std::string _relation_update;
				std::string _relation_remove;
				if (_found != zpt::Generator::datums.end()) {
					for (auto _f : _datum["fields"]->obj()) {
						if (!_f.second["ref"]->ok()) {
							continue;
						}
						_relation_insert += _found->second->build_associations_for_insert(_f.first, _f.second);
						_relation_replace += _found->second->build_associations_for_replace(_f.first, _f.second);
						_relation_update += _found->second->build_associations_for_update(_f.first, _f.second);
						_relation_remove += _found->second->build_associations_for_remove(_f.first, _f.second);
					}
				}
				zpt::replace(_datum_cxx, "$[datum.relations.insert]", _relation_insert);
				zpt::replace(_datum_cxx, "$[datum.relations.save]", _relation_replace);
				zpt::replace(_datum_cxx, "$[datum.relations.set]", _relation_update);
				zpt::replace(_datum_cxx, "$[datum.relations.remove]", _relation_remove);


				zpt::replace(_datum_cxx, "$[datum.method.ordered.clients]", zpt::GenDatum::build_ordered_data_client(_datum["dbms"], { zpt::array, "postgresql", "mariadb", "couchdb", "mongodb", "redis" }, _namespace));
				
				struct stat _buffer;
				bool _cxx_exists = stat(_cxx_file.c_str(), &_buffer) == 0;
				bool _h_exists = stat(_h_file.c_str(), &_buffer) == 0;
				bool _sql_exists = stat(_sql_file.c_str(), &_buffer) == 0;
				if (bool(this->__options["force-data"][0]) || (!bool(this->__options["force-data"][0]) && !_h_exists)) {
					std::ofstream _h_ofs(_h_file.data());
					_h_ofs << _datum_h << endl << flush;
					_h_ofs.close();
					ztrace(std::string("processed ") + _h_file);
				}
				if (bool(this->__options["force-data"][0]) || (!bool(this->__options["force-data"][0]) && !_cxx_exists)) {
					std::ofstream _cxx_ofs(_cxx_file.data());
					_cxx_ofs << _datum_cxx << endl << flush;
					_cxx_ofs.close();
					ztrace(std::string("processed ") + _cxx_file);
				}
				if (bool(this->__options["force-data"][0]) || (!bool(this->__options["force-data"][0]) && !_sql_exists)) {
					std::ofstream _sql_ofs(_sql_file.data());
					_sql_ofs << _datum_sql << endl << flush;
					_sql_ofs.close();
					ztrace(std::string("processed ") + _sql_file);
				}
			}
		}
	}
}

auto zpt::Generator::build_container() -> void {
	zpt::mkdir_recursive(std::string(this->__options["resource-out-cxx"][0]) + std::string("/mutations/"));
	std::string _mutation_cxx_file = std::string(this->__options["resource-out-cxx"][0]) + std::string("/mutations/api.cpp");
	std::string _mutation_am_file = std::string(this->__options["resource-out-cxx"][0]) + std::string("/mutations/Makefile.am");
	std::string _mutation_cxx;
	std::string _mutation_make_files;
	std::string _mutation_child_includes;
	std::string _mutation_registry;
	std::string _mutation_dyn_link;
	std::string _mutation_dyn_dir;
	zpt::json _mutation_dbms = zpt::json::object();
	zpt::load_path("/usr/share/zapata/gen/MutationsLoad.cpp", _mutation_cxx);
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
				_mutation_dbms << _dbms.first << _dbms.first;
			}
			_connectors_initialize += "}";
			zpt::replace(_container_cxx, "$[datum.connectors.initialize]", _connectors_initialize);
			_mutation_registry += std::string("\n_emitter->connector(") + _connectors_initialize + std::string(");\n");

			zpt::replace(_container_cxx, "$[namespace]", std::string(_spec["namespace"]));

			std::string _h_make_files;
			std::string _make_files;
			std::string _child_includes;
			std::string _registry;
			std::string _dyn_link;
			std::string _dyn_dir;
			if (_spec["datums"]->is_array()) {
				for (auto _datum : _spec["datums"]->arr()) {
					if (_datum["dbms"]->is_array()) {
						for (auto _dbms : _datum["dbms"]->arr()) {
							if (_dyn_link.find(std::string(" -lzapata-") + std::string(_dbms)) == std::string::npos) {
								_dyn_link += std::string(" -lzapata-") + std::string(_dbms);
							}
						}
					}
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
						_mutation_child_includes += std::string("#include <") + _include + std::string(">\n");
						_h_make_files += std::string("./") + _include + std::string(" \\\n");

						std::string _make_file(std::string("./datums/") + std::string(_datum["name"]) + std::string(".cpp \\\n"));
						_make_files += _make_file;
						_make_file.assign(std::string("../") + std::string(_spec["name"]) + std::string("/mutations/") + std::string(_datum["name"]) + std::string(".cpp \\\n"));
						_mutation_make_files += _make_file;
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
						_mutation_registry += zpt::Generator::datums.find(_key)->second->build_mutations(std::string(_spec["name"]), _mutation_child_includes);
					}
				}
			}
		
			std::string _includes;
			if (_spec["resources"]->is_array()) {
				_handlers_built = true;
				for (auto _resource : _spec["resources"]->arr()) {
					std::string _key = std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]->is_array() ? _resource["topic"][0] : _resource["topic"]);

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
				ztrace(std::string("processed ") + _cxx_file);	
			}

			size_t _cxx_out_split = zpt::split(std::string(this->__options["resource-out-cxx"][0]), "/")->arr()->size() + zpt::split(std::string(_spec["name"]), "/")->arr()->size();
			std::string _parent_dir;
			for (size_t _i = 0; _i != _cxx_out_split; _i++) _parent_dir += "../";
			std::string _make;
			std::string _lib_escaped = zpt::r_replace(std::string(_spec["lib"]), "-", "_");
			_make += std::string("lib_LTLIBRARIES = lib") + std::string(_spec["lib"]) + std::string(".la\n\n");
			_make += std::string("lib") + _lib_escaped + std::string("_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest ") + _dyn_link + std::string("\n");
			_make += std::string("lib") + _lib_escaped + std::string("_la_LDFLAGS = -version-info ") + (this->__options["version"]->is_array() ? std::to_string(int(this->__options["version"][0]) + int(this->__options["version"][1])) + std::string(":") + std::string(this->__options["version"][2]) + std::string(":") + std::string(this->__options["version"][1]) : std::string("0:1:0")) + _dyn_dir + std::string("\n");
			_make += std::string("lib") + _lib_escaped + std::string("_la_CPPFLAGS = -O3 -std=c++14 -I") + _parent_dir + std::string("include\n\n");
			_make += std::string("lib") + _lib_escaped + std::string("_la_SOURCES = \\\n");
			_make += _make_files;
			_make += std::string("./api.cpp\n");
			if (bool(this->__options["force-makefile"][0]) || (!bool(this->__options["force-makefile"][0]) && !_am_exists)) {
				std::ofstream _am_ofs(_am_file.data());
				_am_ofs << _make << endl << flush;
				_am_ofs.close();
				ztrace(std::string("processed ") + _am_file);
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
			if (_spec["resources"]->is_array()) {
				for (auto _resource : _spec["resources"]->arr()) {
					std::string _key = std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]->is_array() ? _resource["topic"][0] : _resource["topic"]);

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
				ztrace(std::string("processed ") + _lisp_file);	
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

			_py_ofs.open((std::string(this->__options["resource-out-scripts"][0]) + std::string("/__init__.py")).data());
			_py_ofs << endl << flush;
			_py_ofs.close();
			_py_ofs.open((std::string(this->__options["resource-out-scripts"][0]).substr(0, std::string(this->__options["resource-out-scripts"][0]).rfind("/")) + std::string("/__init__.py")).data());
			_py_ofs << endl << flush;
			_py_ofs.close();
			
			std::string _py_file = std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_spec["name"]) + std::string("/api.py");
				
			std::string _includes;
			if (_spec["resources"]->is_array()) {
				for (auto _resource : _spec["resources"]->arr()) {
					std::string _key = std::string(_resource["namespace"]) + std::string("::") + std::string(_resource["topic"]->is_array() ? _resource["topic"][0] : _resource["topic"]);

					std::string _include = std::string("import ") + zpt::r_replace(std::string(this->__options["prefix"]), "/", "") + std::string(".") + std::string(this->__options["abbr"]) + std::string(".") + std::string(zpt::r_replace(_spec["name"]->str(), "-", "_")) + std::string(".") + std::string(_resource["type"]) + std::string("s.") + std::string(zpt::r_replace(_resource["name"]->str(), "-", "_")) + std::string("");
					_includes += _include + std::string("\n");
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
				ztrace(std::string("processed ") + _py_file);	
			}
		}
	}

	if (!this->__options["resource-out-lang"]->ok() || std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("c++")) != std::end(this->__options["resource-out-lang"]->arr())) {
		for (auto _dbms : _mutation_dbms->obj()) {
			_mutation_child_includes += std::string("#include <zapata/") + _dbms.first + std::string(".h>\n");
			_mutation_dyn_link += std::string(" -lzapata-") + _dbms.first;
		}
	
		zpt::replace(_mutation_cxx, "$[mutations.api.path.h]", _mutation_child_includes);
		zpt::replace(_mutation_cxx, "$[mutations.handlers.delegate]", _mutation_registry);
		zpt::replace(_mutation_cxx, "_emitter->connector({ });", "");

		struct stat _buffer;
		bool _mutation_cxx_exists = stat(_mutation_cxx_file.c_str(), &_buffer) == 0;
		bool _mutation_am_exists = stat(_mutation_am_file.c_str(), &_buffer) == 0;
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_mutation_cxx_exists)) {
			std::ofstream _cxx_ofs(_mutation_cxx_file.data());
			_cxx_ofs << _mutation_cxx << endl << flush;
			_cxx_ofs.close();
			ztrace(std::string("processed ") + _mutation_cxx_file);	
		}

		size_t _cxx_out_split = zpt::split(std::string(this->__options["resource-out-cxx"][0]), "/")->arr()->size() + 1;
		std::string _parent_dir;
		for (size_t _i = 0; _i != _cxx_out_split; _i++) _parent_dir += "../";
	
		std::string _make;
		std::string _lib_escaped = zpt::r_replace(this->__options["name"]->str(), "-", "_") + std::string("_mutations");
		_make += std::string("lib_LTLIBRARIES = lib") + std::string(this->__options["name"]) + std::string("-mutations.la\n\n");
		_make += std::string("lib") + _lib_escaped + std::string("_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest ") + _mutation_dyn_link + std::string("\n");
		_make += std::string("lib") + _lib_escaped + std::string("_la_LDFLAGS = -version-info ") + (this->__options["version"]->is_array() ? std::to_string(int(this->__options["version"][0]) + int(this->__options["version"][1])) + std::string(":") + std::string(this->__options["version"][2]) + std::string(":") + std::string(this->__options["version"][1]) : std::string("0:1:0")) + _mutation_dyn_dir + std::string("\n");
		_make += std::string("lib") + _lib_escaped + std::string("_la_CPPFLAGS = -O3 -std=c++14 -I") + _parent_dir + std::string("include\n\n");
		_make += std::string("lib") + _lib_escaped + std::string("_la_SOURCES = \\\n");
		_make += _mutation_make_files;
		_make += std::string("./api.cpp\n");
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_mutation_am_exists)) {
			std::ofstream _am_ofs(_mutation_am_file.data());
			_am_ofs << _make << endl << flush;
			_am_ofs.close();
			ztrace(std::string("processed ") + _mutation_am_file);
		}
	}	
	//ztrace(_handler_cxx);
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

auto zpt::GenDatum::build_query(zpt::json _field) -> std::string {
	std::string _base = zpt::Generator::get_datum(std::string(_field["ref"]));
	std::string _query(std::string("std::string(\"SELECT ") + _base + std::string(".* FROM "));
	_query +=  _base + std::string(" JOIN ");
	zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));

	auto _second = _rel->obj()->begin();
	_second++;
	std::string _other_ref = std::string(_second->second);
	std::string _on_field = _other_ref.substr(_other_ref.rfind("/") + 1);
	_other_ref = _other_ref.substr(0, _other_ref.rfind("/"));
	std::string _join_with = zpt::Generator::get_datum(_other_ref);
	_query += _join_with + std::string(" ON ") + _base + std::string(".") + _second->first + std::string("=") + _join_with + std::string(".") + _on_field;

	auto _first = _rel->obj()->begin();
	_other_ref = std::string(_first->first);
	std::string _where_field = _other_ref.substr(_other_ref.rfind("/") + 1);
	_query += std::string(" WHERE ") + _join_with + std::string(".") + _where_field + std::string("='\") + std::string(");
	_query += (_first->second->str().front() == '{' ? std::string("_r_data[\"") + _first->second->str().substr(1, _first->second->str().length() - 2) + std::string("\"]") : std::string("\"") + _first->second->str() + std::string("\""));
	_query += std::string(") + std::string(\"'\")");
	return _query;
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
			_params += std::string("\"") + _param.second->str().substr(1, _param.second->str().length() - 2) + std::string("\", ") + std::string("std::string(_r_element[\"") + std::string(zpt::split(_param.first, "/")->arr()->back()) + std::string("\"])");
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
	if (this->__spec["dbms"]->is_array()) {
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

auto zpt::GenDatum::build_dbms_source() -> std::string {
	if (this->__spec["dbms"]->is_array()) {
		for (auto _dbms : this->__spec["dbms"]->arr()) {
			if (_dbms == zpt::json::string("postgresql") || _dbms == zpt::json::string("mariadb")) {
				return std::string("\"") + zpt::GenDatum::build_data_client(this->__spec["dbms"], { zpt::array, _dbms->str() }, std::string(this->__spec["namespace"])) + std::string("\"");
			}
		}
	}
	return "";
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

		zpt::replace(_mutation_cxx, "$[mutation.topic.self.regex]", std::string("std::string(\"") + zpt::gen::url_pattern_to_regexp({ zpt::array, zpt::path::join({ zpt::array, "\") + _emitter->version() + std::string(\"", "mutations", "{operation}", this->__spec["name"] } ) }) + std::string("\")"));
		if (false && this->__spec["dbms"]->is_array() && this->__spec["dbms"]->arr()->size() > 1) {
			std::string _mutation;
			bool _first = true;
			_mutation.assign(this->build_insert());
			zpt::replace(_mutation_cxx, "$[mutation.handler.self.insert]", _mutation);
			_first = _mutation.length() == 0;
			_mutation.assign(this->build_update());
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string("\n") + _mutation;
			}
			zpt::replace(_mutation_cxx, "$[mutation.handler.self.update]", _mutation);
			_mutation.assign(this->build_remove());
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string("\n") + _mutation;
			}		
			zpt::replace(_mutation_cxx, "$[mutation.handler.self.remove]", _mutation);
			_mutation.assign(this->build_replace());
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string("\n") + _mutation;
			}		
			zpt::replace(_mutation_cxx, "$[mutation.handler.self.replace]", _mutation);
			zpt::replace(_mutation_cxx, "$[mutation.self.handler.begin]", "");
			zpt::replace(_mutation_cxx, "$[mutation.self.handler.end]", "");
		}
		else {
			size_t _begin = _mutation_cxx.find("$[mutation.self.handler.begin]");
			size_t _end = _mutation_cxx.find("$[mutation.self.handler.end]") + 28;
			if (_begin != std::string::npos) {
				_mutation_cxx.erase(_begin, _end - _begin);
			}
		}
		
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
			std::string _topic = zpt::gen::url_pattern_to_regexp(_field.second["ref"]);
			auto _found = zpt::Generator::alias.find(_topic);
			_topic = (_found != zpt::Generator::alias.end() ? _found->second : _topic);
			if (std::string(_field.second["rel"]).find("/") != std::string::npos) {
				zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
				auto _second = _rel->obj()->begin();
				_second++;
				std::string _other_ref = std::string(_second->second);
				_other_ref = _other_ref.substr(0, _other_ref.rfind("/"));
				_topic.assign(_other_ref);
			}
			_topic = std::string("std::string(\"^/\") + _emitter->version() + std::string(\"/mutations/([^/]+)\") + zpt::r_replace(\"") + zpt::r_replace(zpt::r_replace(_topic, "^", ""), "$", "") + std::string("\", std::string(\"/\") + _emitter->version(), \"\") + std::string(\"(.*)$\")");
			zpt::replace(_mutation_on, "$[mutation.topic.regex]", _topic);
			
			std::string _mutation;
			bool _first = true;
			_mutation.assign(this->build_associations_insert(_field.first, _field.second));
			zpt::replace(_mutation_on, "$[mutation.handler.insert]", _mutation);
			_first = _mutation.length() == 0;
			_mutation.assign(this->build_associations_update(_field.first, _field.second));
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string("\n") + _mutation;
			}
			zpt::replace(_mutation_on, "$[mutation.handler.update]", _mutation);
			_mutation.assign(this->build_associations_remove(_field.first, _field.second));
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string("\n") + _mutation;
			}		
			zpt::replace(_mutation_on, "$[mutation.handler.remove]", _mutation);
			_mutation.assign(this->build_associations_replace(_field.first, _field.second));
			if (_mutation.length() != 0 && !_first) {
				_first = false;
				_mutation = std::string("\n") + _mutation;
			}		
			zpt::replace(_mutation_on, "$[mutation.handler.replace]", _mutation);

			zpt::json _resource_opts = zpt::json::object();
			if (this->__spec["protocols"]->is_array()) {
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
			ztrace(std::string("processed ") + _h_file);
		}
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_cxx_exists)) {
			std::ofstream _cxx_ofs(_cxx_file.data());
			_cxx_ofs << _mutation_cxx << endl << flush;
			_cxx_ofs.close();
			ztrace(std::string("processed ") + _cxx_file);
		}
		return std::string(this->__spec["namespace"]) + std::string("::mutations::") + std::string(zpt::r_replace(this->__spec["name"]->str(), "-", "_")) + std::string("::mutify(_emitter);\n");
	}
	return _return;
}

auto zpt::GenDatum::build_insert() -> std::string {
	std::string _return;
	std::string _source = this->build_dbms_source();
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0 && _source.length() != 0) {
		_return += std::string("if (_tv_operation == \"insert\") {\n");	
		_return += std::string("std::string _c_source = ");
		_return += _source;
		_return += std::string(";\n");
		_return += std::string("zpt::connector _s = _emitter->connector(_c_source);\n");
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += _dbms;
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("zpt::json _r_data = _envelope[\"payload\"][\"new\"];\n");
		
		for (auto _r : this->__spec["fields"]->obj()) {
			zpt::json _field = _r.second;
			zpt::json _opts = zpt::gen::get_opts(_field);
			std::string _name = _r.first;
			if (_field["ref"]->ok()) {
				zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
				_return += std::string("zpt::json _r_") + _name + std::string(" = _s->query(\"") + zpt::Generator::get_datum(std::string(_field["ref"])) + std::string("\", ");
				if (_rel->ok() && _rel->obj()->size() != 0) {
					if (std::string(_field["rel"]).find("/") != std::string::npos) {
						_return += this->build_query(_field);
					}
					else {
						_return += std::string("{ ");
						_return += this->build_params(_rel, false);
						_return += std::string(" }");
					}
				}
				else {
					_return += std::string("zpt::json::object()");
				}
				_return += std::string(");\n");

				if (std::string(_field["rel"]).find("/") != std::string::npos) {
					if (_field["type"] == zpt::json::string("array")) {
						_return += std::string("if (_r_") + _name + std::string("->ok()) _r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("->is_array() ? _r_") + _name + std::string(" : zpt::json({ zpt::array, _r_") + _name + std::string(" }));\n");
					}
					else if (_field["type"] == zpt::json::string("object")) {
						_return += std::string("if (_r_") + _name + std::string("->ok()) _r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("->is_array() ? _r_") + _name + std::string("[0] : _r_") + _name + std::string(");\n");
					}
				}
				else {
					if (_field["type"] == zpt::json::string("array")) {
						_return += std::string("if (_r_") + _name + std::string("->ok()) _r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"] : zpt::json({ zpt::array, _r_") + _name + std::string(" }));\n");
					}
					else if (_field["type"] == zpt::json::string("object")) {
						_return += std::string("if (_r_") + _name + std::string("->ok()) _r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(");\n");
					}
				}
			}
		}

		_return += std::string("_c->upsert(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"href\"]->str(), _r_data, { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("return;\n");
		_return += std::string("}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_update() -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		_return += std::string("if (_tv_operation == \"update\") {\n");	
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += this->build_dbms();
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("if (_envelope[\"payload\"][\"filter\"]->ok()) {\n");
		_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"filter\"], _envelope[\"payload\"][\"changes\"], { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("else {\n");
		_return += std::string("zpt::json _split = zpt::split(_envelope[\"payload\"][\"href\"]->str(), \"/\");\n");
		_return += std::string("_split->arr()->pop_back();\n");
		_return += std::string("_c->upsert(\"") + std::string(this->__spec["name"]) + std::string("\", zpt::path::join(_split), _envelope[\"payload\"][\"changes\"] + zpt::json{ \"href\", _envelope[\"payload\"][\"href\"] }, { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("}\n");	
		_return += std::string("return;\n");
		_return += std::string("}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_remove() -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		_return += std::string("if (_tv_operation == \"remove\") {\n");	
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
		_return += std::string("return;\n");
		_return += std::string("}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_replace() -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	std::string _source = this->build_dbms_source();
	if (_dbms.length() != 0 && _source.length() != 0) {
		_return += std::string("if (_tv_operation == \"replace\") {\n");	
		_return += std::string("std::string _c_source = ");
		_return += _source;
		_return += std::string(";\n");
		_return += std::string("zpt::connector _s = _emitter->connector(_c_source);\n");
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += _dbms;
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("zpt::json _r_data = _envelope[\"payload\"][\"new\"];\n");

		for (auto _r : this->__spec["fields"]->obj()) {
			zpt::json _field = _r.second;
			zpt::json _opts = zpt::gen::get_opts(_field);
			std::string _name = _r.first;
			if (_field["ref"]->ok()) {
				zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
				_return += std::string("zpt::json _r_") + _name + std::string(" = _s->query(\"") + zpt::Generator::get_datum(std::string(_field["ref"])) + std::string("\", ");
				if (_rel->ok() && _rel->obj()->size() != 0) {
					if (std::string(_field["rel"]).find("/") != std::string::npos) {
						_return += this->build_query(_field);
					}
					else {
						_return += std::string("{ ");
						_return += this->build_params(_rel, false);
						_return += std::string(" }");
					}
				}
				else {
					_return += std::string("zpt::json::object()");
				}
				_return += std::string(");\n");

				if (std::string(_field["rel"]).find("/") != std::string::npos) {
					if (_field["type"] == zpt::json::string("array")) {
						_return += std::string("_r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("->is_array() ? _r_") + _name + std::string(" : zpt::json({ zpt::array, _r_") + _name + std::string(" }));\n");
					}
					else if (_field["type"] == zpt::json::string("object")) {
						_return += std::string("_r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("->is_array() ? _r_") + _name + std::string("[0] : _r_") + _name + std::string(");\n");
					}
				}
				else {
					if (_field["type"] == zpt::json::string("array")) {
						_return += std::string("_r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"] : zpt::json({ zpt::array, _r_") + _name + std::string(" }));\n");
					}
					else if (_field["type"] == zpt::json::string("object")) {
						_return += std::string("_r_data << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(");\n");
					}
				}
			}
		}

		_return += std::string("_c->save(\"") + std::string(this->__spec["name"]) + std::string("\", _envelope[\"payload\"][\"href\"]->str(), _r_data, { \"mutated-event\", true });\n");
		_return += std::string("}\n");	
		_return += std::string("return;\n");
		_return += std::string("}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_insert(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	std::string _dbms = this->build_dbms();
	std::string _source = this->build_dbms_source();
	if (_dbms.length() != 0 && _source.length() != 0) {
		zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
		std::string _on_field;
		std::string _join_with;
		if (std::string(_field["rel"]).find("/") != std::string::npos) {
			auto _second = _rel->obj()->begin();
			_second++;
			std::string _other_ref = std::string(_second->second);
			_on_field = _other_ref.substr(_other_ref.rfind("/") + 1);
			_other_ref = _other_ref.substr(0, _other_ref.rfind("/"));
			_join_with = zpt::Generator::get_datum(_other_ref);
		}
		_return += std::string("if (_tv_operation == \"insert\" || _tv_operation == \"update\" || _tv_operation == \"replace\"") + (std::string(_field["rel"]).find("/") != std::string::npos ? std::string(" || _tv_operation == \"remove\"") : std::string("")) + std::string(") {\n");	
		_return += std::string("try {\n");
		_return += std::string("std::string _c_source = ");
		_return += _source;
		_return += std::string(";\n");
		_return += std::string("zpt::connector _s = _emitter->connector(_c_source);\n");
		if (std::string(_field["rel"]).find("/") != std::string::npos) {
			_return += std::string("zpt::json _r_base = _s->query(\"") + _join_with + std::string("\", zpt::json((_envelope[\"payload\"][\"href\"]->ok() ? zpt::json({ \"href\", _envelope[\"payload\"][\"href\"] }) : zpt::undefined) + (_envelope[\"payload\"][\"filter\"]->ok() ? _envelope[\"payload\"][\"filter\"] : zpt::undefined)));\n");
		}
		else {
			_return += std::string("zpt::json _r_base = _s->query(\"") + zpt::Generator::get_datum(std::string(_field["ref"])) + std::string("\", zpt::json(zpt::json({ \"href\",  _envelope[\"payload\"][\"href\"] }) + (_envelope[\"payload\"][\"filter\"]->ok() ? _envelope[\"payload\"][\"filter\"] : zpt::undefined)));\n");
		}
		//_return += std::string("if (_r_base[\"elements\"]->type() != zpt::JSArray) {\n_r_base = { \"elements\", { zpt::array, _r_base } };\n}\n");
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += _dbms;
		_return += std::string(" };\n");
		_return += std::string("for (auto _r_element : _r_base[\"elements\"]->arr()) {\n");
		_return += std::string("zpt::json _r_targets = _s->query(\"") + std::string(this->__spec["name"]) + std::string("\", { ");
		std::string _inverted_params = this->build_inverted_params(_rel);
		if (_inverted_params.length() != 0) {
			_return += _inverted_params;
		}
		else {
			_return += std::string("\"") + _name + std::string("\", { \"href\", _r_element[\"href\"] }");
		}
		_return += std::string(" });\n");
		_return += std::string("if (!_r_targets[\"elements\"]->is_array()) continue;\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("for (auto _r_data : _r_targets[\"elements\"]->arr()) {\n");
		_return += std::string("zpt::json _r_") + _name + std::string(" = _s->query(\"") + zpt::Generator::get_datum(std::string(_field["ref"])) + std::string("\", ");
		if (_rel->ok() && _rel->obj()->size() != 0) {
			if (std::string(_field["rel"]).find("/") != std::string::npos) {
				_return += this->build_query(_field);
			}
			else {
				_return += std::string("{ ");
				_return += this->build_params(_rel, false);
				_return += std::string(" }");
			}
		}
		else {
			_return += std::string("zpt::json::object()");
		}
		_return += std::string(");\n");

		
		if (std::string(_field["rel"]).find("/") != std::string::npos) {
			if (_field["type"] == zpt::json::string("array")) {
				_return += std::string("zpt::json _split = zpt::split(_r_data[\"href\"]->str(), \"/\");\n");
				_return += std::string("_split->arr()->pop_back();\n");
				_return += std::string("_c->upsert(\"") + std::string(this->__spec["name"]) + std::string("\", zpt::path::join(_split), { \"href\", _r_data[\"href\"], \"") + _name + std::string("\", (_r_") + _name + std::string("->is_array() ? _r_") + _name + std::string(" : zpt::json({ zpt::array, _r_") + _name + std::string(" })) }, { \"href\", _r_data[\"href\"], \"mutated-event\", true });\n");
			}
			else if (_field["type"] == zpt::json::string("object")) {
				_return += std::string("zpt::json _split = zpt::split(_r_data[\"href\"]->str(), \"/\");\n");
				_return += std::string("_split->arr()->pop_back();\n");
				_return += std::string("_c->upsert(\"") + std::string(this->__spec["name"]) + std::string("\", zpt::path::join(_split), { \"href\", _r_data[\"href\"], \"") + _name + std::string("\", (_r_") + _name + std::string("->is_array() ? _r_") + _name + std::string("[0] : _r_") + _name + std::string(") }, { \"href\", _r_data[\"href\"], \"mutated-event\", true });\n");
			}
		}
		else {
			if (_field["type"] == zpt::json::string("array")) {
				_return += std::string("zpt::json _split = zpt::split(_r_data[\"href\"]->str(), \"/\");\n");
				_return += std::string("_split->arr()->pop_back();\n");
				_return += std::string("_c->upsert(\"") + std::string(this->__spec["name"]) + std::string("\", zpt::path::join(_split), { \"href\", _r_data[\"href\"], \"") + _name + std::string("\", (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"] : zpt::json({ zpt::array, _r_") + _name + std::string(" })) }, { \"href\", _r_data[\"href\"], \"mutated-event\", true });\n");
			}
			else if (_field["type"] == zpt::json::string("object")) {
				_return += std::string("zpt::json _split = zpt::split(_r_data[\"href\"]->str(), \"/\");\n");
				_return += std::string("_split->arr()->pop_back();\n");
				_return += std::string("_c->upsert(\"") + std::string(this->__spec["name"]) + std::string("\", zpt::path::join(_split), { \"href\", _r_data[\"href\"], \"") + _name + std::string("\", (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(") }, { \"href\", _r_data[\"href\"], \"mutated-event\", true });\n");
			}
		}		
		_return += std::string("}\n");
		_return += std::string("}\n");
		_return += std::string("}\n");	
		_return += std::string("} catch (zpt::assertion& _e) { zlog(_e.what() + std::string(\": \") + _e.description(), zpt::error); }\n");
		_return += std::string("}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_update(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_associations_remove(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	if (std::string(_field["rel"]).find("/") != std::string::npos) {
		return _return;
	}
	std::string _dbms = this->build_dbms();
	if (_dbms.length() != 0) {
		zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
		_return += std::string("if (_tv_operation == \"remove\") {\n");	
		_return += std::string("try {\nif (_envelope[\"payload\"][\"filter\"]->ok()) return;\n");
		_return += std::string("zpt::json _c_names = { zpt::array, ");
		_return += this->build_dbms();
		_return += std::string(" };\n");
		_return += std::string("for (auto _c_name : _c_names->arr()) {\n");
		_return += std::string("zpt::connector _c = _emitter->connector(_c_name->str());\n");
		_return += std::string("zpt::json _r_targets = _c->query(\"") + std::string(this->__spec["name"]) + std::string("\", { \"") + _name + std::string("\", { \"href\", _envelope[\"payload\"][\"href\"] } });\n");
		_return += std::string("for (auto _e : _r_targets[\"elements\"]->arr()) {\n");
		_return += std::string("zpt::json _r_data(_e);\n");		
		_return += std::string("_emitter->route(zpt::ev::Get, zpt::path::join({ zpt::array, ");
		_return += this->build_topic(zpt::split(std::string(_field["ref"]), "/"));
		_return += std::string(" }), ");
		std::string _params = this->build_params(_rel, false);
		if (_params.length() != 0) {
			if (std::string(_field["rel"]).find("/") != std::string::npos) {
				_return += this->build_query(_field);
			}
			else {
				_return += std::string("{ \"params\", { ");
				_return += _params;
				_return += std::string(" } }");
			}
		}
		else {
			_return += std::string("zpt::undefined");
		}
		_return += std::string(", zpt::undefined, \n[ & ] (zpt::ev::performative _p_performative, std::string _p_topic, zpt::json _p_envelope, zpt::ev::emitter _p_emitter) -> void {\n");
		_return += std::string("zpt::json _r_") + _name + std::string(" = _p_envelope[\"payload\"];\n");
		if (_field["type"] == zpt::json::string("array")) {
			_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _r_data[\"href\"]->str(), { \"") + _name + std::string("\", (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"] : zpt::json({ zpt::array, _r_") + _name + std::string(" })) }, { \"href\", _r_data[\"href\"], \"mutated-event\", true });\n");
		}
		else if (_field["type"] == zpt::json::string("object")) {
			_return += std::string("_c->set(\"") + std::string(this->__spec["name"]) + std::string("\", _r_data[\"href\"]->str(), { \"") + _name + std::string("\", (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(") }, { \"href\", _r_data[\"href\"], \"mutated-event\", true });\n");
		}
		_return += std::string("});\n");		
		
		_return += std::string("}\n");
		_return += std::string("}\n");
		_return += std::string("} catch (zpt::assertion& _e) { zlog(_e.what() + std::string(\": \") + _e.description(), zpt::error); }\n");
		_return += std::string("}\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_replace(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_associations_get() -> std::string{
	std::string _return;
	// if (this->__spec["fields"]->is_object()) {
	// 	for (auto _field : this->__spec["fields"]->obj()) {
	// 		std::string _type(_field.second["type"]);
	// 		zpt::json _opts = zpt::gen::get_opts(_field.second);

	// 		if (_type == "object") {
	// 			if (_field.second["ref"]->ok() && _opts["on-demand"]->ok()) {
	// 				std::string _topic;
	// 				zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
	// 				zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
	// 				for (auto _part : _splited->arr()) {
	// 					_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
	// 				}

	// 				std::string _remote_invoke;
	// 				if (_rel->obj()->size() != 0) {
	// 					std::string _params = this->build_params(_rel, false);
	// 					_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n)[\"payload\"];\n");
	// 				}
	// 				else {
	// 					_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
	// 				}
					
	// 				_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
	// 				_return += std::string("_r_data << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->is_array() ? _d_") + _field.first + std::string("[\"elements\"][0] : _d_") + _field.first + std::string(");\n");
	// 			}
	// 		}
	// 		else if (_type == "array") {
	// 			if (_field.second["ref"]->type() == zpt::JSString && _opts["on-demand"]->ok()) {
	// 				std::string _topic;
	// 				zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
	// 				zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
	// 				for (auto _part : _splited->arr()) {
	// 					_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_r_data[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
	// 				}

	// 				std::string _remote_invoke;
	// 				if (_rel->obj()->size() != 0) {
	// 					std::string _params = this->build_params(_rel, false);
	// 					_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n)[\"payload\"];\n");
	// 				}
	// 				else {
	// 					_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
	// 				}

	// 				_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
	// 				_return += std::string("_r_data << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->is_array() ? _d_") + _field.first + std::string("[\"elements\"] : zpt::json({ zpt::array, _d_") + _field.first + std::string(" }));\n");
	// 			}
	// 		}
	// 	}
	// }
	return _return;
}

auto zpt::GenDatum::build_associations_query() -> std::string{
	std::string _return;
	// if (this->__spec["fields"]->is_object()) {
	// 	bool _found = false;
	// 	for (auto _field : this->__spec["fields"]->obj()) {
	// 		std::string _type(_field.second["type"]);
	// 		zpt::json _opts = zpt::gen::get_opts(_field.second);

	// 		if (_type == "object") {
	// 			if (_field.second["ref"]->ok() && _opts["on-demand"]->ok()) {
	// 				if (!_found) {
	// 					_return += std::string("if (_r_data[\"elements\"]->is_array()) {\n");
	// 					_return += std::string("for (auto _d_element : _r_data[\"elements\"]->arr()) {\n");
	// 				}
	// 				std::string _topic;
	// 				zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
	// 				zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
	// 				for (auto _part : _splited->arr()) {
	// 					_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_d_element[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
	// 				}

	// 				std::string _remote_invoke;
	// 				if (_rel->obj()->size() != 0) {
	// 					std::string _params = this->build_params(_rel, true);
	// 					_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n)[\"payload\"];\n");
	// 				}
	// 				else {
	// 					_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
	// 				}
					
	// 				_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
	// 				_return += std::string("_d_element << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->is_array() ? _d_") + _field.first + std::string("[\"elements\"][0] : _d_") + _field.first + std::string(");\n");

	// 				_found = true;
	// 			}
	// 		}
	// 		else if (_type == "array") {
	// 			if (_field.second["ref"]->ok() && _opts["on-demand"]->ok()) {
	// 				if (!_found) {
	// 					_return += std::string("if (_r_data[\"elements\"]->is_array()) {\n");
	// 					_return += std::string("for (auto _d_element : _r_data[\"elements\"]->arr()) {\n");
	// 				}

	// 				std::string _topic;
	// 				zpt::json _rel = zpt::uri::query::parse(std::string(_field.second["rel"]));
	// 				zpt::json _splited = zpt::split(std::string(_field.second["ref"]), "/");
	// 				for (auto _part : _splited->arr()) {
	// 					_topic += std::string(", ") + (_part->str().front() == '{' ? std::string("std::string(_d_element[\"") + _part->str().substr(1, _part->str().length() - 2) + std::string("\"])") : std::string("\"") + _part->str() + std::string("\""));
	// 				}

	// 				std::string _remote_invoke;
	// 				if (_rel->obj()->size() != 0) {
	// 					std::string _params = this->build_params(_rel, true);
	// 					_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) }\n)[\"payload\"];\n");
	// 				}
	// 				else {
	// 					_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] }\n)[\"payload\"];\n");
	// 				}
					
	// 				_return += std::string("zpt::json _d_") + _field.first + std::string(" = ") + _remote_invoke;
	// 				_return += std::string("_d_element << \"") + _field.first + std::string("\" << (_d_") + _field.first + std::string("[\"elements\"]->is_array() ? _d_") + _field.first + std::string("[\"elements\"] : zpt::json({ zpt::array, _d_") + _field.first + std::string(" }));\n");

	// 				_found = true;
	// 			}
	// 		}
	// 	}
	// 	if (_found) {
	// 		_return += std::string("}\n");
	// 		_return += std::string("}\n");
	// 	}
	// }
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

auto zpt::GenDatum::build_associations_for_insert(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	zpt::json _rel = zpt::uri::query::parse(std::string(_field["rel"]));
	_return += std::string("zpt::json _r_") + _name + std::string(" = _c->query(\"") + zpt::Generator::get_datum(std::string(_field["ref"])) + std::string("\", ");
	if (_rel->ok() && _rel->obj()->size() != 0) {
		if (std::string(_field["rel"]).find("/") != std::string::npos) {
			_return += this->build_query(_field);
		}
		else {
			_return += std::string("{ ");
			_return += this->build_params(_rel, false);
			_return += std::string(" }");
		}
	}
	else {
		_return += std::string("zpt::json::object()");
	}
	_return += std::string(");\n");
	if (_field["type"] == zpt::json::string("object")) {
		_return += std::string("if (_r_") + _name + std::string("->ok()) _document << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"][0] : _r_") + _name + std::string(");\n");
	}
	else if (_field["type"] == zpt::json::string("array")) {
		_return += std::string("if (_r_") + _name + std::string("->ok()) _document << \"") + _name + std::string("\" << (_r_") + _name + std::string("[\"elements\"]->is_array() ? _r_") + _name + std::string("[\"elements\"] : zpt::json{ zpt::array, _r_") + _name + std::string(" });\n");
	}
	return _return;
}

auto zpt::GenDatum::build_associations_for_update(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_associations_for_remove(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_associations_for_replace(std::string _name, zpt::json _field) -> std::string {
	std::string _return;
	return _return;
}

auto zpt::GenDatum::build_validation() -> std::string {
	zpt::json _fields;
	std::string _return;
	_fields = this->__spec["fields"];
	if (_fields->is_object()) {
		for (auto _field : _fields->obj()) {
			std::string _type(_field.second["type"]);
			zpt::json _opts = zpt::gen::get_opts(_field.second);
			
			if (_opts["mandatory"]->ok() && !_opts["read-only"]->ok() && !_opts["default"]->ok() && !_opts["auto"]->ok()
			) {
				_return += std::string("assertz_mandatory(_document, \"") + _field.first + std::string("\", 412);\n");
			}

			if (!_opts["read-only"]->ok()) {
				if (_type == "json") {
					_return += std::string("assertz_complex(_document, \"") + _field.first + std::string("\", 412);\n");
				}
				else {
					_return += std::string("assertz_") + _type + std::string("(_document, \"") + _field.first + std::string("\", 412);\n");
				}
			}
			else {
				_return += std::string("_document >> \"") + _field.first + std::string("\";\n");
			}
			
			if (_opts["default"]->ok()) {
				if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email" || _type == "phone") {
					_return += std::string("if (!_document[\"") + _field.first + std::string("\"]->ok()) {\n");
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::json::string(\"") + std::string(_opts["default"]) + std::string("\");\n");
					_return += std::string("}\n");
				}
				else if (_type == "int") {
					_return += std::string("if (!_document[\"") + _field.first + std::string("\"]->ok()) {\n");
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::json::integer(") + std::string(_opts["default"]) + std::string(");\n");
					_return += std::string("}\n");
				}
				else if (_type == "boolean") {
					_return += std::string("if (!_document[\"") + _field.first + std::string("\"]->ok()) {\n");
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::json::boolean(") + std::string(_opts["default"]) + std::string(");\n");
					_return += std::string("}\n");
				}
				else if (_type == "double") {
					_return += std::string("if (!_document[\"") + _field.first + std::string("\"]->ok()) {\n");
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::json::floating(") + std::string(_opts["default"]) + std::string(");\n");
					_return += std::string("}\n");
				}
				else if (_type == "timestamp") {
					_return += std::string("if (!_document[\"") + _field.first + std::string("\"]->ok()) {\n");
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::json::date(") + std::string(_opts["default"]) + std::string(");\n");
					_return += std::string("}\n");
				}
				else if (_type == "object") {
					_return += std::string("if (!_document[\"") + _field.first + std::string("\"]->ok()) {\n");
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::json(\"") + std::string(_opts["default"]) + std::string("\");\n");
					_return += std::string("}\n");
				}
			}
			
			if (_opts["auto"]->ok()) {
				if (_type == "token") {
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::generate::r_key(24);\n");
				}
				else if (_type == "uuid") {
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::generate::r_uuid();\n");
				}
				else if (_type == "hash") {
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::generate::r_key(64);\n");
				}
				else if (_type == "timestamp") {
					_return += std::string("_document << \"") + _field.first + std::string("\" << zpt::json::date();\n");
				}
			}
		}
	}
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
	std::string _fields_arr = zpt::gen::get_fields_array(this->__spec["fields"]);
	std::string _return(
		"std::string _r_id;\n"
		"size_t _c_idx = 0;\n"
		"zpt::json _c_names = { zpt::array, $[datum.method.ordered.clients] };\n\n"
		"_document <<\n\"created\" << zpt::json::date() <<\n\"updated\" << zpt::json::date();\n");
	_return += this->build_validation();
	_return += std::string(
		"\nfor (auto _c_name : _c_names->arr()) {\n"
		"zpt::connector _c = _emitter->connector(_c_name);\n"
		"if (zpt::is_sql(std::string(_c_name))) {\n"
		"if (_r_id.length() == 0) {\n"
		"_r_id = _c->insert(\"$[datum.collection]\", _topic, _document, { \"href\", _topic, \"fields\", ") + _fields_arr + std::string(", \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });\n"
			"_document << \"id\" << _r_id;\n"
			"}\nelse {\n"
			"_c->insert(\"$[datum.collection]\", _topic, _document, { \"href\", _topic, \"fields\", ") + _fields_arr + std::string(", \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });\n"
				"}\n"
				"}\n"
				"else {\n"
				"$[datum.relations.insert]\n"
				"if (_r_id.length() == 0) {\n"
				"_r_id = _c->insert(\"$[datum.collection]\", _topic, _document, { \"href\", _topic, \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });\n"
				"_document << \"id\" << _r_id;\n"
				"}\nelse {\n"
				"_c->insert(\"$[datum.collection]\", _topic, _document, { \"href\", _topic, \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });\n"
				"}\n"
				"}\n"
				"_c_idx++;\n"
				"}\n");

	if (this->__spec["extends"]["name"]->ok()) {
		std::string _name = zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_");
		auto _found = zpt::Generator::datums.find(_name);
		if (_found != zpt::Generator::datums.end()) {
			_name = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_");
			_return += _name + std::string("::insert(_topic, _document, _emitter, _identity, _envelope);\n");
		
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
		}
	}
	_return += std::string("_r_data = { \"id\", _r_id, \"href\", (_topic + std::string(\"/\") + _r_id) };\n");
	return _return;
}

auto zpt::GenDatum::build_extends_save() -> std::string {
	std::string _fields_arr = zpt::gen::get_fields_array(this->__spec["fields"]);
	std::string _return = std::string(
		"size_t _n_updated = 0;\n"
		"size_t _c_idx = 0;\n"
		"zpt::json _c_names = { zpt::array, $[datum.method.ordered.clients] };\n\n"
		"_document <<\n\"updated\" << zpt::json::date();\n"
		"\nfor (auto _c_name : _c_names->arr()) {\n"
		"zpt::connector _c = _emitter->connector(_c_name);\n"
		"if (zpt::is_sql(std::string(_c_name))) {\n"
		"_n_updated = _c->save(\"$[datum.collection]\", _topic, _document, { \"href\", _topic, \"fields\", ") + _fields_arr + std::string(", \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });\n"
			"}\nelse {\n"
			"_n_updated = _c->save(\"$[datum.collection]\", _topic, _document, { \"href\", _topic, \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });\n"
			"}\n"
			"_c_idx++;\n"
			"}\n");

	if (this->__spec["extends"]["name"]->ok()) {
		std::string _name = zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_");
		auto _found = zpt::Generator::datums.find(_name);
		if (_found != zpt::Generator::datums.end()) {
			_name = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_");
			_return += _name + std::string("::save(_topic, _document, _emitter, _identity, _envelope);\n");
		}
	}
	_return += std::string("\n_r_data = { \"href\", _topic, \"n_updated\", _n_updated };\n");
	return _return;
}

auto zpt::GenDatum::build_extends_set_topic() -> std::string {
	std::string _fields_arr = zpt::gen::get_fields_array(this->__spec["fields"]);
	std::string _return = std::string(
		"size_t _n_updated = 0;\n"
		"size_t _c_idx = 0;\n"
		"zpt::json _c_names = { zpt::array, $[datum.method.ordered.clients] };\n\n"
		"_document <<\n\"updated\" << zpt::json::date();\n"
		"\nfor (auto _c_name : _c_names->arr()) {\n"
		"zpt::connector _c = _emitter->connector(_c_name);\n"
		"if (zpt::is_sql(std::string(_c_name))) {\n"
		"_n_updated = _c->set(\"$[datum.collection]\", _topic, _document, { \"href\", _topic, \"fields\", ") + _fields_arr + std::string(", \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });\n"
			"}\nelse {\n"
			"_n_updated = _c->set(\"$[datum.collection]\", _topic, _document, { \"href\", _topic, \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });\n"
			"}\n"
			"_c_idx++;\n"
			"}\n");

	if (this->__spec["extends"]["name"]->ok()) {
		std::string _name = zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_");
		auto _found = zpt::Generator::datums.find(_name);
		if (_found != zpt::Generator::datums.end()) {
			_name = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_");
			_return += _name + std::string("::set(_topic, _document, _emitter, _identity, _envelope);\n");
		}
	}
	_return += std::string("\n_r_data = { \"href\", _topic, \"n_updated\", _n_updated };\n");
	return _return;
}

auto zpt::GenDatum::build_extends_set_pattern() -> std::string {
	std::string _fields_arr = zpt::gen::get_fields_array(this->__spec["fields"]);
	std::string _return = std::string(
		"size_t _n_updated = 0;\n"
		"size_t _c_idx = 0;\n"
		"zpt::json _c_names = { zpt::array, $[datum.method.ordered.clients] };\n\n"
		"_document <<\n\"updated\" << zpt::json::date();\n"
		"\nfor (auto _c_name : _c_names->arr()) {\n"
		"zpt::connector _c = _emitter->connector(_c_name);\n"
		"if (zpt::is_sql(std::string(_c_name))) {\n"
		"_n_updated = _c->set(\"$[datum.collection]\", _filter, _document, { \"href\", _topic, \"fields\", ") + _fields_arr + std::string(", \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });"
			"}\nelse {\n"
			"_n_updated = _c->set(\"$[datum.collection]\", _filter, _document, { \"href\", _topic, \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });"
			"}\n"
			"_c_idx++;\n"
			"}\n");

	if (this->__spec["extends"]["name"]->ok()) {
		std::string _name = zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_");
		auto _found = zpt::Generator::datums.find(_name);
		if (_found != zpt::Generator::datums.end()) {
			_name = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_");
			_return += _name + std::string("::set(_topic, _document, _filter, _emitter, _identity, _envelope);\n");
		}
	}
	_return += std::string("\n_r_data = { \"href\", _topic, \"n_updated\", _n_updated };\n");
	return _return;
}

auto zpt::GenDatum::build_extends_remove_topic() -> std::string {
	std::string _fields_arr = zpt::gen::get_fields_array(this->__spec["fields"]);
	std::string _return(
		"size_t _n_deleted = 0;\n"
		"size_t _c_idx = 0;\n"
		"zpt::json _c_names = { zpt::array, $[datum.method.ordered.clients] };\n\n");

	if (this->__spec["extends"]["name"]->ok()) {
		std::string _name = zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_");
		auto _found = zpt::Generator::datums.find(_name);
		if (_found != zpt::Generator::datums.end()) {
			_name = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_");
			_return += _name + std::string("::remove(_topic, _emitter, _identity, _envelope);\n");
		}
	}

	_return += std::string(
		"\nfor (auto _c_name : _c_names->arr()) {\n"
		"zpt::connector _c = _emitter->connector(_c_name);\n"
		"_n_deleted = _c->remove(\"$[datum.collection]\", _topic, { \"href\", _topic, \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });"
		"_c_idx++;\n"
		"}\n");

	_return += std::string("\n_r_data = { \"href\", _topic, \"n_deleted\", _n_deleted };\n");
	return _return;
}

auto zpt::GenDatum::build_extends_remove_pattern() -> std::string {
	std::string _fields_arr = zpt::gen::get_fields_array(this->__spec["fields"]);
	std::string _return(
		"size_t _n_deleted = 0;\n"
		"size_t _c_idx = 0;\n"
		"zpt::json _c_names = { zpt::array, $[datum.method.ordered.clients] };\n\n");

	if (this->__spec["extends"]["name"]->ok()) {
		std::string _name = zpt::r_replace(this->__spec["extends"]["name"]->str(), "-", "_");
		auto _found = zpt::Generator::datums.find(_name);
		if (_found != zpt::Generator::datums.end()) {
			_name = std::string(_found->second->spec()["namespace"]) + std::string("::datums::") + zpt::r_replace(_found->second->spec()["name"]->str(), "-", "_");
			_return += _name + std::string("::remove(_topic, _filter, _emitter, _identity, _envelope);\n");
		}
	}

	_return += std::string(
		"\nfor (auto _c_name : _c_names->arr()) {\n"
		"zpt::connector _c = _emitter->connector(_c_name);\n"
		"_n_deleted = _c->remove(\"$[datum.collection]\", _topic, { \"href\", _topic, \"mutated-event\", (_c_idx != _c_names->arr()->size() - 1) });"
		"_c_idx++;\n"
		"}\n");

	_return += std::string("\n_r_data = { \"href\", _topic, \"n_deleted\", _n_deleted };\n");
	return _return;
}

auto zpt::GenDatum::build_initialization(std::string _dbms, std::string _namespace) -> std::string {
	if (_dbms == "postgresql") {
		return "{ \"dbms.pgsql." + zpt::r_replace(_namespace, "::", ".") + "\", zpt::connector(new zpt::pgsql::Client(_emitter->options(), \"pgsql." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	else if (_dbms == "mariadb") {
		return "{ \"dbms.mariadb." + zpt::r_replace(_namespace, "::", ".") + "\", zpt::connector(new zpt::mariadb::Client(_emitter->options(), \"mariadb." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
	}
	else if (_dbms == "couchdb") {
		return "{ \"dbms.couchdb." + zpt::r_replace(_namespace, "::", ".") + "\", zpt::connector(new zpt::couchdb::Client(_emitter->options(), \"couchdb." + zpt::r_replace(_namespace, "::", ".") + "\")) }, ";
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

auto zpt::GenDatum::build_ordered_data_client(zpt::json _dbms, zpt::json _ordered, std::string _namespace) -> std::string {
	std::string _return;
	for (auto _db : _ordered->arr()) {
		if (std::find(std::begin(_dbms->arr()), std::end(_dbms->arr()), _db) != std::end(_dbms->arr())) {
			std::string _db_client(_db->str());
			if (_db->str() == "postgresql") {
				_db_client.assign("pgsql");
			}
			if (_return.length() != 0) {
				_return += std::string(", ");
			}
			_return += std::string("\"dbms.") + _db_client + std::string(".") + zpt::r_replace(_namespace, "::", ".") + std::string("\"");
		}
	}
	return _return;
}

auto zpt::GenDatum::get_type(zpt::json _field) -> std::string {
	zpt::json _opts = zpt::gen::get_opts(_field);
	std::string _type(_field["type"]);
	std::string _return;
	
	if (_opts["serial"]->ok()) {
		return "serial";
	}
	
	if (_type == "utf8" || _type == "text") {
		_return += std::string("text");
 	}
	else if (_type == "string") {
		_return += std::string("varchar(1024)");
	}
	else if (_type == "ascii") {
		_return += std::string("varchar(128)");
	}
	else if (_type == "token" || _type == "hash" || _type == "uri" || _type == "email") {
		_return += std::string("varchar(512)");
	}
	else if (_type == "phone") {
		_return += std::string("varchar(20)");
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
	else if (_type == "location") {
		_return += std::string("point");
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
	if (_opts["foreign"]->ok()) {
		if (_return.length() != 0) {
			_return += std::string(" ");
		}
		zpt::json _splited = zpt::split(std::string(_opts["foreign"]), ":");
		_return += std::string("references ") + _splited->arr()->back()->str();
		if (_opts["cascade"]->ok()) {
			_return += std::string(" on delete cascade");
		}
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
		if (this->__spec["protocols"]->is_array()) {
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
			ztrace(std::string("processed ") + _h_file);
		}
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_cxx_exists)) {
			std::ofstream _cxx_ofs(_cxx_file.data());
			_cxx_ofs << _handler_cxx << endl << flush;
			_cxx_ofs.close();
			ztrace(std::string("processed ") + _cxx_file);
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
		if (this->__spec["protocols"]->is_array()) {
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
			ztrace(std::string("processed ") + _lisp_file);
		}
	}
	if (this->__options["resource-out-lang"]->ok() && std::find(std::begin(this->__options["resource-out-lang"]->arr()), std::end(this->__options["resource-out-lang"]->arr()), zpt::json::string("python")) != std::end(this->__options["resource-out-lang"]->arr())) {
		std::string _handler_py;
		zpt::load_path("/usr/share/zapata/gen/Handlers.py", _handler_py);
		std::string _py_file = std::string(this->__options["resource-out-scripts"][0]) + std::string("/") + std::string(_parent_name) + std::string("/") + std::string(this->__spec["type"]) + std::string("s/") + std::string(this->__spec["name"]) + std::string(".py");

		zpt::json _performatives = { zpt::array, "get", "post", "put", "patch", "delete", "head" };
		bool _first = true;
		std::string _type = std::string(this->__spec["type"]);
		std::transform(std::begin(_type), std::begin(_type) + 1, std::begin(_type), ::toupper);
		std::string _c_name = zpt::r_replace(zpt::r_prettify_header_name(std::string(this->__spec["name"])), "-", "") + _type;
		zpt::replace(_handler_py, "$[resource.handler.class.name]", _c_name);
		for (auto _perf : _performatives->arr()) {
			if (this->__spec["performatives"]->ok() && std::find(std::begin(this->__spec["performatives"]->arr()), std::end(this->__spec["performatives"]->arr()), zpt::json::string(_perf->str())) == std::end(this->__spec["performatives"]->arr())) {
				zpt::replace(_handler_py, std::string("$[resource.handler.") + _perf->str() + std::string("]"), std::string(""));
				zpt::replace(_handler_py, std::string("$[resource.handler.") + _perf->str() + std::string(".name]"), std::string(""));
				continue;
			}

			std::string _f_name = _c_name + std::string(".") + _perf->str();
			std::string _function;
			if (_perf->str() == "get") {
				_function.assign(this->build_python_get());
			}
			else if (_perf->str() == "post") {
				_function.assign(this->build_python_post());
			}
			else if (_perf->str() == "put") {
				_function.assign(this->build_python_put());
			}
			else if (_perf->str() == "patch") {
				_function.assign(this->build_python_patch());
			}
			else if (_perf->str() == "delete") {
				_function.assign(this->build_python_delete());
			}
			else if (_perf->str() == "head") {
				_function.assign(this->build_python_head());
			}

			zpt::replace(_handler_py, std::string("$[resource.handler.") + _perf->str() + std::string("]"), _function);
			zpt::replace(_handler_py, std::string("$[resource.handler.") + _perf->str() + std::string(".name]"), (!_first ? std::string(",\n                ") : std::string("")) + std::string("'") + _perf->str() + std::string("' : ") + _f_name + std::string(""));
			_first = false;
				
		}
		zpt::replace(_handler_py, "$[resource.topic.regex]", zpt::gen::url_pattern_to_regexp(this->__spec["topic"]));

		std::string _resource_opts("{ ");
		if (this->__spec["protocols"]->is_array()) {
			for (auto _proto : this->__spec["protocols"]->arr()) {
				if (_resource_opts.length() != 2) _resource_opts += std::string(", ");
				_resource_opts += std::string(" \"") + _proto->str() + std::string("\" : True");
			}
			_resource_opts += std::string(" }");
		}
		zpt::replace(_handler_py, "$[resource.opts]", _resource_opts);
		
		struct stat _buffer;
		bool _py_exists = stat(_py_file.c_str(), &_buffer) == 0;
		if (bool(this->__options["force-resource"][0]) || (!bool(this->__options["force-resource"][0]) && !_py_exists)) {
			std::ofstream _py_ofs(_py_file.data());
			_py_ofs << _handler_py << endl << flush;
			_py_ofs.close();
			ztrace(std::string("processed ") + _py_file);
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
	if (_fields->is_object()) {
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
					if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email" || _type == "phone") {
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
					else if (std::string(this->__spec["type"]) == "document" && (_perf == zpt::ev::Put)) {
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
	if (this->__spec["topic"]->is_array()) {
		_return += std::string("zpt::json _identity;\n");
		_return += zpt::gen::url_pattern_to_var_decl(this->__spec["topic"]);
		bool _first = true;;
		for (auto _topic : this->__spec["topic"]->arr()) {
			_return += (!_first ? std::string("else ") : std::string("")) + std::string("if (zpt::test::regex(_topic, \"") + zpt::gen::url_pattern_to_regexp(_topic) + std::string("\")) {\n");
			_return += zpt::r_replace(zpt::gen::url_pattern_to_vars(std::string(_topic)), "zpt::json ", "");
			_return += std::string("_identity = zpt::rest::authorization::validate(\"") + std::string(_topic) + std::string("\", _envelope, _emitter);\n");
			_return += std::string("}\n");
			_first = false;
		}
	}
	else {
		_return += zpt::gen::url_pattern_to_vars(std::string(this->__spec["topic"]));
		_return += std::string("zpt::json _identity = zpt::rest::authorization::validate(\"") + std::string(this->__spec["topic"]) + std::string("\", _envelope, _emitter);\n");
	}
	_return += std::string("\nzpt::json _r_body;\n");
	_return += std::string("/* ---> YOUR CODE HERE <---*/\n");
	if (_perf != zpt::ev::Reply && _perf != zpt::ev::Delete && this->__spec["links"]->is_array()) {
		_return += std::string("if (_r_body[\"payload\"]->ok() && int(_r_body[\"status\"]) < 300) {\n");
		_return += std::string("_r_body[\"payload\"] << \"links\" << zpt::json({ ");
		
		for (auto _link : this->__spec["links"]->arr()) {
			std::string _topic;
			zpt::json _url_params = zpt::gen::url_pattern_to_params(_link["ref"]);		
			zpt::json _splited = zpt::split(std::string(_link["ref"]), "/");
			for (auto _part : _splited->arr()) {
				_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
			}
			_return += std::string("\"") + std::string(_link["rel"]) + std::string("\", zpt::path::join({ zpt::array") + _topic + std::string(" }), ");
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

	std::string _return("{ zpt::ev::Get,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {\n");
	_return += this->build_handler_header(zpt::ev::Get);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body });\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", (_r_body->ok() ? 200 : 204), \"payload\", _r_body  });\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("_emitter->reply(_envelope, { \"status\", (_r_body->ok() ? 200 : 404), \"payload\", _r_body  });\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::path::join({ zpt::array") + _topic + std::string(" });\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_get(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
		
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
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
		}
		else {
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Get,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
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

	std::string _return("{ zpt::ev::Post,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {\n");
	_return += this->build_handler_header(zpt::ev::Post);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", 201, \"payload\", _r_body });\n");
		}
		if (std::string(this->__spec["type"]) == "controller") {
			_return += std::string("_emitter->reply(_envelope, { \"status\", 200, \"payload\", _r_body });\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::path::join({ zpt::array") + _topic + std::string(" });\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_post(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
		
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
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Post,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", (_envelope[\"payload\"] + zpt::json({ ") + _params + std::string(" })) },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
		}
		else {
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Post,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"] },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
		}
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

	std::string _return("{ zpt::ev::Put,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {\n");
	_return += this->build_handler_header(zpt::ev::Put);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", 201, \"payload\", _r_body });\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("_emitter->reply(_envelope, { \"status\", 200, \"payload\", _r_body });\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::path::join({ zpt::array") + _topic + std::string(" });\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_put(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
		
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
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Put,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", (_envelope[\"payload\"] + zpt::json({ ") + _params + std::string(" })) },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
		}
		else {
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Put,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"] },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
		}
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

	std::string _return("{ zpt::ev::Patch,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {\n");
	_return += this->build_handler_header(zpt::ev::Patch);
	
	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", 200, \"payload\", _r_body });\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", 200, \"payload\", _r_body });\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("_emitter->reply(_envelope, { \"status\", 200, \"payload\", _r_body });\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::path::join({ zpt::array") + _topic + std::string(" });\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_patch(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
		
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
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Patch,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"], \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
		}
		else {
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Patch,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"payload\", _envelope[\"payload\"], \"params\", _envelope[\"params\"] },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
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

	std::string _return("{ zpt::ev::Delete,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {\n");
	_return += this->build_handler_header(zpt::ev::Delete);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", 200, \"payload\", _r_body });\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", 200, \"payload\", _r_body });\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("_emitter->reply(_envelope, { \"status\", 200, \"payload\", _r_body });\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::path::join({ zpt::array") + _topic + std::string(" });\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_delete(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
		
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
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Delete,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
		}
		else {
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Delete,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
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

	std::string _return("{ zpt::ev::Head,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {\n");
	_return += this->build_handler_header(zpt::ev::Head);

	if (!this->__spec["datum"]["ref"]->ok()) {
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", (_r_body->ok() ? 200 : 204) });\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("_emitter->reply(_envelope, { \"status\", (_r_body->ok() ? 200 : 204) });\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("_emitter->reply(_envelope, { \"status\", (_r_body->ok() ? 200 : 204) });\n");
		}
	}
	_return += std::string("\n}\n}");

	if (this->__spec["datum"]["name"]->ok()) {
		std::map<std::string, zpt::gen::datum>::iterator _found = zpt::Generator::datums.find(this->__spec["datum"]["name"]->str());	
		if (_found != zpt::Generator::datums.end()) {
			std::string _topic_transform;
			if (this->__spec["datum"]["href"]->ok()) {
				std::string _topic;
				zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
				zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["href"]), "/");
				for (auto _part : _splited->arr()) {
					_topic += std::string(", ") + (_url_params[_part->str()]->ok() ? std::string("std::string(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("\"") + _part->str() + std::string("\""));
				}
				_topic_transform = std::string("\n_topic = zpt::path::join({ zpt::array") + _topic + std::string(" });\n");
			}
			zpt::replace(_return, "/* ---> YOUR CODE HERE <---*/", _topic_transform + _found->second->build_head(this->__spec));
		}
	}
	else if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params(this->__spec["topic"]);
		
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
				_params += std::string("\"") + _param.first + std::string("\", ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Head,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", (_envelope[\"params\"] + zpt::json({ ") + _params + std::string(" })) },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
		}
		else {
			_remote_invoke += std::string("_emitter->route(\nzpt::ev::Head,\nzpt::path::join({ zpt::array") + _topic + std::string(" }),\n{ \"headers\", zpt::rest::authorization::headers(_identity[\"access_token\"]), \"params\", _envelope[\"params\"] },\n[ _envelope ] (zpt::ev::performative _c_performative, std::string _c_topic, zpt::json _c_result, zpt::ev::emitter _c_emitter) -> void {\n_c_emitter->reply(_envelope, _c_result)\n}\n);\n");
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

	std::string _return("{ zpt::ev::Reply,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {\n");
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
	if (_fields->is_object()) {
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
						(std::string(this->__spec["type"]) == "controller") ||
						(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
						(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put) ||
						(std::string(this->__spec["type"]) == "document" && _perf == zpt::ev::Put)
					)
				) {
					if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email" || _type == "phone") {
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
						(
							(std::string(this->__spec["type"]) == "controller") ||
							(std::string(this->__spec["type"]) == "collection" && _perf == zpt::ev::Post) ||
							(std::string(this->__spec["type"]) == "store" && _perf == zpt::ev::Put)
						) ||
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
							(std::string(this->__spec["type"]) == "controller") ||
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
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("        (zpt:route \"get\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"get\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope)))))");
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
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("        (zpt:route \"post\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"post\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope)))))");
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
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("        (zpt:route \"put\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"put\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope)))))");
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
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("        (zpt:route \"patch\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"patch\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope)))))");
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
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("        (zpt:route \"delete\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"delete\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope)))))");
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
		zpt::json _url_params = zpt::gen::url_pattern_to_params_lisp(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			_topic += std::string(" ") + (_url_params[_part->str()]->ok() ? std::string(_url_params[_part->str()]) : std::string("\"") + _part->str() + std::string("\""));
		}

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(" ");
				}
				_params += std::string("\"") + _param.first + std::string("\" ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("\"") + std::string(_param.second->str()) + std::string("\""));
			}
			_remote_invoke += std::string("        (zpt:route \"head\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (zpt:merge (gethash \"params\" envelope) (json ") + _params + std::string("))))))");
		}
		else {
			_remote_invoke += std::string("        (zpt:route \"head\"\n                   (zpt:join (list ") + _topic + std::string(" ) \"/\")\n                   (json \"headers\" (zpt:authorization-headers identity) \"params\" (gethash \"params\" envelope)))))");
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

	std::string _return("{ zpt::ev::Reply,\n[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {\n");
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
	if (_fields->is_object()) {
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
					if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email" || _type == "phone") {
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
					if (_type == "utf8" || _type == "ascii" || _type == "token" || _type == "uri" || _type == "uuid" || _type == "email" || _type == "phone") {
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
	
	_return += this->build_python_validation(_perf);
	_return += std::string("                t_split = topic.split('/')\n");
	_return += zpt::gen::url_pattern_to_vars_python(std::string(this->__spec["topic"]));
	_return += std::string("                identity = zpt.authorize('") + std::string(this->__spec["topic"]) + std::string("', envelope)\n");
	_return += std::string("                # ---> YOUR CODE HERE <---\n");
	if (_perf != zpt::ev::Reply && _perf != zpt::ev::Delete && this->__spec["links"]->is_array()) {
		_return += std::string("                if 'payload' in r_body and r_body['status'] < 300 : \n");
		_return += std::string("                        r_body['payload']['links'] = { ");
		
		for (auto _link : this->__spec["links"]->arr()) {
			std::string _topic;
			zpt::json _url_params = zpt::gen::url_pattern_to_params(_link["ref"]);	
			zpt::json _splited = zpt::split(std::string(_link["ref"]), "/");
			for (auto _part : _splited->arr()) {
				if (_topic.length() != 0) _topic += std::string(", ");
				_topic += (_url_params[_part->str()]->ok() ? std::string("str(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("'") + _part->str() + std::string("'"));
			}
			_return += std::string("'") + std::string(_link["rel"]) + std::string("': zpt.path_join([ ") + _topic + std::string(" ]), ");
		}
		_return += std::string(" }\n");
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

	std::string _f_name = std::string("get");
	std::string _return = std::string("        @staticmethod\n        def ") + _f_name + std::string("(performative, topic, envelope, context) :\n");
	_return += this->build_python_handler_header(zpt::ev::Get);

	if (!this->__spec["datum"]["ref"]->ok()) {
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", "                t_reply = None\n                # ---> YOUR CODE HERE <---\n");
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("                zpt.reply(envelope, { 'status' : (len(t_reply) != 0 ? 200 : 404), 'payload' : t_reply  })\n");
		}
		else {
			_return += std::string("                zpt.reply(envelope, { 'status' : (len(t_reply) != 0 ? 200 : 204), 'payload' : t_reply  })\n");
		}
	}
	_return += std::string("\n");

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_python(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			if (_topic.length() != 0) _topic += std::string(", ");
			_topic += (_url_params[_part->str()]->ok() ? std::string("str(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("'") + _part->str() + std::string("'"));
		}

		_remote_invoke += std::string("\n                def callback(performative, topic, t_reply, context) :\n                        zpt.reply(context, t_reply)\n\n");
		
		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("'") + _param.first + std::string("' : ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("'") + std::string(_param.second->str()) + std::string("'"));
			}
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'get',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'params' : zpt.merge(envelope['params'], { ") + _params + std::string(" }) },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n"
				"");
		}
		else {
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'get',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'params' : envelope['params'] },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n");
		}
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", _remote_invoke);
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

	std::string _f_name = std::string("post");
	std::string _return = std::string("        @staticmethod\n        def ") + _f_name + std::string("(performative, topic, envelope, context) :\n");
	_return += this->build_python_handler_header(zpt::ev::Post);

	if (!this->__spec["datum"]["ref"]->ok()) {
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", "                t_reply = None\n                # ---> YOUR CODE HERE <---\n");
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("                zpt.reply(envelope, { 'status' : 201, 'payload' : t_reply  })\n");
		}
		if (std::string(this->__spec["type"]) == "controller") {
			_return += std::string("                zpt.reply(envelope, { 'status' : 200, 'payload' : t_reply  })\n");
		}
	}
	_return += std::string("\n");

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_python(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			if (_topic.length() != 0) _topic += std::string(", ");
			_topic += (_url_params[_part->str()]->ok() ? std::string("str(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("'") + _part->str() + std::string("'"));
		}

		_remote_invoke += std::string("\n                def callback(performative, topic, t_reply, context) :\n                        zpt.reply(context, t_reply)\n\n");
		
		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("'") + _param.first + std::string("' : ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("'") + std::string(_param.second->str()) + std::string("'"));
			}
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'post',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'payload' : zpt.merge(envelope['payload'], { ") + _params + std::string(" }) },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n"
				"");
		}
		else {
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'post',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'payload' : envelope['payload'] },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n");
		}
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", _remote_invoke);
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

	std::string _f_name = std::string("put");
	std::string _return = std::string("        @staticmethod\n        def ") + _f_name + std::string("(performative, topic, envelope, context) :\n");
	_return += this->build_python_handler_header(zpt::ev::Put);

	if (!this->__spec["datum"]["ref"]->ok()) {
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", "                t_reply = None\n                # ---> YOUR CODE HERE <---\n");
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("                zpt.reply(envelope, { 'status' : 201, 'payload' : t_reply  })\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("                zpt.reply(envelope, { 'status' : 200, 'payload' : t_reply  })\n");
		}
	}
	_return += std::string("\n");

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_python(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			if (_topic.length() != 0) _topic += std::string(", ");
			_topic += (_url_params[_part->str()]->ok() ? std::string("str(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("'") + _part->str() + std::string("'"));
		}

		_remote_invoke += std::string("\n                def callback(performative, topic, t_reply, context) :\n                        zpt.reply(context, t_reply)\n\n");
		
		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("'") + _param.first + std::string("' : ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("'") + std::string(_param.second->str()) + std::string("'"));
			}
			_remote_invoke += std::string(
				"        zpt.route(\n"
				"                        'put',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'payload' : zpt.merge(envelope['payload'], { ") + _params + std::string(" }) },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n"
				"");
		}
		else {
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'put',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'payload' : envelope['payload'] },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n");
		}
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", _remote_invoke);
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

	std::string _f_name = std::string("patch");
	std::string _return = std::string("        @staticmethod\n        def ") + _f_name + std::string("(performative, topic, envelope, context) :\n");
	_return += this->build_python_handler_header(zpt::ev::Patch);
	
	if (!this->__spec["datum"]["ref"]->ok()) {
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", "                t_reply = None\n                # ---> YOUR CODE HERE <---\n");
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("                zpt.reply(envelope, { 'status' : 200, 'payload' : t_reply  })\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("                zpt.reply(envelope, { 'status' : 200, 'payload' : t_reply  })\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("                zpt.reply(envelope, { 'status' : 200, 'payload' : t_reply  })\n");
		}
	}
	_return += std::string("\n");

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_python(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			if (_topic.length() != 0) _topic += std::string(", ");
			_topic += (_url_params[_part->str()]->ok() ? std::string("str(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("'") + _part->str() + std::string("'"));
		}

		_remote_invoke += std::string("\n                def callback(performative, topic, t_reply, context) :\n                        zpt.reply(context, t_reply)\n\n");

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("'") + _param.first + std::string("' : ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("'") + std::string(_param.second->str()) + std::string("'"));
			}
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'patch',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'payload' : envelope['payload'], 'params' : zpt.merge(envelope['params'], { ") + _params + std::string(" }) },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n"
				"");
		}
		else {
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'patch',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'payload' : envelope['payload'], 'params' : envelope['params'] },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n");
		}
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", _remote_invoke);
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

	std::string _f_name = std::string("delete");
	std::string _return = std::string("        @staticmethod\n        def ") + _f_name + std::string("(performative, topic, envelope, context) :\n");
	_return += this->build_python_handler_header(zpt::ev::Delete);

	if (!this->__spec["datum"]["ref"]->ok()) {
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", "                t_reply = None\n                # ---> YOUR CODE HERE <---\n");
		if (std::string(this->__spec["type"]) == "collection") { 
			_return += std::string("                zpt.reply(envelope, { 'status' : 200, 'payload' : t_reply  })\n");
		}
		if (std::string(this->__spec["type"]) == "store") { 
			_return += std::string("                zpt.reply(envelope, { 'status' : 200, 'payload' : t_reply  })\n");
		}
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("                zpt.reply(envelope, { 'status' : 200, 'payload' : t_reply  })\n");
		}
	}
	_return += std::string("\n");

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_python(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			if (_topic.length() != 0) _topic += std::string(", ");
			_topic += (_url_params[_part->str()]->ok() ? std::string("str(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("'") + _part->str() + std::string("'"));
		}

		_remote_invoke += std::string("\n                def callback(performative, topic, t_reply, context) :\n                        zpt.reply(context, t_reply)\n\n");

		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("'") + _param.first + std::string("' : ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("'") + std::string(_param.second->str()) + std::string("'"));
			}
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'delete',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'params' : zpt.merge(envelope['params'], { ") + _params + std::string(" }) },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n"
				"");
		}
		else {
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'delete',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'params' : envelope['params'] },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n");
		}
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", _remote_invoke);
	}
	
	return _return;
}

auto zpt::GenResource::build_python_head() -> std::string {
	zpt::json _performatives = this->__spec["performatives"];
	if (_performatives->ok() && std::find(std::begin(_performatives->arr()), std::end(_performatives->arr()), zpt::json::string("get")) == std::end(_performatives->arr())) {
		return "";
	}
	if (std::string(this->__spec["type"]) == "controller") {
		return "";
	}

	std::string _f_name = std::string("head");
	std::string _return = std::string("        @staticmethod\n        def ") + _f_name + std::string("(performative, topic, envelope, context) :\n");
	_return += this->build_python_handler_header(zpt::ev::Head);

	if (!this->__spec["datum"]["ref"]->ok()) {
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", "                t_reply = None\n                # ---> YOUR CODE HERE <---\n");
		if (std::string(this->__spec["type"]) == "document") {
			_return += std::string("                zpt.reply(envelope, { 'status' : (len(t_reply) != 0 ? 200 : 404) })\n");
		}
		else {
			_return += std::string("                zpt.reply(envelope, { 'status' : (len(t_reply) != 0 ? 200 : 204) })\n");
		}
	}
	_return += std::string("\n");

	if (this->__spec["datum"]["ref"]->ok()) {
		std::string _remote_invoke;
		std::string _topic;
		zpt::json _rel = zpt::uri::query::parse(std::string(this->__spec["datum"]["rel"]));
		zpt::json _url_params = zpt::gen::url_pattern_to_params_python(this->__spec["topic"]);
		
		zpt::json _splited = zpt::split(std::string(this->__spec["datum"]["ref"]), "/");
		for (auto _part : _splited->arr()) {
			if (_topic.length() != 0) _topic += std::string(", ");
			_topic += (_url_params[_part->str()]->ok() ? std::string("str(") + std::string(_url_params[_part->str()]) + std::string(")") : std::string("'") + _part->str() + std::string("'"));
		}

		_remote_invoke += std::string("\n                def callback(performative, topic, t_reply, context) :\n                        zpt.reply(context, { 'status' : t_reply['status'] })\n\n");
		
		if (_rel->obj()->size() != 0) {
			std::string _params;
			for (auto _param : _rel->obj()) {
				if (_params.length() != 0) {
					_params += std::string(", ");
				}
				_params += std::string("'") + _param.first + std::string("' : ") + (_url_params[_param.second->str()]->ok() ? std::string(_url_params[_param.second->str()]) : std::string("'") + std::string(_param.second->str()) + std::string("'"));
			}
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'head',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'params' : zpt.merge(envelope['params'], { ") + _params + std::string(" }) },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n"
				"");
		}
		else {
			_remote_invoke += std::string(
				"                zpt.route(\n"
				"                        'head',\n"
				"                        zpt.path_join([ ") + _topic + std::string(" ]),\n"
				"                        { \"headers\" : zpt.auth_headers(identity), 'params' : envelope['params'] },\n"
				"                        { 'bubble-error' : True, 'context' : envelope },\n"
				"                        callback\n"
				"                )\n");
		}
		zpt::replace(_return, "# ---> YOUR CODE HERE <---", _remote_invoke);
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

	std::string _f_name = zpt::r_replace(std::string(this->__spec["name"]), "-", "_") + std::string("_") + std::string(this->__spec["type"]) + std::string("_reply");
	std::string _return = std::string("def ") + _f_name + std::string("(performative, topic, envelope) :\n");
	_return += this->build_python_handler_header(zpt::ev::Reply);
	_return += std::string("\n");
	return _return;
}

auto zpt::GenResource::build_mutations() -> std::string {
	std::string _return;
	return _return;
}

auto zpt::gen::url_pattern_to_regexp(zpt::json _url) -> std::string {
	zpt::json _url_list;
	if (!_url->is_array()) {
		_url_list = { zpt::array, _url };
	}
	else {
		_url_list = _url;
	}
	std::string _return;
	for (auto _u : _url_list->arr()) {
		if (_return.length() != 0) {
			_return += "|";
		}
		_return += std::string("^");		

		zpt::json _splited = zpt::split(std::string(_u), "/");
		for (auto _part : _splited->arr()) {
			_return += std::string("/") + (_part->str().find("{") != std::string::npos ? "([^/]+)" : _part->str());
		}
		
		_return += std::string("$");
	}
	return _return;
}

auto zpt::gen::url_pattern_to_var_decl(zpt::json _url) -> std::string {
	zpt::json _url_list;
	if (!_url->is_array()) {
		_url_list = { zpt::array, _url };
	}
	else {
		_url_list = _url;
	}
	std::map< std::string, std::string > _defined;
	std::string _return;
	for (auto _u : _url_list->arr()) {
		zpt::json _splited = zpt::split(_u, "/");
		for (auto _part : _splited->arr()) {
			if (_part->str().find("{") != std::string::npos) {
				std::string _name = std::string("_tv_") + _part->str().substr(1, _part->str().length() - 2);
				if (_defined.find(_name) == _defined.end()) {
					_return += std::string("zpt::json ") + _name + std::string(";\n");
					_defined.insert(std::make_pair(_name, _name));
				}
			}
		}
	}

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

auto zpt::gen::url_pattern_to_var_decl_lisp(zpt::json _url) -> std::string {
	zpt::json _url_list;
	if (!_url->is_array()) {
		_url_list = { zpt::array, _url };
	}
	else {
		_url_list = _url;
	}
	std::map< std::string, std::string > _defined;
	std::string _return;
	for (auto _u : _url_list->arr()) {
		zpt::json _splited = zpt::split(_u, "/");
		for (auto _part : _splited->arr()) {
			if (_part->str().find("{") != std::string::npos) {
				std::string _name = std::string("_tv_") + _part->str().substr(1, _part->str().length() - 2);
				if (_defined.find(_name) == _defined.end()) {
					_return += std::string("zpt::json ") + _name + std::string(";\n");
					_defined.insert(std::make_pair(_name, _name));
				}
			}
		}
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

auto zpt::gen::url_pattern_to_vars_python(std::string _url) -> std::string {
	zpt::json _splited = zpt::split(_url, "/");
	std::string _return;

	short _i = 0;
	for (auto _part : _splited->arr()) {
		if (_part->str().find("{") != std::string::npos) {
			_return += std::string("                tv_") + zpt::r_replace(_part->str().substr(1, _part->str().length() - 2), "-", "_") + std::string(" = t_split[") + std::to_string(_i + 1) + std::string("]\n");
		}
		_i++;
	}
	return _return;
}

auto zpt::gen::url_pattern_to_params(zpt::json _url) -> zpt::json {
	zpt::json _url_list;
	if (_url->is_array()) {
		_url_list = _url;
	}
	else {
		_url_list = { zpt::array, _url };
	}
	zpt::json _return = zpt::json::object();
	for (auto _u : _url_list->arr()) {
		zpt::json _splited = zpt::split(std::string(_u), "/");

		short _i = 0;
		for (auto _part : _splited->arr()) {
			if (_part->str().find("{") != std::string::npos) {
				_return << _part->str() << (std::string("_tv_") + _part->str().substr(1, _part->str().length() - 2));
			}
			_i++;
		}
	}
	return _return;
}

auto zpt::gen::url_pattern_to_params_lisp(zpt::json _url) -> zpt::json {
	zpt::json _url_list;
	if (_url->is_array()) {
		_url_list = _url;
	}
	else {
		_url_list = { zpt::array, _url };
	}
	zpt::json _return = zpt::json::object();
	for (auto _u : _url_list->arr()) {
		zpt::json _splited = zpt::split(std::string(_u), "/");

		short _i = 0;
		for (auto _part : _splited->arr()) {
			if (_part->str().find("{") != std::string::npos) {
				_return << _part->str() << (std::string("tv-") + zpt::r_replace(_part->str().substr(1, _part->str().length() - 2), "_", "-"));
			}
			_i++;
		}
	}
	return _return;
}

auto zpt::gen::url_pattern_to_params_python(zpt::json _url) -> zpt::json {
	zpt::json _url_list;
	if (_url->is_array()) {
		_url_list = _url;
	}
	else {
		_url_list = { zpt::array, _url };
	}
	zpt::json _return = zpt::json::object();
	for (auto _u : _url_list->arr()) {
		zpt::json _splited = zpt::split(std::string(_u), "/");

		short _i = 0;
		for (auto _part : _splited->arr()) {
			if (_part->str().find("{") != std::string::npos) {
				_return << _part->str() << (std::string("tv_") + zpt::r_replace(_part->str().substr(1, _part->str().length() - 2), "-", "_"));
			}
			_i++;
		}
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

auto zpt::gen::get_fields_array(zpt::json _fields) -> std::string {
	if (!_fields->is_object()) {
		return "";
	}
	std::string _return("{ zpt::array, \"id\", \"href\", \"created\", \"updated\"");
	for (auto _field : _fields->obj()) {
		if (_field.second["type"] == zpt::json::string("object") || _field.second["type"] == zpt::json::string("array")) {
			continue;
		}
		_return += std::string(", \"") + _field.first + std::string("\"");;
	}
	_return += std::string(" }");
	return _return;
}


extern "C" auto zpt_gen() -> int {
	return 1;
}
