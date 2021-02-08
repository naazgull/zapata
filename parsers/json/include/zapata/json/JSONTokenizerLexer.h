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

#pragma once

#include <zapata/json/JSONLexer.h>
#include <zapata/json/JSONClass.h>

namespace zpt {
class JSONTokenizerLexer : public JSONLexer {
  public:
    JSONTokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~JSONTokenizerLexer();

    void switchRoots(zpt::json& _root);

    void result(zpt::JSONType _in);
    void finish(zpt::JSONType _in);

    void init(zpt::JSONType _in_type, const std::string _in_str);
    void init(zpt::JSONType _in_type);
    void init(bool _in);
    void init(long long _in);
    void init(double _in);
    void init(std::string const& _in);
    void init(zpt::lambda _in);
    void init(zpt::regex _in);
    void init();

    void add();

    zpt::JSONElementT* __root{ nullptr };
    zpt::JSONType __root_type;
    zpt::JSONElementT* __parent{ nullptr };
};
} // namespace zpt
