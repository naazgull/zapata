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
	zpt::log_fd = &std::cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new string(argv[0]);
	zpt::log_format = 1;
	zpt::log_lvl = 0;

	try {
		// z0mqc tcp://platform.muzzley.com:993/v2/channels/74885d26-e15b-11e6 -N req -X GET -H 'Authorization: q2093rnw98n0f8iquwefqwuef980wnj98f0ufj9842uf9n08j2'
		zpt::json _opts = zpt::conf::getopt(argc, argv);
		if (!_opts["files"]->ok()) {
			exit(-1);
		}
	
		zpt::json _uri = zpt::uri::parse(std::string(_opts["files"][0]));
		
		if (std::string(_opts["N"][0]) == "req") {
			std::cout << "* connecting to " << (std::string(">") + std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["authority"])) << "..." << endl << flush;
			zpt::ZMQReq _socket(std::string(">") + std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["authority"]), _opts);
			zpt::json _envelope = {
				"channel", std::string(_uri["path"]),
				"performative", zpt::ev::from_str(std::string(_opts["X"][0])),
				"resource", std::string(_uri["path"]),
				"params", _uri["query"],
				"payload", (_opts["d"]->ok() ? zpt::json(std::string(_opts["d"][0])) : zpt::undefined)
			};
			if (_opts["H"]->ok()) {
				zpt::json _headers = zpt::json::object();
				for (auto _header : _opts["H"]->arr()) {
					zpt::json _splited = zpt::split(_header->str(), ":");
					_headers << zpt::r_trim(_splited[0]->str()) << zpt::r_trim(_splited[1]->str());
				}
				_envelope << "headers" << _headers;
			}
			zpt::json _reply = _socket.send(_envelope);
			std::cout << "> " << zpt::r_replace(zpt::json::pretty(_envelope), "\n", "\n> ") << endl << endl << flush;
			std::cout << "< " << zpt::r_replace(zpt::json::pretty(_reply), "\n", "\n< ") << endl << flush;
			exit(0);
		}
		else if (std::string(_opts["N"][0]) == "pub-sub") {
		}
		else if (std::string(_opts["N"][0]) == "pull") {
		}
	}
	catch (zpt::assertion& _e) {
		zlog(_e.what() + string("\n") + _e.description(), zpt::error);
		throw;
	}
	
	return 0;
}
