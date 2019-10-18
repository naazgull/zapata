#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>
#include <zapata/json/JSONParser.h>

namespace zpt {
zpt::json undefined;
zpt::json nilptr = undefined;
zpt::json array = zpt::mkptr("1b394520-2fed-4118-b622-940f25b8b35e");
symbol_table __lambdas = zpt::symbol_table(
  new std::map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>());
}

zpt::pretty::pretty(std::string _rhs)
  : __underlying{ _rhs } {}

zpt::pretty::pretty(const char* _rhs)
  : __underlying{ _rhs } {}

zpt::pretty::pretty(const pretty& _rhs) {
    (*this) = _rhs;
}

zpt::pretty::pretty(pretty&& _rhs) {
    (*this) = _rhs;
}

auto
zpt::pretty::operator=(const pretty& _rhs) -> pretty& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::pretty::operator=(pretty&& _rhs) -> pretty& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::pretty::operator-> () -> std::string& {
    return this->__underlying;
}

auto zpt::pretty::operator*() -> std::string& {
    return this->__underlying;
}

zpt::pretty::operator std::string() {
    return this->__underlying;
}

/*JSON POINTER TO ELEMENT*/
zpt::json::json()
  : __underlying{ std::make_shared<zpt::JSONElementT>() } {}

zpt::json::json(std::nullptr_t)
  : __underlying{ nullptr } {}

zpt::json::json(const zpt::JSONElementT& _target)
  : __underlying{ std::make_shared<zpt::JSONElementT>(_target) } {}

zpt::json::json(std::unique_ptr<zpt::JSONElementT> _target)
  : __underlying{ _target.release() } {}

zpt::json::json(std::initializer_list<zpt::JSONElementT> _init)
  : __underlying{ std::make_shared<zpt::JSONElementT>(_init) } {}

zpt::json::json(std::string _rhs) {
    std::istringstream _iss;
    _iss.str(_rhs);
    _iss >> (*this);
}

zpt::json::json(const char* _rhs) {
    std::istringstream _iss;
    _iss.str(std::string(_rhs));
    _iss >> (*this);
}

zpt::json::json(zpt::pretty _rhs) {
    std::istringstream _iss;
    _iss.str(_rhs);
    _iss >> (*this);
}

zpt::json::json(const zpt::json& _rhs) {
    (*this) = _rhs;
}

zpt::json::json(zpt::json&& _rhs) {
    (*this) = _rhs;
}

zpt::json::~json() {}

auto
zpt::json::value() -> zpt::JSONElementT& {
    if (this->__underlying.get() == nullptr) {
        return *(zpt::undefined.__underlying.get());
    }
    return *(this->__underlying.get());
}

auto
zpt::json::operator=(const zpt::json& _rhs) -> zpt::json& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::json::operator=(zpt::json&& _rhs) -> zpt::json& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::json::operator-> () -> std::shared_ptr<zpt::JSONElementT>& {
    return this->__underlying;
}

auto zpt::json::operator*() -> std::shared_ptr<zpt::JSONElementT>& {
    return this->__underlying;
}

zpt::json::operator std::string() {
    if (this->__underlying.get() == nullptr) {
        return "";
    }
    std::string _out;
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            this->__underlying.get()->obj()->stringify(_out);
            break;
        }
        case zpt::JSArray: {
            this->__underlying.get()->arr()->stringify(_out);
            break;
        }
        case zpt::JSString: {
            _out.assign(this->__underlying.get()->str().data());
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->__underlying.get()->intr());
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->__underlying.get()->dbl());
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->__underlying.get()->bln());
            break;
        }
        case zpt::JSNil: {
            _out.assign("");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), zpt::timestamp(this->__underlying.get()->date()));
            break;
        }
        case zpt::JSLambda: {
            _out.assign(this->__underlying.get()->lbd()->signature());
            break;
        }
    }
    return _out;
}

zpt::json::operator zpt::pretty() {
    if (this->__underlying.get() == nullptr) {
        return zpt::pretty("");
    }
    std::string _out;
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            this->__underlying.get()->obj()->prettify(_out);
            break;
        }
        case zpt::JSArray: {
            this->__underlying.get()->arr()->prettify(_out);
            break;
        }
        case zpt::JSString: {
            _out.assign(this->__underlying.get()->str().data());
            break;
        }
        case zpt::JSInteger: {
            zpt::tostr(_out, this->__underlying.get()->intr());
            break;
        }
        case zpt::JSDouble: {
            zpt::tostr(_out, this->__underlying.get()->dbl());
            break;
        }
        case zpt::JSBoolean: {
            zpt::tostr(_out, this->__underlying.get()->bln());
            break;
        }
        case zpt::JSNil: {
            _out.assign("");
            break;
        }
        case zpt::JSDate: {
            _out.insert(_out.length(), zpt::timestamp(this->__underlying.get()->date()));
            break;
        }
        case zpt::JSLambda: {
            _out.assign(this->__underlying.get()->lbd()->signature());
            break;
        }
    }
    return zpt::pretty(_out);
}

zpt::json::operator bool() {
    if (this->__underlying.get() == nullptr) {
        return false;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return true;
        }
        case zpt::JSArray: {
            return true;
        }
        case zpt::JSString: {
            return this->__underlying.get()->str().length() != 0;
        }
        case zpt::JSInteger: {
            return (bool)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (bool)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return false;
        }
        case zpt::JSDate: {
            return (bool)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return true;
        }
    }
    return false;
}

zpt::json::operator int() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            int _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (int)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (int)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (int)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (int)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator long() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            long _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (long)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (long)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (long)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (long)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator long long() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            long long _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (long long)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (long long)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (long long)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (long long)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

#ifdef __LP64__
zpt::json::operator unsigned int() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            unsigned int _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (unsigned int)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (unsigned int)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (unsigned int)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (unsigned int)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}
#endif

zpt::json::operator size_t() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            size_t _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (size_t)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (size_t)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (size_t)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (size_t)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator double() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            return (double)(***this->__underlying.get()->obj()).size();
        }
        case zpt::JSArray: {
            return (double)(***this->__underlying.get()->arr()).size();
        }
        case zpt::JSString: {
            double _n = 0;
            std::string _s(this->__underlying.get()->str().data());
            zpt::fromstr(_s, &_n);
            return _n;
        }
        case zpt::JSInteger: {
            return (double)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (double)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (double)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return (double)this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator zpt::timestamp_t() {
    if (this->__underlying.get() == nullptr) {
        return 0;
    }
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            struct timeval _tp;
            gettimeofday(&_tp, nullptr);
            return _tp.tv_sec * 1000 + _tp.tv_usec / 1000;
        }
        case zpt::JSArray: {
            struct timeval _tp;
            gettimeofday(&_tp, nullptr);
            return _tp.tv_sec * 1000 + _tp.tv_usec / 1000;
        }
        case zpt::JSString: {
            return this->__underlying.get()->date();
        }
        case zpt::JSInteger: {
            return (zpt::timestamp_t)this->__underlying.get()->intr();
        }
        case zpt::JSDouble: {
            return (zpt::timestamp_t)this->__underlying.get()->dbl();
        }
        case zpt::JSBoolean: {
            return (zpt::timestamp_t)this->__underlying.get()->bln();
        }
        case zpt::JSNil: {
            return 0;
        }
        case zpt::JSDate: {
            return this->__underlying.get()->date();
        }
        case zpt::JSLambda: {
            return 0;
        }
    }
    return 0;
}

zpt::json::operator JSONObj() {
    assertz(this->__underlying.get() != nullptr &&
              this->__underlying.get()->type() == zpt::JSObject,
            std::string("this element is not of type JSObject: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->obj();
}

zpt::json::operator JSONArr() {
    assertz(this->__underlying.get() != nullptr && this->__underlying.get()->type() == zpt::JSArray,
            std::string("this element is not of type JSArray: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->arr();
}

zpt::json::operator JSONObj&() {
    assertz(this->__underlying.get() != nullptr &&
              this->__underlying.get()->type() == zpt::JSObject,
            std::string("this element is not of type JSObject: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->obj();
}

zpt::json::operator JSONArr&() {
    assertz(this->__underlying.get() != nullptr && this->__underlying.get()->type() == zpt::JSArray,
            std::string("this element is not of type JSArray: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->arr();
}

zpt::json::operator zpt::lambda() {
    assertz(this->__underlying.get() != nullptr &&
              this->__underlying.get()->type() == zpt::JSLambda,
            std::string("this element is not of type JSLambda: ") + ((std::string) * this),
            0,
            0);
    return this->__underlying.get()->lbd();
}

auto
zpt::json::parse(std::istream& _in) -> void {
    zpt::JSONParser _p;
    _p.switchRoots(*this);
    _p.switchStreams(_in);
    _p.parse();
}

void
zpt::json::stringify(std::string& _str) {
    zpt::utf8::encode(_str, true);
}

auto
zpt::json::begin() -> zpt::json::iterator {
    return zpt::json::iterator{ *this };
}

auto
zpt::json::end() -> zpt::json::iterator {
    size_t _size{ 0 };
    switch (this->__underlying.get()->type()) {
        case zpt::JSObject: {
            _size = (***this->__underlying.get()->obj()).size();
            break;
        }
        case zpt::JSArray: {
            _size = (***this->__underlying.get()->arr()).size();
            break;
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
    return zpt::json::iterator{ *this, _size };
}

auto
zpt::json::object() -> zpt::json {
    zpt::JSONObj _empty;
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_empty) };
}

auto
zpt::json::array() -> zpt::json {
    zpt::JSONArr _empty;
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_empty) };
}

auto
zpt::json::date(std::string _e) -> zpt::json {
    zpt::timestamp_t _v(zpt::timestamp(_e));
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}

auto
zpt::json::date() -> zpt::json {
    zpt::timestamp_t _v((zpt::timestamp_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count());
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}

auto
zpt::json::lambda(std::string _name, unsigned short _n_args) -> zpt::json {
    zpt::lambda _v(_name, _n_args);
    return zpt::json{ std::make_unique<zpt::JSONElementT>(_v) };
}

zpt::json::iterator::iterator(zpt::json& _target, size_t _pos)
  : __target{ _target }
  , __pos{ _pos } {}

zpt::json::iterator::iterator(const iterator& _rhs)
  : __target{ _rhs.__target }
  , __pos{ _rhs.__pos } {}

auto
zpt::json::iterator::operator=(const iterator& _rhs) -> iterator& {
    this->__target = _rhs.__target;
    this->__pos = _rhs.__pos;
    return (*this);
}

auto
zpt::json::iterator::operator++() -> iterator& {
    switch (this->__target->type()) {
        case zpt::JSObject: {
            if (this->__pos != (***this->__target->obj()).size()) {
                ++this->__pos;
            }
            break;
        }
        case zpt::JSArray: {
            if (this->__pos != (***this->__target->arr()).size()) {
                ++this->__pos;
            }
            break;
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
    return (*this);
}

auto zpt::json::iterator::operator*() const -> reference {
    switch (this->__target->type()) {
        case zpt::JSObject: {
            return this->__target->element(this->__pos);
        }
        case zpt::JSArray: {
            return this->__target->element(this->__pos);
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
    return std::make_tuple(0, "", this->__target);
}

auto
zpt::json::iterator::operator++(int) -> iterator {
    zpt::json::iterator _to_return = (*this);
    ++(*this);
    return _to_return;
}

auto zpt::json::iterator::operator-> () const -> pointer {
    switch (this->__target->type()) {
        case zpt::JSObject: {
            return this->__target->element(this->__pos);
        }
        case zpt::JSArray: {
            return this->__target->element(this->__pos);
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
    return std::make_tuple(0, "", this->__target);
}

auto
zpt::json::iterator::operator==(iterator _rhs) const -> bool {
    return this->__target == _rhs.__target && this->__pos == _rhs.__pos;
}

auto
zpt::json::iterator::operator!=(iterator _rhs) const -> bool {
    return !((*this) == _rhs);
}

auto
zpt::json::iterator::operator--() -> iterator& {
    switch (this->__target->type()) {
        case zpt::JSObject:
        case zpt::JSArray: {
            if (this->__pos != 0) {
                --this->__pos;
            }
            break;
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
    return (*this);
}

auto
zpt::json::iterator::operator--(int) -> iterator {
    zpt::json::iterator _to_return = (*this);
    --(*this);
    return _to_return;
}

zpt::json
zpt::get(std::string _path, zpt::json _source) {
    return _source->get_path(_path);
}

zpt::timestamp_t
zpt::timestamp(std::string _json_date) {
    if (_json_date.length() == 0) {
        return (zpt::timestamp_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
          .count();
    }
    time_t _n = 0;
    int _ms = 0;
    std::string _s(_json_date.data());
    size_t _idx = _s.rfind(".");
    std::string _mss;
    if (_idx != std::string::npos) {
        bool _prev_is_zero = true;
        if (_s[_idx + 1] != '0') {
            _mss.push_back(_s[_idx + 1]);
            _prev_is_zero = false;
        }
        if (!_prev_is_zero || _s[_idx + 2] != '0') {
            _mss.push_back(_s[_idx + 2]);
        }
        _mss.push_back(_s[_idx + 3]);
        _s.erase(_idx, 4);
        if (_s.length() < 20) {
            zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S", true);
        }
        else if (_s[_idx] == '+' || _s[_idx] == '-') {
            zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S%z");
        }
        else {
            zpt::fromstr(_s, &_n, "%Y-%m-%dT%H:%M:%S%Z");
        }
        zpt::fromstr(_mss, &_ms);
    }
    return _n * 1000 + _ms;
}

zpt::timestamp_t
zpt::timestamp(zpt::json _json_date) {
    return (zpt::timestamp_t)_json_date;
}

std::string
zpt::timestamp(zpt::timestamp_t _timestamp) {
    std::string _date = zpt::tostr((size_t)(_timestamp / 1000), "%Y-%m-%dT%H:%M:%S");
    _date.insert(_date.length(), ".");
    size_t _remainder = _timestamp % 1000;
    if (_remainder < 100) {
        _date.insert(_date.length(), "0");
        if (_remainder < 10) {
            _date.insert(_date.length(), "0");
        }
    }
    zpt::tostr(_date, _remainder);
    size_t _time_offset = (size_t)(_timestamp / 1000);
    zpt::tostr(_date, _time_offset, "%z");
    return _date;
}
