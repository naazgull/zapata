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
 * @brief MongoDB storage connector implementation using the mongocxx C++ driver.
 *
 * Implements the storage abstraction layer for MongoDB using the mongocxx
 * client library. Provides the full connector hierarchy: connection, session,
 * database, collection, action, and result types.
 *
 * @see zpt::storage::connection
 * @see zpt::storage::make_connection
 */

#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <zapata/connector.h>
#include <zapata/json.h>
#include <zapata/mongodb/translate.h>

namespace zpt {
namespace storage {
namespace mongodb {
class connection;
class session;
class database;
class collection;
class action;
class result;

using mongodb_ptr = std::shared_ptr<mongocxx::client>;
using mongodb_cursor_ptr = std::shared_ptr<mongocxx::cursor>;

/** @brief RAII wrapper for mongocxx library initialization (must exist exactly once per process).
 */
class library {
  public:
    /** @brief Initializes the mongocxx driver instance. */
    library();
    /** @brief Destructor. */
    virtual ~library();

  private:
    mongocxx::instance __instance{};
};

/** @brief Returns the global mongocxx library instance (initializes on first call). */
auto init() -> library&;

/** @brief MongoDB connection implementation. */
class connection : public zpt::storage::connection::type {
  public:
    /** @brief Constructs a connection with options (host, port, user, password, db). */
    connection(zpt::json _options);
    /** @brief Destructor. */
    virtual ~connection() override = default;

    /** @brief Opens or re-opens the MongoDB connection with the given options. */
    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    /** @brief Closes the MongoDB connection. */
    virtual auto close() -> zpt::storage::connection::type* override;
    /** @brief Creates a new session from this connection. */
    virtual auto session() -> zpt::storage::session override;
    /** @brief Returns the connection configuration options. */
    virtual auto options() const -> zpt::json;

    /** @brief Returns the underlying mongocxx client handle. */
    auto mongodb() const -> mongodb_ptr;

  private:
    zpt::json __options;
    mongodb_ptr __mongodb{ nullptr };
};

/** @brief MongoDB session implementation. */
class session : public zpt::storage::session::type {
  public:
    /** @brief Constructs a session from the given MongoDB connection. */
    session(zpt::storage::mongodb::connection const& _connection);
    session(zpt::storage::mongodb::session const& _rhs) = delete;
    session(zpt::storage::mongodb::session&& _rhs) = delete;
    /** @brief Destructor. */
    virtual ~session() override = default;
    /** @brief Returns true if the underlying client is valid. */
    virtual auto is_open() const -> bool override;
    /** @brief No-op for MongoDB (transactions require replica sets). */
    virtual auto commit() -> zpt::storage::session::type* override;
    /** @brief No-op for MongoDB (transactions require replica sets). */
    virtual auto rollback() -> zpt::storage::session::type* override;
    /** @brief Not supported for MongoDB; always throws. */
    virtual auto sql(std::string const& _statement) -> zpt::storage::session::type* override;
    /** @brief Selects a database within this session. */
    virtual auto database(std::string const& _db) const -> zpt::storage::database override;

    /** @brief Returns the underlying mongocxx client handle. */
    auto mongodb() const -> mongodb_ptr;

  private:
    mongodb_ptr __mongodb{ nullptr };
};

/** @brief MongoDB database implementation. */
class database : public zpt::storage::database::type {
  public:
    /** @brief Constructs a database handle for the given database name. */
    database(zpt::storage::mongodb::session const& _session, std::string const& _db);
    database(zpt::storage::mongodb::database const& _rhs) = delete;
    database(zpt::storage::mongodb::database&& _rhs) = delete;
    /** @brief Destructor. */
    virtual ~database() override = default;
    /** @brief Not supported for MongoDB; always throws. */
    virtual auto sql(std::string const& _statement) -> zpt::storage::database::type* override;
    /** @brief Returns a collection handle for the given name. */
    virtual auto collection(std::string const& _name) const -> zpt::storage::collection override;

    /** @brief Returns the database name. */
    auto db() const -> std::string const&;
    /** @brief Returns the underlying mongocxx client handle. */
    auto mongodb() const -> mongodb_ptr;

  private:
    mongodb_ptr __mongodb{ nullptr };
    std::string __db;
};

/** @brief MongoDB collection implementation. */
class collection : public zpt::storage::collection::type {
  public:
    /** @brief Constructs a collection handle for the given collection within a database. */
    collection(zpt::storage::mongodb::database const& _database, std::string const& _collection);
    /** @brief Destructor. */
    virtual ~collection() override = default;
    /** @brief Creates an insert action for the given document. */
    virtual auto add(zpt::json _document) const -> zpt::storage::action override;
    /** @brief Creates an update action with the given filter. */
    virtual auto modify(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Creates a delete action with the given filter. */
    virtual auto remove(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Creates a replace/upsert action for the document with the given ID. */
    virtual auto replace(std::string const& _id, zpt::json _document) const
      -> zpt::storage::action override;
    /** @brief Creates a find action with the given filter. */
    virtual auto find(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Returns the total number of documents matching the filter. */
    virtual auto count(zpt::json _search = zpt::undefined) -> size_t override;

    /** @brief Returns the collection name. */
    auto coll() const -> std::string const&;
    /** @brief Returns the database name. */
    auto db() const -> std::string const&;
    /** @brief Returns the underlying mongocxx client handle. */
    auto mongodb() const -> mongodb_ptr;

  private:
    mongodb_ptr __mongodb{ nullptr };
    std::string __collection;
    std::string __db;
};

/** @brief Base class for MongoDB action operations. */
class action : public zpt::storage::action::type {
  public:
    /** @brief Constructs an action bound to the given collection. */
    action(zpt::storage::mongodb::collection const& _collection);
    /** @brief Destructor. */
    virtual ~action() override = default;

    /** @brief Returns the cursor from the last find execution. */
    auto cursor() const -> mongodb_cursor_ptr;
    /** @brief Returns the MongoDB client handle. */
    auto mongodb() const -> mongodb_ptr;

  protected:
    mongodb_ptr __mongodb{ nullptr };
    mongodb_cursor_ptr __cursor{ nullptr };
    std::string __collection;
    std::string __db;
};

/** @brief MongoDB insert action builder. */
class action_add : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs an insert action for the given document. */
    action_add(zpt::storage::mongodb::collection const& _collection, zpt::json _document);
    /** @brief Destructor. */
    virtual ~action_add() override = default;
    /** @brief Queues an additional document for insertion. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; always throws. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; always throws. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; always throws. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; always throws. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries). */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the insert and returns a result with generated IDs. */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Returns the auto-generated IDs from the last insert execution. */
    auto get_generated_ids() const -> zpt::json;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __generated_ids{ nullptr };
};

/** @brief MongoDB update action builder. */
class action_modify : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs an update action with the given filter. */
    action_modify(zpt::storage::mongodb::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_modify() override = default;
    /** @brief Not applicable to update; always throws. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; always throws. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; always throws. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; always throws. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; always throws. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Adds a field assignment to the update document. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Marks a field for removal in the update document. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Merges a JSON patch document into the update. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; always throws. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries). */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the update and returns the affected document count. */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Returns the number of documents modified by the last execution. */
    auto get_affected() const -> size_t;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __filter{ nullptr };
    size_t __affected{ 0 };
};

/** @brief MongoDB delete action builder. */
class action_remove : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs a delete action targeting documents that match the given filter. */
    action_remove(zpt::storage::mongodb::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_remove() override = default;
    /** @brief Not applicable to delete; always throws. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; always throws. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; always throws. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; always throws. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries). */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the delete and returns the affected document count. */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Returns the number of documents deleted by the last execution. */
    auto get_affected() const -> size_t;

  private:
    zpt::json __filter{ nullptr };
    size_t __affected{ 0 };
};

/** @brief MongoDB replace/upsert action builder. */
class action_replace : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs a replace action for the document with the given ID. */
    action_replace(zpt::storage::mongodb::collection const& _collection,
                   std::string _id,
                   zpt::json _document);
    /** @brief Destructor. */
    virtual ~action_replace() override = default;
    /** @brief Not applicable to replace; always throws. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; always throws. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; always throws. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Updates the replacement document and target ID. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; always throws. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries). */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the replace/upsert and returns the result. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    std::string __id;
    zpt::json __underlying{ nullptr };
};

/** @brief MongoDB find action builder with filtering, sorting, and pagination. */
class action_find : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs a find action that returns all documents in the collection. */
    action_find(zpt::storage::mongodb::collection const& _collection);
    /** @brief Constructs a find action with an initial filter. */
    action_find(zpt::storage::mongodb::collection const& _collection, zpt::json _search);
    /** @brief Destructor. */
    virtual ~action_find() override = default;
    /** @brief Not applicable to find; always throws. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; always throws. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; always throws. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; always throws. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; always throws. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Adds a sort field and direction to the query. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Restricts the returned fields to the given list. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Sets the number of documents to skip. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Sets the maximum number of documents to return. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries). */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the find query and returns the result cursor. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __filter{ nullptr };
    zpt::json __fields{ nullptr };
    zpt::json __suffix{ nullptr };
};

/** @brief MongoDB query result set. */
class result : public zpt::storage::result::type {
  public:
    /** @brief Constructs a result from a generic action (cursor and client only). */
    result(zpt::storage::mongodb::action& _action);
    /** @brief Constructs a result from an insert action, capturing generated IDs. */
    result(zpt::storage::mongodb::action_add& _action);
    /** @brief Constructs a result from an update action, capturing affected count. */
    result(zpt::storage::mongodb::action_modify& _action);
    /** @brief Constructs a result from a delete action, capturing affected count. */
    result(zpt::storage::mongodb::action_remove& _action);
    /** @brief Constructs a result from a replace action. */
    result(zpt::storage::mongodb::action_replace& _action);
    /** @brief Constructs a result from a find action, holding the cursor. */
    result(zpt::storage::mongodb::action_find& _action);
    /** @brief Destructor. */
    virtual ~result() override = default;
    /** @brief Fetches up to @p _amount documents as a JSON array (0 = all). */
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    /** @brief Returns the auto-generated ID(s) from the last insert. */
    virtual auto generated_id() -> zpt::json override;
    /** @brief Returns the number of documents in the result or affected by the operation. */
    virtual auto count() const -> size_t override;
    /** @brief Returns the HTTP-style status code reflecting the operation outcome. */
    virtual auto status() const -> zpt::status override;
    /** @brief Returns a human-readable message describing the operation outcome. */
    virtual auto message() const -> std::string override;
    /** @brief Serializes the result state to a JSON representation. */
    virtual auto to_json() const -> zpt::json override;

  private:
    mongodb_cursor_ptr __cursor{ nullptr };
    zpt::json __generated_ids{ nullptr };
    size_t __affected{ 0 };
    bool __is_cursor_result{ false };
};

} // namespace mongodb
} // namespace storage
} // namespace zpt
