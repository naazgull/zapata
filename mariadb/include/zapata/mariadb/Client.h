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

#include <zapata/mariadb/convert_mongo.h>
#include <zapata/addons.h>
#include <ossp/uuid++.hh>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	namespace mariadb {

		class Client : public zpt::Connector {
		public:
			Client(zpt::json _options, std::string _conf_path);
			virtual ~Client();

			virtual zpt::json options();
			virtual std::string name();
			virtual bool& broadcast();
			virtual zpt::ev::emitter addons();

			virtual std::string insert(std::string _collection, std::string _id_prefix, zpt::json _record);
			virtual int save(std::string _collection, zpt::json _pattern, zpt::json _record);
			virtual int set(std::string _collection, zpt::json _pattern, zpt::json _document);
			virtual int unset(std::string _collection, zpt::json _pattern, zpt::json _document);
			virtual int remove(std::string _collection, zpt::json _pattern);
			virtual zpt::json query(std::string _collection, zpt::json _pattern);

		private:
			zpt::json __options;
			zpt::json __mariadb_conf;
			std::string __conf_path;
			mongo::ScopedDbConnection __conn;
			bool __broadcast;
			zpt::ev::emitter __addons;
		};

		class ClientPtr : public std::shared_ptr<zpt::mariadb::Client> {
		public:
			/**
			 * @brief Creates an std::shared_ptr to an Self instance.
			 * 
			 * @param _options the configuration object retrieved from the configuration JSON file
			 */
			 ClientPtr(zpt::mariadb::Client * _target);
			 ClientPtr(zpt::json _options, std::string _conf_path);

			/**
			 * @brief Destroys the current Self instance, freeing all allocated memory.
			 */
			 virtual ~ClientPtr();
		};

		typedef zpt::mariadb::ClientPtr client;
	}
	
}
