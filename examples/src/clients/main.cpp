/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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

#include <zapata/couchdb.h>
#include <zapata/upnp.h>
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
		if (argc == 1) {
			zpt::upnp::broker _upnp(zpt::undefined);
			for (; true;) {
				std::cout << "listening..." << endl << flush;
				try {
					std::cout << std::string(_upnp->listen()) << flush;
				} catch (...) {
				}
			}
			return 0;
		} else {
			zpt::upnp::broker _upnp(zpt::undefined);
			_upnp->search("urn:schemas-upnp-org:service:/v3/data-layer/tokens");
			return 0;
		}

		std::cout << std::string(zpt::email::parse("pedro.figueiredo@muzzley.com")) << endl << flush;
		std::cout << std::string(zpt::email::parse("\"José Paulo\" <pedro.figueiredo@muzzley.com>")) << endl
			  << flush;
		std::cout << std::string(zpt::email::parse("<pedro.figueiredo@muzzley.com>")) << endl << flush;
		return 0;

		time_t _now = time(nullptr);
		zpt::tm_ptr _tm_now = zpt::get_time(_now);
		time_t _then = _now - (3600 * 24 * 30);
		zpt::tm_ptr _tm_then = zpt::get_time(_then);

		std::cout << "DST: " << _then << " " << std::put_time(_tm_then.get(), "%c %Z") << " > "
			  << _tm_then->tm_gmtoff << endl
			  << flush;
		std::cout << "NO DST: " << _now << " " << std::put_time(_tm_now.get(), "%c %Z") << " > "
			  << _tm_now->tm_gmtoff << endl
			  << flush;
		return 0;

	} catch (zpt::assertion& _e) {
		zlog(_e.what() + string("\n") + _e.description(), zpt::error);
	}

	return 0;
}
