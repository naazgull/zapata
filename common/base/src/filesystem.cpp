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

#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zapata/base/expect.h>
#include <zapata/file/manip.h>

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
