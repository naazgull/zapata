#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>

/*JSON POINTER TO REGEX*/
zpt::JSONRegex::JSONRegex()
  : zpt::JSONRegex::JSONRegex{ ".*" } {}

zpt::JSONRegex::JSONRegex(const JSONRegex& _rhs) {
    (*this) = _rhs;
}

zpt::JSONRegex::JSONRegex(JSONRegex&& _rhs) {
    (*this) = _rhs;
}

zpt::JSONRegex::JSONRegex(std::string _target)
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

auto zpt::JSONRegex::operator-> () -> std::shared_ptr<std::regex>& {
    return this->__underlying;
}

auto zpt::JSONRegex::operator*() -> std::shared_ptr<std::regex>& {
    return this->__underlying;
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
    return _rhs->type() == zpt::JSRegex && (*this) == _rhs->rgx();
}

auto
zpt::JSONRegex::JSONRegex::operator!=(zpt::regex _rhs) -> bool {
    return !((*this) == _rhs);
}

auto
zpt::JSONRegex::JSONRegex::operator!=(zpt::json _rhs) -> bool {
    return !((*this) == _rhs);
}

// auto
// zpt::JSONRegex::JSONRegex::operator<(zpt::regex _rhs) -> bool {
//     return;
// }

// auto
// zpt::JSONRegex::JSONRegex::operator<(zpt::json _rhs) -> bool {
//     return;
// }

// auto
// zpt::JSONRegex::JSONRegex::operator>(zpt::regex _rhs) -> bool {
//     return;
// }

// auto
// zpt::JSONRegex::JSONRegex::operator>(zpt::json _rhs) -> bool {
//     return;
// }

// auto
// zpt::JSONRegex::JSONRegex::operator<=(zpt::regex _rhs) -> bool {
//     return;
// }

// auto
// zpt::JSONRegex::JSONRegex::operator<=(zpt::json _rhs) -> bool {
//     return;
// }

// auto
// zpt::JSONRegex::JSONRegex::operator>=(zpt::regex _rhs) -> bool {
//     return;
// }

// auto
// zpt::JSONRegex::JSONRegex::operator>=(zpt::json _rhs) -> bool {
//     return;
// }
