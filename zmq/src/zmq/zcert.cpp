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
#include <getopt.h>
#include <zapata/zmq.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int argc, char* argv[]) {
	try {
		char _c;
		std::string _email;
		std::string _name;
		std::string _organization;
		std::string _out_cert;
		std::string _out_key;

		static struct option _literal_options[] = { {"email", required_argument, 0, 0}, {"name", required_argument, 0, 0}, {"org", required_argument, 0, 0}, {"priv", required_argument, 0, 0}, {"pub", required_argument, 0, 0}, {0, 0, 0, 0} };
		int _literal_option_index = 0;
	
		while ((_c = getopt_long(argc, argv, "", _literal_options, &_literal_option_index)) != -1) {
			switch (_c) {
				case 0 : {
					if (_literal_options[_literal_option_index].flag != 0) {
						break;
					}
					if (std::string(_literal_options[_literal_option_index].name) == "email") {
						_email.assign(std::string(optarg));
					}
					if (std::string(_literal_options[_literal_option_index].name) == "name") {
						_name.assign(std::string(optarg));
					}
					if (std::string(_literal_options[_literal_option_index].name) == "org") {
						_organization.assign(std::string(optarg));
					}
					if (std::string(_literal_options[_literal_option_index].name) == "priv") {
						_out_cert.assign(std::string(optarg));
					}
					if (std::string(_literal_options[_literal_option_index].name) == "pub") {
						_out_key.assign(std::string(optarg));
					}
					break;
				}
			}
		}

		zcert_t *_cert = zcert_new();
		zcert_set_meta(_cert, "email", _email.data());
		zcert_set_meta(_cert, "name", _name.data());
		zcert_set_meta(_cert, "organization", _organization.data());

		zcert_save_public(_cert, _out_key.data());
		zcert_save_secret(_cert, _out_cert.data());

		zcert_destroy(&_cert);
	}
	catch (zpt::AssertionException& _e) {
		std::cout << _e.what() << ": " << _e.description() << endl << flush;
		exit(-10);
	}
	catch (std::exception& _e) {
		std::cout << _e.what() << endl << flush;
		exit(-10);
	}
	
	return 0;
}
