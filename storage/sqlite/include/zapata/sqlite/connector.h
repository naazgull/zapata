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

using sqlite3_ptr = std::shared_ptr<sqlite3>;           ///< Managed SQLite database handle.
using sqlite3_stmt_ptr = std::shared_ptr<sqlite3_stmt>; ///< Managed SQLite statement handle.

/** @brief Tests if a SQLite return code indicates an error state.
 * @param _error SQLite return code to test (e.g., sqlite3_errcode() result)
 * @return True if the code indicates an error (SQLITE_ERROR, SQLITE_BUSY, SQLITE_LOCKED, etc.)
 */
auto is_error(long _error) -> bool;
/** @brief Converts a SQLite result row from a prepared statement into a JSON document.
 * @param _stmt Prepared statement with at least one row fetched
 * @return JSON document with column names as keys and cell values as values
 */
auto from_db_doc(sqlite3_stmt* _stmt) -> zpt::json;
/** @brief Serializes a JSON value to a raw byte array for BLOB storage in SQLite.
 * @param _value JSON value to serialize
 * @return Tuple of (allocated_byte_array_pointer, byte_array_size) - caller must free with free_byte_array()
 */
auto to_byte_array(zpt::json _value) -> std::tuple<char*, size_t>;
/** @brief Frees a byte array allocated by to_byte_array().
 * @param _to_delete Pointer returned by to_byte_array() to deallocate
 */
auto free_byte_array(void* _to_delete) -> void;
/** @brief Binds a JSON value to a named parameter in a prepared SQLite statement.
 * @param _stmt Prepared statement to bind to
 * @param _name Parameter name (e.g., ":param_name")
 * @param _value JSON value to bind (auto-converts to appropriate SQLite type)
 * @return void
 */
auto bind(sqlite3_stmt* _stmt, std::string const& _name, zpt::json _value) -> void;
/** @brief Converts a SQLite return code to its corresponding HTTP status code.
 * @param _error SQLite return code to convert
 * @return Corresponding zpt::status value for the error code
 */
auto to_status(int _error) -> zpt::status;

/** @brief SQLite connection implementation. */
class connection : public zpt::storage::connection::type {
  public:
    friend class session;

    /** @brief Constructs a connection with the given options (e.g., "path" to database file).
     * @param _options Connection options including database file path and other SQLite settings
     */
    explicit connection(zpt::json _options);
    virtual ~connection() override = default;

    /** @brief Opens the SQLite connection using the provided options (e.g., database file path).
     * @param _options Connection options including database file path and other SQLite settings
     * @return Pointer to this connection instance
     */
    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    /** @brief Closes the SQLite connection and releases associated resources.
     * @return Pointer to this connection instance
     */
    virtual auto close() -> zpt::storage::connection::type* override;
    /** @brief Creates and returns a new SQLite session for this connection.
     * @return Session object wrapping the underlying SQLite connection handles
     */
    virtual auto session() -> zpt::storage::session override;
    /** @brief Returns the configuration options used for this connection.
     * @return JSON object containing the connection options (e.g., database file path)
     */
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

    /** @brief Constructs a session by opening all database files listed in the connection options.
     * @param _connection Connection whose options are used to open database files
     */
    explicit session(zpt::storage::sqlite::connection const& _connection);
    session(zpt::storage::sqlite::session const& _rhs) = delete;
    session(zpt::storage::sqlite::session&& _rhs) = delete;
    virtual ~session() override = default;
    /** @brief Returns true if at least one underlying SQLite handle is open.
     * @return True if open, false otherwise
     */
    virtual auto is_open() const -> bool override;
    /** @brief Commits the current transaction on all underlying SQLite connections.
     * @return Pointer to this session instance
     */
    virtual auto commit() -> zpt::storage::session::type* override;
    /** @brief Rolls back the current transaction on all underlying SQLite connections.
     * @return Pointer to this session instance
     */
    virtual auto rollback() -> zpt::storage::session::type* override;
    /** @brief Executes a raw SQL statement directly on the session's connections.
     * @param _statement SQL statement to execute
     * @return Result object containing query results or error information
     */
    virtual auto sql(std::string const& _statement) -> zpt::storage::result override;
    /** @brief Selects and returns the named database (SQLite file) within this session.
     * @param _db Name of the SQLite database file to open/switch to
     * @return Database accessor for the specified SQLite file
     */
    virtual auto database(std::string const& _db) const -> zpt::storage::database override;
    /** @brief Registers an additional SQLite database handle with this session.
     * Allows managing multiple SQLite files within a single transaction session.
     * @param _database Shared pointer to sqlite3 handle to register
     * @return None
     */
    auto add_database_connection(sqlite3_ptr _database) -> void;

  private:
    std::vector<sqlite3_ptr> __underlying;
    zpt::json __options;
};
/** @brief SQLite database implementation (represents a single .db file). */
class database : public zpt::storage::database::type {
  public:
    friend class collection;

    /** @brief Constructs a database handle for the named SQLite file within the given session.
     * @param _session Session that owns the SQLite connection handle
     * @param _db Name of the SQLite database file (e.g., "main" or a path)
     */
    explicit database(zpt::storage::sqlite::session const& _session, std::string const& _db);
    database(zpt::storage::sqlite::database const& _rhs) = delete;
    database(zpt::storage::sqlite::database&& _rhs) = delete;
    virtual ~database() override = default;
    /** @brief Executes a raw SQL statement directly against the SQLite database file.
     * @param _statement SQL statement to execute
     * @return Result object containing query results or error information
     */
    virtual auto sql(std::string const& _statement) -> zpt::storage::result override;
    /** @brief Returns a collection (table) accessor for the given table name.
     * @param _name Name of the table to access
     * @return Collection accessor for the specified table
     */
    virtual auto collection(std::string const& _name) const -> zpt::storage::collection override;
    /** @brief Returns the underlying sqlite3 handle for this database.
     * @return Shared pointer to sqlite3 database connection
     */
    auto connection() const -> sqlite3_ptr;
    /** @brief Returns the filesystem path of the SQLite database file.
     * @return Path string of the SQLite database file (may be in-memory for :memory: databases)
     */
    auto path() const -> std::string const&;

  private:
    std::string __path;
    sqlite3_ptr __underlying{ nullptr };
};
/** @brief SQLite collection implementation (represents a database table). */
class collection : public zpt::storage::collection::type {
  public:
    friend class action;

    /** @brief Constructs a collection accessor bound to the given database and table name.
     * @param _database Database that contains the table
     * @param _collection Name of the table to access
     */
    collection(zpt::storage::sqlite::database const& _database, std::string const& _collection);
    virtual ~collection() override = default;
    /** @brief Returns an INSERT action builder for inserting a document into the SQLite table.
     * @param _document Document to insert
     * @return INSERT action builder
     */
    virtual auto add(zpt::json _document) const -> zpt::storage::action override;
    /** @brief Returns an UPDATE action builder for rows matching the given search criteria.
     * @param _search Search criteria to match rows
     * @return UPDATE action builder
     */
    virtual auto modify(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Returns a DELETE action builder for rows matching the given search criteria.
     * @param _search Search criteria to match rows
     * @return DELETE action builder
     */
    virtual auto remove(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Returns a REPLACE action builder for replacing the document with the given ID.
     * @param _id Identifier of the document to replace
     * @param _document New document data
     * @return REPLACE action builder
     */
    virtual auto replace(std::string const& _id, zpt::json _document) const
      -> zpt::storage::action override;
    /** @brief Returns a SELECT action builder for rows matching the given search criteria.
     * @param _search Search criteria to match rows
     * @return SELECT action builder
     */
    virtual auto find(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Returns the total number of rows in the SQLite table.
     * @param _search Optional search criteria to filter the count
     * @return Number of matching rows
     */
    virtual auto count(zpt::json _search = zpt::undefined) -> size_t override;

  private:
    sqlite3_ptr __underlying{ nullptr };
    std::string __collection_name;
};
/** @brief Base class for SQLite action operations (manages prepared statements). */
class action : public zpt::storage::action::type {
  public:
    /** @brief Constructs an action bound to the given collection's SQLite handle and table name.
     * @param _collection Collection that provides the SQLite connection and table name
     */
    explicit action(zpt::storage::sqlite::collection const& _collection);
    virtual ~action() override = default;

    /** @brief Records a SQLite error code as action state.
     * @param _error SQLite return code to record
     */
    auto set_state(int _error) -> void;
    /** @brief Returns the current action state (code + message).
     * @return JSON object with "code" and "message" fields describing the action state
     */
    auto get_state() const -> zpt::json;

  protected:
    std::string __collection_name;
    sqlite3_ptr __underlying{ nullptr };
    std::vector<sqlite3_stmt_ptr> __prepared;
    zpt::json __state{ "code", 0, "message", "Success" };

    /** @brief Prepares a SQL statement for later execution.
     * @param _statement SQL statement to prepare
     */
    virtual auto prepare(std::string const& _statement) -> void;
};
/** @brief SQLite INSERT action builder. */
class action_add : public zpt::storage::sqlite::action {
  public:
    /** @brief Constructs an INSERT action and prepares the statement for the given document.
     * @param _collection Collection providing the SQLite handle and table name
     * @param _document Document to insert
     */
    action_add(zpt::storage::sqlite::collection const& _collection, zpt::json _document);
    virtual ~action_add() override = default;
    /** @brief Queues an additional document for insertion into the SQLite table.
     * @param _document Document to insert
     * @return Pointer to this action instance
     */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _id Unused document identifier
     * @param _document Unused document data
     * @return Pointer to this action instance
     */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _attribute Unused attribute name
     * @param _value Unused attribute value
     * @return Pointer to this action instance
     */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _attribute Unused attribute name
     * @return Pointer to this action instance
     */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _document Unused patch document
     * @return Pointer to this action instance
     */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _attribute Unused column name
     * @param asc Unused sort direction
     * @return Pointer to this action instance
     */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _fields Unused field selector
     * @return Pointer to this action instance
     */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _rows Unused offset count
     * @return Pointer to this action instance
     */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to INSERT; returns this action unchanged.
     * @param _number Unused limit count
     * @return Pointer to this action instance
     */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values from the map into the prepared INSERT statement.
     * @param _map JSON object mapping parameter names to values
     * @return Pointer to this action instance
     */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes all prepared INSERT statements and returns a result with generated IDs.
     * @return Result object containing the inserted rows and generated IDs
     */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __generated_uuid{ zpt::json::array() };

    auto add_insert(zpt::json _document) -> void;
};
/** @brief SQLite UPDATE action builder. */
class action_modify : public zpt::storage::sqlite::action {
  public:
    /** @brief Constructs an UPDATE action targeting rows that match the given search criteria.
     * @param _collection Collection providing the SQLite handle and table name
     * @param _search Search criteria to match rows to update
     */
    action_modify(zpt::storage::sqlite::collection const& _collection, zpt::json _search);
    virtual ~action_modify() override = default;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _document Unused document
     * @return Pointer to this action instance
     */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria for the UPDATE.
     * @param _search Additional search criteria to match rows
     * @return Pointer to this action instance
     */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _id Unused document identifier
     * @param _document Unused document data
     * @return Pointer to this action instance
     */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Adds a SET clause assigning the given value to the named column.
     * @param _attribute Column name to set
     * @param _value Value to assign to the column
     * @return Pointer to this action instance
     */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Adds a SET clause that clears (nullifies) the named column.
     * @param _attribute Column name to unset
     * @return Pointer to this action instance
     */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Applies a JSON patch document as multiple SET clauses on the UPDATE.
     * @param _document Patch document describing changes
     * @return Pointer to this action instance
     */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _attribute Unused column name
     * @param asc Unused sort direction
     * @return Pointer to this action instance
     */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _fields Unused field selector
     * @return Pointer to this action instance
     */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _rows Unused offset count
     * @return Pointer to this action instance
     */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to UPDATE; returns this action unchanged.
     * @param _number Unused limit count
     * @return Pointer to this action instance
     */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values from the map into the prepared UPDATE statement.
     * @param _map JSON object mapping parameter names to values
     * @return Pointer to this action instance
     */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Builds and executes the SQLite UPDATE statement, returning the affected row count.
     * @return Result object containing the update status and affected row count
     */
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
    /** @brief Constructs a DELETE action targeting rows that match the given search criteria.
     * @param _collection Collection providing the SQLite handle and table name
     * @param _search Search criteria to match rows to delete
     */
    action_remove(zpt::storage::sqlite::collection const& _collection, zpt::json _search);
    virtual ~action_remove() override = default;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _document Unused document
     * @return Pointer to this action instance
     */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria for the DELETE.
     * @param _search Additional search criteria to match rows
     * @return Pointer to this action instance
     */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _id Unused document identifier
     * @param _document Unused document data
     * @return Pointer to this action instance
     */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _attribute Unused attribute name
     * @param _value Unused attribute value
     * @return Pointer to this action instance
     */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _attribute Unused attribute name
     * @return Pointer to this action instance
     */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _document Unused patch document
     * @return Pointer to this action instance
     */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _attribute Unused column name
     * @param asc Unused sort direction
     * @return Pointer to this action instance
     */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _fields Unused field selector
     * @return Pointer to this action instance
     */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _rows Unused offset count
     * @return Pointer to this action instance
     */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to DELETE; returns this action unchanged.
     * @param _number Unused limit count
     * @return Pointer to this action instance
     */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values from the map into the prepared DELETE statement.
     * @param _map JSON object mapping parameter names to values
     * @return Pointer to this action instance
     */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Builds and executes the SQLite DELETE statement, returning the affected row count.
     * @return Result object containing the delete status and affected row count
     */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __search;
    bool __added{ false };

    auto add_delete() -> void;
};
/** @brief SQLite REPLACE (INSERT OR REPLACE) action builder. */
class action_replace : public zpt::storage::sqlite::action {
  public:
    /** @brief Constructs a REPLACE action for the document with the given ID.
     * @param _collection Collection providing the SQLite handle and table name
     * @param _id Identifier of the document to replace
     * @param _document New document data
     */
    action_replace(zpt::storage::sqlite::collection const& _collection,
                   std::string _id,
                   zpt::json _document);
    virtual ~action_replace() override = default;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _document Unused document
     * @return Pointer to this action instance
     */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Updates the replacement document and target ID for the REPLACE action.
     * @param _id New document identifier
     * @param _document New document data
     * @return Pointer to this action instance
     */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _attribute Unused attribute name
     * @param _value Unused attribute value
     * @return Pointer to this action instance
     */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _attribute Unused attribute name
     * @return Pointer to this action instance
     */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _document Unused patch document
     * @return Pointer to this action instance
     */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _attribute Unused column name
     * @param asc Unused sort direction
     * @return Pointer to this action instance
     */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _fields Unused field selector
     * @return Pointer to this action instance
     */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _rows Unused offset count
     * @return Pointer to this action instance
     */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to REPLACE; returns this action unchanged.
     * @param _number Unused limit count
     * @return Pointer to this action instance
     */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values from the map into the prepared REPLACE statement.
     * @param _map JSON object mapping parameter names to values
     * @return Pointer to this action instance
     */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Builds and executes the SQLite INSERT OR REPLACE statement, returning the result.
     * @return Result object containing the replace status and affected row
     */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Executes the replacement of a single row identified by the stored ID.
     * @return void
     */
    virtual auto replace_one() -> void;

  private:
    std::string __id;
    zpt::json __set;

    auto add_replace() -> void;
};
/** @brief SQLite SELECT action builder with filtering, sorting, and pagination. */
class action_find : public zpt::storage::sqlite::action {
  public:
    /** @brief Constructs a SELECT action that will return all rows from the collection.
     * @param _collection Collection providing the SQLite handle and table name
     */
    action_find(zpt::storage::sqlite::collection const& _collection);
    /** @brief Constructs a SELECT action with an initial WHERE clause from the search criteria.
     * @param _collection Collection providing the SQLite handle and table name
     * @param _search Initial search criteria for the WHERE clause
     */
    action_find(zpt::storage::sqlite::collection const& _collection, zpt::json _search);
    virtual ~action_find() override = default;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _document Unused document
     * @return Pointer to this action instance
     */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _search Unused search criteria
     * @return Pointer to this action instance
     */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _id Unused document identifier
     * @param _document Unused document data
     * @return Pointer to this action instance
     */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Refines the WHERE clause with additional search criteria for the SELECT.
     * @param _search Additional search criteria to add to the WHERE clause
     * @return Pointer to this action instance
     */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _attribute Unused attribute name
     * @param _value Unused attribute value
     * @return Pointer to this action instance
     */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _attribute Unused attribute name
     * @return Pointer to this action instance
     */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to SELECT; returns this action unchanged.
     * @param _document Unused patch document
     * @return Pointer to this action instance
     */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Adds an ORDER BY clause for the given column, ascending or descending.
     * @param _attribute Column name to sort by
     * @param asc Sort direction (true for ascending, false for descending)
     * @return Pointer to this action instance
     */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Restricts the SELECT output to the specified columns.
     * @param _fields JSON array or object specifying which columns to include
     * @return Pointer to this action instance
     */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Sets the OFFSET for the SQLite SELECT query (number of rows to skip).
     * @param _rows Number of rows to skip before returning results
     * @return Pointer to this action instance
     */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Sets the LIMIT for the SQLite SELECT query (maximum rows to return).
     * @param _number Maximum number of rows to return
     * @return Pointer to this action instance
     */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief Binds named parameter values from the map into the prepared SELECT statement.
     * @param _map JSON object mapping parameter names to values
     * @return Pointer to this action instance
     */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Builds and executes the SQLite SELECT query, returning matching rows as a result.
     * @return Result object containing the matching rows and query status
     */
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
    /** @brief Constructs a result from a pre-materialized JSON value (e.g., error or empty set).
     * @param _result Pre-materialized result data
     */
    explicit result(zpt::json _result);
    /** @brief Constructs a result backed by live prepared statements for lazy row fetching.
     * @param _result Result metadata (status, message, etc.)
     * @param _prepared Vector of prepared statement handles for lazy row fetching
     */
    result(zpt::json _result, std::vector<sqlite3_stmt_ptr> _prepared);
    virtual ~result() override = default;
    /** @brief Fetches up to @p _amount rows from the SQLite result set (0 means all remaining).
     * @param _amount Number of rows to fetch (0 fetches all remaining rows)
     * @return JSON array of rows, each row being a JSON object of column names and values
     */
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    /** @brief Returns the UUIDs generated for rows inserted by the preceding INSERT action.
     * @return JSON array of generated UUID strings
     */
    virtual auto generated_id() -> zpt::json override;
    /** @brief Returns the number of rows in the result set.
     * @return Number of rows in the result set
     */
    virtual auto count() const -> size_t override;
    /** @brief Returns the HTTP-style status code reflecting the outcome of the SQLite operation.
     * @return HTTP-style status code (e.g., 200, 500)
     */
    virtual auto status() const -> zpt::status override;
    /** @brief Returns a human-readable message describing the outcome of the SQLite operation.
     * @return Message describing the operation outcome
     */
    virtual auto message() const -> std::string override;
    /** @brief Serializes the entire result set (rows, status, message) to a JSON object.
     * @return JSON object containing rows, status code, and message fields
     */
    virtual auto to_json() const -> zpt::json override;

  private:
    zpt::json __result;
    std::vector<sqlite3_stmt_ptr> __prepared;
};
} // namespace sqlite
} // namespace storage
} // namespace zpt
