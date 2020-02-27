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

#include <zapata/json.h>
#include <zapata/connector.h>

namespace zpt {
namespace storage {
namespace layer {
class connection;
class session;
class database;
class collection;
class filter;
class result;

class connection : public zpt::storage::connection::type {
  public:
    friend class zpt::storage::layer::session;

    connection(zpt::json _config);
    virtual auto open(zpt::json _options) -> zpt::storage::connection::type& override;
    virtual auto close() -> zpt::storage::connection::type& override;
    virtual auto session() -> zpt::storage::session override;

    virtual auto add(std::string _name, zpt::storage::connection _connector)
      -> zpt::storage::layer::connection& final;

  private:
    zpt::json __config;
    std::map<std::string, zpt::storage::connection> __connectors;
};
class session : public zpt::storage::session::type {
  public:
    friend class zpt::storage::layer::connection;
    friend class zpt::storage::layer::database;

    session(zpt::storage::layer::connection& _connection);
    virtual auto is_open() -> bool override;
    virtual auto commit() -> zpt::storage::session::type& override;
    virtual auto rollback() -> zpt::storage::session::type& override;
    virtual auto database(std::string _db) -> zpt::storage::database override;

  private:
    zpt::storage::layer::connection& __connection;
    std::map<std::string, zpt::storage::session> __sessions;
};
class database : public zpt::storage::database::type {
  public:
    friend class zpt::storage::layer::session;
    friend class zpt::storage::layer::collection;

    database(zpt::storage::layer::session& _session, std::string _db);
    virtual auto collection(std::string _name) -> zpt::storage::collection override;

  private:
    zpt::storage::layer::session& __session;
    std::map<std::string, zpt::storage::database> __databases;
};
class collection : public zpt::storage::collection::type {
  public:
    friend class zpt::storage::layer::database;
    friend class zpt::storage::layer::filter;

    collection(zpt::storage::layer::database& _database, std::string _collection);
    virtual auto add(zpt::json _document) -> zpt::storage::filter override;
    virtual auto modify(zpt::json _search) -> zpt::storage::filter override;
    virtual auto remove(zpt::json _search) -> zpt::storage::filter override;
    virtual auto replace(std::string _id, zpt::json _document) -> zpt::storage::filter override;
    virtual auto find(zpt::json _search) -> zpt::storage::filter override;
    virtual auto count() -> size_t override;

  private:
    zpt::storage::layer::database& __database;
    std::map<std::string, zpt::storage::collection> __collections;
};
class filter : public zpt::storage::filter::type {
  public:
    friend class zpt::storage::layer::collection;
    friend class zpt::storage::layer::result;

    filter(zpt::storage::layer::collection* _collection);
    virtual auto add(zpt::json _document) -> zpt::storage::filter::type& override;
    virtual auto modify(zpt::json _search) -> zpt::storage::filter::type& override;
    virtual auto remove(zpt::json _search) -> zpt::storage::filter::type& override;
    virtual auto replace(std::string _id, zpt::json _document)
      -> zpt::storage::filter::type& override;
    virtual auto find(zpt::json _search) -> zpt::storage::filter::type& override;
    virtual auto set(std::string _attribute, zpt::json _value)
      -> zpt::storage::filter::type& override;
    virtual auto unset(std::string _attribute) -> zpt::storage::filter::type& override;
    virtual auto patch(zpt::json _document) -> zpt::storage::filter::type& override;
    virtual auto sort(std::string _attribute) -> zpt::storage::filter::type& override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::filter::type& override;
    virtual auto limit(size_t _number) -> zpt::storage::filter::type& override;
    virtual auto bind(std::string _attribute, zpt::json _Value)
      -> zpt::storage::filter::type& override;
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::storage::layer::collection& __collection;
    std::map<std::string, zpt::storage::session> __filters;
};
class result {
  public:
    friend class zpt::storage::layer::filter;

    virtual auto fetch(size_t _amount = 1) -> zpt::json override;
    virtual auto all() -> zpt::json override;

  private:
    std::map<std::string, zpt::storage::result> __results;
};
} // namespace layer
} // namespace storage
} // namespace zpt
