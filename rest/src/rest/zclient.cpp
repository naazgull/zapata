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
	try {
		zpt::rest::client _api = zpt::rest::client::launch(argc, argv);
		
		switch(_api->options()["zmq"]["type"]->intr()) {
			case ZMQ_REQ: {
				zpt::socket _client = _api->bind(_api->options()["zmq"]["type"]->intr(), _api->options()["zmq"]["bind"]->str());
				zpt::json _envelope(
					{
						"channel", _api->options()["rest"]["target"],
						"performative", (zpt::ev::performative) _api->options()["rest"]["method"]->intr(),
						"resource", _api->options()["rest"]["target"],
						"payload", _api->options()["rest"]["body"]
					}
				);
				if (_api->options()["rest"]["token"]->ok()) {
					_envelope << "headers" << zpt::json({ "Authorization", std::string("OAuth2.0 ") + _api->options()["rest"]["token"]->str() });
				}
				zpt::json _reply = _client->send(_envelope);
				zlog(zpt::pretty(_reply), zpt::info);
				exit(0);
				break;
			}
			case ZMQ_PULL: {
				_api->emitter()->on(zpt::ev::Reply, _api->options()["rest"]["target"]->str(),
					[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
						zlog(zpt::pretty(_envelope), zpt::info);
						return zpt::undefined;
					}
				);
				_api->start();
				break;
			}
			case ZMQ_PUB: {
				zpt::socket _client = _api->bind(_api->options()["zmq"]["type"]->intr(), _api->options()["zmq"]["bind"]->str());
				zpt::json _envelope(
					{
						"channel", _api->options()["rest"]["target"],
						"performative", (zpt::ev::performative) _api->options()["rest"]["method"]->intr(),
						"resource", _api->options()["rest"]["target"],
						"payload", _api->options()["rest"]["body"]
					}
				);
				if (_api->options()["rest"]["token"]->ok()) {
					_envelope << "headers" << zpt::json({ "Authorization", std::string("OAuth2.0 ") + _api->options()["rest"]["token"]->str() });
				}
				_client->send(_envelope);
				exit(0);
				break;
			}			    
			case ZMQ_SUB: {
				_api->emitter()->on(zpt::ev::Reply, _api->options()["rest"]["target"]->str(),
					[] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
						zlog(zpt::pretty(_envelope), zpt::info);
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
		throw;
	}
	
	return 0;
}
