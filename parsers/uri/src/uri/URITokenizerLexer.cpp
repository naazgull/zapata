/* Author: n@zgul <n@zgul.me> */

#include <zapata/uri/URITokenizerLexer.h>

zpt::URITokenizerLexer::URITokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::URILexer(_in, _out) {}

zpt::URITokenizerLexer::~URITokenizerLexer() {}

auto
zpt::URITokenizerLexer::switchRoots(zpt::json& _root) -> void {
    this->__root = _root;
    this->begin(zpt::URILexerBase::StartCondition_::INITIAL);
}

auto
zpt::URITokenizerLexer::justLeave() -> void {
    this->leave(-1);
}

auto
zpt::URITokenizerLexer::operator->() -> zpt::json& {
    return this->__root;
}

auto
zpt::URITokenizerLexer::operator*() -> zpt::json& {
    return this->__root;
}
