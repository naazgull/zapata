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

auto zpt::ontology::to_str(zpt::performative _performative) -> std::string {
    switch (_performative) {
        case zpt::Get: {
            return "GET";
        }
        case zpt::Put: {
            return "PUT";
        }
        case zpt::Post: {
            return "POST";
        }
        case zpt::Delete: {
            return "DELETE";
        }
        case zpt::Head: {
            return "HEAD";
        }
        case zpt::Options: {
            return "OPTIONS";
        }
        case zpt::Patch: {
            return "PATCH";
        }
        case zpt::Reply: {
            return "REPLY";
        }
        case zpt::Msearch: {
            return "M-SEARCH";
        }
        case zpt::Notify: {
            return "NOTIFY";
        }
        case zpt::Trace: {
            return "TRACE";
        }
        case zpt::Connect: {
            return "CONNECT";
        }
    }
    return "HEAD";
}

auto zpt::ontology::from_str(std::string _performative) -> zpt::performative {
    std::transform(_performative.begin(), _performative.end(), _performative.begin(), ::toupper);
    if (_performative == "GET") { return zpt::Get; }
    if (_performative == "PUT") { return zpt::Put; }
    if (_performative == "POST") { return zpt::Post; }
    if (_performative == "DELETE") { return zpt::Delete; }
    if (_performative == "HEAD") { return zpt::Head; }
    if (_performative == "OPTIONS") { return zpt::Options; }
    if (_performative == "PATCH") { return zpt::Patch; }
    if (_performative == "REPLY") { return zpt::Reply; }
    if (_performative == "M-SEARCH") { return zpt::Msearch; }
    if (_performative == "NOTIFY") { return zpt::Notify; }
    if (_performative == "TRACE") { return zpt::Msearch; }
    if (_performative == "CONNECT") { return zpt::Connect; }
    return 0;
}
