/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <zapata/json/JSONTokenizerLexer.h>

zpt::JSONTokenizerLexer::JSONTokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::JSONLexer(_in, _out) {
    this->__root = this->__parent = nullptr;
}

zpt::JSONTokenizerLexer::~JSONTokenizerLexer() {}

void
zpt::JSONTokenizerLexer::switchRoots(zpt::json& _root) {
    this->__root = this->__parent = (*_root).get();
}

void
zpt::JSONTokenizerLexer::result(zpt::JSONType _in) {
    try {
        this->__root_type = _in;
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::finish(zpt::JSONType _in) {
    try {
        zpt::JSONElementT* _cur = this->__parent;
        this->__parent = _cur->parent();
        _cur->parent(nullptr);
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(zpt::JSONType _in_type, const std::string _in_str) {
    try {
        (*this->__parent) << _in_str;
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(zpt::JSONType _in_type) {
    if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
        this->__parent->type(_in_type);
        return;
    }
    switch (_in_type) {
        case zpt::JSObject: {
            JSONObj _obj;
            JSONElementT* _ptr = new JSONElementT(_obj);
            _ptr->parent(this->__parent);
            try {
                (*this->__parent) << _ptr;
                this->__parent = _ptr;
            }
            catch (zpt::assertion& _e) {
                std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                          << std::flush;
                delete _ptr;
                this->__parent->type(_in_type);
            }
            break;
        }
        case zpt::JSArray: {
            JSONArr _arr;
            JSONElementT* _ptr = new JSONElementT(_arr);
            _ptr->parent(this->__parent);
            try {
                (*this->__parent) << _ptr;
                this->__parent = _ptr;
            }
            catch (zpt::assertion& _e) {
                std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                          << std::flush;
                delete _ptr;
                this->__parent->type(_in_type);
            }
            break;
        }
        default: {
        }
    }
}

void
zpt::JSONTokenizerLexer::init(bool _in) {
    JSONElementT* _ptr = new JSONElementT(_in);
    _ptr->parent(this->__parent);
    try {
        (*this->__parent) << _ptr;
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        delete _ptr;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(long long _in) {
    JSONElementT* _ptr = new JSONElementT(_in);
    _ptr->parent(this->__parent);
    try {
        (*this->__parent) << _ptr;
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        delete _ptr;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(double _in) {
    JSONElementT* _ptr = new JSONElementT(_in);
    _ptr->parent(this->__parent);
    try {
        (*this->__parent) << _ptr;
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        delete _ptr;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(std::string _in) {
    JSONElementT* _ptr = new JSONElementT(_in);
    _ptr->parent(this->__parent);
    try {
        (*this->__parent) << _ptr;
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        delete _ptr;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(zpt::lambda _in) {
    JSONElementT* _ptr = new JSONElementT(_in);
    _ptr->parent(this->__parent);
    try {
        (*this->__parent) << _ptr;
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        delete _ptr;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init() {
    JSONElementT* _ptr = new JSONElementT();
    _ptr->parent(this->__parent);
    try {
        (*this->__parent) << _ptr;
    }
    catch (zpt::assertion& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        delete _ptr;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::add() {}
