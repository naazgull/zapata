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
 * @brief MySQL storage connector implementation using the C API.
 *
 * Implements the storage abstraction layer for MySQL databases using
 * the MySQL C client library. Provides the full connector hierarchy:
 * connection, session, database, collection, action, and result types.
 *
 * @see zpt::storage::connection
 * @see zpt::storage::make_connection
 */

#pragma once

#include <mysql/mysql.h>
#include <zapata/connector.h>
#include <zapata/json.h>
#include <zapata/mysqlx/translate.h>

namespace zpt {
namespace storage {
namespace mysqlx {
class connection;
class session;
class database;
class collection;
class action;
class result;

using mysql_ptr = std::shared_ptr<MYSQL>;
using mysql_stmt_ptr = std::shared_ptr<MYSQL_STMT>;

/** @brief Deleter for MYSQL handles used with shared_ptr. */
struct mysql_deinit {
    auto operator()(MYSQL*) const -> void;
};

/** @brief RAII wrapper for per-thread MySQL cleanup. */
struct mysql_thread_deinit {
    ~mysql_thread_deinit();
};

/** @brief Deleter for MYSQL_STMT handles used with shared_ptr. */
struct mysql_stmt_end {
    auto operator()(MYSQL_STMT* _to_dispose) const -> void;
};

/** @brief RAII wrapper for MySQL library initialization/cleanup. */
class library {
  public:
    library();
    virtual ~library();
};

/** @brief Returns the global MySQL library instance (initializes on first call). */
auto init() -> library&;

/** @brief MySQL connection implementation. */
class connection : public zpt::storage::connection::type {
  public:
    /** @brief Constructs a connection with options (host, user, password, port). */
    connection(zpt::json _options);
    virtual ~connection() override = default;

    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    virtual auto close() -> zpt::storage::connection::type* override;
    virtual auto session() const -> zpt::storage::session override;
    virtual auto options() const -> zpt::json;

    /** @brief Returns the underlying MYSQL handle. */
    auto mysql() const -> mysql_ptr;

  private:
    zpt::json __options;
    mysql_ptr __mysql{ nullptr };
};

/**
 * @brief MySQL session implementation.
 *
 * Wraps a MYSQL connection handle and provides transaction control
 * and database selection.
 */
class session : public zpt::storage::session::type {
  public:
    session(zpt::storage::mysqlx::connection const& _connection);
    session(zpt::storage::mysqlx::session const& _rhs) = delete;
    session(zpt::storage::mysqlx::session&& _rhs) = delete;
    virtual ~session() override;
    virtual auto is_open() const -> bool override;
    virtual auto commit() -> zpt::storage::session::type* override;
    virtual auto rollback() -> zpt::storage::session::type* override;
    virtual auto sql(std::string const& _statement) -> zpt::storage::session::type* override;
    virtual auto database(std::string const& _db) const -> zpt::storage::database override;

    auto mysql() const -> mysql_ptr;

  private:
    mysql_ptr __mysql{ nullptr };
};
/** @brief MySQL database implementation (represents a schema/database). */
class database : public zpt::storage::database::type {
  public:
    database(zpt::storage::mysqlx::session const& _session, std::string const& _db);
    database(zpt::storage::mysqlx::database const& _rhs) = delete;
    database(zpt::storage::mysqlx::database&& _rhs) = delete;
    virtual ~database() override = default;
    virtual auto sql(std::string const& _statement) -> zpt::storage::database::type* override;
    virtual auto collection(std::string const& _name) const -> zpt::storage::collection override;

    auto mysql() const -> mysql_ptr;

  private:
    mysql_ptr __mysql{ nullptr };
    std::string __database;
};
/** @brief MySQL collection implementation (represents a database table). */
class collection : public zpt::storage::collection::type {
  public:
    collection(zpt::storage::mysqlx::database const& _database, std::string const& _collection);
    virtual ~collection() override = default;
    virtual auto add(zpt::json _document) const -> zpt::storage::action override;
    virtual auto modify(zpt::json _search) const -> zpt::storage::action override;
    virtual auto remove(zpt::json _search) const -> zpt::storage::action override;
    virtual auto replace(std::string const& _id, zpt::json _document) const
      -> zpt::storage::action override;
    virtual auto find(zpt::json _search) const -> zpt::storage::action override;
    virtual auto count() -> size_t override;

    auto table() const -> std::string const&;
    auto mysql() const -> mysql_ptr;

  private:
    mysql_ptr __mysql{ nullptr };
    std::string __table;
};
/** @brief Base class for MySQL action operations (manages prepared statements). */
class action : public zpt::storage::action::type {
  public:
    action(zpt::storage::mysqlx::collection const& _collection);
    virtual ~action() override = default;

    /** @brief Returns the prepared statement handle. */
    auto statement() const -> mysql_stmt_ptr;
    /** @brief Returns the MySQL connection handle. */
    auto mysql() const -> mysql_ptr;

  protected:
    mysql_ptr __mysql{ nullptr };
    mysql_stmt_ptr __statement{ nullptr };
    std::string __table;
};
/** @brief MySQL INSERT action builder. */
class action_add : public zpt::storage::mysqlx::action {
  public:
    action_add(zpt::storage::mysqlx::collection const& _collection, zpt::json _document);
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
    auto get_generated_ids() const -> zpt::json;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __generated_ids{ nullptr };
};
/** @brief MySQL UPDATE action builder. */
class action_modify : public zpt::storage::mysqlx::action {
  public:
    action_modify(zpt::storage::mysqlx::collection const& _collection, zpt::json _search);
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
    zpt::json __underlying{ nullptr };
    zpt::json __filter{ nullptr };
    zpt::json __bind{ nullptr };
};
/** @brief MySQL DELETE action builder. */
class action_remove : public zpt::storage::mysqlx::action {
  public:
    action_remove(zpt::storage::mysqlx::collection const& _collection, zpt::json _search);
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
    zpt::json __filter{ nullptr };
    zpt::json __bind{ nullptr };
};
/** @brief MySQL REPLACE action builder. */
class action_replace : public zpt::storage::mysqlx::action {
  public:
    action_replace(zpt::storage::mysqlx::collection const& _collection,
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

  private:
    zpt::json __underlying{ nullptr };
};
/** @brief MySQL SELECT action builder with filtering, sorting, and pagination. */
class action_find : public zpt::storage::mysqlx::action {
  public:
    action_find(zpt::storage::mysqlx::collection const& _collection);
    action_find(zpt::storage::mysqlx::collection const& _collection, zpt::json _search);
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
    zpt::json __underlying{ nullptr };
    zpt::json __fields{ nullptr };
    zpt::json __bind{ nullptr };
    zpt::json __suffix{ nullptr };
};
/** @brief MySQL query result set. */
class result : public zpt::storage::result::type {
  public:
    result(zpt::storage::mysqlx::action& _action);
    result(zpt::storage::mysqlx::action_add& _action);
    result(zpt::storage::mysqlx::action_modify& _action);
    result(zpt::storage::mysqlx::action_remove& _action);
    result(zpt::storage::mysqlx::action_replace& _action);
    result(zpt::storage::mysqlx::action_find& _action);
    virtual ~result() override;
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    virtual auto generated_id() -> zpt::json override;
    virtual auto count() const -> size_t override;
    virtual auto status() const -> zpt::status override;
    virtual auto message() const -> std::string override;
    virtual auto to_json() const -> zpt::json override;

  private:
    mysql_ptr __mysql{ nullptr };
    mysql_stmt_ptr __statement{ nullptr };
    result_set_metadata __metadata;
    bool __is_doc_result{ false };
    zpt::json __generated_ids{ nullptr };
};
} // namespace mysqlx
} // namespace storage
} // namespace zpt
