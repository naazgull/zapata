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
 * @brief PostgreSQL storage connector implementation using the libpq C API.
 *
 * Implements the storage abstraction layer for PostgreSQL databases using
 * the PostgreSQL libpq client library. Provides the full connector hierarchy:
 * connection, session, database, collection, action, and result types.
 *
 * @see zpt::storage::connection
 * @see zpt::storage::make_connection
 */

#pragma once

#include <libpq-fe.h>
#include <zapata/connector.h>
#include <zapata/json.h>
#include <zapata/pgsql/translate.h>

namespace zpt {
namespace storage {
namespace pgsql {
class connection;
class session;
class database;
class collection;
class action;
class result;

using pgsql_ptr = std::shared_ptr<PGconn>;
using pgsql_result_ptr = std::shared_ptr<PGresult>;

/** @brief Deleter for PGconn handles used with shared_ptr. */
struct pgsql_conn_deinit {
    auto operator()(PGconn*) const -> void;
};

/** @brief Deleter for PGresult handles used with shared_ptr. */
struct pgsql_result_deinit {
    auto operator()(PGresult*) const -> void;
};

/** @brief RAII wrapper for PostgreSQL library initialization/cleanup. */
class library {
  public:
    /** @brief Initializes the PostgreSQL client library (no-op, libpq is lazy). */
    library();
    /** @brief Finalizes the PostgreSQL client library (no-op, libpq is lazy). */
    virtual ~library();
};

/** @brief Returns the global PostgreSQL library instance (initializes on first call). */
auto init() -> library&;

/** @brief PostgreSQL connection implementation. */
class connection : public zpt::storage::connection::type {
  public:
    /** @brief Constructs a connection with options (host, user, password, port, db). */
    connection(zpt::json _options);
    /** @brief Destructor. */
    virtual ~connection() override = default;

    /** @brief Opens or re-opens the PostgreSQL connection with the given options. */
    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    /** @brief Closes the PostgreSQL connection. */
    virtual auto close() -> zpt::storage::connection::type* override;
    /** @brief Creates a new session from this connection. */
    virtual auto session() -> zpt::storage::session override;
    /** @brief Returns the connection configuration options. */
    virtual auto options() const -> zpt::json;

    /** @brief Returns the underlying PGconn handle. */
    auto pgsql() const -> pgsql_ptr;

  private:
    zpt::json __options;
    pgsql_ptr __pgsql{ nullptr };
};

/**
 * @brief PostgreSQL session implementation.
 *
 * Wraps a PGconn connection handle and provides transaction control
 * and database selection.
 */
class session : public zpt::storage::session::type {
  public:
    /** @brief Constructs a session from the given PostgreSQL connection. */
    session(zpt::storage::pgsql::connection const& _connection);
    session(zpt::storage::pgsql::session const& _rhs) = delete;
    session(zpt::storage::pgsql::session&& _rhs) = delete;
    /** @brief Destructor; ends the session thread. */
    virtual ~session() override;
    /** @brief Returns true if the underlying PostgreSQL connection is active. */
    virtual auto is_open() const -> bool override;
    /** @brief Commits the current transaction. */
    virtual auto commit() -> zpt::storage::session::type* override;
    /** @brief Rolls back the current transaction. */
    virtual auto rollback() -> zpt::storage::session::type* override;
    /** @brief Executes a raw SQL statement on this session. */
    virtual auto sql(std::string const& _statement) -> zpt::storage::session::type* override;
    /** @brief Selects a database (schema) within this session. */
    virtual auto database(std::string const& _db) const -> zpt::storage::database override;

    /** @brief Returns the underlying PGconn handle. */
    auto pgsql() const -> pgsql_ptr;

  private:
    pgsql_ptr __pgsql{ nullptr };
};
/** @brief PostgreSQL database implementation (represents a schema/database). */
class database : public zpt::storage::database::type {
  public:
    /** @brief Constructs a database handle for the given schema name. */
    database(zpt::storage::pgsql::session const& _session, std::string const& _db);
    database(zpt::storage::pgsql::database const& _rhs) = delete;
    database(zpt::storage::pgsql::database&& _rhs) = delete;
    /** @brief Destructor. */
    virtual ~database() override = default;
    /** @brief Executes a raw SQL statement on this database. */
    virtual auto sql(std::string const& _statement) -> zpt::storage::database::type* override;
    /** @brief Returns a collection (table) handle for the given name. */
    virtual auto collection(std::string const& _name) const -> zpt::storage::collection override;

    /** @brief Returns the underlying PGconn handle. */
    auto pgsql() const -> pgsql_ptr;

  private:
    pgsql_ptr __pgsql{ nullptr };
    std::string __database;
};
/** @brief PostgreSQL collection implementation (represents a database table). */
class collection : public zpt::storage::collection::type {
  public:
    /** @brief Constructs a collection handle for the given table within a database. */
    collection(zpt::storage::pgsql::database const& _database, std::string const& _collection);
    /** @brief Destructor. */
    virtual ~collection() override = default;
    /** @brief Creates an INSERT action for the given document. */
    virtual auto add(zpt::json _document) const -> zpt::storage::action override;
    /** @brief Creates an UPDATE action with the given search criteria. */
    virtual auto modify(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Creates a DELETE action with the given search criteria. */
    virtual auto remove(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Creates a REPLACE action for the document with the given ID. */
    virtual auto replace(std::string const& _id, zpt::json _document) const
      -> zpt::storage::action override;
    /** @brief Creates a SELECT action with the given search criteria. */
    virtual auto find(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Returns the total number of rows in the table. */
    virtual auto count(zpt::json _search = zpt::undefined) -> size_t override;

    /** @brief Returns the table name. */
    auto table() const -> std::string const&;
    /** @brief Returns the underlying PGconn handle. */
    auto pgsql() const -> pgsql_ptr;

  private:
    pgsql_ptr __pgsql{ nullptr };
    std::string __table;
};
/** @brief Base class for PostgreSQL action operations (manages result sets). */
class action : public zpt::storage::action::type {
  public:
    /** @brief Constructs an action bound to the given collection. */
    action(zpt::storage::pgsql::collection const& _collection);
    /** @brief Destructor. */
    virtual ~action() override = default;

    /** @brief Returns the result set from the last execution. */
    auto result() const -> pgsql_result_ptr;
    /** @brief Returns the PostgreSQL connection handle. */
    auto pgsql() const -> pgsql_ptr;

  protected:
    pgsql_ptr __pgsql{ nullptr };
    pgsql_result_ptr __result{ nullptr };
    std::string __table;
};
/** @brief PostgreSQL INSERT action builder. */
class action_add : public zpt::storage::pgsql::action {
  public:
    /** @brief Constructs an INSERT action for the given document. */
    action_add(zpt::storage::pgsql::collection const& _collection, zpt::json _document);
    /** @brief Destructor. */
    virtual ~action_add() override = default;
    /** @brief Queues an additional document for insertion. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared INSERT statement. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the INSERT statement and returns a result with generated IDs. */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Returns the auto-generated IDs from the last INSERT execution. */
    auto get_generated_ids() const -> zpt::json;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __generated_ids{ nullptr };
};
/** @brief PostgreSQL UPDATE action builder. */
class action_modify : public zpt::storage::pgsql::action {
  public:
    /** @brief Constructs an UPDATE action with the given search criteria. */
    action_modify(zpt::storage::pgsql::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_modify() override = default;
    /** @brief Not applicable to UPDATE; returns this action unchanged. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Adds a SET clause assigning the given value to the named column. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Adds a SET clause that clears (nullifies) the named column. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Applies a JSON patch document as multiple SET clauses. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared UPDATE statement. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the UPDATE statement and returns the affected row count. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __filter{ nullptr };
    zpt::json __bind{ nullptr };
};
/** @brief PostgreSQL DELETE action builder. */
class action_remove : public zpt::storage::pgsql::action {
  public:
    /** @brief Constructs a DELETE action targeting rows that match the given search criteria. */
    action_remove(zpt::storage::pgsql::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_remove() override = default;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared DELETE statement. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the DELETE statement and returns the affected row count. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __filter{ nullptr };
    zpt::json __bind{ nullptr };
};
/** @brief PostgreSQL REPLACE action builder. */
class action_replace : public zpt::storage::pgsql::action {
  public:
    /** @brief Constructs a REPLACE action for the document with the given ID. */
    action_replace(zpt::storage::pgsql::collection const& _collection,
                   std::string _id,
                   zpt::json _document);
    /** @brief Destructor. */
    virtual ~action_replace() override = default;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Updates the replacement document and target ID. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared REPLACE statement. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the PostgreSQL REPLACE statement and returns the result. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __underlying{ nullptr };
};
/** @brief PostgreSQL SELECT action builder with filtering, sorting, and pagination. */
class action_find : public zpt::storage::pgsql::action {
  public:
    /** @brief Constructs a SELECT action that returns all rows in the collection. */
    action_find(zpt::storage::pgsql::collection const& _collection);
    /** @brief Constructs a SELECT action with an initial WHERE clause from the search document. */
    action_find(zpt::storage::pgsql::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_find() override = default;
    /** @brief Not applicable to SELECT; returns this action unchanged. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Adds an ORDER BY clause for the given column. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Restricts the columns returned by the SELECT to the given field list. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Sets the number of rows to skip (OFFSET) in the result set. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Sets the maximum number of rows (LIMIT) to return. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared SELECT statement. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the SELECT query and returns the result set. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __fields{ nullptr };
    zpt::json __bind{ nullptr };
    zpt::json __suffix{ nullptr };
};
/** @brief PostgreSQL query result set. */
class result : public zpt::storage::result::type {
  public:
    /** @brief Constructs a result from a generic action (executes the statement). */
    result(zpt::storage::pgsql::action& _action);
    /** @brief Constructs a result from an INSERT action, capturing generated IDs. */
    result(zpt::storage::pgsql::action_add& _action);
    /** @brief Constructs a result from an UPDATE action, capturing affected row count. */
    result(zpt::storage::pgsql::action_modify& _action);
    /** @brief Constructs a result from a DELETE action, capturing affected row count. */
    result(zpt::storage::pgsql::action_remove& _action);
    /** @brief Constructs a result from a REPLACE action. */
    result(zpt::storage::pgsql::action_replace& _action);
    /** @brief Constructs a result from a SELECT action, holding the result set. */
    result(zpt::storage::pgsql::action_find& _action);
    /** @brief Destructor; frees the PostgreSQL result set. */
    virtual ~result() override;
    /** @brief Fetches up to @p _amount rows as a JSON array (0 = all). */
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    /** @brief Returns the auto-generated ID(s) from the last INSERT or REPLACE. */
    virtual auto generated_id() -> zpt::json override;
    /** @brief Returns the number of rows in the result set or affected by the statement. */
    virtual auto count() const -> size_t override;
    /** @brief Returns the HTTP-style status code reflecting the operation outcome. */
    virtual auto status() const -> zpt::status override;
    /** @brief Returns a human-readable message describing the operation outcome. */
    virtual auto message() const -> std::string override;
    /** @brief Serializes the entire result to a JSON representation. */
    virtual auto to_json() const -> zpt::json override;

  private:
    pgsql_ptr __pgsql{ nullptr };
    pgsql_result_ptr __result{ nullptr };
    bool __is_doc_result{ false };
    zpt::json __generated_ids{ nullptr };
};
} // namespace pgsql
} // namespace storage
} // namespace zpt
