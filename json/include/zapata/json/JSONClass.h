#pragma once

#include <chrono>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <vector>
#include <zapata/base/assertz.h>
#include <zapata/json/config.h>
#include <zapata/log/log.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

namespace zpt {

using timestamp_t = unsigned long long;

class JSONElementT;
class JSONObj;
class JSONArr;
class JSONLambda;
class lambda;
class json;
}

namespace zpt {
class pretty {
  public:
    pretty(const pretty& _rhs);
    pretty(pretty&& _rhs);
    pretty(std::string _rhs);
    pretty(const char* _rhs);
    template<typename T>
    pretty(T _rhs);
    virtual ~pretty() = default;

    operator std::string();

    auto operator=(const pretty& _rhs) -> pretty&;
    auto operator=(pretty&& _rhs) -> pretty&;

    auto operator-> () -> std::string&;
    auto operator*() -> std::string&;

    friend auto operator<<(std::ostream& _out, zpt::pretty _in) -> std::ostream& {
        _out << std::string(_in.__underlying.data());
        return _out;
    }

  private:
    std::string __underlying{ "" };
};
}

namespace zpt {
class json {
  public:
    using element = std::tuple<size_t, std::string, zpt::json>;

    class iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = zpt::json::element;
        using pointer = zpt::json::element;
        using reference = zpt::json::element;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit iterator(zpt::json& _target, size_t _pos = 0);
        iterator(const iterator& _rhs);
        virtual ~iterator() = default;

        // BASIC ITERATOR METHODS //
        auto operator=(const iterator& _rhs) -> iterator&;
        auto operator++() -> iterator&;
        auto operator*() const -> reference;
        // END / BASIC ITERATOR METHODS //

        // INPUT ITERATOR METHODS //
        auto operator++(int) -> iterator;
        auto operator-> () const -> pointer;
        auto operator==(iterator _rhs) const -> bool;
        auto operator!=(iterator _rhs) const -> bool;
        // END / INPUT ITERATOR METHODS //

        // OUTPUT ITERATOR METHODS //
        // reference operator*() const; <- already defined
        // iterator operator++(int); <- already defined
        // END / OUTPUT ITERATOR METHODS //

        // FORWARD ITERATOR METHODS //
        // Enable support for both input and output iterator <- already enabled
        // END / FORWARD ITERATOR METHODS //

        // BIDIRECTIOANL ITERATOR METHODS //
        auto operator--() -> iterator&;
        auto operator--(int) -> iterator;
        // END / BIDIRECTIOANL ITERATOR METHODS //

      private:
        zpt::json& __target;
        size_t __pos{ 0 };
    };

    json();
    json(std::nullptr_t);
    json(const zpt::JSONElementT& _target);
    json(std::unique_ptr<zpt::JSONElementT> _target);
    json(std::initializer_list<zpt::JSONElementT> _init);
    json(std::string _rhs);
    json(const char* _rhs);
    json(zpt::pretty _rhs);
    json(const zpt::json& _rhs);
    json(zpt::json&& _rhs);
    virtual ~json();

    auto size() -> size_t;
    auto value() -> zpt::JSONElementT&;
    auto parse(std::istream& _in) -> zpt::json&;

    auto begin() -> zpt::json::iterator;
    auto end() -> zpt::json::iterator;

    auto operator=(const zpt::json& _rhs) -> zpt::json&;
    auto operator=(zpt::json&& _rhs) -> zpt::json&;
    template<typename T>
    auto operator=(T _rhs) -> zpt::json&;

    auto operator-> () -> std::shared_ptr<zpt::JSONElementT>&;
    auto operator*() -> std::shared_ptr<zpt::JSONElementT>&;

    template<typename T>
    auto operator==(T _rhs) -> bool;
    template<typename T>
    auto operator!=(T _rhs) -> bool;
    template<typename T>
    auto operator<(T _rhs) -> bool;
    template<typename T>
    auto operator>(T _rhs) -> bool;
    template<typename T>
    auto operator<=(T _rhs) -> bool;
    template<typename T>
    auto operator>=(T _rhs) -> bool;
    template<typename T>
    auto operator<<(T _in) -> json&;
    template<typename T>
    auto operator>>(T _in) -> json&;
    template<typename T>
    auto operator[](T _idx) -> json;

    template<typename T>
    operator T();
    operator std::string();
    operator zpt::pretty();
    operator bool();
    operator int();
    operator long();
    operator long long();
    operator size_t();
    operator double();
    operator timestamp_t();
    operator JSONObj();
    operator JSONArr();
    operator JSONObj&();
    operator JSONArr&();
#ifdef __LP64__
    operator unsigned int();
#endif
    operator zpt::lambda();

    template<typename T>
    auto operator+(T _in) -> json;
    template<typename T>
    auto operator-(T _in) -> json;
    template<typename T>
    auto operator/(T _in) -> json;
    template<typename T>
    auto operator|(T _in) -> json;

    friend auto operator>>(std::istream& _in, zpt::json& _out) -> std::istream& {
        _out.parse(_in);
        return _in;
    }

    template<typename T>
    static auto data(const T _delegate) -> zpt::json;

    static auto stringify(std::string& _str) -> void;

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
    static auto date(std::string _e) -> zpt::json;
    static auto date() -> zpt::json;
    template<typename T>
    static auto date(T _e) -> zpt::json;
    template<typename T>
    static auto lambda(T _e) -> zpt::json;
    static auto lambda(std::string _name, unsigned short _n_args) -> zpt::json;

  private:
    std::shared_ptr<zpt::JSONElementT> __underlying{ nullptr };
};
}

namespace zpt {
extern zpt::json undefined;
extern zpt::json nilptr;
extern zpt::json array;
}

namespace zpt {
class JSONObjT {
  public:
    JSONObjT();
    virtual ~JSONObjT();

    virtual auto stringify(std::string& _out) -> zpt::JSONObjT&;
    virtual auto stringify(std::ostream& _out) -> zpt::JSONObjT&;

    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;

    virtual auto push(std::string _name) -> zpt::JSONObjT&;
    virtual auto push(zpt::JSONElementT& _value) -> zpt::JSONObjT&;
    virtual auto push(std::unique_ptr<zpt::JSONElementT> _value) -> JSONObjT&;
    virtual auto push(zpt::json& _value) -> zpt::JSONObjT&;

    virtual auto pop(int _idx) -> zpt::JSONObjT&;
    virtual auto pop(size_t _idx) -> zpt::JSONObjT&;
    virtual auto pop(const char* _idx) -> zpt::JSONObjT&;
    virtual auto pop(std::string _idx) -> zpt::JSONObjT&;

    virtual auto key_for(size_t _idx) -> std::string;
    virtual auto index_for(std::string _name) -> size_t;

    auto get_path(std::string _path, std::string _separator = ".") -> zpt::json;
    auto set_path(std::string _path, zpt::json _value, std::string _separator = ".")
      -> zpt::JSONObjT&;
    auto del_path(std::string _path, std::string _separator = ".") -> JSONObjT&;

    auto clone() -> zpt::json;

    auto operator-> () -> std::map<std::string, std::tuple<zpt::json, size_t>>&;
    auto operator*() -> std::map<std::string, std::tuple<zpt::json, size_t>>&;

    auto operator==(zpt::JSONObjT& _in) -> bool;
    auto operator==(zpt::JSONObj& _in) -> bool;
    template<typename T>
    auto operator==(T _in) -> bool;
    auto operator!=(zpt::JSONObjT& _in) -> bool;
    auto operator!=(zpt::JSONObj& _in) -> bool;
    template<typename T>
    auto operator!=(T _in) -> bool;
    auto operator<(zpt::JSONObjT& _in) -> bool;
    auto operator<(zpt::JSONObj& _in) -> bool;
    template<typename T>
    auto operator<(T _in) -> bool;
    auto operator>(zpt::JSONObjT& _in) -> bool;
    auto operator>(zpt::JSONObj& _in) -> bool;
    template<typename T>
    auto operator>(T _in) -> bool;
    auto operator>=(zpt::JSONObjT& _in) -> bool;
    auto operator>=(zpt::JSONObj& _in) -> bool;
    template<typename T>
    auto operator>=(T _in) -> bool;
    auto operator<=(zpt::JSONObjT& _in) -> bool;
    auto operator<=(zpt::JSONObj& _in) -> bool;
    template<typename T>
    auto operator<=(T _in) -> bool;

    auto operator[](int _idx) -> zpt::json&;
    auto operator[](size_t _idx) -> zpt::json&;
    auto operator[](const char* _idx) -> zpt::json&;
    auto operator[](std::string _idx) -> zpt::json&;

    friend std::ostream& operator<<(std::ostream& _out, zpt::JSONObjT& _in) {
        _in.stringify(_out);
        return _out;
    }

  private:
    std::string __name{ "" };
    std::vector<std::string> __name_to_index;
    std::map<std::string, std::tuple<zpt::json, size_t>> __underlying;
};
}

namespace zpt {
class JSONArrT {
  public:
    JSONArrT();
    virtual ~JSONArrT();

    virtual auto stringify(std::string& _out) -> zpt::JSONArrT&;
    virtual auto stringify(std::ostream& _out) -> zpt::JSONArrT&;

    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;

    virtual auto push(zpt::JSONElementT& _value) -> zpt::JSONArrT&;
    virtual auto push(std::unique_ptr<zpt::JSONElementT> _value) -> zpt::JSONArrT&;
    virtual auto push(zpt::json& _value) -> zpt::JSONArrT&;

    virtual auto pop(int _idx) -> zpt::JSONArrT&;
    virtual auto pop(size_t _idx) -> zpt::JSONArrT&;
    virtual auto pop(const char* _idx) -> zpt::JSONArrT&;
    virtual auto pop(std::string _idx) -> zpt::JSONArrT&;

    virtual auto sort() -> zpt::JSONArrT&;
    virtual auto sort(std::function<bool(zpt::json, zpt::json)> _comparator) -> zpt::JSONArrT&;

    auto get_path(std::string _path, std::string _separator = ".") -> zpt::json;
    auto set_path(std::string _path, zpt::json _value, std::string _separator = ".")
      -> zpt::JSONArrT&;
    auto del_path(std::string _path, std::string _separator = ".") -> zpt::JSONArrT&;

    auto clone() -> zpt::json;

    auto operator-> () -> std::vector<zpt::json>&;
    auto operator*() -> std::vector<zpt::json>&;

    auto operator==(zpt::JSONArrT& _in) -> bool;
    auto operator==(zpt::JSONArr& _in) -> bool;
    template<typename T>
    auto operator==(T _in) -> bool;
    auto operator!=(zpt::JSONArrT& _in) -> bool;
    auto operator!=(zpt::JSONArr& _in) -> bool;
    template<typename T>
    auto operator!=(T _in) -> bool;
    auto operator<(zpt::JSONArrT& _in) -> bool;
    auto operator<(zpt::JSONArr& _in) -> bool;
    template<typename T>
    auto operator<(T _in) -> bool;
    auto operator>(zpt::JSONArrT& _in) -> bool;
    auto operator>(zpt::JSONArr& _in) -> bool;
    template<typename T>
    auto operator>(T _in) -> bool;
    auto operator<=(zpt::JSONArrT& _in) -> bool;
    auto operator<=(zpt::JSONArr& _in) -> bool;
    template<typename T>
    auto operator<=(T _in) -> bool;
    auto operator>=(zpt::JSONArrT& _in) -> bool;
    auto operator>=(zpt::JSONArr& _in) -> bool;
    template<typename T>
    auto operator>=(T _in) -> bool;

    auto operator[](int _idx) -> zpt::json&;
    auto operator[](size_t _idx) -> zpt::json&;
    auto operator[](const char* _idx) -> zpt::json&;
    auto operator[](std::string _idx) -> zpt::json&;

    friend auto operator<<(std::ostream& _out, zpt::JSONArrT& _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

  private:
    std::vector<zpt::json> __underlying;
};
}

namespace zpt {
class JSONObj {
  public:
    JSONObj();
    JSONObj(const zpt::JSONObj& _rhs);
    JSONObj(zpt::JSONObj&& _rhs);
    JSONObj(zpt::JSONObjT* _target);
    virtual ~JSONObj();

    auto operator=(const zpt::JSONObj& _rhs) -> zpt::JSONObj&;
    auto operator=(zpt::JSONObj&& _rhs) -> zpt::JSONObj&;

    auto operator-> () -> std::shared_ptr<zpt::JSONObjT>&;
    auto operator*() -> std::shared_ptr<zpt::JSONObjT>&;

    operator std::string();
    operator zpt::pretty();
    template<typename T>
    auto operator==(T _rhs) -> bool;
    template<typename T>
    auto operator!=(T _rhs) -> bool;
    template<typename T>
    auto operator<(T _rhs) -> bool;
    template<typename T>
    auto operator>(T _rhs) -> bool;
    template<typename T>
    auto operator<=(T _rhs) -> bool;
    template<typename T>
    auto operator>=(T _rhs) -> bool;
    auto operator<<(std::string _in) -> JSONObj&;
    auto operator<<(const char* _in) -> JSONObj&;
    auto operator<<(zpt::JSONElementT& _in) -> JSONObj&;
    auto operator<<(std::initializer_list<zpt::JSONElementT> _list) -> JSONObj&;
    template<typename T>
    auto operator<<(T _in) -> JSONObj&;
    template<typename T>
    auto operator>>(T _in) -> JSONObj&;
    template<typename T>
    auto operator[](T _idx) -> json&;

    friend auto operator<<(std::ostream& _out, JSONObj& _in) -> std::ostream& {
        _in.__underlying->stringify(_out);
        return _out;
    }

  private:
    std::shared_ptr<zpt::JSONObjT> __underlying{ nullptr };
};
}

namespace zpt {
class JSONArr {
  public:
    JSONArr();
    JSONArr(const JSONArr& _rhs);
    JSONArr(JSONArr&& _rhs);
    JSONArr(zpt::JSONArrT* _target);
    virtual ~JSONArr();

    operator std::string();
    operator zpt::pretty();

    auto operator=(const zpt::JSONArr& _rhs) -> zpt::JSONArr&;
    auto operator=(zpt::JSONArr&& _rhs) -> zpt::JSONArr&;

    auto operator-> () -> std::shared_ptr<zpt::JSONArrT>&;
    auto operator*() -> std::shared_ptr<zpt::JSONArrT>&;

    template<typename T>
    auto operator==(T _rhs) -> bool;
    template<typename T>
    auto operator!=(T _rhs) -> bool;
    template<typename T>
    auto operator<(T _rhs) -> bool;
    template<typename T>
    auto operator>(T _rhs) -> bool;
    template<typename T>
    auto operator<=(T _rhs) -> bool;
    template<typename T>
    auto operator>=(T _rhs) -> bool;
    auto operator<<(JSONElementT& _in) -> JSONArr&;
    auto operator<<(std::initializer_list<zpt::JSONElementT> _list) -> JSONArr&;
    template<typename T>
    auto operator<<(T _in) -> JSONArr&;
    template<typename T>
    auto operator>>(T _in) -> JSONArr&;
    template<typename T>
    auto operator[](T _idx) -> json&;

    friend auto operator<<(std::ostream& _out, JSONArr& _in) -> std::ostream& {
        _in.__underlying->stringify(_out);
        return _out;
    }

  private:
    std::shared_ptr<zpt::JSONArrT> __underlying{ nullptr };
};
}

namespace zpt {
using JSONStr = std::shared_ptr<std::string>;
}

namespace zpt {
class JSONContext {
  public:
    JSONContext(void* _target);
    virtual ~JSONContext();

    virtual auto unpack() -> void*;

  private:
    void* __target{ nullptr };
};
}

namespace zpt {
class context {
  public:
    context(void* _target);
    context(const context& _rhs);
    context(context&& _rhs);
    virtual ~context();

    auto operator-> () -> std::shared_ptr<zpt::JSONContext>&;
    auto operator*() -> std::shared_ptr<zpt::JSONContext>&;

    auto operator=(const zpt::context& _rhs) -> zpt::context&;
    auto operator=(zpt::context&& _rhs) -> zpt::context&;

  private:
    std::shared_ptr<zpt::JSONContext> __underlying{ nullptr };
};
}

namespace zpt {
using symbol = std::function<zpt::json(zpt::json, unsigned short, zpt::context)>;
using symbol_table =
  std::shared_ptr<std::map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>>;

extern zpt::symbol_table __lambdas;
}

namespace zpt {
class lambda : public std::shared_ptr<zpt::JSONLambda> {
  public:
    lambda();
    lambda(std::shared_ptr<zpt::JSONLambda> _target);
    lambda(zpt::lambda& _target);
    lambda(zpt::JSONLambda* _target);
    lambda(std::string _signature);
    lambda(std::string _name, unsigned short _n_args);
    virtual ~lambda();

    virtual auto operator()(zpt::json _args, zpt::context _ctx) -> zpt::json;

    static auto add(std::string _signature, zpt::symbol _lambda) -> zpt::lambda&;
    static auto add(std::string _name, unsigned short _n_args, zpt::symbol _lambda) -> zpt::lambda&;

    static auto call(std::string _name, zpt::json _args, zpt::context _ctx) -> zpt::json;

    static auto stringify(std::string _name, unsigned short _n_args) -> std::string;
    static auto parse(std::string _signature) -> std::tuple<std::string, unsigned short>;

  private:
    static auto find(std::string _signature) -> zpt::symbol;
    static auto find(std::string _name, unsigned short _1n_args) -> zpt::symbol;
};
}

namespace zpt {
class JSONLambda {
  public:
    JSONLambda();
    JSONLambda(std::string _signature);
    JSONLambda(std::string _name, unsigned short _n_args);
    virtual ~JSONLambda();

    virtual auto call(zpt::json _args, zpt::context _ctx) -> zpt::json;

    virtual auto name() -> std::string;
    virtual auto n_args() -> unsigned short;
    virtual auto signature() -> std::string;

  private:
    std::string __name{ "" };
    unsigned short __n_args{ 0 };
};
}

namespace zpt {
using JSONUnion = struct JSONStruct {
    JSONStruct()
      : __type{ JSNil }
      , __nil{ nullptr } {};
    ~JSONStruct() {
        switch (__type) {
            case zpt::JSObject: {
                __object.~JSONObj();
                break;
            }
            case zpt::JSArray: {
                __array.~JSONArr();
                break;
            }
            case zpt::JSString: {
                __string.~JSONStr();
                break;
            }
            case zpt::JSLambda: {
                __lambda.~lambda();
                break;
            }
            default: {
                break;
            }
        }
    }

    JSONStruct(JSONStruct const&) = delete;
    JSONStruct& operator=(JSONStruct const&) = delete;

    JSONStruct(JSONStruct&&) = delete;
    JSONStruct& operator=(JSONStruct&&) = delete;

    JSONType __type{ JSNil };
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    union {
#endif
        JSONObj __object;
        JSONArr __array;
        JSONStr __string;
        long long __integer;
        double __double;
        bool __boolean;
        void* __nil;
        zpt::timestamp_t __date;
        zpt::lambda __lambda;
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    };
#endif
};
}

namespace zpt {
class JSONElementT {
  public:
    JSONElementT();
    JSONElementT(const JSONElementT& _element);
    JSONElementT(JSONElementT&& _element);
    JSONElementT(std::initializer_list<zpt::JSONElementT> _init);
    JSONElementT(zpt::json _value);
    JSONElementT(JSONObj& _value);
    JSONElementT(JSONArr& _value);
    JSONElementT(std::string _value);
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
    virtual ~JSONElementT();

    virtual auto type() -> JSONType;
    virtual auto demangle() -> std::string;
    virtual auto type(JSONType _in) -> JSONElementT&;
    virtual auto value() -> JSONUnion&;
    virtual auto ok() -> bool;
    virtual auto empty() -> bool;
    virtual auto nil() -> bool;

    virtual auto size() -> size_t;

    auto parent() -> zpt::json;
    auto parent(zpt::json& _parent) -> JSONElementT&;
    auto parent(std::nullptr_t) -> JSONElementT&;

    auto clone() -> zpt::json;

    virtual auto is_object() -> bool;
    virtual auto is_array() -> bool;
    virtual auto is_string() -> bool;
    virtual auto is_integer() -> bool;
    virtual auto is_double() -> bool;
    virtual auto is_number() -> bool;
    virtual auto is_bool() -> bool;
    virtual auto is_date() -> bool;
    virtual auto is_lambda() -> bool;
    virtual auto is_nil() -> bool;
    virtual auto is_iterable() -> bool;

    virtual auto obj() -> JSONObj&;
    virtual auto arr() -> JSONArr&;
    virtual auto str() -> std::string;
    virtual auto intr() -> long long;
    virtual auto dbl() -> double;
    virtual auto bln() -> bool;
    virtual auto date() -> zpt::timestamp_t;
    virtual auto lbd() -> zpt::lambda&;
    virtual auto number() -> double;

    auto operator=(const JSONElementT& _element) -> JSONElementT&;
    auto operator=(JSONElementT&& _element) -> JSONElementT&;
    auto operator=(std::string _value) -> JSONElementT&;
    auto operator=(std::initializer_list<zpt::JSONElementT> _init) -> JSONElementT&;
    auto operator=(const char* _value) -> JSONElementT&;
    auto operator=(long long _value) -> JSONElementT&;
    auto operator=(double _value) -> JSONElementT&;
    auto operator=(bool _value) -> JSONElementT&;
    auto operator=(int _value) -> JSONElementT&;
    auto operator=(size_t _value) -> JSONElementT&;
#ifdef __LP64__
    auto operator=(unsigned int _value) -> JSONElementT&;
#endif
    auto operator=(zpt::json _value) -> JSONElementT&;
    auto operator=(zpt::timestamp_t _value) -> JSONElementT&;
    auto operator=(zpt::JSONObj& _value) -> JSONElementT&;
    auto operator=(zpt::JSONArr& _value) -> JSONElementT&;
    auto operator=(zpt::lambda _value) -> JSONElementT&;

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
    operator zpt::pretty();
    operator zpt::timestamp_t();
    operator zpt::JSONObj();
    operator zpt::JSONArr();
    operator zpt::JSONObj&();
    operator zpt::JSONArr&();
    operator zpt::lambda();

    auto operator<<(const char* _in) -> JSONElementT&;
    auto operator<<(std::string _in) -> JSONElementT&;
    auto operator<<(JSONElementT) -> JSONElementT&;
    auto operator<<(std::initializer_list<zpt::JSONElementT> _list) -> JSONElementT&;
    template<typename T>
    auto operator<<(T _in) -> JSONElementT&;
    template<typename T>
    auto operator>>(T _in) -> JSONElementT&;
    template<typename T>
    auto operator[](T _idx) -> json&;
    auto operator==(JSONElementT& _in) -> bool;
    auto operator==(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator==(T _in) -> bool;
    auto operator!=(JSONElementT& _in) -> bool;
    auto operator!=(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator!=(T _in) -> bool;
    auto operator<(JSONElementT& _in) -> bool;
    auto operator<(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator<(T _in) -> bool;
    auto operator>(JSONElementT& _in) -> bool;
    auto operator>(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator>(T _in) -> bool;
    auto operator<=(JSONElementT& _in) -> bool;
    auto operator<=(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator<=(T _in) -> bool;
    auto operator>=(JSONElementT& _in) -> bool;
    auto operator>=(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator>=(T _in) -> bool;

    auto operator+(zpt::json _rhs) -> zpt::json;
    auto operator+(zpt::JSONElementT& _rhs) -> zpt::json;
    auto operator-(zpt::json _rhs) -> zpt::json;
    auto operator-(zpt::JSONElementT& _rhs) -> zpt::json;
    auto operator/(zpt::json _rhs) -> zpt::json;
    auto operator/(zpt::JSONElementT& _rhs) -> zpt::json;
    auto operator|(zpt::json _rhs) -> zpt::json;
    auto operator|(zpt::JSONElementT& _rhs) -> zpt::json;

    friend auto operator<<(std::ostream& _out, JSONElementT _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

    auto get_path(std::string _path, std::string _separator = ".") -> zpt::json;
    auto set_path(std::string _path, zpt::json _value, std::string _separator = ".")
      -> JSONElementT&;
    auto del_path(std::string _path, std::string _separator = ".") -> JSONElementT&;

    virtual auto flatten() -> zpt::json;
    virtual auto inspect(
      zpt::json _pattern,
      std::function<void(std::string, std::string, zpt::JSONElementT&)> _callback,
      zpt::JSONElementT* _parent = nullptr,
      std::string _key = "",
      std::string _parent_path = "") -> JSONElementT&;

    virtual auto stringify(std::string& _out) -> JSONElementT&;
    virtual auto stringify(std::ostream& _out) -> JSONElementT&;
    virtual auto stringify() -> std::string;

    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> JSONElementT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> JSONElementT&;
    virtual auto prettify() -> std::string;

    virtual auto element(size_t _pos) -> std::tuple<size_t, std::string, zpt::json>;

  private:
    JSONUnion __target;
    std::shared_ptr<JSONElementT> __parent{ nullptr };

    auto deinit() -> JSONElementT&;
};
}

namespace zpt {
auto
timestamp(std::string _json_date = "") -> zpt::timestamp_t;

template<typename T>
auto
mkelem(T& _e) -> std::unique_ptr<zpt::JSONElementT>;

template<typename T>
auto
mkptr(T _v) -> zpt::json;

auto
get(std::string _path, zpt::json _source) -> zpt::json;

template<typename T>
auto
set(std::string _path, T _value, zpt::json _target = zpt::undefined) -> zpt::json;

auto
timestamp(zpt::json _json_date) -> zpt::timestamp_t;

auto
timestamp(zpt::timestamp_t _timestamp) -> std::string;
}

/// Class `zpt::pretty` methods
template<typename T>
zpt::pretty::pretty(T _rhs) {
    _rhs->prettify(this->__underlying);
}

/// Class `zpt::json` methods
template<typename T>
auto
zpt::json::operator=(T _rhs) -> zpt::json& {
    (***this) = _rhs;
    return (*this);
}
template<typename T>
auto
zpt::json::operator==(T _rhs) -> bool {
    return (***this) == _rhs;
}
template<typename T>
auto
zpt::json::operator!=(T _rhs) -> bool {
    return (***this) != _rhs;
}
template<typename T>
auto
zpt::json::operator<(T _rhs) -> bool {
    return (***this) < _rhs;
}
template<typename T>
auto
zpt::json::operator>(T _rhs) -> bool {
    return (***this) > _rhs;
}
template<typename T>
auto
zpt::json::operator<=(T _rhs) -> bool {
    return (***this) <= _rhs;
}
template<typename T>
auto
zpt::json::operator>=(T _rhs) -> bool {
    return (***this) >= _rhs;
}
template<typename T>
auto
zpt::json::operator<<(T _in) -> zpt::json& {
    (***this) << _in;
    return *this;
}
template<typename T>
auto
zpt::json::operator>>(T _in) -> zpt::json& {
    (***this) >> _in;
    return *this;
}
template<typename T>
auto
zpt::json::operator+(T _rhs) -> zpt::json {
    return (***this) + _rhs;
}
template<typename T>
auto
zpt::json::operator-(T _rhs) -> zpt::json {
    return (***this) - _rhs;
}
template<typename T>
auto
zpt::json::operator/(T _rhs) -> zpt::json {
    return (***this) / _rhs;
}
template<typename T>
auto
zpt::json::operator|(T _rhs) -> zpt::json {
    return (***this) | _rhs;
}
template<typename T>
auto
zpt::json::data(const T _delegate) -> zpt::json {
    return _delegate->get_json();
}
template<typename T>
auto zpt::json::operator[](T _idx) -> zpt::json {
    return zpt::json{ (***this)[_idx] };
}
template<typename T>
auto
zpt::json::pretty(T _e) -> std::string {
    return zpt::pretty{ _e };
}
template<typename T>
auto
zpt::json::string(T _e) -> zpt::json {
    std::string _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto
zpt::json::integer(T _e) -> zpt::json {
    long long int _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto
zpt::json::uinteger(T _e) -> zpt::json {
    unsigned int _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto
zpt::json::floating(T _e) -> zpt::json {
    double _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto
zpt::json::ulong(T _e) -> zpt::json {
    size_t _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto
zpt::json::boolean(T _e) -> zpt::json {
    bool _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto
zpt::json::date(T _e) -> zpt::json {
    zpt::timestamp_t _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}
template<typename T>
auto
zpt::json::lambda(T _e) -> zpt::json {
    zpt::lambda _v(_e);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}

/// Class `zpt::JSONObjT` methods
template<typename T>
auto
zpt::JSONObjT::operator==(T _in) -> bool {
    return false;
}
template<typename T>
auto
zpt::JSONObjT::operator!=(T _in) -> bool {
    return true;
}
template<typename T>
auto
zpt::JSONObjT::operator<(T _in) -> bool {
    return false;
}
template<typename T>
auto
zpt::JSONObjT::operator>(T _in) -> bool {
    return false;
}
template<typename T>
auto
zpt::JSONObjT::operator>=(T _in) -> bool {
    return false;
}
template<typename T>
auto
zpt::JSONObjT::operator<=(T _in) -> bool {
    return false;
}

/// Class `zpt::JSONArrT` methods
template<typename T>
auto
zpt::JSONArrT::operator==(T _in) -> bool {
    return false;
}
template<typename T>
auto
zpt::JSONArrT::operator!=(T _in) -> bool {
    return true;
}
template<typename T>
auto
zpt::JSONArrT::operator<(T _in) -> bool {
    return false;
}
template<typename T>
auto
zpt::JSONArrT::operator>(T _in) -> bool {
    return false;
}
template<typename T>
auto
zpt::JSONArrT::operator>=(T _in) -> bool {
    return false;
}
template<typename T>
auto
zpt::JSONArrT::operator<=(T _in) -> bool {
    return false;
}

/// Class `zpt::JSONObj` methods
template<typename T>
auto
zpt::JSONObj::operator==(T _rhs) -> bool {
    return (***this) == _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator!=(T _rhs) -> bool {
    return (***this) != _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator<(T _rhs) -> bool {
    return (***this) < _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator>(T _rhs) -> bool {
    return (***this) > _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator<=(T _rhs) -> bool {
    return (***this) <= _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator>=(T _rhs) -> bool {
    return (***this) >= _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator<<(T _in) -> JSONObj& {
    (*this)->push(_in);
    return *this;
}
template<typename T>
auto
zpt::JSONObj::operator>>(T _in) -> JSONObj& {
    (*this)->pop(_in);
    return *this;
}
template<typename T>
auto zpt::JSONObj::operator[](T _idx) -> json& {
    return (***this)[_idx];
}

/// Class `zpt::JSONArr` methods
template<typename T>
auto
zpt::JSONArr::operator==(T _rhs) -> bool {
    return (***this) == _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator!=(T _rhs) -> bool {
    return (***this) != _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator<(T _rhs) -> bool {
    return (***this) < _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator>(T _rhs) -> bool {
    return (***this) > _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator<=(T _rhs) -> bool {
    return (***this) <= _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator>=(T _rhs) -> bool {
    return (***this) >= _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator<<(T _in) -> JSONArr& {
    (*this)->push(_in);
    return *this;
}
template<typename T>
auto
zpt::JSONArr::operator>>(T _in) -> JSONArr& {
    (*this)->pop(_in);
    return *this;
}
template<typename T>
auto zpt::JSONArr::operator[](T _idx) -> json& {
    return (***this)[_idx];
}

/// Class `zpt::JSONElementT` methods
template<typename T>
auto
zpt::JSONElementT::operator<<(T _in) -> JSONElementT& {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
    JSONElementT _e{ _in };
    if (this->__target.__type == _e.type() && _e.type() != zpt::JSObject &&
        _e.type() != zpt::JSArray) {
        (*this) = _in;
        return *this;
    }
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->push(_e);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push(_e);
            break;
        }
        default: {
            assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray,
                    "the type must be a JSObject, JSArray or the same type of the "
                    "target, in order "
                    "to push "
                    "JSONElementT*",
                    0,
                    0);
            break;
        }
    }
    return *this;
}
template<typename T>
auto
zpt::JSONElementT::operator>>(T _in) -> JSONElementT& {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object >> _in;
            break;
        }
        case zpt::JSArray: {
            this->__target.__array >> _in;
            break;
        }
        default: {
            break;
        }
    }
    return *this;
}
template<typename T>
auto zpt::JSONElementT::operator[](T _idx) -> json& {
    if (this->__target.__type == zpt::JSObject) {
        return this->__target.__object[_idx];
    }
    else if (this->__target.__type == zpt::JSArray) {
        return this->__target.__array[_idx];
    }
    return zpt::undefined;
}
template<typename T>
auto
zpt::JSONElementT::operator==(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) == _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator!=(T _in) -> bool {
    if (_in == nullptr) {
        return this->__target.__type == zpt::JSNil;
    }
    JSONElementT _rhs{ _in };
    return (*this) == _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator<(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) < _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator>(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) > _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator<=(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) <= _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator>=(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) >= _rhs;
}

/// Namespace `zpt` methods
template<typename T>
auto
zpt::set(std::string _path, T _value, zpt::json _target) -> zpt::json {
    zpt::json _return;
    if (_target->ok()) {
        _return = _target;
    }
    else {
        _return = zpt::json::object();
    }
    _return->set_path(_path, zpt::mkptr(_value));
    return _return;
}
template<typename T>
auto
zpt::mkelem(T& _e) -> std::unique_ptr<zpt::JSONElementT> {
    return std::move(std::make_unique<zpt::JSONElementT>(_e));
}
template<typename T>
auto
zpt::mkptr(T _v) -> zpt::json {
    T _e(_v);
    return zpt::json{ new zpt::JSONElementT(_e) };
}
