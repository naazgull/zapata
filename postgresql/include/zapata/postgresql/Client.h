/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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

#define psql_catch_block(s)                                                                                            \
	catch (pqxx::feature_not_supported & _e) {                                                                     \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 400, s + 2);                                      \
	}                                                                                                              \
	catch (pqxx::insufficient_privilege & _e) {                                                                    \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 403, s + 3);                                      \
	}                                                                                                              \
	catch (pqxx::disk_full & _e) {                                                                                 \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 500, s + 4);                                      \
	}                                                                                                              \
	catch (pqxx::out_of_memory & _e) {                                                                             \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 500, s + 5);                                      \
	}                                                                                                              \
	catch (pqxx::insufficient_resources & _e) {                                                                    \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 500, s + 6);                                      \
	}                                                                                                              \
	catch (pqxx::check_violation & _e) {                                                                           \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 7);                                      \
	}                                                                                                              \
	catch (pqxx::foreign_key_violation & _e) {                                                                     \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 8);                                      \
	}                                                                                                              \
	catch (pqxx::not_null_violation & _e) {                                                                        \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 9);                                      \
	}                                                                                                              \
	catch (pqxx::restrict_violation & _e) {                                                                        \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 10);                                     \
	}                                                                                                              \
	catch (pqxx::unique_violation & _e) {                                                                          \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 11);                                     \
	}                                                                                                              \
	catch (pqxx::integrity_constraint_violation & _e) {                                                            \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 12);                                     \
	}                                                                                                              \
	catch (pqxx::invalid_cursor_name & _e) {                                                                       \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 404, s + 13);                                     \
	}                                                                                                              \
	catch (pqxx::invalid_cursor_state & _e) {                                                                      \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 14);                                     \
	}                                                                                                              \
	catch (pqxx::invalid_sql_statement_name & _e) {                                                                \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 404, s + 15);                                     \
	}                                                                                                              \
	catch (pqxx::undefined_column & _e) {                                                                          \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 404, s + 16);                                     \
	}                                                                                                              \
	catch (pqxx::undefined_function & _e) {                                                                        \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 404, s + 17);                                     \
	}                                                                                                              \
	catch (pqxx::undefined_table & _e) {                                                                           \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 404, s + 18);                                     \
	}                                                                                                              \
	catch (pqxx::syntax_error & _e) {                                                                              \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 19);                                     \
	}                                                                                                              \
	catch (pqxx::sql_error & _e) {                                                                                 \
		zlog(std::string("pgsql: error in '") + _e.query() + std::string("': ") + _e.what(), zpt::trace);      \
		assertz(false, zpt::r_replace(_e.what(), "\n", " "), 412, s + 1);                                      \
	}

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

	virtual auto connect() -> void;
	virtual auto reconnect() -> void;

	virtual auto
	insert(std::string _collection, std::string _href_prefix, zpt::json _record, zpt::json _opts = zpt::undefined)
	    -> std::string;
	virtual auto
	upsert(std::string _collection, std::string _href_prefix, zpt::json _record, zpt::json _opts = zpt::undefined)
	    -> std::string;
	virtual auto
	save(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
	virtual auto
	set(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
	virtual auto set(std::string _collection, zpt::json _query, zpt::json _record, zpt::json _opts = zpt::undefined)
	    -> int;
	virtual auto
	unset(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
	virtual auto
	unset(std::string _collection, zpt::json _query, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
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
	ClientPtr(zpt::pgsql::Client* _target);
	ClientPtr(zpt::json _options, std::string _conf_path);

	/**
	 * @brief Destroys the current Self instance, freeing all allocated memory.
	 */
	virtual ~ClientPtr();
};

typedef zpt::pgsql::ClientPtr client;
}
}
