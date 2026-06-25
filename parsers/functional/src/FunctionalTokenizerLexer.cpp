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

#include <sstream>
#include <zapata/functional/FunctionalTokenizerLexer.h>

zpt::FunctionalTokenizerLexer::FunctionalTokenizerLexer(std::istream& _in, std::ostream& _out)
  : Re2cFunctionalLexer(_in, _out) {}

zpt::FunctionalTokenizerLexer::~FunctionalTokenizerLexer() {}

auto zpt::FunctionalTokenizerLexer::switchRoots(zpt::json& _root) -> void {
    this->clear();
    this->__root = _root;
    this->begin(zpt::re2c_functional_cond::INITIAL);
}

auto zpt::FunctionalTokenizerLexer::justLeave() -> void { this->leave(0); }

auto zpt::FunctionalTokenizerLexer::clear() -> void {
    for (; !this->__stack.empty(); this->__stack.pop()) {}
}

auto zpt::FunctionalTokenizerLexer::set_string() -> void {
    this->__stack.push(zpt::json::string(this->matched()));
}

auto zpt::FunctionalTokenizerLexer::set_number() -> void {
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

auto zpt::FunctionalTokenizerLexer::set_variable() -> void {
    if (this->__stack.size() == 0) { this->__stack.push(this->__root); }
    else { this->__stack.push(zpt::json::object()); }
    this->__stack.top() << "functor" << this->matched();
}

auto zpt::FunctionalTokenizerLexer::add_param() -> void {
    auto _param = this->__stack.top();
    this->__stack.pop();
    if (!this->__stack.top()["params"]->ok()) {
        this->__stack.top() << "params" << zpt::json::array();
    }
    this->__stack.top()["params"] << (_param["params"]->ok() || !_param["functor"]->ok()
                                        ? _param
                                        : _param["functor"]);
}
