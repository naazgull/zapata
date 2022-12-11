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

#include <zapata/base/performative.h>
#include <algorithm>

namespace {
inline constexpr char const* STATUS_NAMES[] = { "GET",      "PUT",     "POST",  "DELETE",
                                                "HEAD",     "OPTIONS", "PATCH", "REPLY",
                                                "M-SEARCH", "NOTIFY",  "TRACE", "CONNECT" };
}

auto zpt::ontology::to_str(zpt::performative _performative) -> std::string {
    if (_performative < zpt::Performative_end) {
        return std::string{ ::STATUS_NAMES[_performative] };
    }
    return "HEAD";
}

auto zpt::ontology::from_str(std::string _performative) -> zpt::performative {
    std::transform(_performative.begin(), _performative.end(), _performative.begin(), ::toupper);
    for (size_t _idx = 0; _idx != zpt::Performative_end; ++_idx) {
        if (_performative == ::STATUS_NAMES[_idx]) { return _idx; }
    }
    return zpt::Head;
}
