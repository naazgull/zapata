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

#include <zapata/text/convert.h>

#include <iomanip>
#include <sys/time.h>
#include <unistd.h>

auto
zpt::ascii::encode(std::string& _out, bool quote) -> void {
    auto wc = zpt::utf8::utf8_to_wstring(_out);
    std::wstring ws{ wc };

    for (size_t i = 0; i != iso.length(); i++) { std::replace(ws.begin(), ws.end(), iso[i], plain[i]); }

    std::ostringstream _oss;
    delete[] wc;
    for (size_t i = 0; i != ws.length(); i++) {
        if (((int)ws[i]) <= 127) { _oss << ((char)ws[i]) << std::flush; }
        else { _oss << " " << std::flush; }
    }
    _out.assign(_oss.str());
}

auto
zpt::generate::key(std::string& _out, size_t _size) -> void {
    static std::string charset = "abcdefghijklmnopqrstuvwxyz0123456789";
    timeval _tv = { 0 };

    for (size_t _idx = 0; _idx != _size; _idx++) {
        gettimeofday(&_tv, nullptr);
        srand(_tv.tv_usec);
        _out.append(1, charset[rand() % charset.length()]);
    }
}

auto
zpt::generate::r_key(size_t _size) -> std::string {
    std::string _out;
    zpt::generate::key(_out, _size);
    return _out;
}

auto
zpt::generate::r_key() -> std::string {
    std::string _out;
    zpt::generate::key(_out);
    return _out;
}

auto
zpt::generate::hash(std::string& _out) -> void {
    static std::string _charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    std::string _randompass;
    _randompass.resize(45);
    timeval _tv;
    gettimeofday(&_tv, nullptr);

    srand(_tv.tv_usec);
    for (int i = 0; i < 45; i++) {
        if (i % 10 == 0) { srand(_tv.tv_usec * i); }
        _randompass[i] = _charset[rand() % _charset.length()];
    };
    _out.insert(_out.length(), _randompass);
}

auto
zpt::generate::r_hash() -> std::string {
    std::string _out;
    zpt::generate::hash(_out);
    return _out;
}

auto
zpt::generate::uuid(std::string& _out) -> void {
    _out.append(zpt::generate::r_uuid());
}

auto
zpt::generate::r_uuid() -> std::string {
    static thread_local ::uuid uuid_gen;
    uuid_gen.make(UUID_MAKE_V1);
    auto _generated = uuid_gen.string();
    std::string _return{ _generated };
    delete _generated;
    return _return;
}

auto
zpt::test::uuid(std::string const& _uuid) -> bool {
    static const std::regex _uuid_rgx("^([a-fA-F0-9]{8})-"
                                      "([a-fA-F0-9]{4})-"
                                      "([a-fA-F0-9]{4})-"
                                      "([a-fA-F0-9]{4})-"
                                      "([a-fA-F0-9]{12})$");
    return std::regex_match(_uuid, _uuid_rgx);
}

auto
zpt::test::utf8(std::string const& _uri) -> bool {
    return true;
}

auto
zpt::test::ascii(std::string const& _ascii) -> bool {
    static const std::regex _ascii_rgx("^([a-zA-Z0-9_@:;./+*|-]+)$");
    return std::regex_match(_ascii, _ascii_rgx);
}

auto
zpt::test::token(std::string const& _token) -> bool {
    return true;
}

auto
zpt::test::uri(std::string _uri) -> bool {
    if (_uri.find(":") >= _uri.find("/")) { _uri = std::string("zpt:") + _uri; }
    static const std::regex _uri_rgx("([@>]{0,1})([a-zA-Z][a-zA-Z0-9+.-]+):" // scheme:
                                     "([^?#]*)"                              // authority and path
                                     "(?:\\?([^#]*))?"                       // ?query
                                     "(?:#(.*))?"                            // #fragment
    );
    return std::regex_match(_uri, _uri_rgx);
}

auto
zpt::test::email(std::string const& _email) -> bool {
    static const std::regex _email_rgx("([a-zA-Z0-9])([a-zA-Z0-9+._-]*)@"
                                       "([a-zA-Z0-9])([a-zA-Z0-9+._-]*)");
    return std::regex_match(_email, _email_rgx);
}

auto
zpt::test::phone(std::string const& _phone) -> bool {
    static const std::regex _phone_rgx("(?:\\(([0-9]){1,3}\\)([ ]*))?"
                                       "([0-9]){3,12}");
    return std::regex_match(_phone, _phone_rgx);
}

auto
zpt::test::regex(std::string const& _target, std::string const& _regex) -> bool {
    std::regex _rgx(_regex);
    return std::regex_match(_target, _rgx);
}

auto
zpt::test::timestamp(std::string const& _timestamp) -> bool {
    static const std::regex _timestamp_rgx("([0-9]){4}"
                                           "(?:[ /_-])?"
                                           "([0-9]){2}"
                                           "(?:[ /_-])?"
                                           "([0-9]){2}"
                                           "T"
                                           "([0-9]){2}"
                                           "(?:[ :])?"
                                           "([0-9]){2}"
                                           "(?:[ :])?"
                                           "([0-9]){2}"
                                           "(?:\\.([0-9]){3})?"
                                           "(?:[Z+-])?"
                                           "(?:[0-9]{2,4})?");
    return std::regex_match(_timestamp, _timestamp_rgx);
}
