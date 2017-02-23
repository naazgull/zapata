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

#pragma once

#include <zapata/json.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class Generator;
	class GeneratorPtr;
	class GenDatum;
	class GenResource;

	typedef std::shared_ptr<zpt::GenDatum> GenDatumPtr;
	typedef std::shared_ptr<zpt::GenResource> GenResourcePtr;

	namespace gen {
		typedef zpt::GeneratorPtr worker;
		typedef zpt::GenDatumPtr datum;
		typedef zpt::GenResourcePtr resource;

		auto url_pattern_to_regexp(zpt::json _url_pattern) -> std::string;
		auto url_pattern_to_vars(std::string _url_pattern) -> std::string;
		auto url_pattern_to_var_decl(zpt::json _url) -> std::string;
		auto url_pattern_to_vars_lisp(std::string _url) -> std::string;
		auto url_pattern_to_var_decl_lisp(zpt::json _url) -> std::string;
		auto url_pattern_to_params(zpt::json _url) -> zpt::json;
		auto url_pattern_to_params_lisp(zpt::json _url) -> zpt::json;

		auto get_opts(zpt::json _field) -> zpt::json;
	}

	namespace conf {
		namespace gen {
			auto init(int argc, char* argv[]) -> zpt::json;
		}
	}

	class GeneratorPtr : public std::shared_ptr<zpt::Generator> {
	public:
		GeneratorPtr(zpt::json _options);
		GeneratorPtr(zpt::Generator * _ptr);
		virtual ~GeneratorPtr();

		static auto launch(int argc, char* argv[]) -> int;
	};

	class Generator {
	public:
		static std::map< std::string, zpt::gen::datum > datums;
		static std::map< std::string, zpt::gen::resource > resources;
		static std::string datum_includes;
		static std::map< std::string, std::string > alias;

		Generator(zpt::json _options);
		virtual ~Generator();

		virtual auto options() -> zpt::json;
		virtual auto load() -> void;
		virtual auto build() -> void;
		virtual auto build_data_layer() -> void;
		virtual auto build_container() -> void;
		virtual auto build_mutations() -> void;

	private:
		zpt::json __options;
		zpt::json __specs;
	};
	
	class GenDatum {
	public:
		GenDatum(zpt::json _spec, zpt::json _options);
		virtual ~GenDatum();

		virtual auto spec() -> zpt::json;
		virtual auto build() -> std::string;
		virtual auto build_data_layer() -> std::string;
		virtual auto build_params(zpt::json _rel, bool _multi) -> std::string;
		virtual auto build_inverted_params(zpt::json _rel) -> std::string;
		virtual auto build_topic(zpt::json _topic) -> std::string;
		virtual auto build_dbms() -> std::string;
		virtual auto build_get(zpt::json _resource) -> std::string;
		virtual auto build_post(zpt::json _resource) -> std::string;
		virtual auto build_put(zpt::json _resource) -> std::string;
		virtual auto build_patch(zpt::json _resource) -> std::string;
		virtual auto build_delete(zpt::json _resource) -> std::string;
		virtual auto build_head(zpt::json _resource) -> std::string;
		virtual auto build_mutations(std::string _parent_name, std::string _child_includes) -> std::string;
		virtual auto build_insert() -> std::string;
		virtual auto build_update() -> std::string;
		virtual auto build_remove() -> std::string;
		virtual auto build_replace() -> std::string;
		virtual auto build_validation() -> std::string;
		virtual auto build_associations_insert(std::string _name, zpt::json _field) -> std::string;
		virtual auto build_associations_update(std::string _name, zpt::json _field) -> std::string;
		virtual auto build_associations_remove(std::string _name, zpt::json _field) -> std::string;
		virtual auto build_associations_replace(std::string _name, zpt::json _field) -> std::string;
		virtual auto build_associations_get() -> std::string;
		virtual auto build_associations_query() -> std::string;
		virtual auto build_associations_insert() -> std::string;
		virtual auto build_associations_save() -> std::string;
		virtual auto build_associations_set() -> std::string;
		virtual auto build_associations_remove() -> std::string;
		virtual auto build_extends_get() -> std::string;
		virtual auto build_extends_query() -> std::string;
		virtual auto build_extends_insert() -> std::string;
		virtual auto build_extends_save() -> std::string;
		virtual auto build_extends_set_topic() -> std::string;
		virtual auto build_extends_set_pattern() -> std::string;
		virtual auto build_extends_remove_topic() -> std::string;
		virtual auto build_extends_remove_pattern() -> std::string;

		static auto build_initialization(std::string _dbms, std::string _namespace = "") -> std::string;
		static auto build_data_client(zpt::json _dbms, zpt::json _ordered, std::string _namespace) -> std::string;
		static auto get_type(zpt::json _field) -> std::string;
		static auto get_restrictions(zpt::json _field) -> std::string;

	private:
		zpt::json __spec;
		zpt::json __options;

	};

	class GenResource {
	public:
		GenResource(zpt::json _spec, zpt::json _options);
		virtual ~GenResource();

		virtual auto spec() -> zpt::json;
		virtual auto build() -> std::string;
		virtual auto build_data_layer() -> std::string;
		virtual auto build_handlers(std::string _parent_name, std::string _child_includes) -> std::string;
		virtual auto build_mutations() -> std::string;

		virtual auto build_validation(zpt::ev::performative _performative) -> std::string;
		virtual auto build_handler_header(zpt::ev::performative _per = zpt::ev::Get) -> std::string;
		virtual auto build_get() -> std::string;
		virtual auto build_post() -> std::string;
		virtual auto build_put() -> std::string;
		virtual auto build_patch() -> std::string;
		virtual auto build_delete() -> std::string;
		virtual auto build_head() -> std::string;
		virtual auto build_reply() -> std::string;

		virtual auto build_lisp_validation(zpt::ev::performative _performative) -> std::string;
		virtual auto build_lisp_handler_header(zpt::ev::performative _per = zpt::ev::Get) -> std::string;
		virtual auto build_lisp_get() -> std::string;
		virtual auto build_lisp_post() -> std::string;
		virtual auto build_lisp_put() -> std::string;
		virtual auto build_lisp_patch() -> std::string;
		virtual auto build_lisp_delete() -> std::string;
		virtual auto build_lisp_head() -> std::string;
		virtual auto build_lisp_reply() -> std::string;
		
		virtual auto build_python_validation(zpt::ev::performative _performative) -> std::string;
		virtual auto build_python_handler_header(zpt::ev::performative _per = zpt::ev::Get) -> std::string;
		virtual auto build_python_get() -> std::string;
		virtual auto build_python_post() -> std::string;
		virtual auto build_python_put() -> std::string;
		virtual auto build_python_patch() -> std::string;
		virtual auto build_python_delete() -> std::string;
		virtual auto build_python_head() -> std::string;
		virtual auto build_python_reply() -> std::string;
		
	private:
		zpt::json __spec;
		zpt::json __options;

	};
}
