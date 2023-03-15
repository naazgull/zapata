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

#include <zapata/json/JSONTokenizerLexer.h>

zpt::JSONTokenizerLexer::JSONTokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::JSONLexer(_in, _out) {
    this->__root = this->__parent = nullptr;
}

zpt::JSONTokenizerLexer::~JSONTokenizerLexer() {}

void zpt::JSONTokenizerLexer::switchRoots(zpt::json& _root) {
    this->__root = this->__parent = &(*_root);
    this->begin(zpt::JSONLexerBase::StartCondition_::INITIAL);
}

auto zpt::JSONTokenizerLexer::justLeave() -> void { this->leave(-1); }

void zpt::JSONTokenizerLexer::result(zpt::JSONType _in) {
    try {
        this->__root_type = _in;
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::finish(zpt::JSONType) {
    try {
        zpt::JSONElementT* _cur = this->__parent;
        this->__parent = _cur->parent();
        _cur->parent(nullptr);
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::init(zpt::JSONType, const std::string _in_str) {
    try {
        (*this->__parent) << _in_str;
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::init(zpt::JSONType _in_type) {
    if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
        this->__parent->type(_in_type);
        return;
    }
    switch (_in_type) {
        case zpt::JSObject: {
            zpt::json _ref = zpt::json::object();
            _ref->parent(this->__parent);
            try {
                (*this->__parent) << _ref;
                this->__parent = &(*_ref);
            }
            catch (zpt::failed_expectation const& _e) {
                std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                          << std::flush;
                // this->__parent->type(_in_type);
                throw _e;
            }
            break;
        }
        case zpt::JSArray: {
            zpt::json _ref = zpt::json::array();
            _ref->parent(this->__parent);
            try {
                (*this->__parent) << _ref;
                this->__parent = &(*_ref);
            }
            catch (zpt::failed_expectation const& _e) {
                std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                          << std::flush;
                // delete _ptr;
                // this->__parent->type(_in_type);
                throw _e;
            }
            break;
        }
        default: {
        }
    }
}

void zpt::JSONTokenizerLexer::init(bool _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else { (*this->__parent) << _in; }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::init(long long _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else { (*this->__parent) << _in; }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::init(double _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else { (*this->__parent) << _in; }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::init(std::string const& _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else { (*this->__parent) << _in; }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::init(zpt::lambda _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else { (*this->__parent) << _in; }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::init(zpt::regex _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else { (*this->__parent) << _in; }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::init() {
    zpt::json _ref;
    try {
        if (this->__parent->type() == zpt::JSObject || this->__parent->type() == zpt::JSArray) {
            (*this->__parent) << _ref;
        }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void zpt::JSONTokenizerLexer::add() {}
