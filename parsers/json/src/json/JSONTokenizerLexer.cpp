/* Author: n@zgul <n@zgul.me> */

#include <zapata/json/JSONTokenizerLexer.h>

zpt::JSONTokenizerLexer::JSONTokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::JSONLexer(_in, _out) {
    this->__root = this->__parent = nullptr;
}

zpt::JSONTokenizerLexer::~JSONTokenizerLexer() {}

void
zpt::JSONTokenizerLexer::switchRoots(zpt::json& _root) {
    this->__root = this->__parent = &(*_root);
    this->begin(zpt::JSONLexerBase::StartCondition_::INITIAL);
}

void
zpt::JSONTokenizerLexer::result(zpt::JSONType _in) {
    try {
        this->__root_type = _in;
    }
    catch (zpt::failed_expectation const& _e) {
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
    catch (zpt::failed_expectation const& _e) {
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
    catch (zpt::failed_expectation const& _e) {
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
            zpt::json _ref = zpt::json::object();
            _ref->parent(this->__parent);
            try {
                (*this->__parent) << _ref;
                this->__parent = &(*_ref);
            }
            catch (zpt::failed_expectation const& _e) {
                std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                          << std::flush;
                this->__parent->type(_in_type);
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
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else {
            (*this->__parent) << _in;
        }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(long long _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else {
            (*this->__parent) << _in;
        }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(double _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else {
            (*this->__parent) << _in;
        }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(std::string const& _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else {
            (*this->__parent) << _in;
        }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(zpt::lambda _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else {
            (*this->__parent) << _in;
        }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init(zpt::regex _in) {
    try {
        if (this->__parent->type() != zpt::JSObject && this->__parent->type() != zpt::JSArray) {
            (*this->__parent) = _in;
        }
        else {
            (*this->__parent) << _in;
        }
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << std::endl
                  << std::flush;
        throw _e;
    }
}

void
zpt::JSONTokenizerLexer::init() {
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

void
zpt::JSONTokenizerLexer::add() {}
