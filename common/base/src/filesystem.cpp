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

#include <zapata/base/expect.h>
#include <zapata/file/manip.h>

#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

// auto zpt::ls(std::string dir, std::vector<std::string>& result, bool recursive) -> int {
//     DIR* dp{ nullptr };
//     struct dirent* dirp{ nullptr };
//     if ((dp = ::opendir(dir.c_str())) == nullptr) { return errno; }

//     while ((dirp = ::readdir(dp)) != nullptr) {
//         std::string cname{ dirp->d_name };
//         if (cname.find('.') != 0) {
//             cname.insert(0, "/");
//             cname.insert(0, dir);
//             result.push_back(cname);
//             if (recursive) { zpt::ls(cname, result, true); }
//         }
//     }

//     ::closedir(dp);
//     return 0;
// }

// auto zpt::mkdir_recursive(std::string const& _name) -> bool {
//     std::istringstream _iss(_name);
//     std::string _line;
//     auto _count{ 0 };
//     std::ostringstream _dname;

//     while (_iss.good()) {
//         std::getline(_iss, _line, '/');
//         _dname << _line << std::flush;
//         if (mkdir(_dname.str().data(), 0755) == 0) { _count++; }
//         _dname << "/" << std::flush;
//     }
//     return _count != 0;
// }

// auto zpt::copy_path(std::string const& _from, std::string const& _to) -> bool {
//     auto _read_fd{ 0 };
//     auto _write_fd{ 0 };
//     struct stat _stat_buf;

//     _read_fd = open(_from.c_str(), O_RDONLY);
//     if (_read_fd < 0) { return _read_fd; }
//     fstat(_read_fd, &_stat_buf);
//     _write_fd = open(_to.c_str(), O_WRONLY | O_CREAT, _stat_buf.st_mode);
//     auto _error = sendfile(_write_fd, _read_fd, nullptr, _stat_buf.st_size);
//     close(_read_fd);
//     close(_write_fd);

//     return _error != -1;
// }

// auto zpt::move_path(std::string const& _from, std::string const& _to) -> bool {
//     if (zpt::copy_path(_from, _to)) { return std::remove(_from.c_str()) != 0; }
//     return false;
// }

// auto zpt::load_path(std::string const& _in, std::string& _out) -> bool {
//     std::ifstream _ifs;
//     _ifs.open(_in.data());

//     if (_ifs.is_open()) {
//         _ifs.seekg(0, std::ios::end);
//         _out.reserve(_ifs.tellg());
//         _ifs.seekg(0, std::ios::beg);
//         _out.assign((std::istreambuf_iterator<char>(_ifs)), std::istreambuf_iterator<char>());
//         _ifs.close();
//         return true;
//     }
//     return false;
// }

// auto zpt::load_path(std::string const& _in, std::wstring& _out) -> bool {
//     std::wifstream _ifs;
//     _ifs.open(_in.data());

//     if (_ifs.is_open()) {
//         _ifs.seekg(0, std::ios::end);
//         _out.reserve(_ifs.tellg());
//         _ifs.seekg(0, std::ios::beg);
//         _out.assign((std::istreambuf_iterator<wchar_t>(_ifs)), std::istreambuf_iterator<wchar_t>());
//         _ifs.close();
//         return true;
//     }
//     return false;
// }

// auto zpt::dump_path(std::string const& _in, std::string& _content) -> bool {
//     std::ofstream _ofs;
//     _ofs.open(_in.data());
//     _ofs << _content << std::flush;
//     _ofs.flush();
//     _ofs.close();
//     return true;
// }

// auto zpt::dump_path(std::string const& _in, std::wstring& _content) -> bool {
//     std::wofstream _ofs;
//     _ofs.open(_in.data());
//     _ofs << _content << std::flush;
//     _ofs.flush();
//     _ofs.close();
//     return true;
// }

auto zpt::globRegexp(std::string& dir,
                     std::vector<std::string>& result,
                     std::regex& pattern,
                     short recursion) -> int {
    DIR* dp{ nullptr };
    struct dirent* dirp{ nullptr };
    std::vector<std::string> torecurse;

    if ((dp = opendir(dir.c_str())) == nullptr) { return errno; }
    while ((dirp = readdir(dp)) != nullptr) {
        std::string cname{ dirp->d_name };
        if (cname.find('.') != 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
            if (dir[dir.length() - 1] != '/') { cname.insert(0, "/"); }
            cname.insert(0, dir.data());
#pragma GCC diagnostic pop
            if (recursion != 0 && dirp->d_type == 4 && cname != dir) { torecurse.push_back(cname); }
            if (std::regex_match(std::string(dirp->d_name), pattern)) {
                result.insert(result.begin(), cname);
            }
        }
    }
    closedir(dp);

    for (auto i : torecurse) { zpt::globRegexp(i, result, pattern, recursion - 1); }

    return 0;
}

auto zpt::glob(std::string dir,
               std::vector<std::string>& result,
               std::string pattern,
               short recursion) -> int {
    std::regex regexp(pattern);
    return zpt::globRegexp(dir, result, regexp, recursion);
}

// auto zpt::is_dir(std::string const& _path) -> bool {
//     struct stat _s = { 0 };
//     if (::stat(_path.data(), &_s) == 0) { return _s.st_mode & S_IFDIR; }
//     return false;
// }

// auto zpt::file_exists(std::string const& _path) -> bool {
//     struct stat _s = { 0 };
//     if (::stat(_path.data(), &_s) == 0) { return true; }
//     return false;
// }

// auto zpt::dirname(std::string const& _path) -> std::string {
//     if (zpt::is_dir(_path)) { return _path; }

//     auto _idx = _path.rfind("/");
//     if (_idx == std::string::npos) { return "./"; }
//     return _path.substr(0, _idx + 1);
// }
