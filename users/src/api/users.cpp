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

namespace zpt {

	namespace users {

		void collection(zpt::EventEmitterPtr _events) {
			 zpt::ev::Handler _handler_set[7] = {
				//get
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 200 <<
						"payload" << JSON(
							"size" << 10 <<
							"elements" << JSON_ARRAY(
								"a"
							) <<
							"links" << JSON(
								"next" << (_envelope["resource"]->str() + std::string("?page-size=10&page-start-index=20")) << 
								"prev" << (_envelope["resource"]->str() + std::string("?page-size=10&page-start-index=0"))
							)
						)
					);
				},
				no_put,
				//post
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				no_delete,
				//head
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				no_options, 
				no_patch
			};
			_events->on(string("^/api/") + _events->options()["rest"]["version"]->str() + string("/users$"), _handler_set);
		}

		void document(zpt::EventEmitterPtr _events) {
			 zpt::ev::Handler _handler_set[7] = {
				//get
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				//put
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				//post
				no_post,
				//delete
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				//head
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				no_options, 
				//patch
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				}
			};
			_events->on(string("^/api/") + _events->options()["rest"]["version"]->str() + string("/users/([^/]+)$"), _handler_set);
		}

		void login(zpt::EventEmitterPtr _events) {
			_events->on(zpt::ev::Post, "^/auth/login", [ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
				return zpt::mkptr(JSON(
				        "status" << 204
			        ));
			});
		}
	}

	namespace groups {

		void collection(zpt::EventEmitterPtr _events) {	
			 zpt::ev::Handler _handler_set[7] = {
				//get
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				no_put,
				//post
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				//delete
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				//head
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				no_options, 
				//patch
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				}
			};
			_events->on(string("^/api/") + _events->options()["rest"]["version"]->str() + string("/groups$"), _handler_set);
		}

		void document(zpt::EventEmitterPtr _events) {	
			 zpt::ev::Handler _handler_set[7] = {
				//get
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				//put
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				no_post,
				//delete
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				//head
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				},
				no_options, 
				//patch
				[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
					return JOBJ_PTR(
						"status" << 204
					);
				}
			};
			_events->on(string("^/api/") + _events->options()["rest"]["version"]->str() + string("/groups/([^/]+)$"), _handler_set);
		}
	}
}

extern "C" void restify(zpt::EventEmitterPtr _events) {
//	zpt::KBPtr _kb(new zpt::mongodb::Collection(_events->options()["users"]));
//	_events->add_kb("mongodb.users", _kb);

	zpt::users::collection(_events);
	zpt::users::document(_events);
	zpt::users::login(_events);

	zpt::groups::collection(_events);
	zpt::groups::document(_events);
}

extern "C" int zapata_users() {
	return 1;
}
