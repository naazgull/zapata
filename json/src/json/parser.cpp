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

#include <zapata/json.h>
#include <semaphore.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int argc, char* argv[]) {
	if (argc > 1) {
		for (int _i = 1; _i != argc; _i++) {
			zpt::json _ptr;
			std::ifstream _in;
			_in.open(argv[_i]);
			if (!_in.is_open()) {
				zlog("unable to open provided file", zpt::error);
				exit(-10);
			}
			try {
				_in >> _ptr;
				zpt::conf::setup(_ptr);
			}
			catch(zpt::assertion& _e) {
				std::cout << _e.what() << endl << flush;
				return -1;
			}
			catch(zpt::SyntaxErrorException& _e) {
				std::cout << argv[_i] << ": " << _e.what() << endl << flush;
				return -1;
			}
			std::cout << zpt::pretty(_ptr) << endl << flush;
		}
	}
	return 0;
}
