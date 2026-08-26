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
 * @brief Core storage connector interfaces.
 *
 * Defines the abstract interface for database operations. All concrete
 * database implementations (SQLite, MySQL) derive from these types.
 *
 * @par Architecture
 * The storage layer follows a hierarchical model:
 * - connection -> session -> database -> collection -> action -> result
 *
 * @par Query Builder Pattern
 * Actions use a fluent API for building queries:
 * @code
 * auto results = collection->find({ "status", "active" })
 *     ->fields({ "name", "email" })
 *     ->sort("created_at", false)
 *     ->limit(10)
 *     ->execute();
 * @endcode
 */

#pragma once

#include <zapata/json.h>
#include <zapata/ontology.h>

namespace zpt {
namespace storage {
class connection;
class session;
class database;
class collection;
class action;
class result;

/** @brief Function type for custom SQL output.
 * @param _data JSON data to output.
 * @param _out Output stream to write to. */
using functor = std::function<void(zpt::json, std::ostream&)>;
/** @brief Function type for string output.
 * @param _data JSON data to output.
 * @param _out Output stream to write to. */
using string_output = std::function<void(zpt::json, std::ostream&)>;

/** @brief SQL aggregate functions. */
enum sql_functions { COUNT = 0 };

/**
 * @brief Database connection wrapper.
 *
 * Manages the lifecycle of a database connection. Use `zpt::make_connection`
 * to create instances for specific backends.
 *
 * @par Example
 * @code
 * auto conn = zpt::make_connection<zpt::storage::sqlite::connection>(options);
 * conn->open(options);
 * auto session = conn->session();
 * @endcode
 */
class connection {
  public:
    /** @brief Abstract connection implementation. */
    class type {
      public:
        type() = default;
        virtual ~type() = default;

        /** @brief Opens the connection with given options.
         * @param _options Connection options (e.g., host, port, credentials).
         * @return Pointer to this connection type. */
        virtual auto open(zpt::json _options) -> zpt::storage::connection::type* = 0;
        /** @brief Closes the connection.
         * @return Pointer to this connection type. */
        virtual auto close() -> zpt::storage::connection::type* = 0;
        /** @brief Creates a new session.
         * @return New session. */
        virtual auto session() -> zpt::storage::session = 0;
    };

    /** @brief Default constructor (null connection).
     * @return void (constructors implicitly initialize the object). */
    connection() = default;
    /** @brief Wraps a raw connection implementation.
     * @param _underlying Raw connection implementation to wrap.
     * @return void (constructors implicitly initialize the object). */
    connection(zpt::storage::connection::type* _underlying);
    /** @brief Copy constructor (shared semantics).
     * @param _rhs Connection to copy.
     * @return void (constructors implicitly initialize the object). */
    connection(zpt::storage::connection const& _rhs);
    /** @brief Move constructor.
     * @param _rhs Connection to move from.
     * @return void (constructors implicitly initialize the object). */
    connection(zpt::storage::connection&& _rhs);
    /** @brief Destroys the connection.
     * @return void (destructors implicitly clean up the object). */
    virtual ~connection() = default;

    /** @brief Copy assignment.
     * @param _rhs Connection to copy from.
     * @return Reference to this connection. */
    auto operator=(zpt::storage::connection const& _rhs) -> zpt::storage::connection&;
    /** @brief Move assignment.
     * @param _rhs Connection to move from.
     * @return Reference to this connection. */
    auto operator=(zpt::storage::connection&& _rhs) -> zpt::storage::connection&;

    /** @brief Accesses the underlying connection implementation.
     * @return Pointer to the underlying connection implementation. */
    auto operator->() -> zpt::storage::connection::type*;
    /** @brief Dereferences to the underlying connection implementation.
     * @return Reference to the underlying connection implementation. */
    auto operator*() -> zpt::storage::connection::type&;

  private:
    std::shared_ptr<zpt::storage::connection::type> __underlying{ nullptr };
};

/**
 * @brief Database session with transaction support.
 *
 * Sessions provide transaction boundaries and access to databases.
 */
class session {
  public:
    /** @brief Abstract session implementation. */
    class type {
      public:
        type() = default;
        virtual ~type() = default;

        /** @brief Returns true if session is open.
         * @return True if open. */
        virtual auto is_open() const -> bool = 0;
        /** @brief Executes raw SQL statement.
         * @param _statement SQL statement to execute.
         * @return Query result. */
        virtual auto sql(std::string const& _statement) -> zpt::storage::result = 0;
        /** @brief Commits the current transaction.
         * @return Pointer to this session type. */
        virtual auto commit() -> zpt::storage::session::type* = 0;
        /** @brief Rolls back the current transaction.
         * @return Pointer to this session type. */
        virtual auto rollback() -> zpt::storage::session::type* = 0;
        /** @brief Gets a database by name.
         * @param _db Database name.
         * @return Database object. */
        virtual auto database(std::string const& _db) const -> zpt::storage::database = 0;
    };

    /** @brief Default constructor (null session).
     * @return void (constructors implicitly initialize the object). */
    session() = default;
    /** @brief Wraps a raw session implementation.
     * @param _underlying Raw session implementation to wrap.
     * @return void (constructors implicitly initialize the object). */
    session(zpt::storage::session::type* _underlying);
    /** @brief Copy constructor (shared semantics).
     * @param _rhs Session to copy.
     * @return void (constructors implicitly initialize the object). */
    session(zpt::storage::session const& _rhs);
    /** @brief Move constructor.
     * @param _rhs Session to move from.
     * @return void (constructors implicitly initialize the object). */
    session(zpt::storage::session&& _rhs);
    /** @brief Destroys the session.
     * @return void (destructors implicitly clean up the object). */
    virtual ~session() = default;

    /** @brief Copy assignment.
     * @param _rhs Session to copy from.
     * @return Reference to this session. */
    auto operator=(zpt::storage::session const& _rhs) -> zpt::storage::session&;
    /** @brief Move assignment.
     * @param _rhs Session to move from.
     * @return Reference to this session. */
    auto operator=(zpt::storage::session&& _rhs) -> zpt::storage::session&;

    /** @brief Accesses the underlying session implementation.
     * @return Pointer to the underlying session implementation. */
    auto operator->() -> zpt::storage::session::type*;
    /** @brief Dereferences to the underlying session implementation.
     * @return Reference to the underlying session implementation. */
    auto operator*() -> zpt::storage::session::type&;

  private:
    std::shared_ptr<zpt::storage::session::type> __underlying{ nullptr };
};

/**
 * @brief Database namespace container.
 *
 * Represents a database within a session and provides access to collections.
 */
class database {
  public:
    /** @brief Abstract database implementation. */
    class type {
      public:
        type() = default;
        virtual ~type() = default;

        /** @brief Executes raw SQL statement.
         * @param _statement SQL statement to execute.
         * @return Query result. */
        virtual auto sql(std::string const& _statement) -> zpt::storage::result = 0;
        /** @brief Gets a collection/table by name.
         * @param _name Collection name.
         * @return Collection object. */
        virtual auto collection(std::string const& _name) const -> zpt::storage::collection = 0;
        /** @brief Backs up the database to the given path.
         * @param _path The path to the file that will hold the backup.
         * @return The size of the backup, in bytes. */
        virtual auto backup(std::filesystem::path const& _path) const -> size_t = 0;
        /** @brief Loads the database content from the given path.
         * @param _path The path to the file that holds the backup.
         * @return The size of the backup, in bytes. */
        virtual auto restore(std::filesystem::path const& _path) const -> size_t = 0;
    };

    /** @brief Default constructor (null database). */
    database() = default;
    /**
     * @brief Wraps a raw database implementation.
     * @param _underlying Raw database implementation pointer.
     */
    database(zpt::storage::database::type* _underlying);
    /**
     * @brief Copy constructor (shared semantics).
     * @param _rhs Database to copy from.
     */
    database(zpt::storage::database const& _rhs);
    /**
     * @brief Move constructor.
     * @param _rhs Database to move from.
     */
    database(zpt::storage::database&& _rhs);
    /** @brief Destructor. */
    virtual ~database() = default;

    /**
     * @brief Copy assignment.
     * @param _rhs Database to copy from.
     * @return Reference to this database.
     */
    auto operator=(zpt::storage::database const& _rhs) -> zpt::storage::database&;
    /**
     * @brief Move assignment.
     * @param _rhs Database to move from.
     * @return Reference to this database.
     */
    auto operator=(zpt::storage::database&& _rhs) -> zpt::storage::database&;

    /**
     * @brief Accesses the underlying database implementation.
     * @return Pointer to the underlying database implementation.
     */
    auto operator->() -> zpt::storage::database::type*;
    /**
     * @brief Dereferences to the underlying database implementation.
     * @return Reference to the underlying database implementation.
     */
    auto operator*() -> zpt::storage::database::type&;

  private:
    std::shared_ptr<zpt::storage::database::type> __underlying{ nullptr };
};

/**
 * @brief Collection/table interface for CRUD operations.
 *
 * Provides methods to create actions for add, modify, remove, replace,
 * and find operations.
 *
 * @par Example
 * @code
 * auto coll = database->collection("users");
 *
 * // Insert
 * coll->add({ "name", "John", "email", "john@example.com" })->execute();
 *
 * // Query
 * auto results = coll->find({ "active", true })->limit(10)->execute();
 * auto docs = results->fetch();
 * @endcode
 */
class collection {
  public:
    /** @brief Abstract collection implementation. */
    class type {
      public:
        type() = default;
        virtual ~type() = default;

        /** @brief Creates an insert action.
         * @param _document Document to insert.
         * @return Action for the insert. */
        virtual auto add(zpt::json _document) const -> zpt::storage::action = 0;
        /** @brief Creates an update action with search criteria.
         * @param _search Search criteria.
         * @return Action for the update. */
        virtual auto modify(zpt::json _search) const -> zpt::storage::action = 0;
        /** @brief Creates a delete action with search criteria.
         * @param _search Search criteria.
         * @return Action for the delete. */
        virtual auto remove(zpt::json _search) const -> zpt::storage::action = 0;
        /** @brief Creates a replace action for a specific document.
         * @param _id Document ID to replace.
         * @param _document New document.
         * @return Action for the replace. */
        virtual auto replace(std::string const& _id, zpt::json _document) const
          -> zpt::storage::action = 0;
        /** @brief Creates a find action with search criteria.
         * @param _search Search criteria.
         * @return Action for the find. */
        virtual auto find(zpt::json _search) const -> zpt::storage::action = 0;
        /** @brief Returns the total count of documents.
         * @param _search Optional search criteria.
         * @return Total count of matching documents. */
        virtual auto count(zpt::json _search = zpt::undefined) -> size_t = 0;
    };

    /** @brief Default constructor (null collection). */
    collection() = default;
    /** @brief Wraps a raw collection implementation.
     * @param _underlying Raw collection type pointer. */
    collection(zpt::storage::collection::type* _underlying);
    /** @brief Copy constructor (shared semantics).
     * @param _rhs Another collection to copy. */
    collection(zpt::storage::collection const& _rhs);
    /** @brief Move constructor.
     * @param _rhs Collection to move from. */
    collection(zpt::storage::collection&& _rhs);
    /** @brief Destructor. */
    virtual ~collection() = default;

    /** @brief Copy assignment (shared semantics).
     * @param _rhs Another collection to copy.
     * @return Reference to this collection. */
    auto operator=(zpt::storage::collection const& _rhs) -> zpt::storage::collection&;
    /** @brief Move assignment.
     * @param _rhs Collection to move from.
     * @return Reference to this collection. */
    auto operator=(zpt::storage::collection&& _rhs) -> zpt::storage::collection&;

    /** @brief Accesses the underlying collection implementation (non-const pointer).
     * @return Pointer to the collection type. */
    auto operator->() -> zpt::storage::collection::type*;
    /** @brief Dereferences to the underlying collection implementation (non-const reference).
     * @return Reference to the collection type. */
    auto operator*() -> zpt::storage::collection::type&;

  private:
    std::shared_ptr<zpt::storage::collection::type> __underlying{ nullptr };
};

/**
 * @brief Query builder with fluent API.
 *
 * Actions are created by collection methods and configured using
 * a fluent interface. Call `execute()` to run the query.
 *
 * @par Fluent Methods
 * - `set()` / `unset()` - Set/unset fields for update
 * - `patch()` - Apply partial update document
 * - `sort()` - Order results
 * - `fields()` - Select specific fields
 * - `offset()` / `limit()` - Pagination
 * - `bind()` - Bind parameters
 */
class action {
  public:
    /** @brief Abstract action implementation. */
    class type {
      public:
        type() = default;
        virtual ~type() = default;

        /** @brief Adds another document to insert.
         * @param _document Document to add.
         * @return Pointer to this action type. */
        virtual auto add(zpt::json _document) -> zpt::storage::action::type* = 0;
        /** @brief Modifies search criteria.
         * @param _search Search criteria.
         * @return Pointer to this action type. */
        virtual auto modify(zpt::json _search) -> zpt::storage::action::type* = 0;
        /** @brief Modifies delete criteria.
         * @param _search Delete criteria.
         * @return Pointer to this action type. */
        virtual auto remove(zpt::json _search) -> zpt::storage::action::type* = 0;
        /** @brief Sets document to replace.
         * @param _id Document ID to replace.
         * @param _document New document.
         * @return Pointer to this action type. */
        virtual auto replace(std::string const& _id, zpt::json _document)
          -> zpt::storage::action::type* = 0;
        /** @brief Adds find criteria.
         * @param _search Find criteria.
         * @return Pointer to this action type. */
        virtual auto find(zpt::json _search) -> zpt::storage::action::type* = 0;
        /** @brief Sets a field value for update.
         * @param _attribute Field name.
         * @param _value Value to set.
         * @return Pointer to this action type. */
        virtual auto set(std::string const& _attribute, zpt::json _value)
          -> zpt::storage::action::type* = 0;
        /** @brief Removes a field for update.
         * @param _attribute Field name to remove.
         * @return Pointer to this action type. */
        virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* = 0;
        /** @brief Applies partial update document.
         * @param _document Partial update document.
         * @return Pointer to this action type. */
        virtual auto patch(zpt::json _document) -> zpt::storage::action::type* = 0;
        /** @brief Sorts results by attribute.
         * @param _attribute Attribute name to sort by.
         * @param asc Sort direction (default true = ascending).
         * @return Pointer to this action type. */
        virtual auto sort(std::string const& _attribute, bool asc = true)
          -> zpt::storage::action::type* = 0;
        /** @brief Selects specific fields to return.
         * @param _fields JSON array of field names.
         * @return Pointer to this action type. */
        virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* = 0;
        /** @brief Skips first N rows (pagination).
         * @param _rows Number of rows to skip.
         * @return Pointer to this action type. */
        virtual auto offset(size_t _rows) -> zpt::storage::action::type* = 0;
        /** @brief Limits results to N rows.
         * @param _number Maximum number of rows to return.
         * @return Pointer to this action type. */
        virtual auto limit(size_t _number) -> zpt::storage::action::type* = 0;
        /** @brief Binds parameter values.
         * @param _map JSON object mapping parameter names to values.
         * @return Pointer to this action type. */
        virtual auto bind(zpt::json _map) -> zpt::storage::action::type* = 0;
        /** @brief Executes the action and returns results.
         * @return Query result. */
        virtual auto execute() -> zpt::storage::result = 0;
    };

    /** @brief Default constructor (null action). */
    action() = default;
    /**
     * @brief Wraps a raw action implementation.
     * @param _underlying Raw action implementation pointer.
     */
    action(zpt::storage::action::type* _underlying);
    /**
     * @brief Copy constructor (shared semantics).
     * @param _rhs Action to copy from.
     */
    action(zpt::storage::action const& _rhs);
    /**
     * @brief Move constructor.
     * @param _rhs Action to move from.
     */
    action(zpt::storage::action&& _rhs);
    /** @brief Destructor. */
    virtual ~action() = default;

    /**
     * @brief Copy assignment.
     * @param _rhs Action to copy from.
     * @return Reference to this action.
     */
    auto operator=(zpt::storage::action const& _rhs) -> zpt::storage::action&;
    /**
     * @brief Move assignment.
     * @param _rhs Action to move from.
     * @return Reference to this action.
     */
    auto operator=(zpt::storage::action&& _rhs) -> zpt::storage::action&;

    /**
     * @brief Accesses the underlying action implementation.
     * @return Pointer to the underlying action implementation.
     */
    auto operator->() -> zpt::storage::action::type*;
    /**
     * @brief Dereferences to the underlying action implementation.
     * @return Reference to the underlying action implementation.
     */
    auto operator*() -> zpt::storage::action::type&;

  private:
    std::shared_ptr<zpt::storage::action::type> __underlying{ nullptr };
};

/**
 * @brief Query result set.
 *
 * Contains the results of an executed action. Supports fetching
 * documents, retrieving generated IDs, and checking status.
 */
class result {
  public:
    /** @brief Abstract result implementation. */
    class type {
      public:
        type() = default;
        virtual ~type() = default;

        /** @brief Fetches documents (0 = all).
         * @param _amount Number of rows to fetch (0 = all).
         * @return JSON array of documents. */
        virtual auto fetch(size_t _amount = 0) -> zpt::json = 0;
        /** @brief Returns generated IDs from insert.
         * @return JSON value containing generated IDs. */
        virtual auto generated_id() -> zpt::json = 0;
        /** @brief Returns the number of affected/returned rows.
         * @return Row count. */
        virtual auto count() const -> size_t = 0;
        /** @brief Returns the operation status code.
         * @return Status code. */
        virtual auto status() const -> zpt::status = 0;
        /** @brief Returns status message.
         * @return Status message string. */
        virtual auto message() const -> std::string = 0;
        /** @brief Converts result to JSON.
         * @return JSON representation of the result. */
        virtual auto to_json() const -> zpt::json = 0;
    };

    /** @brief Default constructor (null result). */
    result() = default;
    /** @brief Wraps a raw result implementation.
     * @param _underlying Raw result type pointer. */
    result(zpt::storage::result::type* _underlying);
    /** @brief Copy constructor (shared semantics).
     * @param _rhs Another result to copy. */
    result(zpt::storage::result const& _rhs);
    /** @brief Move constructor.
     * @param _rhs Result to move from. */
    result(zpt::storage::result&& _rhs);
    /** @brief Destructor. */
    virtual ~result() = default;

    /** @brief Copy assignment (shared semantics).
     * @param _rhs Another result to copy.
     * @return Reference to this result. */
    auto operator=(zpt::storage::result const& _rhs) -> zpt::storage::result&;
    /** @brief Move assignment.
     * @param _rhs Result to move from.
     * @return Reference to this result. */
    auto operator=(zpt::storage::result&& _rhs) -> zpt::storage::result&;

    /** @brief Accesses the underlying result implementation (non-const pointer).
     * @return Pointer to the result type. */
    auto operator->() -> zpt::storage::result::type*;
    /** @brief Dereferences to the underlying result implementation (non-const reference).
     * @return Reference to the result type. */
    auto operator*() -> zpt::storage::result::type&;

  private:
    std::shared_ptr<zpt::storage::result::type> __underlying{ nullptr };
};

/** @brief Creates a remove action with query parameters.
 * @param _collection Collection to query.
 * @param _params Query parameters.
 * @return Action for the remove. */
auto filter_remove(zpt::storage::collection& _collection, zpt::json _params)
  -> zpt::storage::action;
/** @brief Creates a modify action with query parameters.
 * @param _collection Collection to query.
 * @param _params Query parameters.
 * @return Action for the modify. */
auto filter_modify(zpt::storage::collection& _collection, zpt::json _params)
  -> zpt::storage::action;
/** @brief Creates a find action with query parameters.
 * @param _collection Collection to query.
 * @param _params Query parameters.
 * @return Action for the find. */
auto filter_find(zpt::storage::collection& _collection, zpt::json _params) -> zpt::storage::action;
/** @brief Formats find results into a reply.
 * @param _reply Output reply JSON object.
 * @param _params Query parameters. */
auto reply_find(zpt::json& _reply, zpt::json _params) -> void;
/** @brief Extracts find criteria from JSON.
 * @param _to_parse JSON object containing criteria.
 * @return SQL WHERE clause string. */
auto extract_find(zpt::json _to_parse) -> std::string;
/** @brief Converts functional JSON to SQL.
 * @param _function JSON object describing the function.
 * @param _find Output stream for the SQL fragment.
 * @param _str_output String output callback. */
auto functional_to_sql(zpt::json _function,
                       std::ostream& _find,
                       zpt::storage::string_output _str_output) -> void;
/** @brief Converts functor name and params to SQL.
 * @param _functor Functor name.
 * @param _params Functor parameters.
 * @param _find Output stream for the SQL fragment. */
auto functor_to_sql(std::string const& _functor, zpt::json _params, std::ostream& _find) -> void;
} // namespace storage

/**
 * @brief Creates a thread-local connection of type T.
 * @tparam T Connection implementation type.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to constructor.
 */
template<typename T, typename... Args>
auto make_connection(Args&... _args) -> zpt::storage::connection;

/**
 * @brief Creates a thread-local session of type T.
 * @tparam T Session implementation type.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to the constructor.
 * @return Thread-local session wrapping the session implementation.
 */
template<typename T, typename... Args>
auto make_session(Args&... _args) -> zpt::storage::session;

/**
 * @brief Creates a database of type T.
 * @tparam T Database implementation type.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to the constructor.
 * @return Thread-local database wrapping the database implementation.
 */
template<typename T, typename... Args>
auto make_database(Args&... _args) -> zpt::storage::database;

/**
 * @brief Creates a collection of type T.
 * @tparam T Collection implementation type.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to the constructor.
 * @return Thread-local collection wrapping the collection implementation.
 */
template<typename T, typename... Args>
auto make_collection(Args&... _args) -> zpt::storage::collection;

/**
 * @brief Creates an action of type T.
 * @tparam T Action implementation type.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to the constructor.
 * @return Thread-local action wrapping the action implementation.
 */
template<typename T, typename... Args>
auto make_action(Args&... _args) -> zpt::storage::action;

/**
 * @brief Creates a result of type T.
 * @tparam T Result implementation type.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to the constructor.
 * @return Thread-local result wrapping the result implementation.
 */
template<typename T, typename... Args>
auto make_result(Args&... _args) -> zpt::storage::result;
} // namespace zpt

template<typename T, typename... Args>
auto zpt::make_connection(Args&... _args) -> zpt::storage::connection {
    static thread_local zpt::storage::connection _to_return{ new T{ _args... } };
    return _to_return;
}

template<typename T, typename... Args>
auto zpt::make_session(Args&... _args) -> zpt::storage::session {
    return zpt::storage::session{ new T{ _args... } };
}

template<typename T, typename... Args>
auto zpt::make_database(Args&... _args) -> zpt::storage::database {
    return zpt::storage::database{ new T{ _args... } };
}

template<typename T, typename... Args>
auto zpt::make_collection(Args&... _args) -> zpt::storage::collection {
    return zpt::storage::collection{ new T{ _args... } };
}

template<typename T, typename... Args>
auto zpt::make_action(Args&... _args) -> zpt::storage::action {
    return zpt::storage::action{ new T{ _args... } };
}

template<typename T, typename... Args>
auto zpt::make_result(Args&... _args) -> zpt::storage::result {
    return zpt::storage::result{ new T{ _args... } };
}
