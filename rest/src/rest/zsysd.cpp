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

	zpt::json _args = zpt::conf::getopt(argc, argv);
	if (_args["add"]) {
		zpt::json _global_conf;
		std::ifstream _zfs;
		_zfs.open((std::string("/etc/zapata/zapata.conf")).data());
		if (_zfs.is_open()) {
			zpt::json _conf;
			_zfs >> _conf;
			_zfs.close();
		}
		
		for (auto _to_add : _args["add"]->arr()) {
			std::ifstream _ifs;
			_ifs.open((std::string("/etc/zapata/backend-available/") + std::string(_to_add) + std::string(".conf")).data());
			if (_ifs.is_open()) {
				zpt::json _conf;
				_ifs >> _conf;
				_ifs.close();

				_conf = _global_conf + _conf;
				zpt::conf::setup(_conf);

				if (!_conf["boot"][0]["name"]->is_string()) {
					std::cout << "no bootable configuration found in /etc/zapata/backend-available/" << std::string(_to_add) << ".conf" << endl << flush;
					exit(-1);
				}

				std::ofstream _ofs;
				_ofs.open((std::string("/etc/zapata/backend-enable/") + std::string(_conf["boot"][0]["name"]) + std::string(".conf")).data(), std::ios::out | std::ios::trunc);
				_ofs << zpt::json::pretty(_conf) << flush;
				_ofs.close();

				std::string _sysd(
					"[Unit]\n"
					"Description=${name}\n"
					"${dependencies}\n"
					"${requirements}\n"
					"\n"
					"[Service]\n"
					"TimeoutStartSec=0\n"
					"Restart=on-failure\n"
					"RemainAfterExit=no\n"
					"\n"
					"ExecStart=/usr/bin/zpt -c /etc/zapata/backend-enabled/${name}.conf\n"
					"\n"
					"[Install]\n"
					"WantedBy=multi-user.target\n"
				);

				std::string _after;
				std::string _requires;

				if (_conf["depends"]->is_array()) {
					for (auto _dep : _conf["depends"]->arr()) {
						_after += std::string("After=") + std::string(_dep) + std::string(".service\n");
					}
				}
				if (_conf["requires"]->is_array()) {
					for (auto _req : _conf["requires"]->arr()) {
						_requires += std::string("Requires=") + std::string(_req) + std::string(".service\n");
					}
				}
				zpt::replace(_sysd, "${dependencies}", _after);
				zpt::replace(_sysd, "${requirements}", _requires);
				zpt::replace(_sysd, "${name}", std::string(_conf["boot"][0]["name"]));

				std::ofstream _sfs;
				_sfs.open((std::string("/lib/systemd/system/") + std::string(_conf["boot"][0]["name"]) + std::string(".service")).data());
				if (_sfs.is_open()) {
					_sfs.close();
				}
				else {
					std::cout << "couldn't write to /lib/systemd/system/" << std::string(_conf["boot"][0]["name"]) << ".service" << endl << flush;
					exit(-1);
				}
			}
			else {
				std::cout << "no such file named /etc/zapata/backend-available/" << std::string(_to_add) << ".conf" << endl << flush;
				exit(-1);
			}
		}
						
	}
	else if (_args["remove"]) {
		for (auto _to_remove : _args["remove"]->arr()) {
			if (system((std::string("rm -rf /etc/zapata/backend-enabled/") + std::string(_to_remove) + std::string(".conf")).data())) {
			}
			if (system((std::string("rm -rf /lib/systemd/system/") + std::string(_to_remove) + std::string(".service")).data())) {
			}
		}		
	}
	
	return 0;
}
