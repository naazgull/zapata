/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

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
#include <sys/types.h>
#include <sys/ipc.h>
#include <zapata/log/log.h>
#include <zapata/text/manip.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int _argc, char* _argv[]) {
	std::string _input;

	do {
		std::cout << "Project name: " << flush;
		std::getline(std::cin, _input);
		if (_input.find(" ") != string::npos) {
			std::cout << "   * Project name can't have spaces" << endl << flush;
		}
	}
	while(_input.find(" ") != string::npos);
	std::string _project_name(_input.data());
	
	do {
		std::cout << "Project abbreviation: " << flush;
		std::getline(std::cin, _input);
		if (_input.find(" ") != string::npos) {
			std::cout << "   * Project abbreviation can't have spaces" << endl << flush;
		}
	}
	while(_input.find(" ") != string::npos);
	std::string _project_abbr(_input.data());
	
	std::cout << "Path prefix: " << flush;
	std::getline(std::cin, _input, '\n');
	std::string _path_prefix(_input.data());
	
	std::cout << "Developer name: " << flush;
	std::getline(std::cin, _input, '\n');
	std::string _dev_name(_input.data());
	
	do {
		std::cout << "Developer e-mail address: " << flush;
		std::getline(std::cin, _input);
		if (_input.find(" ") != string::npos) {
			std::cout << "   * E-mail can't have spaces" << endl << flush;
		}
	}
	while(_input.find(" ") != string::npos);
	std::string _dev_email(_input.data());

	if (std::system("tar xvjf /usr/share/zapata/autoconf.template.tar.bz2"));
	if (std::system((std::string("/usr/share/zapata/zinit_setup '") + _project_name + std::string("' '") + _project_abbr + std::string("' '") + _dev_email + std::string("' '") + _dev_name + std::string("' '") + _path_prefix + std::string("'")).data()));
	
	return 0;
}

