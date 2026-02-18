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

/** @brief Function type for custom SQL output. */
using functor = std::function<void(zpt::json, std::ostream&)>;
/** @brief Function type for string output. */
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

        /** @brief Opens the connection with given options. */
        virtual auto open(zpt::json _options) -> zpt::storage::connection::type* = 0;
        /** @brief Closes the connection. */
        virtual auto close() -> zpt::storage::connection::type* = 0;
        /** @brief Creates a new session. */
        virtual auto session() const -> zpt::storage::session = 0;
    };

    /** @brief Default constructor (null connection). */
    connection() = default;
    /** @brief Wraps a raw connection implementation. */
    connection(zpt::storage::connection::type* _underlying);
    /** @brief Copy constructor (shared semantics). */
    connection(zpt::storage::connection const& _rhs);
    /** @brief Move constructor. */
    connection(zpt::storage::connection&& _rhs);
    /** @brief Destructor. */
    virtual ~connection() = default;

    /** @brief Copy assignment. */
    auto operator=(zpt::storage::connection const& _rhs) -> zpt::storage::connection&;
    /** @brief Move assignment. */
    auto operator=(zpt::storage::connection&& _rhs) -> zpt::storage::connection&;

    /** @brief Accesses the underlying connection implementation. */
    auto operator->() -> zpt::storage::connection::type*;
    /** @brief Dereferences to the underlying connection implementation. */
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

        /** @brief Returns true if session is open. */
        virtual auto is_open() const -> bool = 0;
        /** @brief Executes raw SQL statement. */
        virtual auto sql(std::string const& _statement) -> zpt::storage::session::type* = 0;
        /** @brief Commits the current transaction. */
        virtual auto commit() -> zpt::storage::session::type* = 0;
        /** @brief Rolls back the current transaction. */
        virtual auto rollback() -> zpt::storage::session::type* = 0;
        /** @brief Gets a database by name. */
        virtual auto database(std::string const& _db) const -> zpt::storage::database = 0;
    };

    /** @brief Default constructor (null session). */
    session() = default;
    /** @brief Wraps a raw session implementation. */
    session(zpt::storage::session::type* _underlying);
    /** @brief Copy constructor (shared semantics). */
    session(zpt::storage::session const& _rhs);
    /** @brief Move constructor. */
    session(zpt::storage::session&& _rhs);
    /** @brief Destructor. */
    virtual ~session() = default;

    /** @brief Copy assignment. */
    auto operator=(zpt::storage::session const& _rhs) -> zpt::storage::session&;
    /** @brief Move assignment. */
    auto operator=(zpt::storage::session&& _rhs) -> zpt::storage::session&;

    /** @brief Accesses the underlying session implementation. */
    auto operator->() -> zpt::storage::session::type*;
    /** @brief Dereferences to the underlying session implementation. */
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

        /** @brief Executes raw SQL statement. */
        virtual auto sql(std::string const& _statement) -> zpt::storage::database::type* = 0;
        /** @brief Gets a collection/table by name. */
        virtual auto collection(std::string const& _name) const -> zpt::storage::collection = 0;
    };

    /** @brief Default constructor (null database). */
    database() = default;
    /** @brief Wraps a raw database implementation. */
    database(zpt::storage::database::type* _underlying);
    /** @brief Copy constructor (shared semantics). */
    database(zpt::storage::database const& _rhs);
    /** @brief Move constructor. */
    database(zpt::storage::database&& _rhs);
    /** @brief Destructor. */
    virtual ~database() = default;

    /** @brief Copy assignment. */
    auto operator=(zpt::storage::database const& _rhs) -> zpt::storage::database&;
    /** @brief Move assignment. */
    auto operator=(zpt::storage::database&& _rhs) -> zpt::storage::database&;

    /** @brief Accesses the underlying database implementation. */
    auto operator->() -> zpt::storage::database::type*;
    /** @brief Dereferences to the underlying database implementation. */
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

        /** @brief Creates an insert action. */
        virtual auto add(zpt::json _document) const -> zpt::storage::action = 0;
        /** @brief Creates an update action with search criteria. */
        virtual auto modify(zpt::json _search) const -> zpt::storage::action = 0;
        /** @brief Creates a delete action with search criteria. */
        virtual auto remove(zpt::json _search) const -> zpt::storage::action = 0;
        /** @brief Creates a replace action for a specific document. */
        virtual auto replace(std::string const& _id, zpt::json _document) const
          -> zpt::storage::action = 0;
        /** @brief Creates a find action with search criteria. */
        virtual auto find(zpt::json _search) const -> zpt::storage::action = 0;
        /** @brief Returns the total count of documents. */
        virtual auto count() -> size_t = 0;
    };

    /** @brief Default constructor (null collection). */
    collection() = default;
    /** @brief Wraps a raw collection implementation. */
    collection(zpt::storage::collection::type* _underlying);
    /** @brief Copy constructor (shared semantics). */
    collection(zpt::storage::collection const& _rhs);
    /** @brief Move constructor. */
    collection(zpt::storage::collection&& _rhs);
    /** @brief Destructor. */
    virtual ~collection() = default;

    /** @brief Copy assignment. */
    auto operator=(zpt::storage::collection const& _rhs) -> zpt::storage::collection&;
    /** @brief Move assignment. */
    auto operator=(zpt::storage::collection&& _rhs) -> zpt::storage::collection&;

    /** @brief Accesses the underlying collection implementation. */
    auto operator->() -> zpt::storage::collection::type*;
    /** @brief Dereferences to the underlying collection implementation. */
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

        /** @brief Adds another document to insert. */
        virtual auto add(zpt::json _document) -> zpt::storage::action::type* = 0;
        /** @brief Modifies search criteria. */
        virtual auto modify(zpt::json _search) -> zpt::storage::action::type* = 0;
        /** @brief Modifies delete criteria. */
        virtual auto remove(zpt::json _search) -> zpt::storage::action::type* = 0;
        /** @brief Sets document to replace. */
        virtual auto replace(std::string const& _id, zpt::json _document)
          -> zpt::storage::action::type* = 0;
        /** @brief Adds find criteria. */
        virtual auto find(zpt::json _search) -> zpt::storage::action::type* = 0;
        /** @brief Sets a field value for update. */
        virtual auto set(std::string const& _attribute, zpt::json _value)
          -> zpt::storage::action::type* = 0;
        /** @brief Removes a field for update. */
        virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* = 0;
        /** @brief Applies partial update document. */
        virtual auto patch(zpt::json _document) -> zpt::storage::action::type* = 0;
        /** @brief Sorts results by attribute. */
        virtual auto sort(std::string const& _attribute, bool asc = true)
          -> zpt::storage::action::type* = 0;
        /** @brief Selects specific fields to return. */
        virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* = 0;
        /** @brief Skips first N rows (pagination). */
        virtual auto offset(size_t _rows) -> zpt::storage::action::type* = 0;
        /** @brief Limits results to N rows. */
        virtual auto limit(size_t _number) -> zpt::storage::action::type* = 0;
        /** @brief Binds parameter values. */
        virtual auto bind(zpt::json _map) -> zpt::storage::action::type* = 0;
        /** @brief Executes the action and returns results. */
        virtual auto execute() -> zpt::storage::result = 0;
    };

    /** @brief Default constructor (null action). */
    action() = default;
    /** @brief Wraps a raw action implementation. */
    action(zpt::storage::action::type* _underlying);
    /** @brief Copy constructor (shared semantics). */
    action(zpt::storage::action const& _rhs);
    /** @brief Move constructor. */
    action(zpt::storage::action&& _rhs);
    /** @brief Destructor. */
    virtual ~action() = default;

    /** @brief Copy assignment. */
    auto operator=(zpt::storage::action const& _rhs) -> zpt::storage::action&;
    /** @brief Move assignment. */
    auto operator=(zpt::storage::action&& _rhs) -> zpt::storage::action&;

    /** @brief Accesses the underlying action implementation. */
    auto operator->() -> zpt::storage::action::type*;
    /** @brief Dereferences to the underlying action implementation. */
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

        /** @brief Fetches documents (0 = all). */
        virtual auto fetch(size_t _amount = 0) -> zpt::json = 0;
        /** @brief Returns generated IDs from insert. */
        virtual auto generated_id() -> zpt::json = 0;
        /** @brief Returns the number of affected/returned rows. */
        virtual auto count() const -> size_t = 0;
        /** @brief Returns the operation status code. */
        virtual auto status() const -> zpt::status = 0;
        /** @brief Returns status message. */
        virtual auto message() const -> std::string = 0;
        /** @brief Converts result to JSON. */
        virtual auto to_json() const -> zpt::json = 0;
    };

    /** @brief Default constructor (null result). */
    result() = default;
    /** @brief Wraps a raw result implementation. */
    result(zpt::storage::result::type* _underlying);
    /** @brief Copy constructor (shared semantics). */
    result(zpt::storage::result const& _rhs);
    /** @brief Move constructor. */
    result(zpt::storage::result&& _rhs);
    /** @brief Destructor. */
    virtual ~result() = default;

    /** @brief Copy assignment. */
    auto operator=(zpt::storage::result const& _rhs) -> zpt::storage::result&;
    /** @brief Move assignment. */
    auto operator=(zpt::storage::result&& _rhs) -> zpt::storage::result&;

    /** @brief Accesses the underlying result implementation. */
    auto operator->() -> zpt::storage::result::type*;
    /** @brief Dereferences to the underlying result implementation. */
    auto operator*() -> zpt::storage::result::type&;

  private:
    std::shared_ptr<zpt::storage::result::type> __underlying{ nullptr };
};

/** @brief Creates a find action with query parameters. */
auto filter_find(zpt::storage::collection& _collection, zpt::json _params) -> zpt::storage::action;
/** @brief Formats find results into a reply. */
auto reply_find(zpt::json& _reply, zpt::json _params) -> void;
/** @brief Creates a remove action with query parameters. */
auto filter_remove(zpt::storage::collection& _collection, zpt::json _params)
  -> zpt::storage::action;
/** @brief Extracts find criteria from JSON. */
auto extract_find(zpt::json _to_parse) -> std::string;
/** @brief Converts functional JSON to SQL. */
auto functional_to_sql(zpt::json _function,
                       std::ostream& _find,
                       zpt::storage::string_output _str_output) -> void;
/** @brief Converts functor name and params to SQL. */
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
 */
template<typename T, typename... Args>
auto make_session(Args&... _args) -> zpt::storage::session;

/**
 * @brief Creates a database of type T.
 */
template<typename T, typename... Args>
auto make_database(Args&... _args) -> zpt::storage::database;

/**
 * @brief Creates a collection of type T.
 */
template<typename T, typename... Args>
auto make_collection(Args&... _args) -> zpt::storage::collection;

/**
 * @brief Creates an action of type T.
 */
template<typename T, typename... Args>
auto make_action(Args&... _args) -> zpt::storage::action;

/**
 * @brief Creates a result of type T.
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
