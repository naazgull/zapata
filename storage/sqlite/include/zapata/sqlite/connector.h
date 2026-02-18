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

/**
 * @file connector.h
 * @brief SQLite storage connector implementation.
 *
 * Implements the storage abstraction layer for SQLite databases.
 * Provides the full connector hierarchy: connection, session, database,
 * collection, action, and result types.
 *
 * @see zpt::storage::connection
 * @see zpt::storage::make_connection
 */

#pragma once

#include <sqlite3.h>
#include <zapata/connector.h>
#include <zapata/json.h>

namespace zpt {
namespace storage {
namespace sqlite {
class connection;
class session;
class database;
class collection;
class action;
class result;

/** @brief Deleter for sqlite3 handles used with shared_ptr. */
struct close_connection {
    void operator()(sqlite3* _connection) const { sqlite3_close(_connection); }
};

/** @brief Deleter for sqlite3_stmt handles used with shared_ptr. */
struct finalize_statement {
    void operator()(sqlite3_stmt* _statement) const { sqlite3_finalize(_statement); }
};

using sqlite3_ptr = std::shared_ptr<sqlite3>;       ///< Managed SQLite database handle.
using sqlite3_stmt_ptr = std::shared_ptr<sqlite3_stmt>; ///< Managed SQLite statement handle.

/** @brief Tests if a SQLite return code indicates an error. */
auto is_error(long _error) -> bool;
/** @brief Converts a SQLite result row to JSON. */
auto from_db_doc(sqlite3_stmt* _stmt) -> zpt::json;
/** @brief Serializes a JSON value to a byte array for BLOB storage. */
auto to_byte_array(zpt::json _value) -> std::tuple<char*, size_t>;
/** @brief Frees a byte array allocated by to_byte_array(). */
auto free_byte_array(void* _to_delete) -> void;
/** @brief Binds a JSON value to a named parameter in a prepared statement. */
auto bind(sqlite3_stmt* _stmt, std::string const& _name, zpt::json _value) -> void;

/** @brief SQLite connection implementation. */
class connection : public zpt::storage::connection::type {
  public:
    friend class session;

    /** @brief Constructs a connection with the given options (e.g., "path" to database file). */
    connection(zpt::json _options);
    virtual ~connection() override = default;

    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    virtual auto close() -> zpt::storage::connection::type* override;
    virtual auto session() const -> zpt::storage::session override;
    virtual auto options() const -> zpt::json;

  private:
    zpt::json __options;
};

/**
 * @brief SQLite session implementation.
 *
 * Wraps one or more sqlite3 connection handles and provides
 * transaction control (commit/rollback) and database selection.
 */
class session : public zpt::storage::session::type {
  public:
    friend class database;

    session(zpt::storage::sqlite::connection const& _connection);
    session(zpt::storage::sqlite::session const& _rhs) = delete;
    session(zpt::storage::sqlite::session&& _rhs) = delete;
    virtual ~session() override = default;
    virtual auto is_open() const -> bool override;
    virtual auto commit() -> zpt::storage::session::type* override;
    virtual auto rollback() -> zpt::storage::session::type* override;
    virtual auto sql(std::string const& _statement) -> zpt::storage::session::type* override;
    virtual auto database(std::string const& _db) const -> zpt::storage::database override;
    auto add_database_connection(sqlite3_ptr _database) -> void;

  private:
    std::vector<sqlite3_ptr> __underlying;
    zpt::json __options;
};
/** @brief SQLite database implementation (represents a single .db file). */
class database : public zpt::storage::database::type {
  public:
    friend class collection;

    database(zpt::storage::sqlite::session const& _session, std::string const& _db);
    database(zpt::storage::sqlite::database const& _rhs) = delete;
    database(zpt::storage::sqlite::database&& _rhs) = delete;
    virtual ~database() override = default;
    virtual auto sql(std::string const& _statement) -> zpt::storage::database::type* override;
    virtual auto collection(std::string const& _name) const -> zpt::storage::collection override;
    auto connection() const -> sqlite3_ptr;
    auto path() const -> std::string const&;

  private:
    std::string __path;
    sqlite3_ptr __underlying{ nullptr };
};
/** @brief SQLite collection implementation (represents a database table). */
class collection : public zpt::storage::collection::type {
  public:
    friend class action;

    collection(zpt::storage::sqlite::database const& _database, std::string const& _collection);
    virtual ~collection() override = default;
    virtual auto add(zpt::json _document) const -> zpt::storage::action override;
    virtual auto modify(zpt::json _search) const -> zpt::storage::action override;
    virtual auto remove(zpt::json _search) const -> zpt::storage::action override;
    virtual auto replace(std::string const& _id, zpt::json _document) const
      -> zpt::storage::action override;
    virtual auto find(zpt::json _search) const -> zpt::storage::action override;
    virtual auto count() -> size_t override;

  private:
    sqlite3_ptr __underlying{ nullptr };
    std::string __collection_name;
};
/** @brief Base class for SQLite action operations (manages prepared statements). */
class action : public zpt::storage::action::type {
  public:
    action(zpt::storage::sqlite::collection const& _collection);
    virtual ~action() override = default;

    /** @brief Records a SQLite error code as action state. */
    auto set_state(int _error) -> void;
    /** @brief Returns the current action state (code + message). */
    auto get_state() const -> zpt::json;

  protected:
    std::string __collection_name;
    sqlite3_ptr __underlying{ nullptr };
    std::vector<sqlite3_stmt_ptr> __prepared;
    zpt::json __state{ "code", 0, "message", "Success" };

    virtual auto prepare(std::string const& _statement) -> void;
};
/** @brief SQLite INSERT action builder. */
class action_add : public zpt::storage::sqlite::action {
  public:
    action_add(zpt::storage::sqlite::collection const& _collection, zpt::json _document);
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
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __generated_uuid{ zpt::json::array() };

    auto add_insert(zpt::json _document) -> void;
};
/** @brief SQLite UPDATE action builder. */
class action_modify : public zpt::storage::sqlite::action {
  public:
    action_modify(zpt::storage::sqlite::collection const& _collection, zpt::json _search);
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
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __search;
    zpt::json __set;
    zpt::json __unset;

    auto add_update() -> void;
};
/** @brief SQLite DELETE action builder. */
class action_remove : public zpt::storage::sqlite::action {
  public:
    action_remove(zpt::storage::sqlite::collection const& _collection, zpt::json _search);
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
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __search;
    bool __added{ false };

    auto add_delete() -> void;
};
/** @brief SQLite REPLACE (INSERT OR REPLACE) action builder. */
class action_replace : public zpt::storage::sqlite::action {
  public:
    action_replace(zpt::storage::sqlite::collection const& _collection,
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
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;
    virtual auto replace_one() -> void;

  private:
    std::string __id;
    zpt::json __set;

    auto add_replace() -> void;
};
/** @brief SQLite SELECT action builder with filtering, sorting, and pagination. */
class action_find : public zpt::storage::sqlite::action {
  public:
    action_find(zpt::storage::sqlite::collection const& _collection);
    action_find(zpt::storage::sqlite::collection const& _collection, zpt::json _search);
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
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
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
    bool __already_prepared{ false };

    auto add_select() -> void;
};
/** @brief SQLite query result set. */
class result : public zpt::storage::result::type {
  public:
    result(zpt::json _result);
    result(zpt::json _result, std::vector<sqlite3_stmt_ptr> _prepared);
    virtual ~result() override = default;
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    virtual auto generated_id() -> zpt::json override;
    virtual auto count() const -> size_t override;
    virtual auto status() const -> zpt::status override;
    virtual auto message() const -> std::string override;
    virtual auto to_json() const -> zpt::json override;

  private:
    zpt::json __result;
    std::vector<sqlite3_stmt_ptr> __prepared;
};
} // namespace sqlite
} // namespace storage
} // namespace zpt
