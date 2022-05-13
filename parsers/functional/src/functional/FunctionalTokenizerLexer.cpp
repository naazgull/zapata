/* Author: n@zgul <n@zgul.me> */

#include <zapata/functional/FunctionalTokenizerLexer.h>

zpt::FunctionalTokenizerLexer::FunctionalTokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::FunctionalLexer(_in, _out) {}

zpt::FunctionalTokenizerLexer::~FunctionalTokenizerLexer() {}

auto
zpt::FunctionalTokenizerLexer::switchRoots(zpt::json& _root) -> void {
    this->clear();
    this->__root = _root;
    this->begin(zpt::FunctionalLexerBase::StartCondition_::INITIAL);
}

auto
zpt::FunctionalTokenizerLexer::justLeave() -> void {
    this->leave(-1);
}

auto
zpt::FunctionalTokenizerLexer::clear() -> void {
    for (; !this->__stack.empty(); this->__stack.pop()) {}
}

auto
zpt::FunctionalTokenizerLexer::set_string() -> void {
    this->__stack.push(zpt::json::string(this->matched()));
}

auto
zpt::FunctionalTokenizerLexer::set_number() -> void {
    std::istringstream _iss;
    _iss.str(this->matched());
    if (this->matched().find(".") != std::string::npos) {
        double _d;
        _iss >> _d;
        this->__stack.push(zpt::json::floating(_d));
    }
    else {
        long long _l;
        _iss >> _l;
        this->__stack.push(zpt::json::integer(_l));
    }
}

auto
zpt::FunctionalTokenizerLexer::set_variable() -> void {
    if (this->__stack.size() == 0) { this->__stack.push(this->__root); }
    else { this->__stack.push(zpt::json::object()); }
    this->__stack.top() << "functor" << this->matched();
}

auto
zpt::FunctionalTokenizerLexer::add_param() -> void {
    auto _param = this->__stack.top();
    this->__stack.pop();
    if (!this->__stack.top()["params"]->ok()) {
        this->__stack.top() << "params" << zpt::json::array();
    }
    this->__stack.top()["params"] << (_param["params"]->ok() || !_param["functor"]->ok()
                                        ? _param
                                        : _param["functor"]);
}
