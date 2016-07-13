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

/*{{
  # Users API
  }}*/ 
extern "C" void restify(zpt::EventEmitterPtr _emitter) {
	assertz(_emitter->options()["mongodb"]["users"]->ok(), "no 'mongodb.users' object found in provided configuration", 500, 0);
	_emitter->add_kb("mongodb.users", zpt::KBPtr(new zpt::mongodb::Client(_emitter->options(), "mongodb.users")));

	/*{{	  
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
	  }}*/ 
	_emitter->on(std::string("^/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/users$"),
		{
			{
				zpt::ev::Get,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("users", _envelope["payload"]);
					if (!_list->ok()) {
						return JPTR(
							"status" << 204
						);
					}
					return JPTR(
						"status" << 200 <<
						"payload" << _list
					);
				}
			},
			{
				zpt::ev::Post,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["e-mail"]->ok() &&
						_envelope["payload"]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
					
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("users", _resource, _envelope["payload"]);
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"id" << _id <<
							"href" << (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
						)
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("users", _envelope["payload"]);
					if (!_list->ok()) {
						return JPTR(
							"status" << 204
						);
					}
					return JPTR(
						"status" << 200 <<
						"headers" << JSON(
							"Content-Length" << ((std::string) _list).length()
						)
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
					_envelope["payload"] << "_id" << _resource;
					zpt::JSONPtr _document = _db->query("users", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return JPTR(
							"status" << 404
						);
					}
					return JPTR(
						"status" << 200 <<
						"payload" << _document["elements"][0]
					);
				}
			},
			{
				zpt::ev::Put,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["e-mail"]->ok() &&
						_envelope["payload"]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->save("users", JPTR( "_id" << _resource ), _envelope["payload"]);
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"updated" << _size
						)
					);
				}
			},
			{
				zpt::ev::Delete,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->remove("users", JPTR( "_id" << _resource ));
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"removed" << _size
						)
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope["payload"] << "_id" << _resource;
					zpt::JSONPtr _document = _db->query("users", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return JPTR(
							"status" << 404
						);
					}
					return JPTR(
						"status" << 200 <<
						"headers" << JSON(
							"Content-Length" << ((std::string) _document["elements"][0]).length()
						)
					);
				}
			},
			{
				zpt::ev::Patch,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->set("users", JPTR( "_id" << _resource ), _envelope["payload"]);
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"updated" << _size
						)
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
					zpt::JSONPtr _list = _db->query("users", _envelope["payload"]);
					if (!_list->ok()) {
						return JPTR(
							"status" << 204
						);
					}
					return JPTR(
						"status" << 200 <<
						"payload" << _list
					);
				}
			},
			{
				zpt::ev::Post,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["e-mail"]->ok() &&
						_envelope["payload"]["password"]->ok(),
						"required fields: 'name', 'e-mail' and 'password'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("users", _resource, _envelope["payload"]);
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"id" << _id <<
							"href" << (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
						)
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("users", _envelope["payload"]);
					if (!_list->ok()) {
						return JPTR(
							"status" << 204
						);
					}
					return JPTR(
						"status" << 200 <<
						"headers" << JSON(
							"Content-Length" << ((std::string) _list).length()
						)
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
					zpt::JSONPtr _list = _db->query("groups", _envelope["payload"]);
					if (!_list->ok()) {
						return JPTR(
							"status" << 204
						);
					}
					return JPTR(
						"status" << 200 <<
						"payload" << _list
					);
				}
			},
			{
				zpt::ev::Post,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["code"]->ok() &&
						_envelope["payload"]["scopes"]->ok(),
						"required fields: 'name', 'code' and 'scopes'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					std::string _id = _db->insert("groups", _resource, _envelope["payload"]);
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"id" << _id <<
							"href" << (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id)
						)
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					zpt::JSONPtr _list = _db->query("groups", _envelope["payload"]);
					if (!_list->ok()) {
						return JPTR(
							"status" << 204
						);
					}
					return JPTR(
						"status" << 200 <<
						"headers" << JSON(
							"Content-Length" << ((std::string) _list).length()
						)
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
					_envelope["payload"] << "_id" << _resource;
					zpt::JSONPtr _document = _db->query("groups", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return JPTR(
							"status" << 404
						);
					}
					return JPTR(
						"status" << 200 <<
						"payload" << _document["elements"][0]
					);
				}
			},
			{
				zpt::ev::Put,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["code"]->ok() &&
						_envelope["payload"]["scopes"]->ok(),
						"required fields: 'name', 'code' and 'scopes'", 412, 0);
				
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->save("groups", JPTR( "_id" << _resource ), _envelope["payload"]);
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"updated" << _size
						)
					);
				}
			},
			{
				zpt::ev::Delete,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->remove("groups", JPTR( "_id" << _resource ));
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"removed" << _size
						)
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					_envelope["payload"] << "_id" << _resource;
					zpt::JSONPtr _document = _db->query("groups", _envelope["payload"]);
					if (!_document->ok() || _document["size"] == 0) {
						return JPTR(
							"status" << 404
						);
					}
					return JPTR(
						"status" << 200 <<
						"headers" << JSON(
							"Content-Length" << ((std::string) _document["elements"][0]).length()
						)
					);
				}
			},
			{
				zpt::ev::Patch,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
					size_t _size = _db->set("groups", JPTR( "_id" << _resource ), _envelope["payload"]);
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"updated" << _size
						)
					);
				}
			}
		}
	);

}

