/*
                                         ._wwwmQQWQQQmywa,.                                         
                                       syQQ8TY1vlllll1YT$QQ/                                        
                                      jQQEvvvvvvvvvvvvngmQQ[                                        
                                       $QEvnvnvnnnvnnnUQQQQQ,                                       
                             ._ssaaaaaamQmwgwqoonnnonnnnnnQQ6                                       
                    ._aaaZ$WmQmmwmwmmmdgm#WhmUQ@VWQggpno2odQQL                                      
               .sayWQQW#WBWmBmmmmBBmmmWBBWmWWmQmWS$ETWQmpoS$QQ,                                     
            _awW#mmmmmmmmmmmmBmWmmmBmmmmmmmmmmmBWWWmZ4WWQmS2QWm                                     
          .jm##mmmmmmmmmmmmmmmm##mmmmmmmmmmmmmmm##WWQWwmQQZXdQQL                                    
      _aw#m#######################################ZWQQwVQQmXZ#QQ/                                   
    .wW#Z#Z##Z#Z#Z#Z#UU#Z##Z#UZ#Z#Z#Z#Z#Z#Z#Z#Z#ZUZ#QQ@VQQQmZZQQm                                   
   _QWZZ#ZZZZZZ#ZZZ#ZZZZZZZZZZUZZZUZZZZZZZZZZZZZZZUZQWQ@oQQQm##WQc                                  
  <QWZZZZXZZZZZZXZZZXZZZZZZZZZZXZZZXZZZZZZZZZZZZZZZXZQWQQoQQQQm#QQ,                                 
 _QQXXXXXXZXXZXXXZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX$QQQmdQQQQWQ6                                 
 mQ#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXSdWWQmm#WQQQQc                           awaa,
)QQXSSSSSSS2X2SSS2SS2SS2SS2SS2SS2SS2SS2S2S2S2S2S2S2S22S22dWQQQg#$WQQQwaa.                  ._amQWWQQ
jQBo2222222222So2222222222222222222222222222222222222222SooXUQQQQyBQWQWWQQQwaaaaaaaaaaawmQWHBmQQQQQE
mQ#o222o2o2222o222o222o222o22o22o2o2o2o2o2o2o2o2o2o2o2oooo2onoXUWQQWmWQQQQQQBHUBUUUUUUXSS2SSqQQQQQW'
QQEooooooooooooooooooooooooooooooooooooooooooooooooooooooooooonnnXVWQQQQQQQQQQmmqXXXXSXXmmmQWQQQQQ^ 
WQmnnnonononnnnnnnnnnnnnnnnnnnnnogmpnnnnnnnnnnnnnnnnnnnnnnnnnnnnnvnvXVQWQQQQQQQWQQQQQQQQWWWWQQQQ@'  
3QQvnnvnnvnnvnnvnnvnnvnnvnnvnnvndQQQpvnvnnvnnvnnvnvnvnvvnvnvvnvvvnvvvvvXYVVHUHHBHHVHHVVVHWQQQQQY    
)WQpvvvvvvvvvvvvvvvvvvvvvvvvvvvvmQQQWguvvvvvvnnuwyggggmQQWQQmmgwvvvvvvvIvvvIIvIvIIvIIvqmQQQQQY`     
 4QQplvvvvvvvvvvvvvvvvvvvvvvvvomQQQQQQQQQQQQQQQQQQQQQQQQWWWWQWQQQmgggyyyyyyyyyyyymmQQQQQQQD!        
  $QWpiIIlIlIlIlIlIowwywymmQWQQQQQQ$QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQWQQQQQWQWQQWQQWQQQ@T??`          
   4QQgzlllIllllqmQQQQQWQQQQQQQQQQQ-QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ/^"""""""^~-                   
    "$QQmuIllIlqWWQQQQQQQQQQQQQQQQE )WQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQWm,                             
      "9QQQmyuumQQQQQQQQQQQQQQQQQQ'  "$QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQm                             
         "?HWQQQQQQQQQQQQQQQQQQWD'     ??WWWWWWWWWWWBBUHHVTTTV$QWQQQQQQL                            
              -QQWP???????!!""~                                 -9WQQQQQ,                           
               QW[                                                -$QQQQL                           
               $W.                                                 -QWQQQ/                          
                                                                    -^^^"~                          

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

/***
  # Users API
***/ 
extern "C" void restify(zpt::ev::emitter _emitter) {
	assertz(_emitter->options()["mongodb"]["users"]->ok(), "no 'mongodb.users' object found in provided configuration", 500, 0);
	_emitter->add_kb("mongodb.users", zpt::kb(new zpt::mongodb::Client(_emitter->options(), "mongodb.users")));

	/***	  
	  ## Users collection
	  ```
	  /{api-version}/users
	  ```
	  ### Description
	  The _Users_ collections holds the set of _User_ documents for the configured **MongoDB** database and collection. 
	  
	  ### Allowed methods
	  - _GET_
	  - _POST_
	  - _HEAD_
	***/ 
	_emitter->on(zpt::rest::url_pattern({ _emitter->version(), "users" }),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, std::string("/") + _emitter->version() + std::string("/oauth2.0/validate"), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "services", "ar"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::json _list = _db->query("users", zpt::get("payload", _envelope));
					if (!_list->ok()) {
						return zpt::set("status", 204);
					}
					return zpt::set("status", 200, zpt::set("payload", _list));
				}
			},
			{
				zpt::ev::Post,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, std::string("/") + _emitter->version() + std::string("/oauth2.0/validate"), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } });
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "services", "aw"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["e-mail"]->ok() &&
						_envelope["payload"]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
					
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("users", _resource, _envelope["payload"]);
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"id", _id,
								"href", (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::json _auth_data = _emitter->route(zpt::ev::Post, std::string("/") + _emitter->version() + std::string("/oauth2.0/validate"), { "payload", { "access_token", zpt::rest::authorization::extract(_envelope) } } );
					assertz(
						((int) _auth_data["status"]) == 200,
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 401, 0
					);
					assertz(
						zpt::rest::scopes::has_permission(_auth_data["payload"]["scope"], "services", "a"),
						"required authorization: access to this endpoint must be authorized, providing a valid access token", 403, 0
					);
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::json _list = _db->query("users", _envelope["payload"]);
					if (!_list->ok()) {
						return zpt::json(
							{
								"status", 204
							}
						);
					}
					return zpt::json(
						{
							"status", 200,
							"headers", {
								"Content-Length", ((std::string) _list).length()
							}
						}
					);
				}
			}
		}
	);

	_emitter->on(std::string("^/") + _emitter->version() + std::string("/users/(.+)$"),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					if (_resource == (std::string("/") + _emitter->version() + std::string("/users/me"))) {
						assertz((_envelope["payload"]["username"]->ok() && _envelope["payload"]["password"]->ok()) || _envelope["headers"]["Cookie"]->ok() || _envelope["headers"]["Authorization"]->ok(), "access to this endpoint must be authenticated", 401, 0);
						if (_envelope["headers"]["Authorization"]->ok()) {
							zpt::json _validation = _emitter->route(zpt::ev::Post, std::string("/") + _emitter->version() + std::string("/oauth2.0/validate"),
								zpt::json(
									{
										"payload", {
											"access_token", _envelope["headers"]["Authentication"]
										}
									}
								)
							); 
							assertz(_validation->ok(), "Bad authorization", 412, 0);
							_envelope["payload"] << "_id" << _validation["owner"]["_id"]->str();
							zpt::json _document = _db->query("users", _envelope["payload"]);
							assertz(_document->ok() && ((int) _document["size"]) != 0, "access to this endpoint must be authenticated", 401, 0);
							return zpt::json(
								{
									"status", 200,
									"payload", _document["elements"][0]
								}
							);
						}
						else if (_envelope["payload"]["username"]->ok() && _envelope["payload"]["password"]->ok()) {
							zpt::json _document = _db->query("users", _envelope["payload"]);
							assertz(_document->ok() && ((int) _document["size"]) != 0, "access to this endpoint must be authenticated", 401, 0);
							return zpt::json(
								{
									"status", 200,
									"payload", _document["elements"][0]
								}
							);
							
						}
						else if (_envelope["headers"]["Cookie"]->ok()) {
							zpt::json _cookie = zpt::rest::cookies::deserialize(_envelope["headers"]["Cookie"]->str());
							zpt::json _validation = _emitter->route(zpt::ev::Post, std::string("/") + _emitter->version() + std::string("/oauth2.0/validate"),
								zpt::json(
									{
										"payload", {
											"access_token", _cookie["value"]
										}
									}
								)
							);
							assertz(_validation->ok() && _validation["status"] == 200, "Bad cookie", 412, 0);
							zpt::json _document = _db->query("users", { "_id", _validation["payload"]["owner"]["_id"]->str() });
							assertz(_document->ok() && ((int) _document["size"]) != 0, "access to this endpoint must be authenticated", 401, 0);
							return zpt::json(
								{
									"status", 200,
									"payload", _document["elements"][0]
								}
							);
						}
					}
					return zpt::undefined;
				}
			},
			{
				zpt::ev::Put,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["e-mail"]->ok() &&
						_envelope["payload"]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->save("users", { "_id", _resource }, _envelope["payload"]);
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"updated", _size
							}
						}
					);
				}
			},
			{
				zpt::ev::Delete,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->remove("users", { "_id", _resource });
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"removed", _size
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope["payload"] << "_id" << _resource;
					zpt::json _document = _db->query("users", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return zpt::json(
							{
								"status", 404
							}
						);
					}
					return zpt::json(
						{
							"status", 200,
							"headers", {
								"Content-Length", ((std::string) _document["elements"][0]).length()
							}
						}
					);
				}
			},
			{
				zpt::ev::Patch,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->set("users", { "_id", _resource }, _envelope["payload"]);
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"updated", _size
							}
						}
					);
				}
			}
		}
	);

	_emitter->on(std::string("^/") + _emitter->version() + std::string("/users/([^/]+)/groups$"),
		{
			{
				zpt::ev::Get, 
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::json _list = _db->query("users", _envelope["payload"]);
					if (!_list->ok()) {
						return zpt::json(
							{
								"status", 204
							}
						);
					}
					return zpt::json(
						{
							"status", 200,
							"payload", _list
						}
					);
				}
			},
			{
				zpt::ev::Post,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["e-mail"]->ok() &&
						_envelope["payload"]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("users", _resource, _envelope["payload"]);
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"id", _id,
								"href", (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::json _list = _db->query("users", _envelope["payload"]);
					if (!_list->ok()) {
						return zpt::json(
							{
								"status", 204
							}
						);
					}
					return zpt::json(
						{
							"status", 200,
							"headers", {
								"Content-Length", ((std::string) _list).length()
							}
						}
					);
				}
			}
		}
	);
	
	_emitter->on(std::string("^/") + _emitter->version() + std::string("/groups$"),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::json _list = _db->query("groups", _envelope["payload"]);
					if (!_list->ok()) {
						return zpt::json(
							{
								"status", 204
							}
						);
					}
					return zpt::json(
						{
							"status", 200,
							"payload", _list
						}
					);
				}
			},
			{
				zpt::ev::Post,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["code"]->ok() &&
						_envelope["payload"]["scopes"]->ok(),
						"required fields: 'name', 'code' and 'scopes'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("groups", _resource, _envelope["payload"]);
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"id", _id,
								"href", (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::json _list = _db->query("groups", _envelope["payload"]);
					if (!_list->ok()) {
						return zpt::json(
							{
								"status", 204
							}
						);
					}
					return zpt::json(
						{
							"status", 200,
							"headers", {
								"Content-Length", ((std::string) _list).length()
							}
						}
					);
				}
			}
		}
	);

	_emitter->on(std::string("^/") + _emitter->version() + std::string("/groups/(.+)$"),
		{
			{
				zpt::ev::Get,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope["payload"] << "_id" << _resource;
					zpt::json _document = _db->query("groups", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return zpt::json(
							{
								"status", 404
							}
						);
					}
					return zpt::json(
						{
							"status", 200,
							"payload", _document["elements"][0]
						}
					);
				}
			},
			{
				zpt::ev::Put,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["code"]->ok() &&
						_envelope["payload"]["scopes"]->ok(),
						"required fields: 'name', 'code' and 'scopes'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->save("groups", { "_id", _resource }, _envelope["payload"]);
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"updated", _size
							}
						}
					);
				}
			},
			{
				zpt::ev::Delete,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->remove("groups", { "_id", _resource });
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"removed", _size
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope["payload"] << "_id" << _resource;
					zpt::json _document = _db->query("groups", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return zpt::json(
							{
								"status", 404
							}
						);
					}
					return zpt::json(
						{
							"status", 200,
							"headers", {
								"Content-Length", ((std::string) _document["elements"][0]).length()
							}
						}
					);
				}
			},
			{
				zpt::ev::Patch,
				[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->set("groups", { "_id", _resource }, _envelope["payload"]);
					return zpt::json(
						{
							"status", 200,
							"payload", {
								"updated", _size
							}
						}
					);
				}
			}
		}
	);

}

