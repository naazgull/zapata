#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>

/*JSON CONTEXT*/
zpt::JSONContext::JSONContext(void* _target)
  : __target(_target) {}

zpt::JSONContext::~JSONContext() { this->__target = nullptr; }

auto
zpt::JSONContext::unpack() -> void* {
    return this->__target;
}

zpt::context::context(void* _target)
  : __underlying{ std::make_shared<zpt::JSONContext>(_target) } {}

zpt::context::~context() {}

zpt::context::context(const context& _rhs) { (*this) = _rhs; }

zpt::context::context(context&& _rhs) { (*this) = _rhs; }

auto
zpt::context::operator->() -> zpt::JSONContext* {
    return this->__underlying.get();
}

auto
zpt::context::operator*() -> zpt::JSONContext& {
    return *this->__underlying.get();
}

auto
zpt::context::operator->() const -> zpt::JSONContext const* {
    return this->__underlying.get();
}

auto
zpt::context::operator*() const -> zpt::JSONContext const& {
    return *this->__underlying.get();
}

auto
zpt::context::operator=(const zpt::context& _rhs) -> zpt::context& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::context::operator=(zpt::context&& _rhs) -> zpt::context& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

/*JSON LAMBDA */
zpt::JSONLambda::JSONLambda()
  : __name("")
  , __n_args(0) {}

zpt::JSONLambda::JSONLambda(std::string const& _signature) {
    std::tuple<std::string, unsigned short> _parsed = zpt::lambda::parse(_signature);
    this->__name = std::get<0>(_parsed);
    this->__n_args = std::get<1>(_parsed);
}

zpt::JSONLambda::JSONLambda(std::string const& _name, unsigned short _n_args)
  : __name(_name)
  , __n_args(_n_args) {}

zpt::JSONLambda::~JSONLambda() {}

auto
zpt::JSONLambda::name() const -> std::string {
    return this->__name;
}

auto
zpt::JSONLambda::n_args() const -> unsigned short {
    return this->__n_args;
}

auto
zpt::JSONLambda::signature() const -> std::string {
    return zpt::lambda::stringify(this->__name, this->__n_args);
}

auto
zpt::JSONLambda::call(zpt::json _args, zpt::context _ctx) -> zpt::json {
    return zpt::lambda::call(this->__name, _args, _ctx);
}

zpt::lambda::lambda()
  : std::shared_ptr<zpt::JSONLambda>(std::make_shared<zpt::JSONLambda>()) {}

zpt::lambda::lambda(std::shared_ptr<zpt::JSONLambda> _target)
  : std::shared_ptr<zpt::JSONLambda>(_target) {}

zpt::lambda::lambda(zpt::lambda& _target)
  : std::shared_ptr<zpt::JSONLambda>(_target) {}

zpt::lambda::lambda(zpt::JSONLambda* _target)
  : std::shared_ptr<zpt::JSONLambda>(_target) {}

zpt::lambda::lambda(std::string const& _signature)
  : std::shared_ptr<zpt::JSONLambda>(new zpt::JSONLambda(_signature)) {}

zpt::lambda::lambda(std::string const& _name, unsigned short _n_args)
  : std::shared_ptr<zpt::JSONLambda>(new zpt::JSONLambda(_name, _n_args)) {}

zpt::lambda::~lambda() {}

auto
zpt::lambda::operator()(zpt::json _args, zpt::context _ctx) -> zpt::json {
    return this->get()->call(_args, _ctx);
}

auto
zpt::lambda::parse(std::string const& _signature) -> std::tuple<std::string, unsigned short> {
    auto _lpar = _signature.find("(");
    auto _rpar = _signature.find(")");
    auto _comma = _signature.find(",");

    expect(_lpar != std::string::npos && _rpar != std::string::npos && _comma != std::string::npos,
           "lambda signature format not recognized");

    std::string _name{ _signature.substr(_lpar + 1, _comma - _lpar - 1) };
    std::string _args{ _signature.substr(_comma + 1, _rpar - _comma - 1) };
    zpt::replace(_name, "\"", "");
    zpt::trim(_name);
    zpt::trim(_args);
    unsigned short _n_args = std::stoi(_args);
    return std::make_tuple(_name, _n_args);
}

auto
zpt::lambda::stringify(std::string const& _name, unsigned short _n_args) -> std::string {
    return std::string("lambda(\"") + _name + std::string("\",") + std::to_string(_n_args) +
           std::string(")");
}

auto
zpt::lambda::add(std::string const& _signature, zpt::symbol _lambda) -> void {
    try {
        zpt::lambda::find(_signature);
        expect(true, "lambda already defined");
    }
    catch (zpt::failed_expectation const& _e) {
    }
    auto _parsed = zpt::lambda::parse(_signature);
    zpt::__lambdas->insert(std::make_pair(
      _signature, std::make_tuple(std::get<0>(_parsed), std::get<1>(_parsed), _lambda)));
}

auto
zpt::lambda::add(std::string const& _name, unsigned short _n_args, zpt::symbol _lambda) -> void {
    try {
        zpt::lambda::find(_name, _n_args);
        expect(true, "lambda already defined");
    }
    catch (zpt::failed_expectation const& _e) {
    }
    std::string _signature{ zpt::lambda::stringify(_name, _n_args) };
    zpt::__lambdas->insert(std::make_pair(_signature, std::make_tuple(_name, _n_args, _lambda)));
}

auto
zpt::lambda::call(std::string const& _name, zpt::json _args, zpt::context _ctx) -> zpt::json {
    expect(_args->type() == zpt::JSArray, "second argument must be a JSON array");
    zpt::symbol _f = zpt::lambda::find(_name, (**_args->array()).size());
    return _f(_args, (**_args->array()).size(), _ctx);
}

auto
zpt::lambda::find(std::string const& _signature) -> zpt::symbol {
    auto _found = zpt::__lambdas->find(_signature);
    expect(_found != zpt::__lambdas->end(),
           std::string("symbol for ") + _signature + std::string(" was not found"));
    return std::get<2>(_found->second);
}

auto
zpt::lambda::find(std::string const& _name, unsigned short _n_args) -> zpt::symbol {
    auto _signature = zpt::lambda::stringify(_name, _n_args);
    return zpt::lambda::find(_signature);
}
