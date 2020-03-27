#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>

/*JSON POINTER TO REGEX*/
zpt::JSONRegex::JSONRegex()
  : zpt::JSONRegex::JSONRegex{ "" } {}

zpt::JSONRegex::JSONRegex(const JSONRegex& _rhs) {
    (*this) = _rhs;
}

zpt::JSONRegex::JSONRegex(JSONRegex&& _rhs) {
    (*this) = _rhs;
}

zpt::JSONRegex::JSONRegex(std::string const& _target)
  : __underlying_original{ _target }
  , __underlying{ std::make_shared<std::regex>(_target) } {}

zpt::JSONRegex::~JSONRegex() {}

auto
zpt::JSONRegex::operator=(const zpt::JSONRegex& _rhs) -> zpt::JSONRegex& {
    this->__underlying_original = _rhs.__underlying_original;
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::JSONRegex::operator=(zpt::JSONRegex&& _rhs) -> zpt::JSONRegex& {
    this->__underlying_original = std::move(_rhs.__underlying_original);
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::JSONRegex::operator-> () -> std::regex* {
    return this->__underlying.get();
}

auto zpt::JSONRegex::operator*() -> std::regex& {
    return *this->__underlying.get();
}

zpt::JSONRegex::operator std::string() {
    std::string _out{ std::string("/") + this->__underlying_original + std::string("/") };
    return _out;
}

zpt::JSONRegex::operator zpt::pretty() {
    std::string _out{ std::string("/") + this->__underlying_original + std::string("/") };
    return _out;
}

zpt::JSONRegex::operator std::regex&() {
    return (*this->__underlying.get());
}

auto
zpt::JSONRegex::JSONRegex::operator==(zpt::regex _rhs) -> bool {
    return this->__underlying_original == _rhs.__underlying_original;
}

auto
zpt::JSONRegex::JSONRegex::operator==(zpt::json _rhs) -> bool {
    if (_rhs->type() == zpt::JSRegex) {
        return (*this) == _rhs->rgx();
    }
    else {
        return std::regex_match(static_cast<std::string>(_rhs), (*this->__underlying.get()));
    }
}

auto
zpt::JSONRegex::JSONRegex::operator==(std::string const& _rhs) -> bool {
    return std::regex_match(_rhs, (*this->__underlying.get()));
}

auto
zpt::JSONRegex::JSONRegex::operator!=(zpt::regex _rhs) -> bool {
    return !((*this) == _rhs);
}

auto
zpt::JSONRegex::JSONRegex::operator!=(zpt::json _rhs) -> bool {
    return !((*this) == _rhs);
}

auto
zpt::JSONRegex::JSONRegex::operator!=(std::string const& _rhs) -> bool {
    return !((*this) == _rhs);
}

auto
zpt::JSONRegex::JSONRegex::to_string() -> std::string& {
    return this->__underlying_original;
}
