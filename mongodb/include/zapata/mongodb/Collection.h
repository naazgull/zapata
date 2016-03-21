/*

   ██████████╗
  ████████████╗    ███╗   ███╗██╗   ██╗████████╗████████╗██╗      ██████╗██╗   ██╗
 ████  █  █  ██╗   ████╗ ████║██║   ██║     ██╔╝     ██╔╝██║     ██╔════╝██║   ██║
████  █  █  ████╗  ██╔████╔██║██║   ██║   ██╔═╝    ██╔═╝ ██║     ████╗   ╚██████╔╝
 ██  █  █  ████╔╝  ██║╚██╔╝██║██║   ██║ ██╔═╝    ██╔═╝   ██║     ██╔═╝    ╚═██╔═╝
  ████████████╔╝   ██║ ╚═╝ ██║╚██████╔╝████████╗████████╗╚██████╗╚██████╗   ██║
   ██████████╔╝    ╚═╝     ╚═╝ ╚═════╝ ╚═══════╝╚═══════╝ ╚═════╝ ╚═════╝   ╚═╝
    ╚════════╝

Copyright (c) 2014, Muzzley

Permission to use, copy, modify, and/or distribute this software for 
any purpose with or without fee is hereby granted, provided that the 
above copyright notice and this permission notice appear in all 
copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL 
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE 
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR 
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER 
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
PERFORMANCE OF THIS SOFTWARE.*/
#pragma once

#include <zapata/mongodb/convert_mongo.h>
#include <ossp/uuid++.hh>

 using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zapata {

	namespace mongodb {

		class Collection {
		public:
			explicit Collection(zapata::JSONObj& _options);
			virtual ~Collection();

			virtual zapata::JSONObj& options();

			virtual std::string insert(std::string _collection, std::string _id_prefix, zapata::JSONPtr _record);
			virtual int update(std::string _collection, zapata::JSONPtr _pattern, zapata::JSONPtr _record);
			virtual int unset(std::string _collection, zapata::JSONPtr _pattern, zapata::JSONPtr _document);
			virtual int remove(std::string _collection, zapata::JSONPtr _pattern);
			virtual zapata::JSONPtr query(std::string _collection, zapata::JSONPtr _pattern);

		private:
			zapata::JSONObj __options;
			mongo::ScopedDbConnection __conn;
		};

		class CollectionPtr : public std::shared_ptr<zapata::mongodb::Collection> {
		public:
			/**
			 * @brief Creates an std::shared_ptr to an Self instance.
			 * 
			 * @param _options the configuration object retrieved from the configuration JSON file
			 */
			 explicit CollectionPtr(zapata::JSONObj& _options);

			/**
			 * @brief Destroys the current Self instance, freeing all allocated memory.
			 */
			 virtual ~CollectionPtr();
		};
	}
}
