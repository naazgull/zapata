/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>

#include <semaphore.h>
#include <zapata/base.h>
#include <zapata/events.h>
#include <zapata/json.h>

auto
generate(zpt::json _to_add, zpt::json _global_conf) -> void {
    std::ifstream _ifs;
    _ifs.open(
      (std::string("/etc/zapata/backend-available/") + std::string(_to_add) + std::string(".conf"))
        .data());
    if (_ifs.is_open()) {
        zpt::json _conf;
        _ifs >> _conf;
        _ifs.close();

        _conf = _global_conf + _conf;
        zpt::conf::setup(_conf);

        _conf << "!warning"
              << "AUTOMATIC GENERATED FILE, do NOT edit by hand";
        _conf << "$source" << _to_add;

        if (!_conf["boot"][0]["name"]->is_string()) {
            std::cout << "no bootable configuration found in /etc/zapata/backend-available/"
                      << std::string(_to_add) << ".conf" << std::endl
                      << std::flush;
            exit(-1);
        }

        std::ofstream _ofs;
        _ofs.open((std::string("/etc/zapata/backend-enabled/") +
                   std::string(_conf["boot"][0]["name"]) + std::string(".conf"))
                    .data(),
                  std::ios::out | std::ios::trunc);
        _ofs << zpt::json::pretty(_conf) << std::flush;
        _ofs.close();
        std::cout << "> wrote /etc/zapata/backend-enabled/" << std::string(_conf["boot"][0]["name"])
                  << ".conf" << std::endl
                  << std::flush;

        std::string _sysd("[Unit]\n"
                          "Description=${name}\n"
                          "${dependencies}\n"
                          "${requirements}\n"
                          "\n"
                          "[Service]\n"
                          "LimitNOFILE=${fd_max}\n"
                          "Type=notify\n"
                          "TimeoutStartSec=0\n"
                          "TimeoutStopSec=2\n"
                          "Restart=${restart}\n"
                          "RemainAfterExit=no\n"
                          "WatchdogSec=${keep_alive}\n"
                          "\n"
                          "ExecStart=/usr/bin/zpt -c /etc/zapata/backend-enabled/${name}.conf\n"
                          "\n"
                          "[Install]\n"
                          "WantedBy=multi-user.target\n");

        std::string _after;
        std::string _requires;

        if (_conf["boot"][0]["depends"]->is_array()) {
            for (auto [_idx, _key, _dep] : _conf["boot"][0]["depends"]) {
                _after += std::string("After=") + std::string(_dep) + std::string(".service\n");
                _requires +=
                  std::string("Requires=") + std::string(_dep) + std::string(".service\n");
            }
        }
        zpt::replace(_sysd, "${dependencies}", _after);
        zpt::replace(_sysd, "${requirements}", _requires);
        zpt::replace(_sysd, "${name}", std::string(_conf["boot"][0]["name"]));

        if (_conf["boot"][0]["keep_alive"]->ok()) {
            zpt::replace(_sysd, "${keep_alive}", std::string(_conf["boot"][0]["keep_alive"]));
        }
        else { zpt::replace(_sysd, "${keep_alive}", "0"); }

        if (_conf["boot"][0]["fd_max"]->ok()) {
            zpt::replace(_sysd, "${fd_max}", std::string(_conf["boot"][0]["fd_max"]));
        }
        else { zpt::replace(_sysd, "${fd_max}", "0"); }

        if (_conf["boot"][0]["restart_policy"]->ok()) {
            zpt::replace(_sysd, "${restart}", std::string(_conf["boot"][0]["restart_policy"]));
        }
        else { zpt::replace(_sysd, "${restart}", "no"); }

        std::ofstream _sfs;
        _sfs.open((std::string("/lib/systemd/system/") + std::string(_conf["boot"][0]["name"]) +
                   std::string(".service"))
                    .data());
        if (_sfs.is_open()) {
            _sfs << _sysd << std::endl << std::flush;
            _sfs.close();
            std::cout << "> wrote /lib/systemd/system/" << std::string(_conf["boot"][0]["name"])
                      << ".service" << std::endl
                      << std::flush;
        }
        else {
            std::cout << "couldn't write to /lib/systemd/system/"
                      << std::string(_conf["boot"][0]["name"]) << ".service" << std::endl
                      << std::flush;
            exit(-1);
        }
    }
    else {
        std::cout << "no such file named /etc/zapata/backend-available/" << std::string(_to_add)
                  << ".conf" << std::endl
                  << std::flush;
        exit(-1);
    }
}

int
main(int argc, char* argv[]) {

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

        for (auto _to_add : _args["add"]->array()) { generate(_to_add, _global_conf); }
    }
    else if (_args["reconfigure"]) {
        zpt::json _global_conf;
        std::ifstream _zfs;
        _zfs.open((std::string("/etc/zapata/zapata.conf")).data());
        if (_zfs.is_open()) {
            zpt::json _conf;
            _zfs >> _conf;
            _zfs.close();
        }

        std::vector<std::string> _files;
        zpt::glob("/etc/zapata/backend-enabled/", _files, "(.*)\\.conf");

        for (auto _file : _files) {
            std::ifstream _ifs;
            _ifs.open(_file.data());
            if (_ifs.is_open()) {
                zpt::json _conf;
                _ifs >> _conf;
                _ifs.close();
                generate(_conf["$source"], _global_conf);
            }
            else {
                std::cout << "no such file named " << _file << std::endl << std::flush;
                exit(-1);
            }
        }
    }
    else if (_args["remove"]) {
        for (auto _to_remove : _args["remove"]->array()) {
            std::ifstream _ifs;
            _ifs.open((std::string("/etc/zapata/backend-available/") + std::string(_to_remove) +
                       std::string(".conf"))
                        .data());
            if (_ifs.is_open()) {
                zpt::json _conf;
                _ifs >> _conf;
                _ifs.close();

                if (system((std::string("rm -rf /etc/zapata/backend-enabled/") +
                            std::string(_conf["boot"][0]["name"]) + std::string(".conf"))
                             .data())) {}
                std::cout << "> removing /etc/zapata/backend-enabled/"
                          << std::string(_conf["boot"][0]["name"]) << ".conf" << std::endl
                          << std::flush;
                if (system((std::string("rm -rf /lib/systemd/system/") +
                            std::string(_conf["boot"][0]["name"]) + std::string(".service"))
                             .data())) {}
                std::cout << "> removing /lib/systemd/system/"
                          << std::string(_conf["boot"][0]["name"]) << ".service" << std::endl
                          << std::flush;
            }
        }
    }

    return 0;
}
