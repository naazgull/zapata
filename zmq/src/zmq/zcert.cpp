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
#include <zapata/zmq.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int argc, char* argv[]) {
	try {
		zpt::json _args = zpt::conf::getopt(argc, argv);
		
		std::string _email(_args["email"]->ok() ? std::string(_args["email"][0]) : "");
		std::string _name(_args["name"]->ok() ? std::string(_args["name"][0]) : "");
		std::string _organization(_args["org"]->ok() ? std::string(_args["org"][0]) : "");
		std::string _out_cert(_args["priv"]->ok() ? std::string(_args["priv"][0]) : "");
		std::string _out_key(_args["pub"]->ok() ? std::string(_args["pub"][0]) : "");

		assertz(_email.length() != 0, "--email is required", 412, 0);
		assertz(_name.length() != 0, "--name is required", 412, 0);
		assertz(_organization.length() != 0, "--org is required", 412, 0);
		assertz(_out_cert.length() != 0, "--priv is required", 412, 0);
		assertz(_out_key.length() != 0, "--pub is required", 412, 0);

		zcert_t *_cert = zcert_new();
		zcert_set_meta(_cert, "email", _email.data());
		zcert_set_meta(_cert, "name", _name.data());
		zcert_set_meta(_cert, "organization", _organization.data());

		zcert_save_public(_cert, _out_key.data());
		zcert_save_secret(_cert, _out_cert.data());

		zcert_destroy(&_cert);
	}
	catch (zpt::assertion& _e) {
		std::cout << _e.what() << ": " << _e.description() << endl << flush;
		exit(-10);
	}
	catch (std::exception& _e) {
		std::cout << _e.what() << endl << flush;
		exit(-10);
	}
	
	return 0;
}
