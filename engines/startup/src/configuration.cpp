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

#include <zapata/startup/configuration.h>

auto zpt::startup::configuration::load(zpt::json _parameters, zpt::json& _output) -> void {
    for (auto [_, __, _conf_file] : _parameters["--conf-file"]) {
        try {
            zpt::conf::file(static_cast<std::string>(_conf_file), _output);
        }
        catch (zpt::failed_expectation const& _e) {
            zlog("Found " << _e, zpt::emergency);
        }
    }
    for (auto [_, __, _conf_dir] : _parameters["--conf-dir"]) {
        try {
            zpt::conf::dirs(static_cast<std::string>(_conf_dir), _output);
        }
        catch (zpt::failed_expectation const& _e) {
            zlog("Found " << _e, zpt::emergency);
        }
    }
    zpt::conf::dirs(_output);
    zpt::conf::env(_output);
}
