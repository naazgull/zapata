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

#include <iostream>
#include <string>
#include <zapata/uri.h>

namespace {
auto check(bool _cond, std::string const& _what) -> void {
    if (!_cond) {
        std::cerr << "FAILED: " << _what << std::endl;
        std::exit(1);
    }
    std::cout << "ok: " << _what << std::endl;
}

// Round-trip: parse → stringify → parse again; both sides must agree.
auto check_roundtrip(std::string const& _input, std::string const& _label) -> void {
    auto _parsed = zpt::uri::parse(_input);
    auto _repr = zpt::uri::to_string(_parsed);
    auto _reparsed = zpt::uri::parse(_repr);
    check(zpt::uri::to_string(_reparsed) == _repr, "roundtrip: " + _label);
}

} // namespace

int main() {
    // Wildcard
    check_roundtrip("*", "wildcard");

    // Root path
    check_roundtrip("/", "root path");

    // Host with port
    check_roundtrip("localhost:8080", "host with port");

    // Relative path with scheme
    check_roundtrip("file:./users.json", "relative path with scheme");

    // Placeholders in path
    check_roundtrip("{(.*)}:/{(.*)}", "placeholders in path");
    check_roundtrip("/home/pf/{tmp:/(.*)/}/a.txt", "nested placeholder in path");

    // Relative paths (./ and ../)
    check_roundtrip("./pf/{tmp:/(.*)/}/a.txt", "dot-relative path");
    check_roundtrip("../pf/{tmp:/(.*)/}/a.txt", "dotdot-relative path");

    // Scheme options (+)
    check_roundtrip("file+json:/home/pf/{tmp:/(.*)/}/a.j", "scheme with options");
    check_roundtrip("file+json:./pf/{tmp:/(.*)/}/a.j", "scheme options + relative");
    check_roundtrip("file+json:../pf/{tmp:/(.*)/}/a.j", "scheme options + dotdot");

    // Anchor
    check_roundtrip("file+json:/home/pf/{tmp:/(.*)/}/a.j#some_point_in_doc",
                    "scheme + path + anchor");

    // Multi-option scheme
    check_roundtrip("tcp+json+ssl:/home/pf/{tmp:/(.*)/}/a.j#some_point_in_doc",
                    "multi-option scheme");

    // Dotdot in absolute path
    check_roundtrip("tcp+json+ssl:/home/pf/../{tmp:/(.*)/}/a.j#some_point_in_doc",
                    "dotdot in absolute path");

    // Alternation placeholders
    check_roundtrip("{/http|ftp/}://api:8081/2.0/users/me?a=2&b=3&c=",
                    "alternation placeholder + params");
    check_roundtrip("{/http|ftp/}://api:8081/2.0/users/me?a=2&b=3&c=#some_point_in_doc",
                    "alternation placeholder + params + anchor");

    // Params edge cases
    check_roundtrip("?a=2&b=3&c=1&d=lower(Strong)", "params without scheme/path");
    check_roundtrip("?a=2&b=3&c=1&d=", "param with empty value");

    // User info
    check_roundtrip("http://pf@na.zgul.me/api/2.0/users/me?a=2&b=3&c=",
                    "user info + host + path + params");

    // Placeholder in params value
    check_roundtrip(
      "http://pf@na.zgul.me/api/2.0/users/"
      "me?a=2&b=3&c=&d={.ge(integer(0,2),float(1,2,3)).}",
      "placeholder function in param value");

    check_roundtrip(
      "http://pf@na.zgul.me/api/2.0/users/"
      "me?a=2&b=3&c=&d=integer(0,2)&e={.ge(integer(0,2),float(1,2,3)).}",
      "multiple params with placeholder");

    // FTP with user info
    check_roundtrip("ftp://pf@na.zgul.me/files/movies", "ftp with user info");

    // Anchor only
    check_roundtrip("#some_point_in_doc", "anchor only");

    // Placeholder in query param
    check_roundtrip("http://localhost:8080/users?name={.lower(u).}",
                    "placeholder in query param");

    std::cout << "all tests passed" << std::endl;
    return 0;
}
