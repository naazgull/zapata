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
#include <mysqlx/xdevapi.h>

namespace zpt {
namespace storage {
namespace mysqlx {
class connection;
class session;
class database;
class collection;
class action;
class result;

auto
expression_operators() -> std::map<std::string, std::function<zpt::json(zpt::json, std::string)>>&;
auto
bind_operators() -> std::map<std::string, std::function<zpt::json(zpt::json)>>&;
auto
cast(zpt::json _expression) -> zpt::json;
auto
cast(zpt::json _expression, std::string _cast) -> zpt::json;
auto
cast_to_db_value(zpt::json _value) -> ::mysqlx::Value;

class lower {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class upper {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class boolean {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class date {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class integer {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class floating {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class regex {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class string {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class ne {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class gt {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class gte {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class lt {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class lte {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

class between {
  public:
    auto operator()(zpt::json _expression, std::string _attribute) -> zpt::json;
    auto operator()(zpt::json _expression) -> zpt::json;
};

auto
evaluate_expression(zpt::json _expression, std::string _attribute) -> zpt::json;
auto
evaluate_bind(zpt::json _expression) -> zpt::json;
auto
to_search_str(zpt::json _search) -> std::string;
auto
to_binded_object(zpt::json _binded) -> zpt::json;
auto
to_db_doc(zpt::json _document) -> ::mysqlx::DbDoc;
auto
from_db_doc(const ::mysqlx::DbDoc& _document) -> zpt::json;

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
    session(zpt::storage::mysqlx::connection& _connection);
    session(const zpt::storage::mysqlx::session& _rhs) = delete;
    session(zpt::storage::mysqlx::session&& _rhs) = delete;
    virtual ~session() override;
    virtual auto is_open() -> bool override;
    virtual auto commit() -> zpt::storage::session::type* override;
    virtual auto rollback() -> zpt::storage::session::type* override;
    virtual auto database(std::string const& _db) -> zpt::storage::database override;

    virtual auto operator->() -> ::mysqlx::Session*;

  private:
    ::mysqlx::Session __underlying;
};
class database : public zpt::storage::database::type {
  public:
    database(zpt::storage::mysqlx::session& _session, std::string const& _db);
    database(const zpt::storage::mysqlx::database& _rhs) = delete;
    database(zpt::storage::mysqlx::database&& _rhs) = delete;
    virtual ~database() override = default;
    virtual auto collection(std::string const& _name) -> zpt::storage::collection override;

    virtual auto operator->() -> ::mysqlx::Schema*;

  private:
    ::mysqlx::Schema __underlying;
};
class collection : public zpt::storage::collection::type {
  public:
    collection(zpt::storage::mysqlx::database& _database, std::string const& _collection);
    virtual ~collection() override = default;
    virtual auto add(zpt::json _document) -> zpt::storage::action override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action override;
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action override;
    virtual auto find(zpt::json _search) -> zpt::storage::action override;
    virtual auto count() -> size_t override;

    virtual auto operator->() -> ::mysqlx::Collection*;

  private:
    ::mysqlx::Collection __underlying;
};
class action : public zpt::storage::action::type {
  public:
    action(zpt::storage::mysqlx::collection& _collection);
    virtual ~action() override = default;
};
class action_add : public zpt::storage::mysqlx::action {
  public:
    action_add(zpt::storage::mysqlx::collection& _collection, zpt::json _document);
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

    virtual auto operator->() -> ::mysqlx::CollectionAdd*;

  private:
    ::mysqlx::CollectionAdd __underlying;
};
class action_modify : public zpt::storage::mysqlx::action {
  public:
    action_modify(zpt::storage::mysqlx::collection& _collection, zpt::json _search);
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

    virtual auto operator->() -> ::mysqlx::CollectionModify*;

  private:
    ::mysqlx::CollectionModify __underlying;
};
class action_remove : public zpt::storage::mysqlx::action {
  public:
    action_remove(zpt::storage::mysqlx::collection& _collection, zpt::json _search);
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

    virtual auto operator->() -> ::mysqlx::CollectionRemove*;

  private:
    ::mysqlx::CollectionRemove __underlying;
};
class action_replace : public zpt::storage::mysqlx::action {
  public:
    action_replace(zpt::storage::mysqlx::collection& _collection,
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
    virtual auto replace_one() -> ::mysqlx::Result;

    virtual auto operator->() -> ::mysqlx::Collection*;

  private:
    std::string __id;
    zpt::json __document;
    ::mysqlx::Collection* __underlying{ nullptr };
};
class action_find : public zpt::storage::mysqlx::action {
  public:
    action_find(zpt::storage::mysqlx::collection& _collection);
    action_find(zpt::storage::mysqlx::collection& _collection, zpt::json _search);
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

    virtual auto operator->() -> ::mysqlx::CollectionFind*;

  private:
    std::string __find_criteria;
    ::mysqlx::CollectionFind __underlying;
};
class result : public zpt::storage::result::type {
  public:
    result(zpt::storage::mysqlx::action_add& _action);
    result(zpt::storage::mysqlx::action_modify& _action);
    result(zpt::storage::mysqlx::action_remove& _action);
    result(zpt::storage::mysqlx::action_replace& _action);
    result(zpt::storage::mysqlx::action_find& _action);
    virtual ~result() override = default;
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    virtual auto generated_id() -> zpt::json override;
    virtual auto count() -> size_t override;
    virtual auto status() -> zpt::status override;
    virtual auto message() -> std::string override;
    virtual auto to_json() -> zpt::json override;

  private:
    bool __is_doc_result{ false };
    ::mysqlx::Result __result;
    ::mysqlx::DocResult __doc_result;
};
} // namespace mysqlx
} // namespace storage
} // namespace zpt
