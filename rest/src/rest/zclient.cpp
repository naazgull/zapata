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

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

#include <zapata/rest.h>
#include <zapata/mem/usage.h>
#include <semaphore.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int argc, char* argv[]) {
	char _c;
	std::string _host;
	std::string _port;
	zpt::ev::Performative _method = zpt::ev::Get;
	std::string _url;
	short _type = ZMQ_REQ;
	zpt::JSONPtr _body;
	bool _verbose = false;

	while ((_c = getopt(argc, argv, "vh:p:m:u:t:j:")) != -1) {
		switch (_c) {
			case 'v': {
				_verbose = true;
				break;
			}
			case 'h': {
				_host.assign(string(optarg));
				break;
			}
			case 'p': {
				_port.assign(string(optarg));
				break;
			}
			case 'm': {
				_method = zpt::ev::from_str(string(optarg));
				break;
			}
			case 'u': {
				_url.assign(string(optarg));
				break;
			}
			case 't': {
				_type = zpt::str2type(string(optarg));
				break;
			}
			case 'j': {
				_body = zpt::JSONPtr(zpt::json(string(optarg)));
				break;
			}
		}
	}

	zpt::log_fd = & cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new string(argv[0]);
	zpt::log_lvl = (_verbose ? 7 : 0);

	try {
		zpt::RESTClientPtr _api(zpt::mkobj());

		switch(_type) {
			case ZMQ_REQ: {
				zpt::ZMQPtr _client = _api->bind(_type, std::string("tcp://") + _host + std::string(":") + _port);
				zpt::JSONPtr _reply = _client->send(_method, _url, _body);
				if (_verbose) {
					zlog(zpt::pretty(_reply), zpt::info);
				}
				exit(0);
				break;
			}
			case ZMQ_PULL: {
				_api->emitter()->on(zpt::ev::Reply, _url,
					[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
						if (_verbose) {
							zlog(zpt::pretty(_envelope), zpt::info);
						}
						return zpt::undefined;
					}
				);
				_api->start();
				break;
			}
			case ZMQ_PUB: {
				zpt::ZMQPtr _client = _api->bind(_type, std::string("tcp://") + _host + std::string(":") + _port);
				_client->send(_method, _url, _body);
				exit(0);
				break;
			}			    
			case ZMQ_SUB: {
				_api->emitter()->on(zpt::ev::Reply, _url,
					[ & ] (zpt::ev::Performative _performative, std::string _resource, zpt::JSONPtr _envelope, zpt::EventEmitterPtr _events) -> zpt::JSONPtr {
						if (_verbose) {
							zlog(zpt::pretty(_envelope), zpt::info);
						}
						return zpt::undefined;
					}
				);
				_api->start();
				break;
			}
		}
	}
	catch (zpt::AssertionException& _e) {
		zlog(_e.what() + string("\n") + _e.description(), zpt::error);
	}
	
	return 0;
}
