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
#include <sqlite3.h>

namespace zpt {
namespace storage {
namespace sqlite {
class connection;
class session;
class database;
class collection;
class action;
class result;

struct close_connection {
    void operator()(sqlite3* connection) const {
        sqlite_expect(sqlite3_close(connection), "unable to properly close database handle");
    }
};

using sqlite3_ptr = std::shared_ptr<sqlite3>;
using prepared_list = std::vector<sqlite3_stmt*>;

auto
to_db_key(zpt::json _document) -> std::string;
auto
to_db_doc(zpt::json _document) -> std::string;
auto
from_db_doc(std::string const& _document) -> zpt::json;
auto
prepare_insert(prepared_list _prepared, std::string const& _table_name, zpt::json _document)
  -> void;

class connection : public zpt::storage::connection::type {
  public:
    friend class session;

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
    friend class session;

    session(zpt::storage::sqlite::connection& _connection);
    session(const zpt::storage::sqlite::session& _rhs) = delete;
    session(zpt::storage::sqlite::session&& _rhs) = delete;
    virtual ~session() override = default;
    virtual auto is_open() -> bool override;
    virtual auto commit() -> zpt::storage::session::type* override;
    virtual auto rollback() -> zpt::storage::session::type* override;
    virtual auto database(std::string const& _db) -> zpt::storage::database override;

  private:
    sqlite3_ptr __underlying{ nullptr };
};
class database : public zpt::storage::database::type {
  public:
    friend class collection;

    database(zpt::storage::sqlite::session& _session, std::string const& _db);
    database(const zpt::storage::sqlite::database& _rhs) = delete;
    database(zpt::storage::sqlite::database&& _rhs) = delete;
    virtual ~database() override = default;
    virtual auto collection(std::string const& _name) -> zpt::storage::collection override;

    auto path() -> std::string&;

  private:
    std::string __path;
    sqlite3_ptr __underlying{ nullptr };
};
class collection : public zpt::storage::collection::type {
  public:
    friend class action;

    collection(zpt::storage::sqlite::database& _database, std::string const& _collection);
    virtual ~collection() override = default;
    virtual auto add(zpt::json _document) -> zpt::storage::action override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action override;
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action override;
    virtual auto find(zpt::json _search) -> zpt::storage::action override;
    virtual auto count() -> size_t override;

  private:
    sqlite3_ptr __underlying{ nullptr };
    std::string __collection_name;
};
class action : public zpt::storage::action::type {
  public:
    action(zpt::storage::sqlite::collection& _collection);
    virtual ~action() override = default;

    auto set_state(int _error) -> void;
    auto get_state() -> zpt::json;

  protected:
    sqlite3_ptr __underlying{ nullptr };
    std::string& __collection_name;
    prepared_list __prepared;
    zpt::json __to_bind{ zpt::json::object() };
    zpt::json __state{ "code", 0, "message", "Success" };
};
class action_add : public zpt::storage::sqlite::action {
  public:
    action_add(zpt::storage::sqlite::collection& _collection, zpt::json _document);
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

  private:
    zpt::json __to_add{ zpt::json::array() };
    zpt::json __generated_uuid{ zpt::json::array() };
};
class action_modify : public zpt::storage::sqlite::action {
  public:
    action_modify(zpt::storage::sqlite::collection& _collection, zpt::json _search);
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

  private:
    zpt::json __search;
    zpt::json __set;
    zpt::json __unset;
};
class action_remove : public zpt::storage::sqlite::action {
  public:
    action_remove(zpt::storage::sqlite::collection& _collection, zpt::json _search);
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

  private:
    zpt::json __search;
};
class action_replace : public zpt::storage::sqlite::action {
  public:
    action_replace(zpt::storage::sqlite::collection& _collection,
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

  private:
    std::string __id;
    zpt::json __set;
};
class action_find : public zpt::storage::sqlite::action {
  public:
    action_find(zpt::storage::sqlite::collection& _collection);
    action_find(zpt::storage::sqlite::collection& _collection, zpt::json _search);
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

  private:
    zpt::json __search;
    zpt::json __sort;
    zpt::json __fields;
    size_t __limit{ std::numeric_limits<size_t>::max() };
    size_t __offset{ 0 };
};
class result : public zpt::storage::result::type {
  public:
    result(zpt::json _result);
    virtual ~result() override = default;
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    virtual auto generated_id() -> zpt::json override;
    virtual auto count() -> size_t override;
    virtual auto status() -> zpt::status override;
    virtual auto message() -> std::string override;
    virtual auto to_json() -> zpt::json override;

  private:
    zpt::json __result;
    zpt::json::iterator __current;
};
} // namespace sqlite
} // namespace storage
} // namespace zpt
