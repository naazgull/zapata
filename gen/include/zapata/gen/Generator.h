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

		auto url_pattern_to_regexp(std::string _url_pattern) -> std::string;
		auto url_pattern_to_vars(std::string _url_pattern) -> std::string;
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
		static std::map<std::string, zpt::gen::datum> datums;
		static std::map<std::string, zpt::gen::resource> resources;

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
		GenDatum(zpt::json _spec);
		virtual ~GenDatum();

		virtual auto spec() -> zpt::json;
		virtual auto build() -> std::string;
		virtual auto build_data_layer() -> std::string;
		virtual auto build_get(zpt::json _resource) -> std::string;
		virtual auto build_post(zpt::json _resource) -> std::string;
		virtual auto build_put(zpt::json _resource) -> std::string;
		virtual auto build_patch(zpt::json _resource) -> std::string;
		virtual auto build_delete(zpt::json _resource) -> std::string;
		virtual auto build_head(zpt::json _resource) -> std::string;
		virtual auto build_mutations() -> std::string;

		static auto build_initialization(std::string _dbms, std::string _namespace = "") -> std::string;
	private:
		zpt::json __spec;

	};

	class GenResource {
	public:
		GenResource(zpt::json _spec);
		virtual ~GenResource();

		virtual auto spec() -> zpt::json;
		virtual auto build() -> std::string;
		virtual auto build_validation(bool _mandatory = false) -> std::string;
		virtual auto build_handler_header(zpt::ev::performative _per = zpt::ev::Get) -> std::string;
		virtual auto build_data_layer() -> std::string;
		virtual auto build_handlers() -> std::string;
		virtual auto build_get() -> std::string;
		virtual auto build_post() -> std::string;
		virtual auto build_put() -> std::string;
		virtual auto build_patch() -> std::string;
		virtual auto build_delete() -> std::string;
		virtual auto build_head() -> std::string;
		virtual auto build_mutations() -> std::string;

	private:
		zpt::json __spec;

	};
}
