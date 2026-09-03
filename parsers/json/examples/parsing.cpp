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

#include <chrono>
#include <fstream>
#include <iostream>
#include <semaphore.h>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <zapata/json.h>

namespace {
auto parse(std::string const& _text) -> zpt::json {
    zpt::json _out;
    _out.load_from(_text);
    return _out;
}

auto check(bool _cond, std::string const& _what) -> void {
    if (!_cond) {
        std::cerr << "FAILED: " << _what << std::endl;
        std::exit(1);
    }
    std::cout << "ok: " << _what << std::endl;
}

auto test_json_map() -> void {
    std::map<zpt::json, int> _map;

    _map[zpt::json{ "a" }] = 1;
    _map[zpt::json{ 1 }] = 2;
    _map[zpt::json{ "c", 10 }] = 3;
    _map[zpt::json{ "d" }] = 4;
    _map[zpt::json{ 2 }] = 5;
    _map[zpt::json{ zpt::array, 1, 2, 3 }] = 6;

    for (auto& [_key, _value] : _map) {
        std::cout << "map[" << _key << "] = " << _value << std::endl << std::flush;
    }

    check(_map.size() == 6, "JSON types can be used as map keys");
}

auto test_json_init() -> void {
    zpt::json _obj1 = zpt::json::object();
    _obj1["a"]["c"] = 1;
    _obj1["a"]["b"][0] = "hello";
    _obj1["a"]["b"][1] = "world";
    _obj1["a"]["b"][3] = "!!!";
    zpt::json _obj2 = zpt::json::object();
    _obj2["a"]["b"][0] = "hello";
    _obj2["a"]["b"][1] = "world";
    _obj2["a"]["b"][3] = "!!";
    _obj2["date"]["begin"] = zpt::timestamp("2000-12-11T16:09:54");
    check(_obj1->stringify() == R"({"a":{"b":["hello","world",null,"!!!"],"c":1}})",
          "implicit nested JSON element initialization");
    check(
      _obj2->stringify() ==
        R"({"a":{"b":["hello","world",null,"!!"]},"date":{"begin":"2000-12-11T16:09:54.000+00:00"}})",
      "implicit nested JSON element initialization");
}

auto test_primitives() -> void {
    check(parse("null")->ok() == false, "null parses to undefined/null");
    check(parse("true")->is_bool(), "true parses to bool true");
    check(parse("false")->is_bool(), "false parses to bool false");
    check(parse("\"true\"")->is_string(), "\"true\" parses to string true");
    check(parse("\"false\"")->is_string(), "\"false\" parses to string false");
    check(static_cast<long long>(parse("42")) == 42, "integer parses correctly");
    check(static_cast<double>(parse("3.14")) == 3.14, "double parses correctly");
    check(static_cast<std::string>(parse("\"hello\"")) == "hello", "double-quoted string parses");
    check(static_cast<std::string>(parse("'hello'")) == "hello", "single-quoted string parses");
}

auto test_object() -> void {
    zpt::json _obj = parse(R"({"name": "John", "age": 30})");
    check(_obj->type() == zpt::JSObject, "object parses to JSObject");
    check(static_cast<std::string>(_obj["name"]) == "John", "object string field parses");
    check(static_cast<long long>(_obj["age"]) == 30, "object integer field parses");
}

auto test_array() -> void {
    zpt::json _arr = parse("[1, 2, 3, \"four\"]");
    check(_arr->type() == zpt::JSArray, "array parses to JSArray");
    check(static_cast<long long>(_arr[0]) == 1, "array element 0 parses");
    check(static_cast<std::string>(_arr[3]) == "four", "array element 3 parses");
}

auto test_nested() -> void {
    zpt::json _doc = parse(R"({"a": {"b": [1, 2, {"c": 3}]}})");
    check(static_cast<long long>(_doc["a"]["b"][2]["c"]) == 3, "deeply nested structure parses");
}

auto test_escapes() -> void {
    check(static_cast<std::string>(parse(R"("a\nb")")) == "a\nb", "\\n escape decodes");
    check(static_cast<std::string>(parse(R"("a\tb")")) == "a\tb", "\\t escape decodes");
    check(static_cast<std::string>(parse(R"("a\\b")")) == "a\\b", "\\\\ escape decodes");
    check(static_cast<std::string>(parse(R"("a\"b")")) == "a\"b",
          "escaped quote inside string decodes");
}

auto test_unicode() -> void {
    // A is 'A'
    check(static_cast<std::string>(parse(R"("A")")) == "A", "unicode BMP escape decodes to ASCII");
    // é is 'é' (UTF-8: 0xC3 0xA9)
    std::string _e = static_cast<std::string>(parse(R"("é")"));
    check(_e.size() == 2 && static_cast<unsigned char>(_e[0]) == 0xC3 &&
            static_cast<unsigned char>(_e[1]) == 0xA9,
          "unicode escape decodes to correct UTF-8 bytes");
    // U+1F600 (GRINNING FACE) is outside the BMP; JSON encodes it as the
    // UTF-16 surrogate pair 😀 (UTF-8: 0xF0 0x9F 0x98 0x80)
    std::string _emoji = static_cast<std::string>(parse(R"("\uD83D\uDE00")"));
    check(_emoji.size() == 4 && static_cast<unsigned char>(_emoji[0]) == 0xF0 &&
            static_cast<unsigned char>(_emoji[1]) == 0x9F &&
            static_cast<unsigned char>(_emoji[2]) == 0x98 &&
            static_cast<unsigned char>(_emoji[3]) == 0x80,
          "surrogate pair escape decodes to correct 4-byte UTF-8 sequence");
    // Surrogate pair adjacent to plain text on both sides
    check(static_cast<std::string>(parse(R"("a\uD83D\uDE00b")")) == "a\xF0\x9F\x98\x80" "b",
          "surrogate pair decodes correctly surrounded by plain characters");
}

auto test_lambda_and_regex() -> void {
    zpt::json _l = parse(R"(lambda("my_func", 2))");
    check(_l->type() == zpt::JSLambda, "lambda literal parses to JSLambda");

    zpt::json _r = parse("/^[a-z]+$/");
    check(_r->type() == zpt::JSRegex, "regex literal parses to JSRegex");
}

auto test_deep_nesting() -> void {
    std::string _text;
    int _depth = 50;
    for (int _i = 0; _i != _depth; ++_i) { _text += "["; }
    _text += "1";
    for (int _i = 0; _i != _depth; ++_i) { _text += "]"; }

    zpt::json _doc = parse(_text);
    zpt::json _cur = _doc;
    for (int _i = 0; _i != _depth - 1; ++_i) { _cur = _cur[0]; }
    check(static_cast<long long>(_cur[0]) == 1, "deeply nested array parses to correct depth");
}

auto test_stringify_roundtrip() -> void {
    zpt::json _doc = parse(R"({"a": [1, 2.5, "three", true, null], "b": {"c": "d"}})");
    std::ostringstream _oss;
    _oss << _doc;
    zpt::json _roundtrip = parse(_oss.str());
    check(static_cast<long long>(_roundtrip["a"][0]) == 1, "stringify/reparse preserves integer");
    check(static_cast<std::string>(_roundtrip["a"][2]) == "three",
          "stringify/reparse preserves string");
    check(static_cast<std::string>(_roundtrip["b"]["c"]) == "d",
          "stringify/reparse preserves nested object");
}

auto test_syntax_error() -> void {
    bool _threw = false;
    try {
        parse("{invalid");
    }
    catch (zpt::SyntaxErrorException const&) {
        _threw = true;
    }
    check(_threw, "malformed JSON throws SyntaxErrorException");
}

auto test_stops_at_value_boundary() -> void {
    // Parsing must stop as soon as one complete value has been read, not
    // consume the rest of the stream looking for EOF - otherwise a second
    // back-to-back value (or trailing bytes) gets silently swallowed.
    {
        std::istringstream _iss(R"({"a":1}{"b":2})");
        zpt::json _first, _second;
        _iss >> _first;
        _iss >> _second;
        check(static_cast<long long>(_first["a"]) == 1 && static_cast<long long>(_second["b"]) == 2,
              "back-to-back objects parse independently");
    }
    {
        std::istringstream _iss(R"([1,2,3][4,5,6])");
        zpt::json _first, _second;
        _iss >> _first;
        _iss >> _second;
        check(static_cast<long long>(_first[0]) == 1 && static_cast<long long>(_second[0]) == 4,
              "back-to-back arrays parse independently");
    }
    {
        std::istringstream _iss(R"(42 99)");
        zpt::json _first;
        _iss >> _first;
        std::string _rest;
        std::getline(_iss, _rest);
        check(static_cast<long long>(_first) == 42 && _rest == " 99",
              "bare integer stops at value boundary, leaving trailing bytes unconsumed");
    }
    {
        // Forces fill()'s buffer shift/resize path before the second value,
        // which is what originally left a stale __data_limit and caused
        // syncBackToStream() to over-consume into the next value.
        std::string _big = "[";
        for (int _i = 0; _i != 2000; ++_i) { _big += "1,"; }
        _big += "1]";
        std::string _doc = _big + R"({"next":true})";
        std::istringstream _iss(_doc);
        zpt::json _first, _second;
        _iss >> _first;
        _iss >> _second;
        check(static_cast<bool>(_second["next"]) == true,
              "large array followed by object parses correctly across buffer growth");
    }
}

} // namespace

int main() {
    ::test_json_map();
    ::test_json_init();
    ::test_primitives();
    ::test_object();
    ::test_array();
    ::test_nested();
    ::test_escapes();
    ::test_unicode();
    ::test_lambda_and_regex();
    ::test_deep_nesting();
    ::test_stringify_roundtrip();
    ::test_syntax_error();
    ::test_stops_at_value_boundary();

    std::cout << "all tests passed" << std::endl;
    return 0;
}
