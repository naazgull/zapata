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
#include <iomanip>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int argc, char* argv[]) {	
	try {
		time_t _now = time(nullptr);
		zpt::tm_ptr _tm_now = zpt::get_time(_now);
		time_t _then = _now - (3600 * 24 * 30);
		zpt::tm_ptr _tm_then = zpt::get_time(_then);

		std::cout << "DST: " << _then << " " << std::put_time(_tm_then.get(), "%c %Z") << " > " << _tm_then->tm_gmtoff << endl << flush;
		std::cout << "NO DST: " << _now << " " << std::put_time(_tm_now.get(), "%c %Z") << " > " << _tm_now->tm_gmtoff << endl << flush;
		return 0;
		
		zpt::rest::client _api = zpt::rest::client::launch(argc, argv);

		size_t _max = 10100;
		size_t * _n = new size_t();
		_api->events()->on(zpt::ev::Reply, zpt::rest::url_pattern({ _api->events()->version(), "apps" }),
			[ _n, _max ] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
				(* _n)++;
				zlog(std::to_string(* _n), zpt::debug);
				if ((* _n) == (_max - 100)) {
					cout << "PROCESSED " << (* _n) << " MESSAGES" << endl << flush;
				exit(0);
				}
				return zpt::undefined;
			}
		);
		zpt::socket_ref _client = _api->bind("zmq");
		zpt::json _message({ "name", "m/(.*)/i" });
		std::thread _sender(
			[ & ] () -> void {
				for (size_t _k = 0; _k != _max; _k++) {
					_client->send(zpt::ev::Get, zpt::path::join({_api->events()->version(), "apps"}), _message);
				}
				if (_api->options()["zmq"]["type"]->str() == "req") {
					cout << "PROCESSED " << (_max) << " MESSAGES" << endl << flush;
					exit(0);				
				}
			}
		);
		_api->start();
		_sender.join();
	}
	catch (zpt::assertion& _e) {
		zlog(_e.what() + string("\n") + _e.description(), zpt::error);
	}
	
	return 0;
}
