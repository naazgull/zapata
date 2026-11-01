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

#include <zapata/os/ps.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

/** @brief Strips leading and trailing whitespace from `_in`. */
auto trim(std::string const& _in) -> std::string {
    auto _begin = _in.find_first_not_of(" \t\r\n");
    if (_begin == std::string::npos) { return {}; }
    auto _end = _in.find_last_not_of(" \t\r\n");
    return _in.substr(_begin, _end - _begin + 1);
}

/** @brief Reads whitespace-separated integers out of a `/proc` status value. */
auto parse_ids(std::string const& _in) -> std::vector<long long> {
    std::vector<long long> _ids;
    std::istringstream _stream{ _in };
    long long _id = 0;
    while (_stream >> _id) { _ids.push_back(_id); }
    return _ids;
}

/** @brief True when `_name` is a `/proc` numeric entry, i.e. a process id. */
auto is_pid(std::string const& _name) -> bool {
    return !_name.empty() &&
           std::all_of(_name.begin(),
                       _name.end(),
                       [](char _c) { return std::isdigit(static_cast<unsigned char>(_c)) != 0; });
}

/** @brief Reads `/proc/<pid>/cmdline`, joining the NUL-separated argv with spaces. */
auto read_cmdline(std::filesystem::path const& _dir) -> std::string {
    std::ifstream _in{ _dir / "cmdline" };
    if (!_in.is_open()) { return {}; }
    std::string _raw{ std::istreambuf_iterator<char>{ _in }, std::istreambuf_iterator<char>{} };
    std::string _cmdline;
    _cmdline.reserve(_raw.size());
    for (char _c : _raw) {
        if (_c == '\0') {
            if (!_cmdline.empty() && _cmdline.back() != ' ') { _cmdline += ' '; }
        }
        else { _cmdline += _c; }
    }
    return trim(_cmdline);
}

/**
 * @brief Parses one `/proc/<pid>` entry and appends it to `_processes` if it passes the filters.
 *
 * Silently returns when the process exited between the directory scan and the
 * open (an expected `/proc` race), when its `status` is unreadable, or when it
 * does not match `_uid`/`_gid`. A filter set to the `static_cast<...>(-1)`
 * sentinel matches every process.
 */
auto append_process(zpt::json& _processes,
                    std::filesystem::path const& _dir,
                    uid_t _uid,
                    gid_t _gid) -> void {
    std::ifstream _status{ _dir / "status" };
    if (!_status.is_open()) { return; }

    std::string _name;
    std::string _state;
    long long _pid = 0;
    long long _ppid = 0;
    std::vector<long long> _uids;
    std::vector<long long> _gids;

    std::string _line;
    while (std::getline(_status, _line)) {
        auto _colon = _line.find(':');
        if (_colon == std::string::npos) { continue; }
        std::string_view _key{ _line.data(), _colon };
        std::string _value = _line.substr(_colon + 1);
        if (_key == "Name") { _name = trim(_value); }
        else if (_key == "State") {
            std::istringstream _stream{ _value };
            _stream >> _state;
        }
        else if (_key == "Pid") {
            std::istringstream _stream{ _value };
            _stream >> _pid;
        }
        else if (_key == "PPid") {
            std::istringstream _stream{ _value };
            _stream >> _ppid;
        }
        else if (_key == "Uid") { _uids = parse_ids(_value); }
        else if (_key == "Gid") { _gids = parse_ids(_value); }
    }

    if (_uids.size() < 2 || _gids.size() < 2) { return; }

    auto _real_uid = static_cast<uid_t>(_uids[0]);
    auto _real_gid = static_cast<gid_t>(_gids[0]);
    if (_uid != static_cast<uid_t>(-1) && _real_uid != _uid) { return; }
    if (_gid != static_cast<gid_t>(-1) && _real_gid != _gid) { return; }

    _processes << zpt::json{ "pid",     _pid,        "ppid",   _ppid,
                             "name",    _name,       "state",  _state,
                             "uid",     _uids[0],    "euid",   _uids[1],
                             "gid",     _gids[0],    "egid",   _gids[1],
                             "cmdline", read_cmdline(_dir) };
}

} // namespace

auto zpt::ps(uid_t _uid, gid_t _gid) -> zpt::json {
    auto _processes = zpt::json::array();
    for (auto const& _entry : std::filesystem::directory_iterator{ "/proc" }) {
        if (!::is_pid(_entry.path().filename().string())) { continue; }
        ::append_process(_processes, _entry.path(), _uid, _gid);
    }
    return _processes;
}
