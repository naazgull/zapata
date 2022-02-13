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

#include <zapata/connector/connector.h>

zpt::storage::connection::connection(zpt::storage::connection const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::connection::connection(zpt::storage::connection&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::connection::connection(zpt::storage::connection::type* _underlying)
  : __underlying{ _underlying } {}

auto
zpt::storage::connection::operator=(zpt::storage::connection const& _rhs)
  -> zpt::storage::connection& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::storage::connection::operator=(zpt::storage::connection&& _rhs) -> zpt::storage::connection& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto
zpt::storage::connection::operator->() -> zpt::storage::connection::type* {
    return this->__underlying.get();
}

auto
zpt::storage::connection::operator*() -> zpt::storage::connection::type& {
    return *this->__underlying.get();
}

zpt::storage::session::session(zpt::storage::session const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::session::session(zpt::storage::session&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::session::session(zpt::storage::session::type* _underlying)
  : __underlying{ _underlying } {}

auto
zpt::storage::session::operator=(zpt::storage::session const& _rhs) -> zpt::storage::session& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::storage::session::operator=(zpt::storage::session&& _rhs) -> zpt::storage::session& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto
zpt::storage::session::operator->() -> zpt::storage::session::type* {
    return this->__underlying.get();
}

auto
zpt::storage::session::operator*() -> zpt::storage::session::type& {
    return *this->__underlying.get();
}

zpt::storage::database::database(zpt::storage::database const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::database::database(zpt::storage::database&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::database::database(zpt::storage::database::type* _underlying)
  : __underlying{ _underlying } {}

auto
zpt::storage::database::operator=(zpt::storage::database const& _rhs) -> zpt::storage::database& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::storage::database::operator=(zpt::storage::database&& _rhs) -> zpt::storage::database& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto
zpt::storage::database::operator->() -> zpt::storage::database::type* {
    return this->__underlying.get();
}

auto
zpt::storage::database::operator*() -> zpt::storage::database::type& {
    return *this->__underlying.get();
}

zpt::storage::collection::collection(zpt::storage::collection const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::collection::collection(zpt::storage::collection&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::collection::collection(zpt::storage::collection::type* _underlying)
  : __underlying{ _underlying } {}

auto
zpt::storage::collection::operator=(zpt::storage::collection const& _rhs)
  -> zpt::storage::collection& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::storage::collection::operator=(zpt::storage::collection&& _rhs) -> zpt::storage::collection& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto
zpt::storage::collection::operator->() -> zpt::storage::collection::type* {
    return this->__underlying.get();
}

auto
zpt::storage::collection::operator*() -> zpt::storage::collection::type& {
    return *this->__underlying.get();
}

zpt::storage::action::action(zpt::storage::action const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::action::action(zpt::storage::action&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::action::action(zpt::storage::action::type* _underlying)
  : __underlying{ _underlying } {}

auto
zpt::storage::action::operator=(zpt::storage::action const& _rhs) -> zpt::storage::action& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::storage::action::operator=(zpt::storage::action&& _rhs) -> zpt::storage::action& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto
zpt::storage::action::operator->() -> zpt::storage::action::type* {
    return this->__underlying.get();
}

auto
zpt::storage::action::operator*() -> zpt::storage::action::type& {
    return *this->__underlying.get();
}

zpt::storage::result::result(zpt::storage::result const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::result::result(zpt::storage::result&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::result::result(zpt::storage::result::type* _underlying)
  : __underlying{ _underlying } {}

auto
zpt::storage::result::operator=(zpt::storage::result const& _rhs) -> zpt::storage::result& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::storage::result::operator=(zpt::storage::result&& _rhs) -> zpt::storage::result& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto
zpt::storage::result::operator->() -> zpt::storage::result::type* {
    return this->__underlying.get();
}

auto
zpt::storage::result::operator*() -> zpt::storage::result::type& {
    return *this->__underlying.get();
}

auto
zpt::storage::filter_find(zpt::storage::collection& _collection, zpt::json _params)
  -> zpt::storage::action {
    if (_params->ok()) {
        auto _find = _collection->find(_params);

        if (_params->size() != 0) { _find->bind(_params); }

        if (_params["page_size"]->ok()) {
            _find //
              ->limit(_params["page_size"])
              ->offset(_params["page_start_index"]);
        }
        if (_params["page_start_index"]->ok()) {
            _find //
              ->offset(_params["page_start_index"]);
        }
        return _find;
    }
    return _collection->find({});
}

auto
zpt::storage::reply_find(zpt::json& _reply, zpt::json _params) -> void {
    if (_params->ok()) {
        if (_params["page_size"]->ok()) { _reply["body"] << "page_size" << _params["page_size"]; }
        if (_params["page_start_index"]->ok()) {
            _reply["body"] << "page_start_index" << _params["page_start_index"];
        }
    }
}

auto
zpt::storage::filter_remove(zpt::storage::collection& _collection, zpt::json _params)
  -> zpt::storage::action {
    if (_params->ok()) {
        auto _remove = _collection->remove(_params);
        if (_params->size() != 0) { _remove->bind(_params); }
        return _remove;
    }
    return _collection->remove({});
}

auto
zpt::storage::parse_params(std::string& _param) -> zpt::json {
    return zpt::undefined;
}
