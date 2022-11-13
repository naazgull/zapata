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

#include <zapata/base.h>
#include <zapata/globals.h>

auto zpt::globals::to_string() -> std::string {
    zpt::locks::spin_lock::guard _sentry{ zpt::globals::__variables_lock, zpt::locks::spin_lock::shared };
    std::ostringstream _out;
    _out << "Global variables:" << std::endl;
    for (auto [_key, _value] : zpt::globals::__variables) {
        _out << _key << ":" << std::endl << std::flush;
        for (auto _variable : _value) { _out << "\t- " << _variable << std::endl << std::flush; }
    }
    return _out.str();
}

auto zpt::thread_local_table::to_string() -> std::string {
    std::ostringstream _out;
    _out << "Thread local members:" << std::endl;
    for (auto& [_key, _value] : zpt::thread_local_table::__variables) {
        _out << _key << ": " << _value << std::endl << std::flush;
    }
    return _out.str();
}
