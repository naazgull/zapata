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
#include <zapata/redis.h>
#include <ctime>
#include <memory>
#include <ossp/uuid++.hh>

/*{{
  # Applications API
  }}*/ 
extern "C" void restify(zpt::EventEmitterPtr _emitter) {
	assertz(_emitter->options()["redis"]["apps"]->ok(), "no 'redis.apps' object found in provided configuration", 500, 0);
	zpt::KBPtr _kb(new zpt::redis::Client(_emitter->options(), "redis.apps"));
	_emitter->add_kb("redis.apps", _kb);

	/*{{	  
	  ## Applications collection
	  ```
	  /api/{api-version}/apps
	  ```
	  ### Description
	  The _Applications_ collections holds the set of _Application_ documents for the configured **MongoDB** database and collection. 
	  
	  ### Allowed methods
	  - _GET_
	  - _POST_
	  - _HEAD_
	  }}*/ 
	_emitter->on(std::string("^/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/apps$"),
		{
			{
				zpt::ev::Get,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					zpt::JSONPtr _list = _db->query("apps", (!_envelope["payload"]->ok() || _envelope["payload"]->obj()->size() == 0 ? std::string("*") : std::string("*") + ((std::string) _envelope["payload"]->obj()->begin()->second) + std::string("*")));
					zlog(zpt::pretty(_list), zpt::debug);
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
						_envelope["payload"]["description"]->ok() &&
						_envelope["payload"]["redirect_domain"]->ok(),
						"required fields: 'name', 'description' and 'redirect_domain'", 412, 0);

					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					std::string _id = _db->insert("apps", _resource, _envelope["payload"]);
					std::string _href = (_resource + (_resource.back() != '/' ? std::string("/") : std::string("")) + _id);					
					_db->set("apps", _href, JPTR( "client_id" << _id << "client_secret" << zpt::generate_key() ));
					
					return JPTR(
						"status" << 200 <<
						"payload" << JSON(
							"id" << _id <<
							"href" << _href
						)
					);
				}
			},
			{
				zpt::ev::Head,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					zpt::JSONPtr _list = _db->query("apps", _envelope["payload"]);
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

	_emitter->on(std::string("^/api/") + _emitter->options()["rest"]["version"]->str() + std::string("/apps/([^/]+)$"),
		{
			{
				zpt::ev::Get,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					zpt::JSONPtr _document = _db->get("apps", _resource);
					if (!_document->ok()) {
						return JPTR(
							"status" << 404
						);
					}
					return JPTR(
						"status" << 200 <<
						"payload" << _document
					);
				}
			},
			{
				zpt::ev::Put,
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _emitter) -> zpt::JSONPtr {
					assertz(
						_envelope["payload"]->ok() &&
						_envelope["payload"]["name"]->ok() &&
						_envelope["payload"]["description"]->ok() &&
						_envelope["payload"]["redirect_domain"]->ok(),
						"required fields: 'name', 'description' and 'redirect_domain'", 412, 0);
				
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					size_t _size = _db->save("apps", _resource, _envelope["payload"]);
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
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					size_t _size = _db->remove("apps", _resource);
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
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					zpt::JSONPtr _document = _db->get("apps", _resource);
					if (!_document->ok()) {
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
					zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.apps").get();
					size_t _size = _db->set("apps", _resource, _envelope["payload"]);
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

