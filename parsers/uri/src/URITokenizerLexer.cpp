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

#include <zapata/uri/URITokenizerLexer.h>

zpt::URITokenizerLexer::URITokenizerLexer(std::istream& _in, std::ostream& _out)
  : Re2cURILexer(_in, _out) {}

zpt::URITokenizerLexer::~URITokenizerLexer() {}

auto zpt::URITokenizerLexer::switchRoots(zpt::json& _root) -> void {
    this->__root = _root;
    this->begin(zpt::re2c_uri_cond::INITIAL);
}

auto zpt::URITokenizerLexer::justLeave() -> void { this->leave(0); }

auto zpt::URITokenizerLexer::operator->() -> zpt::json& { return this->__root; }

auto zpt::URITokenizerLexer::operator*() -> zpt::json& { return this->__root; }
