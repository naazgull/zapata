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

#include <zapata/postgresql/convert_sql.h>
#include <zapata/events.h>
#include <ossp/uuid++.hh>
#include <mutex>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	namespace pgsql {

		class Client : public zpt::Connector {
		public:
			Client(zpt::json _options, std::string _conf_path);
			virtual ~Client();

			virtual auto conn() -> pqxx::connection&;

			virtual auto name() -> std::string;
			virtual auto options() -> zpt::json;
			virtual auto events(zpt::ev::emitter _emitter) -> void;
			virtual auto events() -> zpt::ev::emitter;
			virtual auto mutations(zpt::mutation::emitter _emitter) -> void;
			virtual auto mutations() -> zpt::mutation::emitter;

			virtual auto connect(zpt::json _opts) -> void;
			virtual auto reconnect() -> void;

			virtual auto insert(std::string _collection, std::string _href_prefix, zpt::json _record, zpt::json _opts = zpt::undefined) -> std::string;
			virtual auto save(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
			virtual auto set(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
			virtual auto set(std::string _collection, zpt::json _query, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
			virtual auto unset(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
			virtual auto unset(std::string _collection, zpt::json _query, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
			virtual auto remove(std::string _collection, std::string _href, zpt::json _opts = zpt::undefined) -> int;
			virtual auto remove(std::string _collection, zpt::json _query, zpt::json _opts = zpt::undefined) -> int;
			virtual auto get(std::string _collection, std::string _href, zpt::json _opts = zpt::undefined) -> zpt::json;
			virtual auto query(std::string _collection, std::string _query, zpt::json _opts = zpt::undefined) -> zpt::json;
			virtual auto query(std::string _collection, zpt::json _query, zpt::json _opts = zpt::undefined) -> zpt::json;
			virtual auto all(std::string _collection, zpt::json _opts = zpt::undefined) -> zpt::json;

		private:
			zpt::json __options;
			std::mutex __mtx;
			std::unique_ptr<pqxx::connection> __conn;
			std::string _conn_str;
			zpt::ev::emitter __events;
		};

		class ClientPtr : public std::shared_ptr<zpt::pgsql::Client> {
		public:
			/**
			 * @brief Creates an std::shared_ptr to an Self instance.
			 * 
			 * @param _options the configuration object retrieved from the configuration JSON file
			 */
			 ClientPtr(zpt::pgsql::Client * _target);
			 ClientPtr(zpt::json _options, std::string _conf_path);

			/**
			 * @brief Destroys the current Self instance, freeing all allocated memory.
			 */
			 virtual ~ClientPtr();
		};

		typedef zpt::pgsql::ClientPtr client;
	}
	
}
