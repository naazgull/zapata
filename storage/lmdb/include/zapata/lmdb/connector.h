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
#include <zapata/mem/ref_ptr.h>
#include <lmdb++.h>

namespace zpt {
namespace storage {
namespace lmdb {
class connection;
class session;
class database;
class collection;
class action;
class result;

// auto
// expression_operators() -> std::map<std::string, std::function<zpt::json(zpt::json,
// std::string)>>&; auto bind_operators() -> std::map<std::string,
// std::function<zpt::json(zpt::json)>>&; auto cast(zpt::json _expression) -> zpt::json; auto
// cast(zpt::json _expression, std::string _cast) -> zpt::json;
// auto
// cast_to_db_value(zpt::json _value) -> ::lmdb::Value;

// auto
// evaluate_expression(zpt::json _expression, std::string _attribute) -> zpt::json;
// auto
// evaluate_bind(zpt::json _expression) -> zpt::json;
// auto
// to_search_str(zpt::json _search) -> std::string;
// auto
// to_binded_object(zpt::json _binded) -> zpt::json;
auto
to_db_key(zpt::json _document) -> std::string;
auto
to_db_doc(zpt::json _document) -> std::string;
auto
from_db_doc(std::string const& _document) -> zpt::json;

class connection : public zpt::storage::connection::type {
  public:
    connection(zpt::json _options);
    virtual ~connection() override = default;

    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    virtual auto close() -> zpt::storage::connection::type* override;
    virtual auto session() -> zpt::storage::session override;
    virtual auto options() -> zpt::json&;

  private:
    zpt::json __options;
};
class session : public zpt::storage::session::type {
  public:
    friend class database;

    session(zpt::storage::lmdb::connection& _connection);
    session(const zpt::storage::lmdb::session& _rhs) = delete;
    session(zpt::storage::lmdb::session&& _rhs) = delete;
    virtual ~session() override;
    virtual auto is_open() -> bool override;
    virtual auto commit() -> zpt::storage::session::type* override;
    virtual auto rollback() -> zpt::storage::session::type* override;
    virtual auto database(std::string const& _db) -> zpt::storage::database override;

    virtual auto operator->() -> ::lmdb::env*;
    virtual auto operator*() -> ::lmdb::env&;

    virtual auto is_to_commit() -> bool&;

  private:
    zpt::json __options;
    ::lmdb::env __underlying;
    bool __commit{ true };
};
class database : public zpt::storage::database::type {
  public:
    database(zpt::storage::lmdb::session& _session, std::string const& _db);
    database(const zpt::storage::lmdb::database& _rhs) = delete;
    database(zpt::storage::lmdb::database&& _rhs) = delete;
    virtual ~database() override = default;
    virtual auto collection(std::string const& _name) -> zpt::storage::collection override;

    virtual auto operator->() -> ::lmdb::env*;
    virtual auto operator*() -> ::lmdb::env&;

    virtual auto is_to_commit() -> bool&;

  private:
    zpt::ref_ptr<::lmdb::env> __underlying;
    zpt::ref_ptr<bool> __commit;
};
class collection : public zpt::storage::collection::type {
  public:
    collection(zpt::storage::lmdb::database& _database, std::string const& _collection);
    virtual ~collection() override = default;
    virtual auto add(zpt::json _document) -> zpt::storage::action override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action override;
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action override;
    virtual auto find(zpt::json _search) -> zpt::storage::action override;
    virtual auto count() -> size_t override;

    virtual auto operator->() -> ::lmdb::env*;
    virtual auto operator*() -> ::lmdb::env&;

    virtual auto is_to_commit() -> bool&;

  private:
    std::string __collection_name;
    zpt::ref_ptr<::lmdb::env> __underlying;
    zpt::ref_ptr<bool> __commit;
};
class action : public zpt::storage::action::type {
  public:
    action(zpt::storage::lmdb::collection& _collection);
    virtual ~action() override = default;

    virtual auto is_to_commit() -> bool&;

  protected:
    zpt::ref_ptr<::lmdb::env> __underlying;
    zpt::ref_ptr<bool> __commit;
};
class action_add : public zpt::storage::lmdb::action {
  public:
    action_add(zpt::storage::lmdb::collection& _collection, zpt::json _document);
    virtual ~action_add() override = default;
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;

    virtual auto operator->() -> ::lmdb::txn*;
    virtual auto operator*() -> ::lmdb::txn&;

  private:
    ::lmdb::txn __underlying;
    ::lmdb::dbi __stub;
};
class action_modify : public zpt::storage::lmdb::action {
  public:
    action_modify(zpt::storage::lmdb::collection& _collection, zpt::json _search);
    virtual ~action_modify() override = default;
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;

    virtual auto operator->() -> ::lmdb::txn*;
    virtual auto operator*() -> ::lmdb::txn&;

  private:
    ::lmdb::txn __underlying;
    ::lmdb::dbi __stub;
    zpt::json __search;
    zpt::json __set;
    zpt::json __unset;
};
class action_remove : public zpt::storage::lmdb::action {
  public:
    action_remove(zpt::storage::lmdb::collection& _collection, zpt::json _search);
    virtual ~action_remove() override = default;
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;

    virtual auto operator->() -> ::lmdb::txn*;
    virtual auto operator*() -> ::lmdb::txn&;

  private:
    ::lmdb::txn __underlying;
    ::lmdb::dbi __stub;
    zpt::json __search;
};
class action_replace : public zpt::storage::lmdb::action {
  public:
    action_replace(zpt::storage::lmdb::collection& _collection,
                   std::string _id,
                   zpt::json _document);
    virtual ~action_replace() override = default;
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;
    virtual auto replace_one() -> void;

    virtual auto operator->() -> ::lmdb::txn*;
    virtual auto operator*() -> ::lmdb::txn&;

  private:
    ::lmdb::txn __underlying;
    ::lmdb::dbi __stub;
    std::string __id;
    zpt::json __set;
};
class action_find : public zpt::storage::lmdb::action {
  public:
    action_find(zpt::storage::lmdb::collection& _collection);
    action_find(zpt::storage::lmdb::collection& _collection, zpt::json _search);
    virtual ~action_find() override = default;
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string const& _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;

    virtual auto operator->() -> ::lmdb::txn*;
    virtual auto operator*() -> ::lmdb::txn&;

  private:
    ::lmdb::txn __underlying;
    ::lmdb::dbi __stub;
    zpt::json __search;
    zpt::json __sort;
    zpt::json __fields;
    size_t __limit{ std::numeric_limits<size_t>::max() };
    size_t __offset{ 0 };
};
class result : public zpt::storage::result::type {
  public:
    result(zpt::storage::lmdb::action_add& _action);
    result(zpt::storage::lmdb::action_modify& _action);
    result(zpt::storage::lmdb::action_remove& _action);
    result(zpt::storage::lmdb::action_replace& _action);
    result(zpt::storage::lmdb::action_find& _action);
    virtual ~result() override = default;
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    virtual auto generated_id() -> zpt::json override;
    virtual auto count() -> size_t override;
    virtual auto status() -> zpt::status override;
    virtual auto message() -> std::string override;

    virtual auto operator->() -> ::lmdb::cursor*;
    virtual auto operator*() -> ::lmdb::cursor&;

  private:
    ::lmdb::cursor __underlying;
};
} // namespace lmdb
} // namespace storage
} // namespace zpt
