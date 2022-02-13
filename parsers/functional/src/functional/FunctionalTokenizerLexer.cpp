/* Author: n@zgul <n@zgul.me> */

#include <zapata/functional/FunctionalTokenizerLexer.h>

zpt::FunctionalTokenizerLexer::FunctionalTokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::FunctionalLexer(_in, _out) {}

zpt::FunctionalTokenizerLexer::~FunctionalTokenizerLexer() {}

auto
zpt::FunctionalTokenizerLexer::switchRoots(zpt::json& _root) -> void {
    this->__root = _root;
    this->begin(zpt::FunctionalLexerBase::StartCondition_::INITIAL);
}

auto
zpt::FunctionalTokenizerLexer::justLeave() -> void {
    this->leave(-1);
}

auto
zpt::FunctionalTokenizerLexer::operator->() -> zpt::json& {
    return this->__root;
}

auto
zpt::FunctionalTokenizerLexer::operator*() -> zpt::json& {
    return this->__root;
}
