#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>

zpt::JSONElementT::JSONElementT() {
    this->__target.__nil = nullptr;
}

zpt::JSONElementT::JSONElementT(const JSONElementT& _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(JSONElementT&& _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(std::initializer_list<zpt::JSONElementT> _list) {
    (*this) = _list;
}

zpt::JSONElementT::JSONElementT(std::string _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(const char* _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(long long _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(double _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(bool _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(int _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(size_t _rhs) {
    (*this) = _rhs;
}

#ifdef __LP64__
zpt::JSONElementT::JSONElementT(unsigned int _rhs) {
    (*this) = _rhs;
}
#endif

zpt::JSONElementT::JSONElementT(zpt::timestamp_t _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(zpt::json _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(zpt::JSONObj& _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(zpt::JSONArr& _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::JSONElementT(zpt::lambda _rhs) {
    (*this) = _rhs;
}

zpt::JSONElementT::~JSONElementT() {}

auto
zpt::JSONElementT::type() -> zpt::JSONType {
    return (zpt::JSONType)this->__target.__type;
}

auto
zpt::JSONElementT::demangle() -> std::string {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            return "object";
        }
        case zpt::JSArray: {
            return "array";
        }
        case zpt::JSString: {
            return "string";
        }
        case zpt::JSInteger: {
            return "integer";
        }
        case zpt::JSDouble: {
            return "number";
        }
        case zpt::JSBoolean: {
            return "boolean";
        }
        case zpt::JSNil: {
            return "null";
        }
        case zpt::JSDate: {
            return "date";
        }
        case zpt::JSLambda: {
            return "lambda";
        }
    }
    return "null";
}

auto
zpt::JSONElementT::type(zpt::JSONType _in) -> void {
    assertz(_in >= 0, "the type must be a valid value", 500, 0);

    if (_in == this->__target.__type) {
        return;
    }

    switch (this->__target.__type) {
        case zpt::JSObject: {
            if ((*this->__target.__object).get() != nullptr) {
                this->__target.__object.~JSONObj();
            }
            break;
        }
        case zpt::JSArray: {
            if ((*this->__target.__array).get() != nullptr) {
                this->__target.__array.~JSONArr();
            }
            break;
        }
        case zpt::JSString: {
            if (this->__target.__string.get() != nullptr) {
                this->__target.__string.~JSONStr();
            }
            break;
        }
        case zpt::JSLambda: {
            if (this->__target.__lambda.get() != nullptr) {
                this->__target.__lambda.~lambda();
            }
            break;
        }
        default: {
            break;
        }
    }
    switch (_in) {
        case zpt::JSObject: {
            new (&this->__target.__object) JSONObj();
            break;
        }
        case zpt::JSArray: {
            new (&this->__target.__array) JSONArr();
            break;
        }
        case zpt::JSString: {
            new (&this->__target.__string) JSONStr();
            break;
        }
        case zpt::JSLambda: {
            new (&this->__target.__lambda) lambda();
            break;
        }
        default: {
            break;
        }
    }

    this->__target.__type = _in;
}

auto
zpt::JSONElementT::value() -> zpt::JSONUnion& {
    return this->__target;
}

auto
zpt::JSONElementT::ok() -> bool {
    return this->__target.__type != zpt::JSNil;
}

auto
zpt::JSONElementT::empty() -> bool {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if ((*this->__target.__object).get() != nullptr) {
                return (***this->__target.__object).size() == 0;
            }
            return true;
        }
        case zpt::JSArray: {
            if ((*this->__target.__array).get() != nullptr) {
                return (***this->__target.__array).size() == 0;
            }
            return true;
        }
        case zpt::JSString: {
            if (this->__target.__string.get() != nullptr) {
                return this->__target.__string->length() == 0;
            }
            return true;
        }
        case zpt::JSInteger: {
            return false;
        }
        case zpt::JSDouble: {
            return false;
        }
        case zpt::JSBoolean: {
            return false;
        }
        case zpt::JSNil: {
            return true;
        }
        case zpt::JSDate: {
            return false;
        }
        case zpt::JSLambda: {
            return false;
        }
    }
    return true;
}

auto
zpt::JSONElementT::nil() -> bool {
    return this->__target.__type == zpt::JSNil;
}

auto
zpt::JSONElementT::size() -> size_t {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if ((*this->__target.__object).get() != nullptr) {
                return (***this->__target.__object).size();
            }
        }
        case zpt::JSArray: {
            if ((*this->__target.__array).get() != nullptr) {
                return (***this->__target.__array).size();
            }
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            break;
        }
    }
    return 0;
}

// auto
// zpt::JSONElementT::assign(JSONElementT& _rhs) -> void {
//     this->type(_rhs.type());
//     switch (this->__target.__type) {
//         case zpt::JSObject: {
//             if ((*this->__target.__object).get() != nullptr) {
//                 this->__target.__object.~JSONObj();
//             }
//             if ((*_rhs.obj()).get() != nullptr) {
//                 this->__target.__object = _rhs.obj();
//             }
//             break;
//         }
//         case zpt::JSArray: {
//             if ((*this->__target.__array).get() != nullptr) {
//                 this->__target.__array.~JSONArr();
//             }
//             if ((*_rhs.arr()).get() != nullptr) {
//                 this->__target.__array = _rhs.arr();
//             }
//             break;
//         }
//         case zpt::JSString: {
//             if (this->__target.__string.get() != nullptr) {
//                 this->__target.__string.~JSONStr();
//             }
//             this->__target.__string = std::make_shared<std::string>(std::string(_rhs.str().data()));
//             break;
//         }
//         case zpt::JSInteger: {
//             this->__target.__integer = _rhs.intr();
//             break;
//         }
//         case zpt::JSDouble: {
//             this->__target.__double = _rhs.dbl();
//             break;
//         }
//         case zpt::JSBoolean: {
//             this->__target.__boolean = _rhs.bln();
//             break;
//         }
//         case zpt::JSNil: {
//             this->__target.__nil = nullptr;
//             break;
//         }
//         case zpt::JSDate: {
//             this->__target.__date = _rhs.date();
//             break;
//         }
//         case zpt::JSLambda: {
//             if (this->__target.__lambda.get() != nullptr) {
//                 this->__target.__lambda.~lambda();
//             }
//             this->__target.__lambda = _rhs.lbd();
//             break;
//         }
//     }
// }

auto
zpt::JSONElementT::parent() -> zpt::json {
    return this->__parent;
}

auto
zpt::JSONElementT::parent(zpt::json& _parent) -> void {
    this->__parent = _parent;
}

auto
zpt::JSONElementT::parent(std::nullptr_t) -> void {
    (*this->__parent).reset();
}

auto
zpt::JSONElementT::is_object() -> bool {
    return this->__target.__type == zpt::JSObject;
}

auto
zpt::JSONElementT::is_array() -> bool {
    return this->__target.__type == zpt::JSArray;
}

auto
zpt::JSONElementT::is_string() -> bool {
    return this->__target.__type == zpt::JSString;
}

auto
zpt::JSONElementT::is_integer() -> bool {
    return this->__target.__type == zpt::JSInteger;
}

auto
zpt::JSONElementT::is_double() -> bool {
    return this->__target.__type == zpt::JSDouble;
}

auto
zpt::JSONElementT::is_number() -> bool {
    return this->__target.__type == zpt::JSInteger || this->__target.__type == zpt::JSDouble;
}

auto
zpt::JSONElementT::is_bool() -> bool {
    return this->__target.__type == zpt::JSBoolean;
}

auto
zpt::JSONElementT::is_date() -> bool {
    return this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSString;
}

auto
zpt::JSONElementT::is_lambda() -> bool {
    return this->__target.__type == zpt::JSLambda;
}

auto
zpt::JSONElementT::is_nil() -> bool {
    return this->__target.__type == zpt::JSNil;
}

auto
zpt::JSONElementT::is_iterable() -> bool {
    return this->__target.__type == zpt::JSNil;
}

auto
zpt::JSONElementT::obj() -> zpt::JSONObj& {
    assertz(this->__target.__type == zpt::JSObject,
            std::string("this element is not of type JSObject: ") + this->stringify(),
            500,
            1100);
    return this->__target.__object;
}

auto
zpt::JSONElementT::arr() -> zpt::JSONArr& {
    assertz(this->__target.__type == zpt::JSArray,
            std::string("this element is not of type JSArray: ") + this->stringify(),
            500,
            1100);
    return this->__target.__array;
}

auto
zpt::JSONElementT::str() -> std::string {
    assertz(this->__target.__type == zpt::JSString,
            std::string("this element is not of type JSString: ") + this->stringify(),
            500,
            1100);
    return *(this->__target.__string.get());
}

auto
zpt::JSONElementT::intr() -> long long {
    assertz(this->__target.__type == zpt::JSInteger,
            std::string("this element is not of type JSInteger: ") + this->stringify(),
            500,
            1100);
    return this->__target.__integer;
}

auto
zpt::JSONElementT::dbl() -> double {
    assertz(this->__target.__type == zpt::JSDouble,
            std::string("this element is not of type JSDouble: ") + this->stringify(),
            500,
            1100);
    return this->__target.__double;
}

auto
zpt::JSONElementT::bln() -> bool {
    assertz(this->__target.__type == zpt::JSBoolean,
            std::string("this element is not of type JSBoolean: ") + this->stringify(),
            500,
            1100);
    return this->__target.__boolean;
}

auto
zpt::JSONElementT::date() -> zpt::timestamp_t {
    assertz(this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSString,
            std::string("this element is not of type JSDate: ") + this->stringify(),
            500,
            1100);
    if (this->__target.__type == zpt::JSString) {
        return zpt::timestamp(*(this->__target.__string.get()));
    }
    return this->__target.__date;
}

auto
zpt::JSONElementT::lbd() -> zpt::lambda& {
    assertz(this->__target.__type == zpt::JSLambda,
            std::string("this element is not of type JSLambda: ") + this->stringify(),
            500,
            1100);
    return this->__target.__lambda;
}

auto
zpt::JSONElementT::number() -> double {
    assertz(this->__target.__type == zpt::JSDate || this->__target.__type == zpt::JSInteger ||
              this->__target.__type == zpt::JSDouble || this->__target.__type == zpt::JSBoolean,
            std::string("this element is not of type JSInteger, JSDouble or JSBoolean: ") +
              this->stringify(),
            500,
            1100);
    switch (this->__target.__type) {
        case zpt::JSInteger: {
            return (double)this->__target.__integer;
        }
        case zpt::JSDouble: {
            return this->__target.__double;
        }
        case zpt::JSBoolean: {
            return (double)this->__target.__boolean;
        }
        case zpt::JSDate: {
            return (double)this->__target.__date;
        }
        default: {
            return 0;
        }
    }
    return 0;
}

auto
zpt::JSONElementT::clone() -> zpt::json {
    switch (this->type()) {
        case zpt::JSObject: {
            return this->obj()->clone();
        }
        case zpt::JSArray: {
            return this->arr()->clone();
        }
        case zpt::JSString: {
            std::string _v = this->str();
            return zpt::mkptr(_v);
        }
        case zpt::JSInteger: {
            int _v = this->intr();
            return zpt::mkptr(_v);
        }
        case zpt::JSDouble: {
            double _v = this->dbl();
            return zpt::mkptr(_v);
        }
        case zpt::JSBoolean: {
            bool _v = this->bln();
            return zpt::mkptr(_v);
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            zpt::timestamp_t _v = this->date();
            return zpt::mkptr(_v);
        }
        case zpt::JSLambda: {
            return zpt::json::lambda(this->lbd()->name(), this->lbd()->n_args());
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::operator=(const JSONElementT& _rhs) -> JSONElementT& {
    this->deinit();
    this->__parent = _rhs.__parent;
    this->__target.__type = _rhs.__target.__type;

    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object = _rhs.__target.__object;
            break;
        }
        case zpt::JSArray: {
            this->__target.__array = _rhs.__target.__array;
            break;
        }
        case zpt::JSString: {
            this->__target.__string = _rhs.__target.__string;
            break;
        }
        case zpt::JSInteger: {
            this->__target.__integer = _rhs.__target.__integer;
            break;
        }
        case zpt::JSDouble: {
            this->__target.__double = _rhs.__target.__double;
            break;
        }
        case zpt::JSBoolean: {
            this->__target.__boolean = _rhs.__target.__boolean;
            break;
        }
        case zpt::JSNil: {
            this->__target.__nil = nullptr;
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = _rhs.__target.__date;
            break;
        }
        case zpt::JSLambda: {
            this->__target.__lambda = _rhs.__target.__lambda;
            break;
        }
    }
    return (*this);
}

auto
zpt::JSONElementT::operator=(JSONElementT&& _rhs) -> JSONElementT& {
    this->deinit();
    this->__target.__type = _rhs.__target.__type;
    this->__parent = _rhs.__parent;
    _rhs.__parent = nullptr;

    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object = std::move(_rhs.__target.__object);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array = std::move(_rhs.__target.__array);
            break;
        }
        case zpt::JSString: {
            this->__target.__string = std::move(_rhs.__target.__string);
            break;
        }
        case zpt::JSInteger: {
            this->__target.__integer = std::move(_rhs.__target.__integer);
            break;
        }
        case zpt::JSDouble: {
            this->__target.__double = std::move(_rhs.__target.__double);
            break;
        }
        case zpt::JSBoolean: {
            this->__target.__boolean = std::move(_rhs.__target.__boolean);
            break;
        }
        case zpt::JSNil: {
            this->__target.__nil = nullptr;
            break;
        }
        case zpt::JSDate: {
            this->__target.__date = std::move(_rhs.__target.__date);
            break;
        }
        case zpt::JSLambda: {
            this->__target.__lambda = std::move(_rhs.__target.__lambda);
            break;
        }
    }
    return (*this);
}

auto
zpt::JSONElementT::operator=(std::string _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(std::initializer_list<zpt::JSONElementT> _list) -> JSONElementT& {
    this->deinit();

    bool _is_array =
      (_list.size() > 1 && _list.begin()->__target.__type == zpt::JSString &&
       (*_list.begin()->__target.__string) == "1b394520-2fed-4118-b622-940f25b8b35e");

    assertz(_is_array || (_list.size() % 2 == 0 && _list.begin()->__target.__type == zpt::JSString),
            "initializer list parameter doesn't seem either an array or an object",
            500,
            0);

    this->type(_is_array ? zpt::JSArray : zpt::JSObject);

    size_t _idx = 0;
    for (auto _element : _list) {
        if (_is_array && _idx == 0) {
            continue;
        }
        if (!_is_array && _idx % 2 == 0) {
            this->__target.__object->push(*_element.__target.__string);
        }
        else {
            zpt::json _other;
            _other->type(_element.__target.__type);
            switch (_other->__target.__type) {
                case zpt::JSObject: {
                    _other->__target.__object = _element.__target.__object;
                    break;
                }
                case zpt::JSArray: {
                    _other->__target.__array = _element.__target.__array;
                    break;
                }
                case zpt::JSString: {
                    _other->__target.__string =
                      std::make_shared<std::string>(*_element.__target.__string);
                    break;
                }
                case zpt::JSInteger: {
                    _other->__target.__integer = _element.__target.__integer;
                    break;
                }
                case zpt::JSDouble: {
                    _other->__target.__double = _element.__target.__double;
                    break;
                }
                case zpt::JSBoolean: {
                    _other->__target.__boolean = _element.__target.__boolean;
                    break;
                }
                case zpt::JSNil: {
                    _other->__target.__nil = nullptr;
                    break;
                }
                case zpt::JSDate: {
                    _other->__target.__date = _element.__target.__date;
                    break;
                }
                case zpt::JSLambda: {
                    if (_element.__target.__lambda.get() != nullptr) {
                        _other->__target.__lambda = _element.__target.__lambda;
                    }
                    break;
                }
            }
            this->__target.__object->push(_other);
        }
        _idx++;
    }
    return (*this);
}

auto
zpt::JSONElementT::operator=(const char* _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(long long _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(double _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(bool _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(int _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(size_t _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

#ifdef __LP64__
auto
zpt::JSONElementT::operator=(unsigned int _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

#endif
auto
zpt::JSONElementT::operator=(zpt::json _rhs) -> JSONElementT& {
    this->deinit();
    (*this) = (***_rhs);
    return (*this);
}

auto
zpt::JSONElementT::operator=(zpt::timestamp_t _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(zpt::JSONObj& _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(zpt::JSONArr& _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

auto
zpt::JSONElementT::operator=(zpt::lambda _rhs) -> JSONElementT& {
    this->deinit();

    return (*this);
}

zpt::JSONElementT::operator std::string() {
    return this->str();
}

zpt::JSONElementT::operator bool() {
    return this->bln();
}

zpt::JSONElementT::operator int() {
    return this->intr();
}

zpt::JSONElementT::operator long() {
    return this->intr();
}

zpt::JSONElementT::operator long long() {
    return this->intr();
}

zpt::JSONElementT::operator size_t() {
    return this->intr();
}

zpt::JSONElementT::operator double() {
    return this->dbl();
}

#ifdef __LP64__
zpt::JSONElementT::operator unsigned int() {
    return this->intr();
}
#endif

zpt::JSONElementT::operator zpt::pretty() {
    return zpt::pretty{ this->prettify() };
}

zpt::JSONElementT::operator zpt::timestamp_t() {
    return this->();
}

zpt::JSONElementT::operator zpt::JSONObj() {
    return;
}

zpt::JSONElementT::operator zpt::JSONArr() {
    return;
}

zpt::JSONElementT::operator zpt::JSONObj&() {
    return;
}

zpt::JSONElementT::operator zpt::JSONArr&() {
    return;
}

zpt::JSONElementT::operator zpt::lambda() {
    return;
}

auto
zpt::JSONElementT::operator<<(const char* _in) -> zpt::JSONElementT& {
    (*this) << std::string(_in);
    return *this;
}

auto
zpt::JSONElementT::operator<<(std::string _in) -> zpt::JSONElementT& {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->push(_in);
            break;
        }
        case zpt::JSArray: {
            zpt::JSONElementT _element{ _in };
            this->__target.__array->push(_element);
            break;
        }
        case zpt::JSString: {
            this->__target.__string.get()->assign(_in);
            break;
        }
        default: {
            assertz(this->__target.__type == zpt::JSObject ||
                      this->__target.__type == zpt::JSArray ||
                      this->__target.__type == zpt::JSString,
                    "the type must be a JSObject, JSArray or JSString in order to push "
                    "a std::string",
                    500,
                    0);
            break;
        }
    }
    return *this;
}

auto
zpt::JSONElementT::operator<<(JSONElementT _in) -> zpt::JSONElementT& {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    if (this->__target.__type == _in.type() && _in.type() != zpt::JSObject &&
        _in.type() != zpt::JSArray) {
        this->assign(_in);
        return *this;
    }
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->push(_in);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push(_in);
            break;
        }
        default: {
            assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray,
                    "the type must be a JSObject, JSArray or the same type of the "
                    "target, in order to push "
                    "JSONElementT*",
                    500,
                    0);
            break;
        }
    }
    return *this;
}

auto
zpt::JSONElementT::operator<<(std::initializer_list<zpt::JSONElementT> _list)
  -> zpt::JSONElementT& {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    zpt::json _other(_list);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (_other->type() == zpt::JSObject) {
                for (auto [_idx, _key, _attribute] : _other) {
                    this->__target.__object->push(_key);
                    this->__target.__object->push(_attribute);
                }
            }
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push(_other);
            break;
        }
        default: {
            assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray,
                    "the type must be a JSObject, JSArray or the same type of the "
                    "target, in order to push "
                    "JSONElementT*",
                    500,
                    0);
            break;
        }
    }
    return *this;
}

auto
zpt::JSONElementT::operator==(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) == *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) == *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) == _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer == _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double == _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean == _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date == _in.number();
        }
        case zpt::JSLambda: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return this->__target.__lambda->signature() == _in.lbd()->signature();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator==(zpt::json _rhs) -> bool {
    return *this == **_rhs;
}

auto
zpt::JSONElementT::operator!=(JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    if (this->__target.__type != _in.type()) {
        return true;
    }
    switch (this->__target.__type) {
        case zpt::JSObject: {
            return this->__target.__object != _in.obj();
        }
        case zpt::JSArray: {
            return this->__target.__array != _in.arr();
        }
        case zpt::JSString: {
            return *(this->__target.__string.get()) != _in.str();
        }
        case zpt::JSInteger: {
            return this->__target.__integer != _in.intr();
        }
        case zpt::JSDouble: {
            return this->__target.__double != _in.dbl();
        }
        case zpt::JSBoolean: {
            return this->__target.__boolean != _in.bln();
        }
        case zpt::JSNil: {
            return true;
        }
        case zpt::JSDate: {
            return this->__target.__date != _in.date();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->signature() != _in.lbd()->signature();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator!=(zpt::json _rhs) -> bool {
    return *this != _rhs;
}

auto
zpt::JSONElementT::operator<(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) < *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) < *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) < _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer < _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double < _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean < _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date < _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() < _in.lbd()->n_args();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator<(zpt::json _rhs) -> bool {
    return *this < _rhs;
}

auto
zpt::JSONElementT::operator>(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) > *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) > *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) > _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer > _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double > _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean > _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date > _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() > _in.lbd()->n_args();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator>(zpt::json _rhs) -> bool {
    return *this > _rhs;
}

auto
zpt::JSONElementT::operator<=(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) <= *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) <= *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) <= _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer <= _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double <= _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean <= _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date <= _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() <= _in.lbd()->n_args();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator<=(zpt::json _rhs) -> bool {
    return *this <= _rhs;
}

auto
zpt::JSONElementT::operator>=(zpt::JSONElementT& _in) -> bool {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__object) >= *(_in.obj());
        }
        case zpt::JSArray: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__array) >= *(_in.arr());
        }
        case zpt::JSString: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return *(this->__target.__string.get()) >= _in.str();
        }
        case zpt::JSInteger: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__integer >= _in.number();
        }
        case zpt::JSDouble: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__double >= _in.number();
        }
        case zpt::JSBoolean: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__boolean >= _in.number();
        }
        case zpt::JSNil: {
            if (this->__target.__type != _in.type()) {
                return false;
            }
            return true;
        }
        case zpt::JSDate: {
            if (_in.type() != zpt::JSDate && _in.type() != zpt::JSInteger &&
                _in.type() != zpt::JSDouble && _in.type() != zpt::JSBoolean) {
                return false;
            }
            return this->__target.__date >= _in.number();
        }
        case zpt::JSLambda: {
            return this->__target.__lambda->n_args() >= _in.lbd()->n_args();
        }
    }
    return false;
}

auto
zpt::JSONElementT::operator>=(zpt::json _rhs) -> bool {
    return *this >= _rhs;
}

auto
zpt::JSONElementT::operator+(zpt::json _rhs) -> zpt::json {
    return (*this) + (**_rhs);
}

auto
zpt::JSONElementT::operator+(zpt::JSONElementT& _rhs) -> zpt::json {
    if (this->__target.__type == zpt::JSNil) {
        zpt::json _rrhs = _rhs.clone();
        return _rrhs;
    }
    if (_rhs.__target.__type == zpt::JSNil) {
        zpt::json _lhs = this->clone();
        return _lhs;
    }
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    assertz(this->__target.__type == zpt::JSArray || _rhs.__target.__type == zpt::JSArray ||
              this->__target.__type == _rhs.__target.__type,
            "can't add JSON objects of different types",
            500,
            0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            zpt::json _lhs = this->clone();
            for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                if (_lhs[_key]->type() == zpt::JSObject || _lhs[_key]->type() == zpt::JSArray) {
                    _lhs << _key << (_lhs[_key] + _e);
                }
                else {
                    _lhs << _key << _e;
                }
            }
            return _lhs;
        }
        case zpt::JSArray: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = this->clone();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << _e;
                }
                return _lhs;
            }
            else {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ this }) {
                    _lhs << (_e + _rhs);
                }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                std::string _lhs((*(this->__target.__string.get())) + _rhs.str());
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSInteger: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__integer + _rhs.intr();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSDouble: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                double _lhs = this->__target.__double + _rhs.dbl();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSBoolean: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                bool _lhs = this->__target.__boolean || _rhs.bln();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) + _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__date + _rhs.number();
                return zpt::mkptr((zpt::timestamp_t)_lhs);
            }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::operator-(zpt::json _rhs) -> zpt::json {
    return (*this) - (**_rhs);
}

auto
zpt::JSONElementT::operator-(zpt::JSONElementT& _rhs) -> zpt::json {
    if (this->__target.__type == zpt::JSNil) {
        zpt::json _rrhs = _rhs.clone();
        return _rrhs;
    }
    if (_rhs.__target.__type == zpt::JSNil) {
        zpt::json _lhs = this->clone();
        return _lhs;
    }
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    assertz(this->__target.__type == zpt::JSArray ||
              (this->__target.__type != zpt::JSObject && _rhs.__target.__type == zpt::JSArray) ||
              this->__target.__type == _rhs.__target.__type,
            "can't substract JSON objects of different types",
            500,
            0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            zpt::json _lhs = this->clone();
            for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                _lhs >> _key;
            }
            return _lhs;
        }
        case zpt::JSArray: {
            if (_rhs.__target.__type == zpt::JSArray) {
                assertz((***this->arr()).size() == (***_rhs.arr()).size(),
                        "both arrays must have the same size",
                        500,
                        0);
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << (this[_idx] - _rhs[_idx]);
                }
                return _lhs;
            }
            else {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << (_e - _rhs);
                }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                std::string _lhs(this->__target.__string.get()->data());
                std::size_t _idx = 0;
                while ((_idx = _lhs.find(_rhs.str(), _idx)) != std::string::npos) {
                    _lhs.erase(_idx, _rhs.str().length());
                }
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSInteger: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__integer - _rhs.intr();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSDouble: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                double _lhs = this->__target.__double - _rhs.dbl();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSBoolean: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                bool _lhs = this->__target.__boolean & _rhs.bln();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) - _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__date - _rhs.number();
                return zpt::mkptr((zpt::timestamp_t)_lhs);
            }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::operator/(zpt::json _rhs) -> zpt::json {
    return (*this) / (**_rhs);
}

auto
zpt::JSONElementT::operator/(zpt::JSONElementT& _rhs) -> zpt::json {
    if (this->__target.__type == zpt::JSNil) {
        zpt::json _rrhs = _rhs.clone();
        return _rrhs;
    }
    if (_rhs.__target.__type == zpt::JSNil) {
        zpt::json _lhs = this->clone();
        return _lhs;
    }
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    assertz(this->__target.__type == zpt::JSArray || _rhs.__target.__type == zpt::JSArray ||
              this->__target.__type == _rhs.__target.__type,
            "can't divide JSON objects of different types",
            500,
            0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            assertz(this->__target.__type == zpt::JSObject, "can't divide JSON objects", 500, 0);
        }
        case zpt::JSArray: {
            if (_rhs.__target.__type == zpt::JSArray) {
                assertz((***this->arr()).size() == (***_rhs.arr()).size(),
                        "both arrays must have the same size",
                        500,
                        0);
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << (this[_idx] / _rhs[_idx]);
                }
                return _lhs;
            }
            else {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << (_e / _rhs);
                }
                return _lhs;
            }
        }
        case zpt::JSString: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                std::string _lhs(this->__target.__string.get()->data());
                std::size_t _idx = 0;
                while ((_idx = _lhs.find(_rhs.str(), _idx)) != std::string::npos) {
                    _lhs.erase(_idx, _rhs.str().length());
                }
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSInteger: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__integer / _rhs.intr();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSDouble: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                double _lhs = this->__target.__double / _rhs.dbl();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSBoolean: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                bool _lhs = this->__target.__boolean / _rhs.bln();
                return zpt::mkptr(_lhs);
            }
        }
        case zpt::JSNil: {
            return zpt::undefined;
        }
        case zpt::JSDate: {
            if (_rhs.__target.__type == zpt::JSArray) {
                zpt::json _lhs = zpt::json::array();
                for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                    _lhs << ((*this) / _e);
                }
                return _lhs;
            }
            else {
                int _lhs = this->__target.__date / _rhs.number();
                return zpt::mkptr((zpt::timestamp_t)_lhs);
            }
        }
        case zpt::JSLambda: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::operator|(zpt::json _rhs) -> zpt::json {
    return (*this) | (**_rhs);
}

auto
zpt::JSONElementT::operator|(zpt::JSONElementT& _rhs) -> zpt::json {
    if (this->__target.__type == zpt::JSNil) {
        zpt::json _rrhs = _rhs.clone();
        return _rrhs;
    }
    if (_rhs.__target.__type == zpt::JSNil) {
        zpt::json _lhs = this->clone();
        return _lhs;
    }
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    assertz(this->__target.__type == _rhs.__target.__type,
            "can't add JSON objects of different types",
            500,
            0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            zpt::json _lhs = this->clone();
            for (auto [_idx, _key, _e] : zpt::json{ _rhs }) {
                if (_lhs[_key]->type() == zpt::JSObject) {
                    _lhs << _key << (_lhs[_key] | _e);
                }
                else {
                    _lhs << _key << _e;
                }
            }
            return _lhs;
        }
        case zpt::JSArray:
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::get_path(std::string _path, std::string _separator) -> zpt::json {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            return this->__target.__object->get_path(_path, _separator);
        }
        case zpt::JSArray: {
            return this->__target.__array->get_path(_path, _separator);
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSDate: {
            return zpt::undefined;
        }
    }
    return zpt::undefined;
}

auto
zpt::JSONElementT::set_path(std::string _path, zpt::json _value, std::string _separator) -> void {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->set_path(_path, _value, _separator);
            return;
        }
        case zpt::JSArray: {
            this->__target.__array->set_path(_path, _value, _separator);
            return;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSDate: {
            return;
        }
    }
    return;
}

auto
zpt::JSONElementT::del_path(std::string _path, std::string _separator) -> void {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 500, 0);
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->del_path(_path, _separator);
            return;
        }
        case zpt::JSArray: {
            this->__target.__array->del_path(_path, _separator);
            return;
        }
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSLambda:
        case zpt::JSDate: {
            return;
        }
    }
}

auto
zpt::JSONElementT::flatten() -> zpt::json {
    if (this->type() == zpt::JSObject || this->type() == zpt::JSArray) {
        zpt::json _return = zpt::json::object();
        this->inspect(
          { "$any", "type" },
          [&](std::string _object_path, std::string _key, zpt::JSONElementT& _parent) -> void {
              zpt::json _self = this->get_path(_object_path);
              if (_self->type() != zpt::JSObject && _self->type() != zpt::JSArray) {
                  _return << _object_path << _self;
              }
          });
        return _return;
    }
    else {
        return this->clone();
    }
}

auto
zpt::JSONElementT::inspect(
  zpt::json _pattern,
  std::function<void(std::string, std::string, zpt::JSONElementT&)> _callback,
  zpt::JSONElementT* _parent,
  std::string _key,
  std::string _parent_path) -> void {
    switch (this->type()) {
        case zpt::JSObject: {
            for (auto [_idx, _name, _item] : zpt::json{ this }) {
                if (_pattern->type() == zpt::JSObject && _pattern[_name]->ok()) {
                    _item->inspect(_pattern[_name],
                                   _callback,
                                   this,
                                   _name,
                                   (_parent_path.length() != 0
                                      ? (_parent_path + std::string(".") + _key)
                                      : _key));
                    continue;
                }
                _item->inspect(
                  _pattern,
                  _callback,
                  this,
                  _name,
                  (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key));
            }
            if (_pattern["$any"]->ok() && _parent_path.length() != 0) {
                _callback(_parent_path + std::string(".") + _key, _key, *_parent);
            }
            else {
                if (*this == _pattern) {
                    _callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key)
                                                          : _key),
                              _key,
                              *_parent);
                }
            }
            break;
        }
        case zpt::JSArray: {
            for (auto [_idx, _name, _item] : zpt::json{ this }) {
                _item->inspect(
                  _pattern,
                  _callback,
                  this,
                  std::to_string(_idx),
                  (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key));
            }
            if (_pattern["$any"]->ok() && _parent_path.length() != 0) {
                _callback(
                  (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key),
                  _key,
                  *_parent);
            }
            else {
                if (*this == _pattern) {
                    _callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key)
                                                          : _key),
                              _key,
                              *_parent);
                }
            }
            break;
        }
        default: {
            if (_pattern["$regexp"]->ok()) {
                std::regex _rgx(((std::string)_pattern["$regexp"]));
                std::string _exp;
                this->stringify(_exp);
                if (std::regex_match(_exp, _rgx)) {
                    _callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key)
                                                          : _key),
                              _key,
                              *_parent);
                }
            }
            else if (_pattern["$any"]->ok()) {
                _callback(
                  (_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key) : _key),
                  _key,
                  *_parent);
            }
            else {
                if (*this == _pattern) {
                    _callback((_parent_path.length() != 0 ? (_parent_path + std::string(".") + _key)
                                                          : _key),
                              _key,
                              *_parent);
                }
            }
            break;
        }
    }
}

auto
zpt::JSONElementT::stringify(std::ostream& _out) -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->stringify(_out);
            break;
        }
        case zpt::JSString: {
            std::string _str(this->str());
            zpt::json::stringify(_str);
            _out << "\"" << _str << "\"" << std::flush;
            break;
        }
        case zpt::JSInteger: {
            _out << this->__target.__integer << std::flush;
            break;
        }
        case zpt::JSDouble: {
            _out << this->__target.__double << std::flush;
            break;
        }
        case zpt::JSBoolean: {
            _out << (this->__target.__boolean ? "true" : "false") << std::flush;
            break;
        }
        case zpt::JSNil: {
            _out << "null" << std::flush;
            break;
        }
        case zpt::JSDate: {
            _out << "\"" << zpt::timestamp(this->__target.__date) << "\"" << std::flush;
            break;
        }
        case zpt::JSLambda: {
            _out << this->__target.__lambda->signature() << std::flush;
            break;
        }
    }
}

auto
zpt::JSONElementT::stringify(std::string& _out) -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->stringify(_out);
            break;
        }
        case zpt::JSString: {
            std::string _str(this->str());
            zpt::json::stringify(_str);
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), _str);
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->__target.__integer);
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->__target.__double);
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->__target.__boolean);
            break;
        }
        case zpt::JSNil: {
            _out.insert(_out.length(), "null");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), zpt::timestamp(this->__target.__date));
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSLambda: {
            _out.insert(_out.length(), this->__target.__lambda->signature());
            break;
        }
    }
}

auto
zpt::JSONElementT::stringify() -> std::string {
    std::string _out;
    this->stringify(_out);
    return _out;
}

auto
zpt::JSONElementT::prettify(std::ostream& _out, uint _n_tabs) -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSString: {
            std::string _str(this->str());
            zpt::json::stringify(_str);
            _out << "\"" << _str << "\"" << std::flush;
            break;
        }
        case zpt::JSInteger: {
            _out << this->__target.__integer << std::flush;
            break;
        }
        case zpt::JSDouble: {
            _out << this->__target.__double << std::flush;
            break;
        }
        case zpt::JSBoolean: {
            _out << (this->__target.__boolean ? "true" : "false") << std::flush;
            break;
        }
        case zpt::JSNil: {
            _out << "null" << std::flush;
            break;
        }
        case zpt::JSDate: {
            _out << "\"" << zpt::timestamp(this->__target.__date) << "\"" << std::flush;
            break;
        }
        case zpt::JSLambda: {
            _out << this->__target.__lambda->signature() << std::flush;
            break;
        }
    }
    if (_n_tabs == 0) {
        _out << std::endl << std::flush;
    }
}

auto
zpt::JSONElementT::prettify(std::string& _out, uint _n_tabs) -> void {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->prettify(_out, _n_tabs);
            break;
        }
        case zpt::JSString: {
            std::string _str(this->str());
            zpt::json::stringify(_str);
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), _str);
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->__target.__integer);
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->__target.__double);
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->__target.__boolean);
            break;
        }
        case zpt::JSNil: {
            _out.insert(_out.length(), "null");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), "\"");
            _out.insert(_out.length(), zpt::timestamp(this->__target.__date));
            _out.insert(_out.length(), "\"");
            break;
        }
        case zpt::JSLambda: {
            _out.insert(_out.length(), this->__target.__lambda->signature());
            break;
        }
    }
    if (_n_tabs == 0) {
        _out.insert(_out.length(), "\n");
    }
}

auto
zpt::JSONElementT::prettify() -> std::string {
    std::string _out{ "" };
    this->prettify(_out);
    return _out;
}

auto
zpt::JSONElementT::element(size_t _pos) -> std::tuple<size_t, std::string, zpt::json> {
    switch (this->__target.__type) {
        case zpt::JSObject:
            return std::make_tuple(
              _pos, this->__target.__object->key_for(_pos), this->__target.__object[_pos]);
        case zpt::JSArray:
            return std::make_tuple(_pos, std::to_string(_pos), this->__target.__array[_pos]);
        case zpt::JSString:
        case zpt::JSInteger:
        case zpt::JSDouble:
        case zpt::JSBoolean:
        case zpt::JSNil:
        case zpt::JSDate:
        case zpt::JSLambda:
            break;
    }
    return std::make_tuple(0, "", zpt::json{ this });
}

auto
zpt::JSONElementT::deinit() -> JSONElementT& {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object.~JSONObj();
            break;
        }
        case zpt::JSArray: {
            this->__target.__array.~JSONArr();
            break;
        }
        case zpt::JSString: {
            this->__target.__string.~JSONStr();
            break;
        }
        case zpt::JSLambda: {
            this->__target.__lambda.~lambda();
            break;
        }
        default: {
            break;
        }
    }
    return (*this);
}
