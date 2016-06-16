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

extern "C" void restify(zpt::EventEmitterPtr _emitter) {
	zpt::KBPtr _kb(new zpt::mongodb::Client(_emitter->options(), "mongodb.users"));
	_emitter->add_kb("mongodb.users", _kb);
	
	zpt::ev::Handler _users_collection[7] = {
		//get
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
			zpt::JSONPtr _list = _db->query("users", _envelope["payload"]);
			if (!_list->ok()) {
				return JPTR(
					"status" << 204
				);
			}
			_list << "links" << JSON(
				"next" << (_envelope["resource"]->str() + std::string("?page-size=10&page-start-index=20")) <<
				"prev" << (_envelope["resource"]->str() + std::string("?page-size=10&page-start-index=0"))
			);
			return JPTR(
				"status" << 200 <<
				"payload" << _list
			);
		},
		no_put,
		//post
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
					"href" << (_resource + (_resource.back() != '/' ? string("/") : string("")) + _id)
				)
			);
		},
		no_delete,
		//head
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
			zpt::JSONPtr _list = _db->query("users", _envelope["payload"]);
			if (!_list->ok()) {
				return JPTR(
					"status" << 204
				);
			}
			_list << "links" << JSON(
				"next" << (_envelope["resource"]->str() + std::string("?page-size=10&page-start-index=20")) <<
				"prev" << (_envelope["resource"]->str() + std::string("?page-size=10&page-start-index=0"))
			);
			return JPTR(
				"status" << 200 <<
				"headers" << JSON(
					"Content-Length" << ((std::string) _list).length()
				)
			);
		},
		no_options, 
		no_patch
	};
	_emitter->on(string("^/api/") + _emitter->options()["rest"]["version"]->str() + string("/users$"), _users_collection);

	zpt::ev::Handler _users_document[7] = {
		//get
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
		},
		//put
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
		},
		//post
		no_post,
		//delete
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			zpt::mongodb::Client* _db = (zpt::mongodb::Client*) _emitter->get_kb("mongodb.users").get();
			size_t _size = _db->remove("users", JPTR( "_id" << _resource ));
			return JPTR(
				"status" << 200 <<
				"payload" << JSON(
					"removed" << _size
				)
			);
		},
		//head
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
		},
		no_options, 
		//patch
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
	};
	_emitter->on(string("^/api/") + _emitter->options()["rest"]["version"]->str() + string("/users/([^/]+)$"), _users_document);

	zpt::ev::Handler _login_controller[7] = {
		no_get,
		no_put,
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		no_delete,
		no_head,
		no_options, 
		no_patch
	};
	_emitter->on(string("^/api/") + _emitter->options()["rest"]["version"]->str() + string("^/auth/login"), _login_controller);

	zpt::ev::Handler _groups_collection[7] = {
		//get
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		no_put,
		//post
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		//delete
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		//head
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		no_options, 
		//patch
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		}
	};
	_emitter->on(string("^/api/") + _emitter->options()["rest"]["version"]->str() + string("/groups$"), _groups_collection);

	zpt::ev::Handler _groups_document[7] = {
		//get
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		//put
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		no_post,
		//delete
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		//head
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		},
		no_options, 
		//patch
		[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
			return JPTR(
				"status" << 204
			);
		}
	};
	_emitter->on(string("^/api/") + _emitter->options()["rest"]["version"]->str() + string("/groups/([^/]+)$"), _groups_document);

}

extern "C" int zapata_users() {
	return 1;
}
