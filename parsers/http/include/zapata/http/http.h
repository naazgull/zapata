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

#pragma once



#include <string>
#include <zapata/http/HTTPObj.h>
#include <zapata/http/HTTPParser.h>

namespace zpt {
namespace http {

auto
from_file(std::ifstream& _in, zpt::HTTPReq& _out) -> zpt::HTTPReq&;
auto
from_file(std::ifstream& _in, zpt::HTTPRep& _out) -> zpt::HTTPRep&;
auto
from_stream(std::istream& _in, zpt::HTTPReq& _out) -> zpt::HTTPReq&;
auto
from_stream(std::istream& _in, zpt::HTTPRep& _out) -> zpt::HTTPRep&;
auto
from_str(std::string& _in, zpt::HTTPReq& _out) -> zpt::HTTPReq&;
auto
from_str(std::string& _in, zpt::HTTPRep& _out) -> zpt::HTTPRep&;

auto
to_str(std::string& _out, HTTPReq& _in) -> void;
auto
to_str(std::string& _out, HTTPRep& _in) -> void;
auto
to_str(std::ostream& _out, HTTPReq& _in) -> void;
auto
to_str(std::ostream& _out, HTTPRep& _in) -> void;

}
} // namespace zpt
