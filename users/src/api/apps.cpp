/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

T he above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <zapata/rest.h>
#include <zapata/redis.h>
#include <ctime>
#include <memory>
#include <ossp/uuid++.hh>
#include <python3.5m/Python.h>

extern "C" void _zpt_load_() {
	Py_Initialize();
	PyRun_SimpleString("name = raw_input('Who are you? ')n"
		"print 'Hi there, %s!' % namen");
	Py_Finalize();
	
	assertz(_emitter->options()["redis"]["apps"]->ok(), "no 'redis.apps' object found in provided configuration", 500, 0);
	_emitter->add_kb("redis.apps", zpt::kb(new zpt::redis::Client(_emitter->options(), "redis.apps")));

	/***
	 * # _**Applications**_ collection
	 *
	 * ```
	 * /{api-version}/apps
	 * ```
	 *
	 * ## Description
	 *
	 * The _**Applications**_ collections holds the set of _Application_ documents for the configured **MongoDB** database and collection. 
	 *  
	 * ## Allowed methods
	 *
	 * - _GET_
	 * - _POST_
	 * - _HEAD_
	 *
	 ***/ 
	_emitter->on(zpt::rest::url_pattern({ _emitter->version(), "apps" }),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "apps", "ar"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					zpt::json _list = _db->query("apps", (!_envelope["payload"]->ok() || _envelope["payload"]->obj()->size() == 0 ? std::string("*") : std::string("*") + ((std::string) _envelope["payload"]->obj()->begin()->second) + std::string("*")));
					if (!_list->ok()) {
						return { "status", 204 };
					}
					return {
						"status", 200,
						"payload", _list
					};
				}
			},
			{
				zpt::ev::Post,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "apps", "aw"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["description"]->ok() &&
						_envelope["payload"]["scope"]->ok() &&
						_envelope["payload"]["redirect_domain"]->ok(),
						"required fields: 'name', 'description' and 'redirect_domain'", 412, 0);
					assertz(
						_envelope["payload"]["redirect_domain"]->type() == zpt::JSArray,
						"invalid field type: 'redirect_domain' must be a list of strings", 400, 0);

					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					std::string _id = _db->insert("apps", _resource, _envelope["payload"]);
					std::string _href = (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id);					
					_db->set("apps", _href, { "client_id", _id, "client_secret", zpt::generate_key() });
					
					return {
						"status", 200,
						"payload", {
							"id", _id,
							"href", _href
						}
					};
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "apps", "ar"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					zpt::json _list = _db->query("apps", _envelope["payload"]);
					if (!_list->ok()) {
						return { "status", 204 };
					}
					return {
						"status", 200,
						"headers", {
							"Content-Length", ((std::string) _list).length()
						}
					};
				}
			}
		}
	);

	_emitter->on(zpt::rest::url_pattern({ _emitter->version(), "apps", "(.+)" }),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					if (_resource != zpt::path::join({ _emitter->version(), "apps", "00000000-0000-0000-0000-000000000000" })) {
						zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
						assertz(
							((int) _auth_data["status"]) == 200,
							"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
						);
						assertz(
							zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "apps", "ar"),
							"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
						);
					}
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					zpt::json _document = _db->get("apps", _resource);
					if (!_document->ok()) {
						return { "status", 404 };
					}
					return {
						"status", 200,
						"payload", _document
					};
				}
			},
			{
				zpt::ev::Put,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "apps", "aw"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["description"]->ok() &&
						_envelope["payload"]["scope"]->ok() &&
						_envelope["payload"]["redirect_domain"]->ok(),
						"required fields: 'name', 'description' and 'redirect_domain'", 412, 0);
				
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					size_t _size = _db->save("apps", _resource, _envelope["payload"]);
					return {
						"status", 200,
						"payload", {
							"updated", _size
						}
					};
				}
			},
			{
				zpt::ev::Delete,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "apps", "aw"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					size_t _size = _db->remove("apps", _resource);
					return {
						"status", 200,
						"payload", {
							"removed", _size
						}
					};
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "apps", "ar"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					zpt::json _document = _db->get("apps", _resource);
					if (!_document->ok()) {
						return { "status", 404 };
					}
					return {
						"status", 200,
						"headers", {
							"Content-Length", ((std::string) _document).length()
						}
					};
				}
			},
			{
				zpt::ev::Patch,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "apps", "aw"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					size_t _size = _db->set("apps", _resource, _envelope["payload"]);
					return {
						"status", 200,
						"payload", {
							"updated", _size
						}
					};
				}
			}
		}
	);
}

