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
#include <zapata/rest.h>
#include <zapata/mongodb.h>
#include <zapata/users/codes_users.h>
#include <ctime>
#include <memory>

/***
  # Users API
***/ 
extern "C" void restify(zpt::EventEmitterPtr _emitter) {
	assertz(_emitter->options()["mongodb"]["users"]->ok(), "no 'mongodb.users' object found in provided configuration", 500, 0);
	_emitter->add_kb("mongodb.users", zpt::KBPtr(new zpt::mongodb::Client(_emitter->options(), "mongodb.users")));

	/***	  
	  ## Users collection
	  ```
	  /api/{api-version}/users
	  ```
	  ### Description
	  The _Users_ collections holds the set of _User_ documents for the configured **MongoDB** database and collection. 
	  
	  ### Allowed methods
	  - _GET_
	  - _POST_
	  - _HEAD_
	***/ 
	_emitter->on(std::string("^/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/users$"),
		{
			{
				zpt::ev::Get,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("users", zpt::get(ZPT_PAYLOAD, _envelope));
					if (!_list->ok()) {
						return zpt::set(ZPT_STATUS, 204);
					}
					return zpt::set(ZPT_STATUS, 200, zpt::set(ZPT_PAYLOAD, _list));
				}
			},
			{
				zpt::ev::Post,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope[ZPT_PAYLOAD]->ok() &&
						_envelope[ZPT_PAYLOAD]["name"]->ok() &&
						_envelope[ZPT_PAYLOAD]["e-mail"]->ok() &&
						_envelope[ZPT_PAYLOAD]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
					
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("users", _resource, _envelope[ZPT_PAYLOAD]);
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"id", _id,
								"href", (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("users", _envelope[ZPT_PAYLOAD]);
					if (!_list->ok()) {
						return Json(
							{
								ZPT_STATUS, 204
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_HEADERS, {
								"Content-Length", ((std::string) _list).length()
							}
						}
					);
				}
			}
		}
	);

	_emitter->on(std::string("^/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/users/([^/]+)$"),
		{
			{
				zpt::ev::Get,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					if (_resource == (std::string("/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/users/me"))) {
						assertz((_envelope[ZPT_PAYLOAD]["username"]->ok() && _envelope[ZPT_PAYLOAD]["password"]->ok()) || _envelope[ZPT_HEADERS]["Cookie"]->ok() || _envelope[ZPT_HEADERS]["Authorization"]->ok(), "access to this endpoint must be authenticated", 401, 0);
						if (_envelope[ZPT_HEADERS]["Cookie"]->ok()) {
							zpt::JSONPtr _user_token = zpt::rest::cookies::unserialize(_envelope[ZPT_HEADERS]["Cookie"]);
							assertz(_user_token->ok() && _user_token["user"]->ok() && _user_token["access_token"]->ok(), "Bad cookie", 412, 0);
							_resource.assign(std::string("/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/users/") + _user_token["user"]->str());
						}
						else if (_envelope[ZPT_HEADERS]["Authorization"]->ok()) {
						}
						else if (_envelope[ZPT_PAYLOAD]["username"]->ok() && _envelope[ZPT_PAYLOAD]["password"]->ok()) {
							zpt::JSONPtr _document = _db->query("users", _envelope[ZPT_PAYLOAD]);
							assertz(_document->ok() && ((int) _document["size"]) != 0, "access to this endpoint must be authenticated", 401, 0);
							return Json(
								{
									ZPT_STATUS, 200,
									ZPT_PAYLOAD, _document["elements"][0]
								}
							);
							
						}
					}
					
					_envelope[ZPT_PAYLOAD] << "_id" << _resource;
					zpt::JSONPtr _document = _db->query("users", _envelope[ZPT_PAYLOAD]);
					if (!_document->ok() || _document["size"] == 0) {
						return Json(
							{
								ZPT_STATUS, 404,
								ZPT_PAYLOAD, {
									"text", "resource not found"
								}
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, _document["elements"][0]
						}
					);
				}
			},
			{
				zpt::ev::Put,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope[ZPT_PAYLOAD]->ok() &&
						_envelope[ZPT_PAYLOAD]["name"]->ok() &&
						_envelope[ZPT_PAYLOAD]["e-mail"]->ok() &&
						_envelope[ZPT_PAYLOAD]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->save("users", Json({ "_id", _resource }), _envelope[ZPT_PAYLOAD]);
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"updated", _size
							}
						}
					);
				}
			},
			{
				zpt::ev::Delete,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->remove("users", Json({ "_id", _resource }));
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"removed", _size
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope[ZPT_PAYLOAD] << "_id" << _resource;
					zpt::JSONPtr _document = _db->query("users", _envelope[ZPT_PAYLOAD]);
					if (!_document->ok() || _document["size"] == 0) {
						return Json(
							{
								ZPT_STATUS, 404
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_HEADERS, {
								"Content-Length", ((std::string) _document["elements"][0]).length()
							}
						}
					);
				}
			},
			{
				zpt::ev::Patch,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->set("users", Json({ "_id", _resource }), _envelope[ZPT_PAYLOAD]);
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"updated", _size
							}
						}
					);
				}
			}
		}
	);

	_emitter->on(std::string("^/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/users/([^/]+)/groups$"),
		{
			{
				zpt::ev::Get, 
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("users", _envelope[ZPT_PAYLOAD]);
					if (!_list->ok()) {
						return Json(
							{
								ZPT_STATUS, 204
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, _list
						}
					);
				}
			},
			{
				zpt::ev::Post,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope[ZPT_PAYLOAD]->ok() &&
						_envelope[ZPT_PAYLOAD]["name"]->ok() &&
						_envelope[ZPT_PAYLOAD]["e-mail"]->ok() &&
						_envelope[ZPT_PAYLOAD]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("users", _resource, _envelope[ZPT_PAYLOAD]);
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"id", _id,
								"href", (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("users", _envelope[ZPT_PAYLOAD]);
					if (!_list->ok()) {
						return Json(
							{
								ZPT_STATUS, 204
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_HEADERS, {
								"Content-Length", ((std::string) _list).length()
							}
						}
					);
				}
			}
		}
	);
	
	_emitter->on(std::string("^/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/groups$"),
		{
			{
				zpt::ev::Get,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("groups", _envelope[ZPT_PAYLOAD]);
					if (!_list->ok()) {
						return Json(
							{
								ZPT_STATUS, 204
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, _list
						}
					);
				}
			},
			{
				zpt::ev::Post,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope[ZPT_PAYLOAD]->ok() &&
						_envelope[ZPT_PAYLOAD]["name"]->ok() &&
						_envelope[ZPT_PAYLOAD]["code"]->ok() &&
						_envelope[ZPT_PAYLOAD]["scopes"]->ok(),
						"required fields: 'name', 'code' and 'scopes'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("groups", _resource, _envelope[ZPT_PAYLOAD]);
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"id", _id,
								"href", (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("groups", _envelope[ZPT_PAYLOAD]);
					if (!_list->ok()) {
						return Json(
							{
								ZPT_STATUS, 204
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_HEADERS, {
								"Content-Length", ((std::string) _list).length()
							}
						}
					);
				}
			}
		}
	);

	_emitter->on(std::string("^/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/groups/([^/]+)$"),
		{
			{
				zpt::ev::Get,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope[ZPT_PAYLOAD] << "_id" << _resource;
					zpt::JSONPtr _document = _db->query("groups", _envelope[ZPT_PAYLOAD]);
					if (!_document->ok() || _document["size"] == 0) {
						return Json(
							{
								ZPT_STATUS, 404
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, _document["elements"][0]
						}
					);
				}
			},
			{
				zpt::ev::Put,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope[ZPT_PAYLOAD]->ok() &&
						_envelope[ZPT_PAYLOAD]["name"]->ok() &&
						_envelope[ZPT_PAYLOAD]["code"]->ok() &&
						_envelope[ZPT_PAYLOAD]["scopes"]->ok(),
						"required fields: 'name', 'code' and 'scopes'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->save("groups", Json({ "_id", _resource }), _envelope[ZPT_PAYLOAD]);
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"updated", _size
							}
						}
					);
				}
			},
			{
				zpt::ev::Delete,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->remove("groups", Json({ "_id", _resource }));
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"removed", _size
							}
						}
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope[ZPT_PAYLOAD] << "_id" << _resource;
					zpt::JSONPtr _document = _db->query("groups", _envelope[ZPT_PAYLOAD]);
					if (!_document->ok() || _document["size"] == 0) {
						return Json(
							{
								ZPT_STATUS, 404
							}
						);
					}
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_HEADERS, {
								"Content-Length", ((std::string) _document["elements"][0]).length()
							}
						}
					);
				}
			},
			{
				zpt::ev::Patch,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->set("groups", Json({ "_id", _resource }), _envelope[ZPT_PAYLOAD]);
					return Json(
						{
							ZPT_STATUS, 200,
							ZPT_PAYLOAD, {
								"updated", _size
							}
						}
					);
				}
			}
		}
	);

}

