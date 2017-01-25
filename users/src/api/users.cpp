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
#include <zapata/mongodb.h>
#include <zapata/rest.h>
#include <zapata/users/codes_users.h>
#include <ctime>
#include <memory>
#include <ossp/uuid++.hh>

zpt::UsersPtr::UsersPtr(zpt::ev::emitter _emitter) : std::shared_ptr< zpt::Users >(new zpt::Users(_emitter)) {
}

zpt::UsersPtr::~UsersPtr() {
}

zpt::Users::Users(zpt::ev::emitter _emitter) : __emitter(_emitter) {
}

zpt::Users::~Users() {
}

std::string zpt::Users::name() {
	return "users.broker";
}

std::tuple< std::string, std::string > zpt::Users::salt_hash(std::string _password) {
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	std::string _salt = _uuid.string();
	std::string _salted = zpt::hash::SHA512(_password + _salt);
	zpt::base64::url_encode(_salted);
	return std::make_tuple(_salt, _salted);
}

zpt::json zpt::Users::validate(std::string _username, std::string _password) {
	zpt::mongodb::Client* _db = (zpt::mongodb::Client*) this->__emitter->get_kb("mongodb.users").get();
	zpt::json _list = _db->query("users", zpt::json({ "username", _username }));
	assertz(_list->ok() && ((size_t) _list["size"]) != 0, "could not find such username", 404, 0);
	std::string _salted = zpt::hash::SHA512(_password + _list["elements"][0]["salt"]->str());
	zpt::base64::url_encode(_salted);
	zpt::json _return = _list["elements"][0];
	if(_salted == _return["password"]->str()) {
		return _return;
	}
	return zpt::undefined;
}

zpt::json zpt::Users::list(std::string _resource, zpt::json _envelope) {
	_envelope["payload"] >> "password";
	_envelope["payload"] >> "_id";
	_envelope["payload"] >> "id";
					
	zpt::mongodb::Client* _db = (zpt::mongodb::Client*) this->__emitter->get_kb("mongodb.users").get();
	zpt::json _list = _db->query("users", zpt::get("payload", _envelope));
	if (!_list->ok() || ((size_t) _list["size"]) == 0) {
		return zpt::set("status", 204);
	}
	return zpt::set("status", 200, zpt::set("payload", _list));
}

zpt::json zpt::Users::get(std::string _resource, zpt::json _envelope) {
	_envelope["payload"] >> "password";
	_envelope["payload"] >> "_id";
	_envelope["payload"] >> "id";
	_envelope["payload"] << "_id" << _resource;
					
	zpt::mongodb::Client* _db = (zpt::mongodb::Client*) this->__emitter->get_kb("mongodb.users").get();
	zpt::json _document = _db->query("users", _envelope["payload"]);
	if (!_document->ok() || ((size_t) _document["size"]) == 0) {
		return { "status", 404 };
	}
	return {
		"status", 200,
		"payload", _document["elements"][0]
	};
}

zpt::json zpt::Users::add(std::string _resource, zpt::json _envelope) {
	assertz(
		_envelope["payload"]->ok() &&
		_envelope["payload"]["name"]->ok() &&
		_envelope["payload"]["role"]->ok() &&
		_envelope["payload"]["e-mail"]->ok() &&
		_envelope["payload"]["password"]->ok(),
		"required fields: 'name', 'role', 'e-mail' and 'password'", 412, 0);
	
	_envelope["payload"] >> "_id";
	_envelope["payload"] >> "id";

	if (!_envelope["payload"]["username"]->ok()) {
		_envelope["payload"] << "username" << _envelope["payload"]["e-mail"];
	}

	std::tuple< std::string, std::string> _secret = this->salt_hash(_envelope["payload"]["password"]->str());
	_envelope["payload"] << "salt" << std::get<0>(_secret);
	_envelope["payload"] << "password" << std::get<1>(_secret);

	zpt::mongodb::Client* _db = (zpt::mongodb::Client*) this->__emitter->get_kb("mongodb.users").get();
	std::string _id = _db->insert("users", _resource, _envelope["payload"]);
	return {
		"status", 200,
		"payload", {
			"id", _id,
			"href", (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
		}
	};
}

zpt::json zpt::Users::replace(std::string _resource, zpt::json _envelope) {
	assertz(
		_envelope["payload"]->ok() &&
		_envelope["payload"]["name"]->ok() &&
		_envelope["payload"]["scope"]->ok() &&
		_envelope["payload"]["e-mail"]->ok() &&
		_envelope["payload"]["password"]->ok(),
		"required fields: 'name', 'scope', 'e-mail' and 'password'", 412, 0);

	_envelope["payload"] >> "_id";
	_envelope["payload"] >> "id";
	
	std::tuple< std::string, std::string> _secret = this->salt_hash(_envelope["payload"]["password"]->str());
	_envelope["payload"] << "salt" << std::get<0>(_secret);
	_envelope["payload"] << "password" << std::get<1>(_secret);

	zpt::mongodb::Client* _db = (zpt::mongodb::Client*) this->__emitter->get_kb("mongodb.users").get();
	size_t _size = _db->save("users", { "_id", _resource }, _envelope["payload"]);
	return {
		"status", 200,
		"payload", {
			"updated", _size
		}
	};
}

zpt::json zpt::Users::patch(std::string _resource, zpt::json _envelope) {
	_envelope["payload"] >> "_id";
	_envelope["payload"] >> "id";

	if (_envelope["payload"]["password"]->ok()) {
		std::tuple< std::string, std::string> _secret = this->salt_hash(_envelope["payload"]["password"]->str());
		_envelope["payload"] << "salt" << std::get<0>(_secret);
		_envelope["payload"] << "password" << std::get<1>(_secret);
	}
	
	zpt::mongodb::Client* _db = (zpt::mongodb::Client*) this->__emitter->get_kb("mongodb.users").get();
	size_t _size = _db->set("users", { "_id", _resource }, _envelope["payload"]);
	return {
		"status", 200,
		"payload", {
			"updated", _size
		}
	};
}

zpt::json zpt::Users::remove(std::string _resource, zpt::json _envelope) {
	zpt::mongodb::Client* _db = (zpt::mongodb::Client*) this->__emitter->get_kb("mongodb.users").get();
	size_t _size = _db->remove("users", { "_id", _resource });
	return {
		"status", 200,
		"payload", {
			"removed", _size
		}
	};
}

extern "C" void restify(zpt::ev::emitter _emitter) {
	assertz(_emitter->options()["mongodb"]["users"]->ok(), "no 'mongodb.users' object found in provided configuration", 500, 0);
	_emitter->add_kb("mongodb.users", zpt::kb(new zpt::mongodb::Client(_emitter->options(), "mongodb.users")));
	_emitter->add_kb("users.broker", zpt::kb(new zpt::users::broker(_emitter)));

	/***	  
	 * # _**Users**_ collection
	 *
	 * ```
	 * /{api-version}/users
	 * ```
	 *
	 * ## Description
	 *
	 * The _**Users**_ collection holds the set of _User_ documents for the configured **MongoDB** database and collection. 
	 *
	 * ## Allowed methods
	 *
	 * - _GET_
	 * - _POST_
	 * - _HEAD_
	 *
	 ***/ 
	_emitter->on(zpt::rest::url_pattern({ _emitter->version(), "users" }),
		{
			/***
			 * ## _**GET**_ /{api-version}/users
			 *
			 * 
			 ***/
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "users", "ar"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);

					return ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->list(_resource, _envelope);
				}
			},
			{
				zpt::ev::Post,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					if (std::string(_envelope["payload"]["role"]) == "administrator") {
						zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
						zpt::json _list = _db->query("users", zpt::json({ "role", "administrator" }));
						assertz(!_list->ok() || ((size_t) _list["size"]) == 0, "you can't add another administrator role to this system", 412, 0);
					}
					else {
						zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
						assertz(
							((int) _auth_data["status"]) == 200,
							"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
						);
						assertz(
							zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "users", "aw"),
							"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
						);
					}
					return ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->add(_resource, _envelope);
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } } );
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "users", "ar"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::json _list = ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->list(_resource, _envelope);
					if (((int) _list["status"]) == 200) {
						return {
							"status", 200,
							"headers", {
								"Content-Length", ((std::string) _list["payload"]).length()
							}
						};
					}
					return _list;
				}
			}
		}
	);

	_emitter->on(zpt::rest::url_pattern({ _emitter->version(), "users", "(.+)" }),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					if (_resource == (std::string("/") + _emitter->version() + std::string("/users/me"))) {
						assertz((_envelope["payload"]["username"]->ok() && _envelope["payload"]["password"]->ok()) || _envelope["headers"]["Cookie"]->ok() || _envelope["headers"]["Authorization"]->ok(), "access to this endpoint must be authenticated", 401, 0);
						if (_envelope["payload"]["username"]->ok() && _envelope["payload"]["password"]->ok()) {
							zpt::json _document = ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->validate(_envelope["payload"]["username"]->str(), _envelope["payload"]["password"]->str());
							assertz(_document->ok(), "access to this endpoint must be authenticated", 401, 0);
							return {
								"status", 200,
								"payload", _document
							};
						}
						else if (_envelope["headers"]["Authorization"]->ok()) {
							zpt::json _validation = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", _envelope["headers"]["Authentication"] } }); 
							assertz(_validation->ok(), "Bad authorization", 412, 0);
							_envelope["payload"] << "_id" << _validation["owner"]["_id"]->str();
							zpt::json _document = _db->query("users", _envelope["payload"]);
							assertz(_document->ok() && ((int) _document["size"]) != 0, "access to this endpoint must be authenticated", 401, 0);
							return {
								"status", 200,
								"payload", _document["elements"][0]
							};
						}
						else if (_envelope["headers"]["Cookie"]->ok()) {
							zpt::json _cookie = zpt::rest::cookies::deserialize(_envelope["headers"]["Cookie"]->str());
							zpt::json _validation = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", _cookie["value"] } });
							assertz(_validation->ok() && _validation["status"] == 200, "Bad cookie", 412, 0);
							zpt::json _document = _db->query("users", { "_id", _validation["payload"]["owner"]["_id"]->str() });
							assertz(_document->ok() && ((int) _document["size"]) != 0, "access to this endpoint must be authenticated", 401, 0);
							return {
								"status", 200,
								"payload", _document["elements"][0]
							};
						}
					}
					else {
						zpt::json _auth_data = _emitter->route(zpt::ev::Post, zpt::path::join({ _emitter->version(), "oauth2.0", "validate" }), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
						assertz(
							((int) _auth_data["status"]) == 200,
							"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
						);
						assertz(
							zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "users", "ar"),
							"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
						);
						return ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->get(_resource, _envelope);
					}
					return zpt::undefined;
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
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "users", "aw"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					return ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->replace(_resource, _envelope);
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
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "users", "aw"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					return ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->remove(_resource, _envelope);
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
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "users", "ar"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::json _document = ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->get(_resource, _envelope);
					if (((int) _document["status"]) == 200) {
						return {
							"status", 200,
							"headers", {
								"Content-Length", ((std::string) _document["payload"]).length()
							}
						};
					}
					return _document;
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
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "users", "aw"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					return ((zpt::users::broker*) _emitter->get_kb("users.broker").get())->patch(_resource, _envelope);
				}
			}
		}
	);

	_emitter->on(zpt::rest::url_pattern({ _emitter->version(), "roles" }),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::json _list = _db->query("roles", _envelope["payload"]);
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
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["id"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["scope"]->ok(),
						"required fields: 'name', 'id' and 'scope'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("roles", _resource, _envelope["payload"]);
					return {
						"status", 200,
						"payload", {
							"id", _id,
							"href", (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
						}
					};
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::json _list = _db->query("roles", _envelope["payload"]);
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

	_emitter->on(zpt::rest::url_pattern({ _emitter->version(), "roles", "(.+)" }),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope["payload"] << "_id" << _resource;
					zpt::json _document = _db->query("roles", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return { "status", 404 };
					}
					return {
						"status", 200,
						"payload", _document["elements"][0]
					};
				}
			},
			{
				zpt::ev::Put,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["id"]->ok() &&
						_envelope["payload"]["scope"]->ok(),
						"required fields: 'name', 'id' and 'scope'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->save("roles", { "_id", _resource }, _envelope["payload"]);
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
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->remove("roles", { "_id", _resource });
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
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope["payload"] << "_id" << _resource;
					zpt::json _document = _db->query("roles", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return { "status", 404 };
					}
					return {
						"status", 200,
						"headers", {
							"Content-Length", ((std::string) _document["elements"][0]).length()
						}
					};
				}
			},
			{
				zpt::ev::Patch,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->set("roles", { "_id", _resource }, _envelope["payload"]);
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

