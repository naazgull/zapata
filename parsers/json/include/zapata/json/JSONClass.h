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

#include <chrono>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <unordered_map>
#include <vector>
#include <variant>
#include <zapata/base/expect.h>

#include <zapata/log/log.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

namespace zpt {
using timestamp_t = unsigned long long;

enum JSONType {
    JSNil = 0,
    JSBoolean = 1,
    JSInteger = 2,
    JSDouble = 3,
    JSString = 4,
    JSDate = 5,
    JSArray = 6,
    JSObject = 7,
    JSRegex = 8,
    JSLambda = 9,
    JSUndefined = 10
};

auto to_string(zpt::JSONType _type) -> std::string;

class JSONElementT;
class JSONObj;
class JSONArr;
class JSONLambda;
class JSONRegex;
class JSONIterator;
class lambda;
class json;

using regex = JSONRegex;
} // namespace zpt

namespace zpt {
class pretty {
  public:
    pretty(const pretty& _rhs);
    pretty(pretty&& _rhs);
    pretty(std::string const& _rhs);
    pretty(const char* _rhs);
    template<typename T>
    pretty(T _rhs);
    virtual ~pretty() = default;

    operator std::string();

    auto operator=(const pretty& _rhs) -> pretty&;
    auto operator=(pretty&& _rhs) -> pretty&;

    auto operator->() -> std::string*;
    auto operator*() -> std::string&;

    friend auto operator<<(std::ostream& _out, zpt::pretty _in) -> std::ostream& {
        _out << std::string(_in.__underlying.data());
        return _out;
    }

  private:
    std::string __underlying{ "" };
};
} // namespace zpt

namespace zpt {
class json {
  public:
    using map = std::map<std::string, zpt::json>;
    using element = std::tuple<size_t, std::string, zpt::json>;
    using iterator = zpt::JSONIterator;
    using const_iterator = const zpt::JSONIterator;

    using traverse_callback =
      std::function<void(std::string const&, zpt::json, std::string const&)>;

    json();
    json(std::nullptr_t _rhs);
    json(std::unique_ptr<zpt::JSONElementT> _target);
    json(std::initializer_list<zpt::json> _init);
    json(zpt::json const& _rhs);
    json(zpt::json&& _rhs);
    template<typename T>
    json(T const& _rhs);
    virtual ~json();

    auto size() const -> size_t;
    auto hash() const -> size_t;
    auto value() -> zpt::JSONElementT&;
    auto load_from(std::string const& _in) -> zpt::json&;
    auto load_from(std::istream& _in) -> zpt::json&;
    auto stringify(std::ostream& _out) -> zpt::json&;
    auto stringify(std::string& _out) -> zpt::json&;
    auto stringify(std::ostream& _out) const -> zpt::json const&;
    auto stringify(std::string& _out) const -> zpt::json const&;

    auto begin() -> zpt::json::iterator;
    auto end() -> zpt::json::iterator;
    auto begin() const -> zpt::json::const_iterator;
    auto end() const -> zpt::json::const_iterator;

    auto operator=(zpt::json const& _rhs) -> zpt::json&;
    auto operator=(zpt::json&& _rhs) -> zpt::json&;
    auto operator=(std::tuple<size_t, std::string, zpt::json> _rhs) -> zpt::json&;
    auto operator=(std::initializer_list<zpt::json> _list) -> zpt::json&;
    template<typename T>
    auto operator=(T const& _rhs) -> zpt::json&;

    auto operator->() -> zpt::JSONElementT*;
    auto operator*() -> zpt::JSONElementT&;
    auto operator->() const -> zpt::JSONElementT const*;
    auto operator*() const -> zpt::JSONElementT const&;

    auto operator==(std::tuple<size_t, std::string, zpt::json> _rhs) const -> bool;
    auto operator!=(std::tuple<size_t, std::string, zpt::json> _rhs) const -> bool;
    auto operator==(std::nullptr_t _rhs) const -> bool;
    auto operator!=(std::nullptr_t _rhs) const -> bool;
    auto operator<<(std::initializer_list<zpt::json> _in) -> json&;
    template<typename T>
    auto operator==(T _rhs) const -> bool;
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    template<typename T>
    auto operator<<(T _in) -> json&;
    // template<typename T>
    // auto operator>>(T _in) -> json&;
    template<typename T>
    auto operator[](T _idx) -> json&;
    template<typename T>
    auto operator[](T _idx) const -> zpt::json const;
    template<typename T>
    auto operator()(T _idx) const -> zpt::json const;

    operator std::string();
    operator bool();
    operator int();
    operator long();
    operator long long();
    operator size_t();
    operator double();
#ifdef __LP64__
    operator unsigned int();
#endif
    operator zpt::timestamp_t();
    operator zpt::JSONObj();
    operator zpt::JSONArr();
    operator zpt::JSONObj&();
    operator zpt::JSONArr&();
    operator zpt::lambda();
    operator zpt::regex();
    operator zpt::regex&();
    operator std::regex&();

    operator std::string() const;
    operator bool() const;
    operator int() const;
    operator long() const;
    operator long long() const;
    operator size_t() const;
    operator double() const;
#ifdef __LP64__
    operator unsigned int() const;
#endif
    operator zpt::timestamp_t() const;
    operator zpt::JSONObj() const;
    operator zpt::JSONArr() const;
    operator zpt::JSONObj&() const;
    operator zpt::JSONArr&() const;
    operator zpt::lambda() const;
    operator zpt::regex() const;
    operator zpt::regex&() const;
    operator std::regex&() const;

    auto operator+(std::initializer_list<zpt::json> _in) const -> json;
    auto operator+=(std::initializer_list<zpt::json> _in) -> json&;
    auto operator-(std::initializer_list<zpt::json> _in) const -> json;
    auto operator-=(std::initializer_list<zpt::json> _in) -> json&;
    auto operator/(std::initializer_list<zpt::json> _in) const -> json;
    auto operator|(std::initializer_list<zpt::json> _in) const -> json;
    auto operator|=(std::initializer_list<zpt::json> _in) -> json&;
    auto operator&(std::initializer_list<zpt::json> _in) const -> json;
    auto operator&=(std::initializer_list<zpt::json> _in) -> json&;
    auto operator+(zpt::json _rhs) const -> json;
    auto operator+=(zpt::json _rhs) -> json&;
    auto operator-(zpt::json _rhs) const -> json;
    auto operator-=(zpt::json _rhs) -> json&;
    auto operator/(zpt::json _rhs) const -> json;
    auto operator|(zpt::json _rhs) const -> json;
    auto operator|=(zpt::json _rhs) -> json&;
    auto operator&(zpt::json _rhs) const -> json;
    auto operator&=(zpt::json _rhs) -> json&;

    friend auto operator>>(std::istream& _in, zpt::json& _out) -> std::istream& {
        _out.load_from(_in);
        return _in;
    }

    friend auto operator<<(std::ostream& _out, zpt::json _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

    template<typename T>
    static auto data(const T _delegate) -> zpt::json;
    static auto parse_json_str(std::string const& _in) -> zpt::json;
    static auto to_unicode(std::string& _str) -> void;
    static auto object() -> zpt::json;
    static auto array() -> zpt::json;
    template<typename T>
    static auto pretty(T _e) -> std::string;
    template<typename T>
    static auto string(T _e) -> zpt::json;
    template<typename T>
    static auto uinteger(T _e) -> zpt::json;
    template<typename T>
    static auto integer(T _e) -> zpt::json;
    template<typename T>
    static auto floating(T _e) -> zpt::json;
    template<typename T>
    static auto ulong(T _e) -> zpt::json;
    template<typename T>
    static auto boolean(T _e) -> zpt::json;
    static auto date(std::string const& _e) -> zpt::json;
    static auto date() -> zpt::json;
    template<typename T>
    static auto date(T _e) -> zpt::json;
    template<typename T>
    static auto lambda(T _e) -> zpt::json;
    static auto lambda(std::string const& _name, unsigned short _n_args) -> zpt::json;
    template<typename T>
    static auto regex(T _e) -> zpt::json;

    static auto type_of(std::string const& _value) -> zpt::JSONType;
    static auto type_of(bool _value) -> zpt::JSONType;
    static auto type_of(int _value) -> zpt::JSONType;
    static auto type_of(long _value) -> zpt::JSONType;
    static auto type_of(long long _value) -> zpt::JSONType;
    static auto type_of(size_t _value) -> zpt::JSONType;
    static auto type_of(double _value) -> zpt::JSONType;
#ifdef __LP64__
    static auto type_of(unsigned int _value) -> zpt::JSONType;
#endif
    static auto type_of(zpt::JSONElementT& _value) -> zpt::JSONType;
    static auto type_of(zpt::timestamp_t _value) -> zpt::JSONType;
    static auto type_of(zpt::pretty _value) -> zpt::JSONType;
    static auto type_of(zpt::JSONObj _value) -> zpt::JSONType;
    static auto type_of(zpt::JSONArr _value) -> zpt::JSONType;
    static auto type_of(zpt::JSONObj& _value) -> zpt::JSONType;
    static auto type_of(zpt::JSONArr& _value) -> zpt::JSONType;
    static auto type_of(zpt::lambda _value) -> zpt::JSONType;
    static auto type_of(zpt::regex& _value) -> zpt::JSONType;
    static auto type_of(zpt::json& _value) -> zpt::JSONType;

    static auto traverse(zpt::json _document, zpt::json::traverse_callback _callback) -> void;
    static auto flatten(zpt::json _document) -> zpt::json;
    static auto find(zpt::json::iterator _begin, zpt::json::iterator _end, zpt::json _to_find)
      -> zpt::json::iterator;
    static auto find(zpt::JSONElementT const& _to_search, zpt::json _to_find)
      -> zpt::json::iterator;
    static auto contains(zpt::JSONElementT const& _to_search, zpt::json _to_find) -> bool;

  private:
    std::shared_ptr<zpt::JSONElementT> __underlying{ nullptr };

    json(std::tuple<size_t, std::string, zpt::json> _rhs);
    auto strict_union(zpt::json _rhs) -> void;
    auto strict_intersection(zpt::json _rhs) -> void;

    static auto traverse(zpt::json _document,
                         zpt::json::traverse_callback _callback,
                         std::string _path) -> void;
};
} // namespace zpt

namespace zpt {
class JSONIterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = zpt::json::element;
    using pointer = zpt::json::element;
    using reference = zpt::json::element;
    using iterator_category = std::bidirectional_iterator_tag;

    explicit JSONIterator(zpt::json const& _target, size_t _pos);
    JSONIterator(JSONIterator const& _rhs);
    JSONIterator(JSONIterator&& _rhs);
    virtual ~JSONIterator() = default;

    // BASIC ITERATOR METHODS //
    auto operator=(JSONIterator const& _rhs) -> JSONIterator&;
    auto operator=(JSONIterator&& _rhs) -> JSONIterator&;
    auto operator++() -> JSONIterator&;
    auto operator*() -> reference;
    // END / BASIC ITERATOR METHODS //

    // INPUT ITERATOR METHODS //
    auto operator++(int) -> JSONIterator;
    auto operator->() -> pointer;
    auto operator==(JSONIterator const& _rhs) const -> bool;
    auto operator!=(JSONIterator const& _rhs) const -> bool;
    // END / INPUT ITERATOR METHODS //

    // OUTPUT ITERATOR METHODS //
    // reference operator*(); <- already defined
    // iterator operator++(int); <- already defined
    // END / OUTPUT ITERATOR METHODS //

    // FORWARD ITERATOR METHODS //
    // Enable support for both input and output iterator <- already enabled
    // END / FORWARD ITERATOR METHODS //

    // BIDIRECTIOANL ITERATOR METHODS //
    auto operator--() -> JSONIterator&;
    auto operator--(int) -> JSONIterator;
    // END / BIDIRECTIOANL ITERATOR METHODS //

    friend auto operator<<(std::ostream& _out, zpt::JSONIterator& _in) -> std::ostream& {
        _out << _in.__index << std::flush;
        return _out;
    }

  private:
    zpt::json __target;
    size_t __index;
    zpt::json::map::const_iterator __iterator;
};
} // namespace zpt

namespace zpt {
inline zpt::json undefined{ zpt::JSUndefined };
inline zpt::json array;
} // namespace zpt

namespace zpt {
class JSONObjT {
  public:
    JSONObjT();
    virtual ~JSONObjT();

    virtual auto stringify(std::string& _out) -> zpt::JSONObjT&;
    virtual auto stringify(std::ostream& _out) -> zpt::JSONObjT&;
    virtual auto stringify(std::string& _out) const -> zpt::JSONObjT const&;
    virtual auto stringify(std::ostream& _out) const -> zpt::JSONObjT const&;

    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> zpt::JSONObjT const&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> zpt::JSONObjT const&;

    virtual auto push(std::string const& _name) -> zpt::JSONObjT&;
    virtual auto push(std::unique_ptr<zpt::JSONElementT> _value) -> JSONObjT&;
    virtual auto push(zpt::json const& _value) -> zpt::JSONObjT&;

    virtual auto pop(int _idx) -> zpt::JSONObjT&;
    virtual auto pop(size_t _idx) -> zpt::JSONObjT&;
    virtual auto pop(const char* _idx) -> zpt::JSONObjT&;
    virtual auto pop(std::string const& _idx) -> zpt::JSONObjT&;

    virtual auto key_for(size_t _idx) const -> std::string;

    auto get_path(std::string const& _path, std::string const& _separator = ".") -> zpt::json;
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> zpt::JSONObjT&;
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> JSONObjT&;

    auto clone() const -> zpt::json;

    auto operator->() -> zpt::json::map*;
    auto operator*() -> zpt::json::map&;
    auto operator->() const -> zpt::json::map const*;
    auto operator*() const -> zpt::json::map const&;

    auto operator==(zpt::JSONObjT const& _in) const -> bool;
    auto operator==(zpt::JSONObj const& _in) const -> bool;
    template<typename T>
    auto operator==(T _in) const -> bool;
    auto operator!=(zpt::JSONObjT const& _in) const -> bool;
    auto operator!=(zpt::JSONObj const& _in) const -> bool;
    template<typename T>
    auto operator!=(T _in) const -> bool;
    auto operator<(zpt::JSONObjT const& _in) const -> bool;
    auto operator<(zpt::JSONObj const& _in) const -> bool;
    template<typename T>
    auto operator<(T _in) const -> bool;
    auto operator>(zpt::JSONObjT const& _in) const -> bool;
    auto operator>(zpt::JSONObj const& _in) const -> bool;
    template<typename T>
    auto operator>(T _in) const -> bool;
    auto operator>=(zpt::JSONObjT const& _in) const -> bool;
    auto operator>=(zpt::JSONObj const& _in) const -> bool;
    template<typename T>
    auto operator>=(T _in) const -> bool;
    auto operator<=(zpt::JSONObjT const& _in) const -> bool;
    auto operator<=(zpt::JSONObj const& _in) const -> bool;
    template<typename T>
    auto operator<=(T _in) const -> bool;

    auto operator[](int _idx) -> zpt::json&;
    auto operator[](size_t _idx) -> zpt::json&;
    auto operator[](const char* _idx) -> zpt::json&;
    auto operator[](std::string const& _idx) -> zpt::json&;
    auto operator[](int _idx) const -> zpt::json const;
    auto operator[](size_t _idx) const -> zpt::json const;
    auto operator[](const char* _idx) const -> zpt::json const;
    auto operator[](std::string const& _idx) const -> zpt::json const;

    friend auto operator<<(std::ostream& _out, zpt::JSONObjT& _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

  private:
    std::string __name{ "" };
    zpt::json::map __underlying;
};
} // namespace zpt

namespace zpt {
class JSONArrT {
  public:
    JSONArrT();
    virtual ~JSONArrT();

    virtual auto stringify(std::string& _out) -> zpt::JSONArrT&;
    virtual auto stringify(std::ostream& _out) -> zpt::JSONArrT&;
    virtual auto stringify(std::string& _out) const -> zpt::JSONArrT const&;
    virtual auto stringify(std::ostream& _out) const -> zpt::JSONArrT const&;

    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> zpt::JSONArrT const&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> zpt::JSONArrT const&;

    virtual auto push(std::unique_ptr<zpt::JSONElementT> _value) -> zpt::JSONArrT&;
    virtual auto push(zpt::json const& _value) -> zpt::JSONArrT&;

    virtual auto pop(int _idx) -> zpt::JSONArrT&;
    virtual auto pop(size_t _idx) -> zpt::JSONArrT&;
    virtual auto pop(const char* _idx) -> zpt::JSONArrT&;
    virtual auto pop(std::string const& _idx) -> zpt::JSONArrT&;

    virtual auto sort() -> zpt::JSONArrT&;
    virtual auto sort(std::function<bool(zpt::json, zpt::json)> _comparator) -> zpt::JSONArrT&;

    auto get_path(std::string const& _path, std::string const& _separator = ".") -> zpt::json;
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> zpt::JSONArrT&;
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> zpt::JSONArrT&;

    auto clone() const -> zpt::json;

    auto operator->() -> std::vector<zpt::json>*;
    auto operator*() -> std::vector<zpt::json>&;
    auto operator->() const -> std::vector<zpt::json> const*;
    auto operator*() const -> std::vector<zpt::json> const&;

    auto operator==(zpt::JSONArrT const& _in) const -> bool;
    auto operator==(zpt::JSONArr const& _in) const -> bool;
    template<typename T>
    auto operator==(T _in) const -> bool;
    auto operator!=(zpt::JSONArrT const& _in) const -> bool;
    auto operator!=(zpt::JSONArr const& _in) const -> bool;
    template<typename T>
    auto operator!=(T _in) const -> bool;
    auto operator<(zpt::JSONArrT const& _in) const -> bool;
    auto operator<(zpt::JSONArr const& _in) const -> bool;
    template<typename T>
    auto operator<(T _in) const -> bool;
    auto operator>(zpt::JSONArrT const& _in) const -> bool;
    auto operator>(zpt::JSONArr const& _in) const -> bool;
    template<typename T>
    auto operator>(T _in) const -> bool;
    auto operator<=(zpt::JSONArrT const& _in) const -> bool;
    auto operator<=(zpt::JSONArr const& _in) const -> bool;
    template<typename T>
    auto operator<=(T _in) const -> bool;
    auto operator>=(zpt::JSONArrT const& _in) const -> bool;
    auto operator>=(zpt::JSONArr const& _in) const -> bool;
    template<typename T>
    auto operator>=(T _in) const -> bool;

    auto operator[](int _idx) -> zpt::json&;
    auto operator[](size_t _idx) -> zpt::json&;
    auto operator[](const char* _idx) -> zpt::json&;
    auto operator[](std::string const& _idx) -> zpt::json&;
    auto operator[](int _idx) const -> zpt::json const;
    auto operator[](size_t _idx) const -> zpt::json const;
    auto operator[](const char* _idx) const -> zpt::json const;
    auto operator[](std::string const& _idx) const -> zpt::json const;

    friend auto operator<<(std::ostream& _out, zpt::JSONArrT& _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

  private:
    std::vector<zpt::json> __underlying;
};
} // namespace zpt

namespace zpt {
class JSONObj {
  public:
    JSONObj();
    JSONObj(const zpt::JSONObj& _rhs);
    JSONObj(zpt::JSONObj&& _rhs);
    JSONObj(zpt::JSONObjT* _target);
    virtual ~JSONObj();

    auto hash() const -> size_t;

    auto operator=(const zpt::JSONObj& _rhs) -> zpt::JSONObj&;
    auto operator=(zpt::JSONObj&& _rhs) -> zpt::JSONObj&;

    auto operator->() -> zpt::JSONObjT*;
    auto operator*() -> zpt::JSONObjT&;
    auto operator->() const -> zpt::JSONObjT const*;
    auto operator*() const -> zpt::JSONObjT const&;

    operator std::string();
    operator zpt::pretty();
    template<typename T>
    auto operator==(T _rhs) const -> bool;
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    auto operator<<(std::string const& _in) -> zpt::JSONObj&;
    auto operator<<(const char* _in) -> zpt::JSONObj&;
    auto operator<<(std::initializer_list<zpt::json> _list) -> zpt::JSONObj&;
    template<typename T>
    auto operator<<(T _in) -> zpt::JSONObj&;
    // template<typename T>
    // auto operator>>(T _in) -> zpt::JSONObj&;
    template<typename T>
    auto operator[](T _idx) -> zpt::json&;
    template<typename T>
    auto operator[](T _idx) const -> zpt::json const;

    friend auto operator<<(std::ostream& _out, JSONObj& _in) -> std::ostream& {
        _in.__underlying->stringify(_out);
        return _out;
    }

  private:
    std::shared_ptr<zpt::JSONObjT> __underlying{ nullptr };
};
} // namespace zpt

namespace zpt {
class JSONArr {
  public:
    JSONArr();
    JSONArr(const JSONArr& _rhs);
    JSONArr(JSONArr&& _rhs);
    JSONArr(zpt::JSONArrT* _target);
    virtual ~JSONArr();

    auto hash() const -> size_t;

    operator std::string();
    operator zpt::pretty();

    auto operator=(const zpt::JSONArr& _rhs) -> zpt::JSONArr&;
    auto operator=(zpt::JSONArr&& _rhs) -> zpt::JSONArr&;

    auto operator->() -> zpt::JSONArrT*;
    auto operator*() -> zpt::JSONArrT&;
    auto operator->() const -> zpt::JSONArrT const*;
    auto operator*() const -> zpt::JSONArrT const&;

    template<typename T>
    auto operator==(T _rhs) const -> bool;
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    auto operator<<(std::initializer_list<zpt::json> _list) -> JSONArr&;
    template<typename T>
    auto operator<<(T _in) -> JSONArr&;
    // template<typename T>
    // auto operator>>(T _in) -> JSONArr&;
    template<typename T>
    auto operator[](T _idx) -> json&;
    template<typename T>
    auto operator[](T _idx) const -> json const;

    friend auto operator<<(std::ostream& _out, JSONArr& _in) -> std::ostream& {
        _in.__underlying->stringify(_out);
        return _out;
    }

  private:
    std::shared_ptr<zpt::JSONArrT> __underlying{ nullptr };
};
} // namespace zpt

namespace zpt {
using JSONStr = std::shared_ptr<std::string>;
}

namespace zpt {
class JSONRegex {
  public:
    JSONRegex();
    JSONRegex(const zpt::JSONRegex& _rhs);
    JSONRegex(zpt::JSONRegex&& _rhs);
    JSONRegex(std::string const& _target);
    virtual ~JSONRegex();

    auto operator=(const zpt::JSONRegex& _rhs) -> zpt::JSONRegex&;
    auto operator=(zpt::JSONRegex&& _rhs) -> zpt::JSONRegex&;

    auto operator->() -> std::regex*;
    auto operator*() -> std::regex&;
    auto operator->() const -> std::regex const*;
    auto operator*() const -> std::regex const&;

    operator zpt::pretty();
    operator std::regex&();
    auto operator==(zpt::regex _rhs) const -> bool;
    auto operator==(zpt::json _rhs) const -> bool;
    auto operator==(std::string const& _rhs) const -> bool;
    auto operator!=(zpt::regex _rhs) const -> bool;
    auto operator!=(zpt::json _rhs) const -> bool;
    auto operator!=(std::string const& _rhs) const -> bool;

    friend auto operator<<(std::ostream& _out, JSONRegex& _in) -> std::ostream& {
        _out << "/" << _in.to_string() << "/" << std::flush;
        return _out;
    }

    auto to_string() const -> std::string const&;

  private:
    std::string __underlying_original{ "" };
    std::shared_ptr<std::regex> __underlying{ nullptr };
};
} // namespace zpt

namespace zpt {
class JSONContext {
  public:
    JSONContext(void* _target);
    virtual ~JSONContext();

    virtual auto unpack() -> void*;

  private:
    void* __target{ nullptr };
};
} // namespace zpt

namespace zpt {
class context {
  public:
    context(void* _target);
    context(const context& _rhs);
    context(context&& _rhs);
    virtual ~context();

    auto operator->() -> zpt::JSONContext*;
    auto operator*() -> zpt::JSONContext&;
    auto operator->() const -> zpt::JSONContext const*;
    auto operator*() const -> zpt::JSONContext const&;

    auto operator=(const zpt::context& _rhs) -> zpt::context&;
    auto operator=(zpt::context&& _rhs) -> zpt::context&;

  private:
    std::shared_ptr<zpt::JSONContext> __underlying{ nullptr };
};
} // namespace zpt

namespace zpt {
using symbol = std::function<zpt::json(zpt::json, unsigned short, zpt::context)>;
using symbol_table = std::shared_ptr<
  std::unordered_map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>>;
inline symbol_table __lambdas{
    new std::unordered_map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>{}
};
} // namespace zpt

namespace zpt {
class lambda : public std::shared_ptr<zpt::JSONLambda> {
  public:
    lambda();
    lambda(std::shared_ptr<zpt::JSONLambda> _target);
    lambda(zpt::lambda& _target);
    lambda(zpt::JSONLambda* _target);
    lambda(std::string const& _signature);
    lambda(std::string const& _name, unsigned short _n_args);
    virtual ~lambda();

    auto hash() const -> size_t;

    virtual auto operator()(zpt::json _args, zpt::context _ctx) -> zpt::json;
    virtual auto operator()(zpt::json _args, zpt::context _ctx) const -> zpt::json;

    static auto add(std::string const& _signature, zpt::symbol _lambda) -> void;
    static auto add(std::string const& _name, unsigned short _n_args, zpt::symbol _lambda) -> void;

    static auto call(std::string const& _name, zpt::json _args, zpt::context _ctx) -> zpt::json;

    static auto stringify(std::string const& _name, unsigned short _n_args) -> std::string;
    static auto parse(std::string const& _signature) -> std::tuple<std::string, unsigned short>;

  private:
    static auto find(std::string const& _signature) -> zpt::symbol;
    static auto find(std::string const& _name, unsigned short _1n_args) -> zpt::symbol;
};
} // namespace zpt

namespace zpt {
class JSONLambda {
  public:
    JSONLambda();
    JSONLambda(std::string const& _signature);
    JSONLambda(std::string const& _name, unsigned short _n_args);
    virtual ~JSONLambda();

    virtual auto call(zpt::json _args, zpt::context _ctx) -> zpt::json;

    virtual auto name() const -> std::string;
    virtual auto n_args() const -> unsigned short;
    virtual auto signature() const -> std::string;

  private:
    std::string __name{ "" };
    unsigned short __n_args{ 0 };
};
} // namespace zpt

namespace zpt {
class JSONElementT {
  public:
    JSONElementT();
    JSONElementT(JSONType _in);
    JSONElementT(const JSONElementT& _element);
    JSONElementT(JSONElementT&& _element);
    JSONElementT(JSONObj& _value);
    JSONElementT(JSONArr& _value);
    JSONElementT(std::string const& _value);
    JSONElementT(const char* _value);
    JSONElementT(long long _value);
    JSONElementT(double _value);
    JSONElementT(bool _value);
    JSONElementT(zpt::timestamp_t _value);
    JSONElementT(int _value);
    JSONElementT(size_t _value);
#ifdef __LP64__
    JSONElementT(unsigned int _value);
#endif
    JSONElementT(zpt::lambda _value);
    JSONElementT(zpt::regex _value);
    JSONElementT(std::nullptr_t _rhs);
    JSONElementT(void* _rhs);
    virtual ~JSONElementT();

    virtual auto type() const -> JSONType;
    virtual auto demangle() const -> std::string;
    virtual auto type(JSONType _in) -> JSONElementT&;
    virtual auto ok() const -> bool;
    virtual auto empty() const -> bool;
    virtual auto nil() const -> bool;

    virtual auto clear() -> void;
    virtual auto size() const -> size_t;
    virtual auto hash() const -> size_t;

    auto find(zpt::json _to_find) const -> zpt::json::iterator;
    auto contains(zpt::json _to_find) const -> bool;

    auto parent() -> JSONElementT*;
    auto parent(JSONElementT* _parent) -> JSONElementT&;

    virtual auto clone() const -> zpt::json;

    virtual auto is_object() const -> bool;
    virtual auto is_array() const -> bool;
    virtual auto is_string() const -> bool;
    virtual auto is_integer() const -> bool;
    virtual auto is_floating() const -> bool;
    virtual auto is_number() const -> bool;
    virtual auto is_bool() const -> bool;
    virtual auto is_date() const -> bool;
    virtual auto is_lambda() const -> bool;
    virtual auto is_regex() const -> bool;
    virtual auto is_nil() const -> bool;
    virtual auto is_undefined() const -> bool;

    virtual auto object() -> JSONObj&;
    virtual auto array() -> JSONArr&;
    virtual auto string() -> std::string&;
    virtual auto integer() -> long long&;
    virtual auto floating() -> double&;
    virtual auto boolean() -> bool&;
    virtual auto date() -> zpt::timestamp_t&;
    virtual auto lambda() -> zpt::lambda&;
    virtual auto regex() -> zpt::regex&;
    virtual auto number() -> double;
    virtual auto object() const -> JSONObj const&;
    virtual auto array() const -> JSONArr const&;
    virtual auto string() const -> std::string const&;
    virtual auto integer() const -> long long const&;
    virtual auto floating() const -> double const&;
    virtual auto boolean() const -> bool const&;
    virtual auto date() const -> zpt::timestamp_t const&;
    virtual auto lambda() const -> zpt::lambda const&;
    virtual auto regex() const -> zpt::regex const&;
    virtual auto number() const -> double;

    auto operator=(const JSONElementT& _rhs) -> JSONElementT&;
    auto operator=(JSONElementT&& _rhs) -> JSONElementT&;
    auto operator=(std::string const& _rhs) -> JSONElementT&;
    auto operator=(std::nullptr_t) -> JSONElementT&;
    auto operator=(const char* _rhs) -> JSONElementT&;
    auto operator=(long long _rhs) -> JSONElementT&;
    auto operator=(double _rhs) -> JSONElementT&;
    auto operator=(bool _rhs) -> JSONElementT&;
    auto operator=(int _rhs) -> JSONElementT&;
    auto operator=(size_t _rhs) -> JSONElementT&;
#ifdef __LP64__
    auto operator=(unsigned int _rhs) -> JSONElementT&;
#endif
    auto operator=(zpt::json _rhs) -> JSONElementT&;
    auto operator=(zpt::timestamp_t _rhs) -> JSONElementT&;
    auto operator=(zpt::JSONObj& _rhs) -> JSONElementT&;
    auto operator=(zpt::JSONArr& _rhs) -> JSONElementT&;
    auto operator=(zpt::lambda _rhs) -> JSONElementT&;
    auto operator=(zpt::regex _rhs) -> JSONElementT&;
    auto operator=(void*) -> JSONElementT&;

    operator std::string();
    operator bool();
    operator int();
    operator long();
    operator long long();
    operator size_t();
    operator double();
#ifdef __LP64__
    operator unsigned int();
#endif
    operator zpt::timestamp_t();
    operator zpt::JSONObj();
    operator zpt::JSONArr();
    operator zpt::JSONObj&();
    operator zpt::JSONArr&();
    operator zpt::lambda();
    operator zpt::regex();
    operator zpt::regex&();

    auto operator<<(const char* _in) -> JSONElementT&;
    auto operator<<(std::string const& _in) -> JSONElementT&;
    auto operator<<(zpt::json _in) -> JSONElementT&;
    template<typename T>
    auto operator<<(T _in) -> JSONElementT&;
    // template<typename T>
    // auto operator>>(T _in) -> JSONElementT&;
    template<typename T>
    auto operator[](T _idx) -> json&;
    template<typename T>
    auto operator[](T _idx) const -> json const;
    auto operator==(JSONElementT const& _in) const -> bool;
    auto operator==(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator==(T _in) const -> bool;
    auto operator!=(JSONElementT const& _in) const -> bool;
    auto operator!=(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator!=(T _in) const -> bool;
    auto operator<(JSONElementT const& _in) const -> bool;
    auto operator<(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator<(T _in) const -> bool;
    auto operator>(JSONElementT const& _in) const -> bool;
    auto operator>(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator>(T _in) const -> bool;
    auto operator<=(JSONElementT const& _in) const -> bool;
    auto operator<=(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator<=(T _in) const -> bool;
    auto operator>=(JSONElementT const& _in) const -> bool;
    auto operator>=(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator>=(T _in) const -> bool;

    friend auto operator<<(std::ostream& _out, JSONElementT _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

    auto get_path(std::string const& _path, std::string const& _separator = ".") -> zpt::json;
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> JSONElementT&;
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> JSONElementT&;

    virtual auto stringify(std::string& _out) -> JSONElementT&;
    virtual auto stringify(std::ostream& _out) -> JSONElementT&;
    virtual auto stringify(std::string& _out) const -> JSONElementT const&;
    virtual auto stringify(std::ostream& _out) const -> JSONElementT const&;
    virtual auto stringify() const -> std::string;

    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> JSONElementT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> JSONElementT&;
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> JSONElementT const&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> JSONElementT const&;
    virtual auto prettify() const -> std::string;

    virtual auto element(size_t _pos) -> std::tuple<size_t, std::string, zpt::json>;

  private:
    JSONElementT* __parent{ nullptr };
    std::variant<std::nullptr_t,   // JSNil
                 bool,             // JSBoolean
                 long long int,    // JSInteger
                 double,           // JSDouble
                 std::string,      // JSString
                 zpt::timestamp_t, // JSDate
                 JSONArr,          // JSArray
                 JSONObj,          // JSObject
                 JSONRegex,        // JSRegex
                 zpt::lambda,      // JSLambda
                 void*>            // JSUndefined
      __underlying;
};
} // namespace zpt

namespace zpt {
auto timestamp(std::string const& _json_date = "") -> zpt::timestamp_t;

auto get(std::string const& _path, zpt::json _source) -> zpt::json;

template<typename T>
auto set(std::string const& _path, T _value, zpt::json _target = zpt::undefined) -> zpt::json;

auto timestamp(zpt::json _json_date) -> zpt::timestamp_t;

auto timestamp(zpt::timestamp_t _timestamp) -> std::string;
} // namespace zpt

namespace std {
template<>
struct hash<zpt::json> {
    auto operator()(zpt::json const& _json) const noexcept -> std::size_t;
};
} // namespace std

/// Class `zpt::pretty` methods
template<typename T>
zpt::pretty::pretty(T _rhs) {
    _rhs->prettify(this->__underlying);
}

/// Class `zpt::json` methods
template<typename T>
zpt::json::json(T const& _rhs)
  : __underlying{ std::make_shared<zpt::JSONElementT>(_rhs) } {}
template<typename T>
auto zpt::json::operator=(T const& _rhs) -> zpt::json& {
    (*this->__underlying.get()) = _rhs;
    return (*this);
}
template<typename T>
auto zpt::json::operator==(T _rhs) const -> bool {
    return (*this->__underlying.get()) == _rhs;
}
template<typename T>
auto zpt::json::operator!=(T _rhs) const -> bool {
    return (*this->__underlying.get()) != _rhs;
}
template<typename T>
auto zpt::json::operator<(T _rhs) const -> bool {
    return (*this->__underlying.get()) < _rhs;
}
template<typename T>
auto zpt::json::operator>(T _rhs) const -> bool {
    return (*this->__underlying.get()) > _rhs;
}
template<typename T>
auto zpt::json::operator<=(T _rhs) const -> bool {
    return (*this->__underlying.get()) <= _rhs;
}
template<typename T>
auto zpt::json::operator>=(T _rhs) const -> bool {
    return (*this->__underlying.get()) >= _rhs;
}
template<typename T>
auto zpt::json::operator<<(T _in) -> zpt::json& {
    (*this->__underlying.get()) << _in;
    return *this;
}
// template<typename T>
// auto
// zpt::json::operator>>(T _in) -> zpt::json& {
//     (*this->__underlying.get()) >> _in;
//     return *this;
// }

template<typename T>
auto zpt::json::data(const T _delegate) -> zpt::json {
    return _delegate->get_json();
}
template<typename T>
auto zpt::json::operator[](T _idx) -> zpt::json& {
    return (*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::json::operator[](T _idx) const -> zpt::json const {
    return const_cast<zpt::JSONElementT const&>(*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::json::operator()(T _idx) const -> zpt::json const {
    return const_cast<zpt::JSONElementT const&>(*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::json::pretty(T _e) -> std::string {
    return zpt::pretty{ _e };
}
template<typename T>
auto zpt::json::string(T _e) -> zpt::json {
    std::string _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::integer(T _e) -> zpt::json {
    long long int _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::uinteger(T _e) -> zpt::json {
    unsigned int _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::floating(T _e) -> zpt::json {
    double _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::ulong(T _e) -> zpt::json {
    size_t _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::boolean(T _e) -> zpt::json {
    bool _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::date(T _e) -> zpt::json {
    zpt::timestamp_t _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::lambda(T _e) -> zpt::json {
    zpt::lambda _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::regex(T _e) -> zpt::json {
    zpt::regex _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}

/// Class `zpt::JSONObjT` methods
template<typename T>
auto zpt::JSONObjT::operator==(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONObjT::operator!=(T _in) const -> bool {
    return true;
}
template<typename T>
auto zpt::JSONObjT::operator<(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONObjT::operator>(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONObjT::operator>=(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONObjT::operator<=(T _in) const -> bool {
    return false;
}

/// Class `zpt::JSONArrT` methods
template<typename T>
auto zpt::JSONArrT::operator==(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONArrT::operator!=(T _in) const -> bool {
    return true;
}
template<typename T>
auto zpt::JSONArrT::operator<(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONArrT::operator>(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONArrT::operator>=(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONArrT::operator<=(T _in) const -> bool {
    return false;
}

/// Class `zpt::JSONObj` methods
template<typename T>
auto zpt::JSONObj::operator==(T _rhs) const -> bool {
    return (*this->__underlying.get()) == _rhs;
}
template<typename T>
auto zpt::JSONObj::operator!=(T _rhs) const -> bool {
    return (*this->__underlying.get()) != _rhs;
}
template<typename T>
auto zpt::JSONObj::operator<(T _rhs) const -> bool {
    return (*this->__underlying.get()) < _rhs;
}
template<typename T>
auto zpt::JSONObj::operator>(T _rhs) const -> bool {
    return (*this->__underlying.get()) > _rhs;
}
template<typename T>
auto zpt::JSONObj::operator<=(T _rhs) const -> bool {
    return (*this->__underlying.get()) <= _rhs;
}
template<typename T>
auto zpt::JSONObj::operator>=(T _rhs) const -> bool {
    return (*this->__underlying.get()) >= _rhs;
}
template<typename T>
auto zpt::JSONObj::operator<<(T _in) -> JSONObj& {
    (*this)->push(_in);
    return *this;
}
// template<typename T>
// auto
// zpt::JSONObj::operator>>(T _in) -> JSONObj& {
//     (*this)->pop(_in);
//     return *this;
// }
template<typename T>
auto zpt::JSONObj::operator[](T _idx) -> json& {
    return (*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::JSONObj::operator[](T _idx) const -> json const {
    return static_cast<JSONObjT const&>(*this->__underlying.get())[_idx];
}

/// Class `zpt::JSONArr` methods
template<typename T>
auto zpt::JSONArr::operator==(T _rhs) const -> bool {
    return (*this->__underlying.get()) == _rhs;
}
template<typename T>
auto zpt::JSONArr::operator!=(T _rhs) const -> bool {
    return (*this->__underlying.get()) != _rhs;
}
template<typename T>
auto zpt::JSONArr::operator<(T _rhs) const -> bool {
    return (*this->__underlying.get()) < _rhs;
}
template<typename T>
auto zpt::JSONArr::operator>(T _rhs) const -> bool {
    return (*this->__underlying.get()) > _rhs;
}
template<typename T>
auto zpt::JSONArr::operator<=(T _rhs) const -> bool {
    return (*this->__underlying.get()) <= _rhs;
}
template<typename T>
auto zpt::JSONArr::operator>=(T _rhs) const -> bool {
    return (*this->__underlying.get()) >= _rhs;
}
template<typename T>
auto zpt::JSONArr::operator<<(T _in) -> JSONArr& {
    (*this)->push(_in);
    return *this;
}
// template<typename T>
// auto
// zpt::JSONArr::operator>>(T _in) -> JSONArr& {
//     (*this)->pop(_in);
//     return *this;
// }
template<typename T>
auto zpt::JSONArr::operator[](T _idx) -> json& {
    return (*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::JSONArr::operator[](T _idx) const -> json const {
    return static_cast<JSONArrT const&>(*this->__underlying.get())[_idx];
}

/// Class `zpt::JSONElementT` methods
template<typename T>
auto zpt::JSONElementT::operator<<(T _in) -> JSONElementT& {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            this->object() << _in;
            break;
        }
        case zpt::JSArray: {
            this->array() << _in;
            break;
        }
        default: {
            (*this) << zpt::json{ _in };
            break;
        }
    }
    return *this;
}
// template<typename T>
// auto
// zpt::JSONElementT::operator>>(T _in) -> JSONElementT& {
//     switch (this->__target.__type) {
//         case zpt::JSObject: {
//             this->__target.__object >> _in;
//             break;
//         }
//         case zpt::JSArray: {
//             this->__target.__array >> _in;
//             break;
//         }
//         default: {
//             break;
//         }
//     }
//     return *this;
// }
template<typename T>
auto zpt::JSONElementT::operator[](T _idx) -> json& {
    if (this->type() == zpt::JSObject) { return this->object()[_idx]; }
    else if (this->type() == zpt::JSArray) { return this->array()[_idx]; }
    else if (this->type() == zpt::JSNil) {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>) {
            this->__underlying = zpt::JSONObj();
            return this->object()[_idx];
        }
        else if constexpr (std::is_integral_v<T>) {
            this->__underlying = zpt::JSONArr();
            return this->array()[_idx];
        }
    }
    return zpt::undefined;
}

template<typename T>
auto zpt::JSONElementT::operator[](T _idx) const -> json const {
    if (this->type() == zpt::JSObject) { return static_cast<JSONObj const&>(this->object())[_idx]; }
    else if (this->type() == zpt::JSArray) {
        return static_cast<JSONArr const&>(this->array())[_idx];
    }
    return zpt::undefined;
}
template<typename T>
auto zpt::JSONElementT::operator==(T _in) const -> bool {
    if constexpr (std::is_same<T, std::nullptr_t>::value || std::is_pointer<T>::value) {
        if (_in == nullptr) {
            return this->type() == zpt::JSNil || this->type() == zpt::JSUndefined;
        }
    }
    JSONElementT _rhs{ _in };
    return (*this) == _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator!=(T _in) const -> bool {
    if constexpr (std::is_same<T, std::nullptr_t>::value || std::is_pointer<T>::value) {
        if (_in == nullptr) {
            return this->type() == zpt::JSNil || this->type() == zpt::JSUndefined;
        }
    }
    JSONElementT _rhs{ _in };
    return (*this) != _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator<(T _in) const -> bool {
    JSONElementT _rhs{ _in };
    return (*this) < _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator>(T _in) const -> bool {
    JSONElementT _rhs{ _in };
    return (*this) > _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator<=(T _in) const -> bool {
    JSONElementT _rhs{ _in };
    return (*this) <= _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator>=(T _in) const -> bool {
    JSONElementT _rhs{ _in };
    return (*this) >= _rhs;
}

/// Namespace `zpt` methods
template<typename T>
auto zpt::set(std::string const& _path, T _value, zpt::json _target) -> zpt::json {
    zpt::json _return;
    if (_target->ok()) { _return = _target; }
    else { _return = zpt::json::object(); }
    _return->set_path(_path, zpt::json{ _value });
    return _return;
}

auto operator"" _JSON(const char* _string, size_t _length) -> zpt::json;
