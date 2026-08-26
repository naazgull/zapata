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
    /**
     * @brief Releases the MYSQL handle.
     * @param _unused MYSQL handle to release.
     */
    auto operator()(MYSQL* /*unused*/) const -> void;
};

/** @brief RAII wrapper for per-thread MySQL cleanup. */
struct mysql_thread_deinit {
    /**
     * @brief Cleans up the current thread's MySQL resources.
     * @return void (thread-local MySQL resources finalized).
     */
    ~mysql_thread_deinit();
};

/** @brief Deleter for MYSQL_STMT handles used with shared_ptr. */
struct mysql_stmt_end {
    /**
     * @brief Finalizes the prepared statement handle.
     * @param _to_dispose Prepared statement handle to finalize.
     */
    auto operator()(MYSQL_STMT* _to_dispose) const -> void;
};

/** @brief RAII wrapper for MySQL library initialization/cleanup. */
class library {
  public:
    /**
     * @brief Initializes the MySQL client library.
     * @return void (constructors implicitly initialize the object).
     */
    library();
    /** @brief Finalizes the MySQL client library.
     * @return void (destructors implicitly clean up the object). */
    virtual ~library();
};

/**
 * @brief Returns the global MySQL library instance (initializes on first call).
 * @return Reference to the global MySQL library instance.
 */
auto init() -> library&;

/** @brief MySQL connection implementation. */
class connection : public zpt::storage::connection::type {
  public:
    /**
     * @brief Constructs a connection with options (host, user, password, port).
     * @param _options Connection configuration (host, user, password, port).
     */
    connection(zpt::json _options);
    /** @brief Destructor. */
    virtual ~connection() override = default;

    /** @brief Opens or re-opens the MySQL connection with the given options.
     * @param _options Connection options (host, user, password, port, etc.).
     * @return Pointer to this connection type. */
    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    /** @brief Closes the MySQL connection.
     * @return Pointer to this connection type. */
    virtual auto close() -> zpt::storage::connection::type* override;
    /** @brief Creates a new session from this connection.
     * @return A new session wrapping the connection. */
    virtual auto session() -> zpt::storage::session override;
    /** @brief Returns the connection configuration options.
     * @return The JSON options object. */
    virtual auto options() const -> zpt::json;

    /** @brief Returns the underlying MYSQL handle.
     * @return The shared MYSQL pointer. */
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
    /** @brief Constructs a session from the given MySQL connection.
     * @param _connection The MySQL connection to wrap. */
    session(zpt::storage::mysqlx::connection const& _connection);
    session(zpt::storage::mysqlx::session const& _rhs) = delete;
    session(zpt::storage::mysqlx::session&& _rhs) = delete;
    /**
     * @brief Destructor; ends the session thread.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~session() override;
    /** @brief Returns true if the underlying MySQL connection is active.
     * @return True if connected, false otherwise. */
    virtual auto is_open() const -> bool override;
    /** @brief Commits the current transaction.
     * @return Pointer to this session type. */
    virtual auto commit() -> zpt::storage::session::type* override;
    /** @brief Rolls back the current transaction.
     * @return Pointer to this session type. */
    virtual auto rollback() -> zpt::storage::session::type* override;
    /** @brief Executes a raw SQL statement on this session.
     * @param _statement The SQL statement to execute.
     * @return A result set with the query outcome. */
    virtual auto sql(std::string const& _statement) -> zpt::storage::result override;
    /** @brief Selects a database (schema) within this session.
     * @param _db The database name to select.
     * @return A database handle for the given schema. */
    virtual auto database(std::string const& _db) const -> zpt::storage::database override;

    /** @brief Returns the underlying MYSQL handle.
     * @return The shared MYSQL pointer. */
    auto mysql() const -> mysql_ptr;

  private:
    mysql_ptr __mysql{ nullptr };
};
/** @brief MySQL database implementation (represents a schema/database). */
class database : public zpt::storage::database::type {
  public:
    /** @brief Constructs a database handle for the given schema name.
     * @param _session The session containing the connection.
     * @param _db The database schema name. */
    database(zpt::storage::mysqlx::session const& _session, std::string const& _db);
    database(zpt::storage::mysqlx::database const& _rhs) = delete;
    database(zpt::storage::mysqlx::database&& _rhs) = delete;
    /** @brief Destructor. */
    virtual ~database() override = default;
    /** @brief Executes a raw SQL statement on this database.
     * @param _statement The SQL statement to execute.
     * @return A result set with the query outcome. */
    virtual auto sql(std::string const& _statement) -> zpt::storage::result override;
    /** @brief Returns a collection (table) handle for the given name.
     * @param _name The table name.
     * @return A collection handle for the given table. */
    virtual auto collection(std::string const& _name) const -> zpt::storage::collection override;
    /** @brief Backs up the database to the given path.
     * @param _path The path to the file that will hold the backup.
     * @return The size of the backup, in bytes. */
    virtual auto backup(std::filesystem::path const& _path) const -> size_t override;
    /** @brief Loads the database content from the given path.
     * @param _path The path to the file that holds the backup.
     * @return The size of the backup, in bytes. */
    virtual auto restore(std::filesystem::path const& _path) const -> size_t override;

    /** @brief Returns the underlying MYSQL handle.
     * @return The shared MYSQL pointer. */
    auto mysql() const -> mysql_ptr;

  private:
    mysql_ptr __mysql{ nullptr };
    std::string __database;
};
/** @brief MySQL collection implementation (represents a database table). */
class collection : public zpt::storage::collection::type {
  public:
    /** @brief Constructs a collection handle for the given table within a database.
     * @param _database The database containing the table.
     * @param _collection The table name. */
    collection(zpt::storage::mysqlx::database const& _database, std::string const& _collection);
    /** @brief Destructor. */
    virtual ~collection() override = default;
    /** @brief Creates an INSERT action for the given document.
     * @param _document The document to insert.
     * @return An INSERT action builder. */
    virtual auto add(zpt::json _document) const -> zpt::storage::action override;
    /** @brief Creates an UPDATE action with the given search criteria.
     * @param _search The search criteria for the update.
     * @return An UPDATE action builder. */
    virtual auto modify(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Creates a DELETE action with the given search criteria.
     * @param _search The search criteria for the delete.
     * @return A DELETE action builder. */
    virtual auto remove(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Creates a REPLACE action for the document with the given ID.
     * @param _id The document ID for the replace.
     * @param _document The replacement document.
     * @return A REPLACE action builder. */
    virtual auto replace(std::string const& _id, zpt::json _document) const
      -> zpt::storage::action override;
    /** @brief Creates a SELECT action with the given search criteria.
     * @param _search The search criteria for the select.
     * @return A SELECT action builder. */
    virtual auto find(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Returns the total number of rows in the table.
     * @param _search Optional search criteria (zpt::undefined for all rows).
     * @return The total row count. */
    virtual auto count(zpt::json _search = zpt::undefined) -> size_t override;

    /** @brief Returns the table name.
     * @return Reference to the table name string. */
    auto table() const -> std::string const&;
    /** @brief Returns the underlying MYSQL handle.
     * @return The shared MYSQL pointer. */
    auto mysql() const -> mysql_ptr;

  private:
    mysql_ptr __mysql{ nullptr };
    std::string __table;
};
/** @brief Base class for MySQL action operations (manages prepared statements). */
class action : public zpt::storage::action::type {
  public:
    /** @brief Constructs an action bound to the given collection.
     * @param _collection The collection this action operates on. */
    action(zpt::storage::mysqlx::collection const& _collection);
    /** @brief Destructor. */
    virtual ~action() override = default;

    /** @brief Returns the prepared statement handle.
     * @return The shared MYSQL_STMT pointer. */
    auto statement() const -> mysql_stmt_ptr;
    /** @brief Returns the MySQL connection handle.
     * @return The shared MYSQL pointer. */
    auto mysql() const -> mysql_ptr;

  protected:
    mysql_ptr __mysql{ nullptr };
    mysql_stmt_ptr __statement{ nullptr };
    std::string __table;
};
/** @brief MySQL INSERT action builder. */
class action_add : public zpt::storage::mysqlx::action {
  public:
    /** @brief Constructs an INSERT action for the given document.
     * @param _collection The collection to insert into.
     * @param _document The document to insert. */
    action_add(zpt::storage::mysqlx::collection const& _collection, zpt::json _document);
    /** @brief Destructor. */
    virtual ~action_add() override = default;
    /** @brief Queues an additional document for insertion.
     * @param _document The document to add.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _id Unused.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _attribute Unused.
     * @param _value Unused.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _attribute Unused.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _attribute Unused.
     * @param asc Unused.
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _fields Unused.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _rows Unused.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _number Unused.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared INSERT statement.
     * @param _map The parameter name-to-value mapping.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the INSERT statement and returns a result with generated IDs.
     * @return A result set with generated IDs. */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Returns the auto-generated IDs from the last INSERT execution.
     * @return JSON object of generated IDs. */
    auto get_generated_ids() const -> zpt::json;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __generated_ids{ nullptr };
};
/** @brief MySQL UPDATE action builder. */
class action_modify : public zpt::storage::mysqlx::action {
  public:
    /** @brief Constructs an UPDATE action with the given search criteria.
     * @param _collection The collection to update.
     * @param _search The initial search criteria. */
    action_modify(zpt::storage::mysqlx::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_modify() override = default;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria.
     * @param _search Additional search criteria.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _id Unused.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Adds a SET clause assigning the given value to the named column.
     * @param _attribute The column name.
     * @param _value The value to assign.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Adds a SET clause that clears (nullifies) the named column.
     * @param _attribute The column name to clear.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Applies a JSON patch document as multiple SET clauses.
     * @param _document The JSON patch document.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _attribute Unused.
     * @param asc Unused.
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _fields Unused.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _rows Unused.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _number Unused.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared UPDATE statement.
     * @param _map The parameter name-to-value mapping.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the UPDATE statement and returns the affected row count.
     * @return A result set with the affected row count. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __filter{ nullptr };
    zpt::json __bind{ nullptr };
};
/** @brief MySQL DELETE action builder. */
class action_remove : public zpt::storage::mysqlx::action {
  public:
    /** @brief Constructs a DELETE action targeting rows that match the given search criteria.
     * @param _collection The collection to delete from.
     * @param _search The initial search criteria. */
    action_remove(zpt::storage::mysqlx::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_remove() override = default;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria.
     * @param _search Additional search criteria.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _id Unused.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _attribute Unused.
     * @param _value Unused.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _attribute Unused.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _attribute Unused.
     * @param asc Unused.
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _fields Unused.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _rows Unused.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _number Unused.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared DELETE statement.
     * @param _map The parameter name-to-value mapping.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the DELETE statement and returns the affected row count.
     * @return A result set with the affected row count. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __filter{ nullptr };
    zpt::json __bind{ nullptr };
};
/** @brief MySQL REPLACE action builder. */
class action_replace : public zpt::storage::mysqlx::action {
  public:
    /** @brief Constructs a REPLACE action for the document with the given ID.
     * @param _collection The collection to replace into.
     * @param _id The document ID.
     * @param _document The replacement document. */
    action_replace(zpt::storage::mysqlx::collection const& _collection,
                   std::string _id,
                   zpt::json _document);
    /** @brief Destructor. */
    virtual ~action_replace() override = default;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Updates the replacement document and target ID.
     * @param _id The new document ID.
     * @param _document The new replacement document.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _attribute Unused.
     * @param _value Unused.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _attribute Unused.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _attribute Unused.
     * @param asc Unused.
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _fields Unused.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _rows Unused.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _number Unused.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared REPLACE statement.
     * @param _map The parameter name-to-value mapping.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the MySQL REPLACE statement and returns the result.
     * @return A result set with the operation outcome. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __underlying{ nullptr };
};
/** @brief MySQL SELECT action builder with filtering, sorting, and pagination. */
class action_find : public zpt::storage::mysqlx::action {
  public:
    /** @brief Constructs a SELECT action that returns all rows in the collection.
     * @param _collection The collection to query. */
    action_find(zpt::storage::mysqlx::collection const& _collection);
    /** @brief Constructs a SELECT action with an initial WHERE clause from the search document.
     * @param _collection The collection to query.
     * @param _search The initial search criteria. */
    action_find(zpt::storage::mysqlx::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_find() override = default;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _id Unused.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria.
     * @param _search Additional search criteria.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _attribute Unused.
     * @param _value Unused.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _attribute Unused.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Adds an ORDER BY clause for the given column.
     * @param _attribute The column to sort by.
     * @param asc Sort direction (true = ascending).
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Restricts the columns returned by the SELECT to the given field list.
     * @param _fields The field list to return.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Sets the number of rows to skip (OFFSET) in the result set.
     * @param _rows Number of rows to skip.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Sets the maximum number of rows (LIMIT) to return.
     * @param _number Maximum number of rows.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values into the prepared SELECT statement.
     * @param _map The parameter name-to-value mapping.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the SELECT query and returns the result set.
     * @return A result set containing the query results. */
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
    /** @brief Constructs a result from the structures resulting from an SQL execution.
     * @param _mysql The MySQL connection handle.
     * @param _statement The prepared statement handle. */
    result(mysql_ptr _mysql, mysql_stmt_ptr _statement);
    /** @brief Constructs a result from a generic action.
     * @param _action The action that produced the result. */
    result(zpt::storage::mysqlx::action& _action);
    /** @brief Constructs a result from an INSERT action, capturing generated IDs.
     * @param _action The INSERT action. */
    result(zpt::storage::mysqlx::action_add& _action);
    /** @brief Constructs a result from an UPDATE action, capturing affected row count.
     * @param _action The UPDATE action. */
    result(zpt::storage::mysqlx::action_modify& _action);
    /** @brief Constructs a result from a DELETE action, capturing affected row count.
     * @param _action The DELETE action. */
    result(zpt::storage::mysqlx::action_remove& _action);
    /** @brief Constructs a result from a REPLACE action.
     * @param _action The REPLACE action. */
    result(zpt::storage::mysqlx::action_replace& _action);
    /** @brief Constructs a result from a SELECT action, holding the result set.
     * @param _action The SELECT action. */
    result(zpt::storage::mysqlx::action_find& _action);
    /**
     * @brief Destructor; frees the MySQL result set.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~result() override;
    /** @brief Fetches up to @p _amount rows as a JSON array (0 = all).
     * @param _amount Maximum number of rows to fetch (0 = fetch all).
     * @return A JSON array of rows. */
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    /** @brief Returns the auto-generated ID(s) from the last INSERT or REPLACE.
     * @return JSON object of generated ID(s). */
    virtual auto generated_id() -> zpt::json override;
    /** @brief Returns the number of rows in the result set or affected by the statement.
     * @return The row count. */
    virtual auto count() const -> size_t override;
    /** @brief Returns the HTTP-style status code reflecting the operation outcome.
     * @return The status code. */
    virtual auto status() const -> zpt::status override;
    /** @brief Returns a human-readable message describing the operation outcome.
     * @return The status message string. */
    virtual auto message() const -> std::string override;
    /** @brief Serializes the entire result to a JSON representation.
     * @return A JSON object representing the full result. */
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
