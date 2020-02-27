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

#include <zapata/connector/layer.h>

zpt::storage::layer::connection::connection(zpt::json _config)
  : __config{ _config } {}

auto
zpt::storage::layer::connection::open(zpt::json _options) -> zpt::storage::connection::type& {
    for (auto [_name, _connector] : this->__connectors) {
        _connector->open(_options[_name]);
    }
    return (*this);
}

auto
zpt::storage::layer::connection::close() -> zpt::storage::connection::type& {
    for (auto [_, _connector] : this->__connectors) {
        _connector->close();
    }
    return (*this);
}

auto
zpt::storage::layer::connection::session() -> zpt::storage::session {
    return zpt::storage::session::alloc<zpt::storage::layer::session>(*this);
}

auto
zpt::storage::layer::connection::add(std::string _name, zpt::storage::connection _connector)
  -> zpt::storage::layer::connection& {
    this->__connectors.insert(std::make_pair(_name, _connector));
    return (*this);
}

zpt::storage::layer::session::session(zpt::storage::layer::connection& _connection)
  : __connection{ _connection } {
    for (auto [_name, _connector] : this->__connection.__connectors) {
        this->__sessions.insert(std::make_pair(_name, _connector->session()));
    }
}

auto
zpt::storage::layer::session::is_open() -> bool {
    return true;
}

auto
zpt::storage::layer::session::commit() -> zpt::storage::session::type& {
    for (auto [_, _session] : this->__sessions) {
        _session->commit();
    }
    return (*this);
}

auto
zpt::storage::layer::session::rollback() -> zpt::storage::session::type& {
    for (auto [_, _session] : this->__sessions) {
        _session->rollback();
    }
    return (*this);
}

auto
zpt::storage::layer::session::database(std::string _db) -> zpt::storage::database {
    return zpt::storage::database::alloc<zpt::storage::layer::database>(*this, _db);
}

zpt::storage::layer::database::database(zpt::storage::layer::session& _session, std::string _db)
  : __session{ _session } {
    for (auto [_name, _session] : this->__session.__sessions) {
        this->__databases.insert(std::make_pair(_name, _session->database(_db)));
    }
}

auto
zpt::storage::layer::database::collection(std::string _collection) -> zpt::storage::collection {
    return zpt::storage::collectiono::alloc<zpt::storage::layer::collection>(*this, _collection);
}

zpt::storage::layer::collection::collection(zpt::storage::layer::database& _database,
                                            std::string _collection)
  : __database{ _database } {
    for (auto [_name, _database] : this->__database.__databases) {
        this->__collections.insert(std::make_pair(_name, _database->collection(_collection)));
    }
}

auto
zpt::storage::layer::collection::add(zpt::json _document) -> zpt::storage::filter {
    return zpt::storage::filter::alloc<zpt::storage::layer::filter>(*this, _document);
}

auto
zpt::storage::layer::collection::modify(zpt::json _search) -> zpt::storage::filter {}

auto
zpt::storage::layer::collection::remove(zpt::json _search) -> zpt::storage::filter {}

auto
zpt::storage::layer::collection::replace(std::string _id, zpt::json _document)
  -> zpt::storage::filter {}

auto
zpt::storage::layer::collection::find(zpt::json _search) -> zpt::storage::filter {}

auto
zpt::storage::layer::collection::count() -> size_t {}
