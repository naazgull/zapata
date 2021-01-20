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

#include <zapata/fsm/fsm.h>

zpt::fsm::element::element(std::string _xpath,
                           std::string _name,
                           zpt::json _content,
                           zpt::json _parent)
  : __xpath{ _xpath }
  , __name{ _name }
  , __content{ _content }
  , __parent{ _parent } {}

zpt::fsm::element::element(zpt::fsm::element const& _rhs)
  : __xpath{ _rhs.__xpath }
  , __name{ _rhs.__name }
  , __content{ _rhs.__content }
  , __parent{ _rhs.__parent } {}

zpt::fsm::element::element(zpt::fsm::element&& _rhs)
  : __xpath{ std::move(_rhs.__xpath) }
  , __name{ std::move(_rhs.__name) }
  , __content{ std::move(_rhs.__content) }
  , __parent{ std::move(_rhs.__parent) } {}

auto
zpt::fsm ::element::operator=(zpt::fsm::element const& _rhs) -> zpt::fsm::element& {
    this->__xpath = _rhs.__xpath;
    this->__name = _rhs.__name;
    this->__content = _rhs.__content;
    this->__parent = _rhs.__parent;
    return (*this);
}

auto
zpt::fsm ::element::operator=(zpt::fsm::element&& _rhs) -> zpt::fsm::element& {
    this->__xpath = std::move(_rhs.__xpath);
    this->__name = std::move(_rhs.__name);
    this->__content = std::move(_rhs.__content);
    this->__parent = std::move(_rhs.__parent);
    return (*this);
}

auto
zpt::fsm ::element::xpath() -> std::string& {
    return this->__xpath;
}

auto
zpt::fsm ::element::name() -> std::string& {
    return this->__name;
}

auto
zpt::fsm ::element::content() -> zpt::json {
    return this->__content;
}

auto
zpt::fsm ::element::parent() -> zpt::json {
    return this->__parent;
}

zpt::fsm::engine::engine(zpt::json _config)
  : zpt::pipeline::engine<zpt::fsm::element>{ _pipeline_size, _config } {
    expect(_config["states"]->ok(), "`states` list must be included in configuration", 500, 0);
    this->set_error_callback(zpt::fsm::engine::on_error);
}

auto
zpt::fsm::engine::add_transition(
  size_t _stage,
  std::string _pattern,
  std::function<void(zpt::pipeline::event<zpt::fsm::element>&)> _callback) -> zpt::fsm::engine& {
    _pattern.insert(0, "/ROOT");
    zpt::pipeline::engine<zpt::fsm::element>::add_listener(_stage, _pattern, _callback);
    return (*this);
}

auto
zpt::fsm::engine::on_error(zpt::json _path,
                           zpt::pipeline::event<zpt::fsm::element>& _event,
                           const char* _what,
                           const char* _description,
                           const char* _backtrace,
                           int _error,
                           int status) -> bool {
    zlog("Error found while processing '" << _event->content().xpath() << "': " << _what << ", "
                                          << _description,
         zpt::error);
    return true;
}
