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
    /** @brief Initializes the mongocxx driver instance.
     * @return void (constructors implicitly initialize the object). */
    library();
    /** @brief Destroys the library instance.
     * @return void (destructors implicitly clean up the object). */
    virtual ~library();

  private:
    mongocxx::instance __instance{};
};

/** @brief Returns the global mongocxx library instance (initializes on first call).
 * @return MongoDB library reference. */
auto init() -> library&;

/** @brief MongoDB connection implementation. */
class connection : public zpt::storage::connection::type {
  public:
    /** @brief Constructs a connection with options (host, port, user, password, db).
     * @param _options Connection options JSON.
     * @return void (constructors implicitly initialize the object). */
    connection(zpt::json _options);
    /** @brief Destroys the connection.
     * @return void (destructors implicitly clean up the object). */
    virtual ~connection() override = default;

    /** @brief Opens or re-opens the MongoDB connection with the given options.
     * @param _options Connection options (host, port, user, password, db).
     * @return Pointer to this connection type. */
    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    /** @brief Closes the MongoDB connection.
     * @return Pointer to this connection type. */
    virtual auto close() -> zpt::storage::connection::type* override;
    /** @brief Creates a new session from this connection.
     * @return New session. */
    virtual auto session() -> zpt::storage::session override;
    /** @brief Returns the connection configuration options.
     * @return Configuration JSON object. */
    virtual auto options() const -> zpt::json;

    /** @brief Returns the underlying mongocxx client handle.
     * @return Shared pointer to mongocxx::client. */
    auto mongodb() const -> mongodb_ptr;

  private:
    zpt::json __options;
    mongodb_ptr __mongodb{ nullptr };
};

/** @brief MongoDB session implementation. */
class session : public zpt::storage::session::type {
  public:
    /** @brief Constructs a session from the given MongoDB connection.
     * @param _connection Connection to create session from.
     * @return void (constructors implicitly initialize the object). */
    session(zpt::storage::mongodb::connection const& _connection);
    session(zpt::storage::mongodb::session const& _rhs) = delete;
    session(zpt::storage::mongodb::session&& _rhs) = delete;
    /** @brief Destroys the session.
     * @return void (destructors implicitly clean up the object). */
    virtual ~session() override = default;
    /** @brief Returns true if the underlying client is valid.
     * @return True if client is valid. */
    virtual auto is_open() const -> bool override;
    /** @brief No-op for MongoDB (transactions require replica sets).
     * @return Pointer to this session type. */
    virtual auto commit() -> zpt::storage::session::type* override;
    /** @brief No-op for MongoDB (transactions require replica sets).
     * @return Pointer to this session type. */
    virtual auto rollback() -> zpt::storage::session::type* override;
    /** @brief Not supported for MongoDB; always throws.
     * @param _statement Unused.
     * @return Throws exception. */
    virtual auto sql(std::string const& _statement) -> zpt::storage::result override;
    /** @brief Selects a database within this session.
     * @param _db Database name.
     * @return Database handle. */
    virtual auto database(std::string const& _db) const -> zpt::storage::database override;

    /** @brief Returns the underlying mongocxx client handle.
     * @return Shared pointer to mongocxx::client. */
    auto mongodb() const -> mongodb_ptr;

  private:
    mongodb_ptr __mongodb{ nullptr };
};

/** @brief MongoDB database implementation. */
class database : public zpt::storage::database::type {
  public:
    /** @brief Constructs a database handle for the given database name.
     * @param _session Session to create database from.
     * @param _db Database name.
     * @return void (constructors implicitly initialize the object). */
    database(zpt::storage::mongodb::session const& _session, std::string const& _db);
    database(zpt::storage::mongodb::database const& _rhs) = delete;
    database(zpt::storage::mongodb::database&& _rhs) = delete;
    /** @brief Destroys the database handle.
     * @return void (destructors implicitly clean up the object). */
    virtual ~database() override = default;
    /** @brief Not supported for MongoDB; always throws.
     * @param _statement Unused.
     * @return Throws exception. */
    virtual auto sql(std::string const& _statement) -> zpt::storage::result override;
    /** @brief Returns a collection handle for the given name.
     * @param _name Collection name.
     * @return Collection handle. */
    virtual auto collection(std::string const& _name) const -> zpt::storage::collection override;
    /** @brief Backs up the database to the given path.
     * @param _path The path to the file that will hold the backup.
     * @return The size of the backup, in bytes. */
    virtual auto backup(std::filesystem::path const& _path) const -> size_t override;
    /** @brief Loads the database content from the given path.
     * @param _path The path to the file that holds the backup.
     * @return The size of the backup, in bytes. */
    virtual auto restore(std::filesystem::path const& _path) const -> size_t override;

    /** @brief Returns the database name.
     * @return Database name. */
    auto db() const -> std::string const&;
    /** @brief Returns the underlying mongocxx client handle.
     * @return Shared pointer to mongocxx::client. */
    auto mongodb() const -> mongodb_ptr;

  private:
    mongodb_ptr __mongodb{ nullptr };
    std::string __db;
};

/** @brief MongoDB collection implementation. */
class collection : public zpt::storage::collection::type {
  public:
    /** @brief Constructs a collection handle for the given collection within a database.
     * @param _database Database containing the collection.
     * @param _collection Collection name.
     * @return void (constructors implicitly initialize the object). */
    collection(zpt::storage::mongodb::database const& _database, std::string const& _collection);
    /** @brief Destroys the collection handle.
     * @return void (destructors implicitly clean up the object). */
    virtual ~collection() override = default;
    /** @brief Creates an insert action for the given document.
     * @param _document Document to insert.
     * @return Action for the insert. */
    virtual auto add(zpt::json _document) const -> zpt::storage::action override;
    /** @brief Creates an update action with the given filter.
     * @param _search Search criteria.
     * @return Action for the update. */
    virtual auto modify(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Creates a delete action with the given filter.
     * @param _search Search criteria.
     * @return Action for the delete. */
    virtual auto remove(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Creates a replace/upsert action for the document with the given ID.
     * @param _id Document ID to replace.
     * @param _document New document.
     * @return Action for the replace. */
    virtual auto replace(std::string const& _id, zpt::json _document) const
      -> zpt::storage::action override;
    /** @brief Creates a find action with the given filter.
     * @param _search Search criteria.
     * @return Action for the find. */
    virtual auto find(zpt::json _search) const -> zpt::storage::action override;
    /** @brief Returns the total number of documents matching the filter.
     * @param _search Optional search criteria.
     * @return Document count. */
    virtual auto count(zpt::json _search = zpt::undefined) -> size_t override;

    /** @brief Returns the collection name.
     * @return Collection name. */
    auto coll() const -> std::string const&;
    /** @brief Returns the database name.
     * @return Database name. */
    auto db() const -> std::string const&;
    /** @brief Returns the underlying mongocxx client handle.
     * @return Shared pointer to mongocxx::client. */
    auto mongodb() const -> mongodb_ptr;

  private:
    mongodb_ptr __mongodb{ nullptr };
    std::string __collection;
    std::string __db;
};

/** @brief Base class for MongoDB action operations. */
class action : public zpt::storage::action::type {
  public:
    /** @brief Constructs an action bound to the given collection.
     * @param _collection Collection to perform action on.
     * @return void (constructors implicitly initialize the object). */
    action(zpt::storage::mongodb::collection const& _collection);
    /** @brief Destroys the action.
     * @return void (destructors implicitly clean up the object). */
    virtual ~action() override = default;

    /** @brief Returns the cursor from the last find execution.
     * @return Shared pointer to mongocxx::cursor. */
    auto cursor() const -> mongodb_cursor_ptr;
    /** @brief Returns the MongoDB client handle.
     * @return Shared pointer to mongocxx::client. */
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
    /** @brief Constructs an insert action for the given document.
     * @param _collection Collection to insert into.
     * @param _document Document to insert.
     * @return void (constructors implicitly initialize the object). */
    action_add(zpt::storage::mongodb::collection const& _collection, zpt::json _document);
    /** @brief Destroys the insert action.
     * @return void (destructors implicitly clean up the object). */
    virtual ~action_add() override = default;
    /** @brief Queues an additional document for insertion.
     * @param _document Document to add.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _id Unused.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _attribute Unused.
     * @param _value Unused.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _attribute Unused.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _attribute Unused.
     * @param asc Unused.
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _fields Unused.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _rows Unused.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to insert; returns this action unchanged.
     * @param _number Unused.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries).
     * @param _map Unused.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the insert and returns a result with generated IDs.
     * @return Query result. */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Returns the auto-generated IDs from the last insert execution.
     * @return JSON array of generated IDs. */
    auto get_generated_ids() const -> zpt::json;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __generated_ids{ nullptr };
};

/** @brief MongoDB update action builder. */
class action_modify : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs an update action with the given filter.
     * @param _collection Collection to update.
     * @param _search Update filter.
     * @return void (constructors implicitly initialize the object). */
    action_modify(zpt::storage::mongodb::collection const& _collection, zpt::json _search);
    /** @brief Destroys the update action.
     * @return void (destructors implicitly clean up the object). */
    virtual ~action_modify() override = default;
    /** @brief Not applicable to update; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Refines the filter with additional search criteria.
     * @param _search Additional search criteria.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged.
     * @param _id Unused.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Adds a field assignment to the update document.
     * @param _attribute Field name.
     * @param _value Value to set.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Marks a field for removal in the update document.
     * @param _attribute Field name to remove.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Merges a JSON patch document into the update.
     * @param _document Patch document with field/value pairs.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged.
     * @param _attribute Unused.
     * @param asc Unused.
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged.
     * @param _fields Unused.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged.
     * @param _rows Unused.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to update; returns this action unchanged.
     * @param _number Unused.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries).
     * @param _map Unused.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the update and returns the affected document count.
     * @return Query result with affected count. */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Returns the number of documents modified by the last execution.
     * @return Document count. */
    auto get_affected() const -> size_t;

  private:
    zpt::json __underlying{ nullptr };
    zpt::json __filter{ nullptr };
    size_t __affected{ 0 };
};

/** @brief MongoDB delete action builder. */
class action_remove : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs a delete action targeting documents that match the given filter.
     * @param _collection Collection to delete from.
     * @param _search Delete filter.
     * @return void (constructors implicitly initialize the object). */
    action_remove(zpt::storage::mongodb::collection const& _collection, zpt::json _search);
    /** @brief Destroys the delete action.
     * @return void (destructors implicitly clean up the object). */
    virtual ~action_remove() override = default;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Refines the filter with additional search criteria.
     * @param _search Additional search criteria.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _id Unused.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _attribute Unused.
     * @param _value Unused.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _attribute Unused.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _attribute Unused.
     * @param asc Unused.
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _fields Unused.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _rows Unused.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to delete; returns this action unchanged.
     * @param _number Unused.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries).
     * @param _map Unused.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the delete and returns the affected document count.
     * @return Query result with affected count. */
    virtual auto execute() -> zpt::storage::result override;
    /** @brief Returns the number of documents deleted by the last execution.
     * @return Document count. */
    auto get_affected() const -> size_t;

  private:
    zpt::json __filter{ nullptr };
    size_t __affected{ 0 };
};

/** @brief MongoDB replace/upsert action builder. */
class action_replace : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs a replace action for the document with the given ID.
     * @param _collection Collection to replace in.
     * @param _id Document ID to replace.
     * @param _document New document.
     * @return void (constructors implicitly initialize the object). */
    action_replace(zpt::storage::mongodb::collection const& _collection,
                   std::string _id,
                   zpt::json _document);
    /** @brief Destroys the replace action.
     * @return void (destructors implicitly clean up the object). */
    virtual ~action_replace() override = default;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Updates the replacement document and target ID.
     * @param _id Document ID to replace.
     * @param _document New document.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _attribute Unused.
     * @param _value Unused.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _attribute Unused.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _attribute Unused.
     * @param asc Unused.
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _fields Unused.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _rows Unused.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Not applicable to replace; returns this action unchanged.
     * @param _number Unused.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries).
     * @param _map Unused.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the replace/upsert and returns the result.
     * @return Query result. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    std::string __id;
    zpt::json __underlying{ nullptr };
};

/** @brief MongoDB find action builder with filtering, sorting, and pagination. */
class action_find : public zpt::storage::mongodb::action {
  public:
    /** @brief Constructs a find action that returns all documents in the collection.
     * @param _collection Collection to query.
     * @return void (constructors implicitly initialize the object). */
    action_find(zpt::storage::mongodb::collection const& _collection);
    /** @brief Constructs a find action with an initial filter.
     * @param _collection Collection to query.
     * @param _search Initial filter criteria.
     * @return void (constructors implicitly initialize the object). */
    action_find(zpt::storage::mongodb::collection const& _collection, zpt::json _search);
    /** @brief Destroys the find action.
     * @return void (destructors implicitly clean up the object). */
    virtual ~action_find() override = default;
    /** @brief Not applicable to find; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged.
     * @param _search Unused.
     * @return Pointer to this action type. */
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged.
     * @param _id Unused.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto replace(std::string const& _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    /** @brief Refines the filter with additional search criteria.
     * @param _search Additional search criteria.
     * @return Pointer to this action type. */
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged.
     * @param _attribute Unused.
     * @param _value Unused.
     * @return Pointer to this action type. */
    virtual auto set(std::string const& _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged.
     * @param _attribute Unused.
     * @return Pointer to this action type. */
    virtual auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    /** @brief Not applicable to find; returns this action unchanged.
     * @param _document Unused.
     * @return Pointer to this action type. */
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    /** @brief Adds a sort field and direction to the query.
     * @param _attribute Field name to sort by.
     * @param asc Sort direction (default true = ascending).
     * @return Pointer to this action type. */
    virtual auto sort(std::string const& _attribute, bool asc = true)
      -> zpt::storage::action::type* override;
    /** @brief Restricts the returned fields to the given list.
     * @param _fields JSON array of field names.
     * @return Pointer to this action type. */
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    /** @brief Sets the number of documents to skip.
     * @param _rows Number of documents to skip.
     * @return Pointer to this action type. */
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    /** @brief Sets the maximum number of documents to return.
     * @param _number Maximum documents to return.
     * @return Pointer to this action type. */
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    /** @brief No-op for MongoDB (no parameterized queries).
     * @param _map Unused.
     * @return Pointer to this action type. */
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    /** @brief Executes the find query and returns the result cursor.
     * @return Query result with fetched documents. */
    virtual auto execute() -> zpt::storage::result override;

  private:
    zpt::json __filter{ nullptr };
    zpt::json __fields{ nullptr };
    zpt::json __suffix{ nullptr };
};

/** @brief MongoDB query result set. */
class result : public zpt::storage::result::type {
  public:
    /** @brief Constructs a result from a cursor.
     * @param __cursor Query result cursor.
     * @return void (constructors implicitly initialize the object). */
    result(mongodb_cursor_ptr __cursor);
    /** @brief Constructs a result from a generic action (cursor and client only).
     * @param _action Action to create result from.
     * @return void (constructors implicitly initialize the object). */
    result(zpt::storage::mongodb::action& _action);
    /** @brief Constructs a result from an insert action, capturing generated IDs.
     * @param _action Insert action to create result from.
     * @return void (constructors implicitly initialize the object). */
    result(zpt::storage::mongodb::action_add& _action);
    /** @brief Constructs a result from an update action, capturing affected count.
     * @param _action Update action to create result from.
     * @return void (constructors implicitly initialize the object). */
    result(zpt::storage::mongodb::action_modify& _action);
    /** @brief Constructs a result from a delete action, capturing affected count.
     * @param _action Delete action to create result from.
     * @return void (constructors implicitly initialize the object). */
    result(zpt::storage::mongodb::action_remove& _action);
    /** @brief Constructs a result from a replace action.
     * @param _action Replace action to create result from.
     * @return void (constructors implicitly initialize the object). */
    result(zpt::storage::mongodb::action_replace& _action);
    /** @brief Constructs a result from a find action, holding the cursor.
     * @param _action Find action to create result from.
     * @return void (constructors implicitly initialize the object). */
    result(zpt::storage::mongodb::action_find& _action);
    /** @brief Destroys the result set.
     * @return void (destructors implicitly clean up the object). */
    virtual ~result() override = default;
    /** @brief Fetches up to @p _amount documents as a JSON array (0 = all).
     * @param _amount Maximum documents to fetch (0 = all).
     * @return JSON array of result documents. */
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    /** @brief Returns the auto-generated ID(s) from the last insert.
     * @return JSON value containing generated ID(s). */
    virtual auto generated_id() -> zpt::json override;
    /** @brief Returns the number of documents in the result or affected by the operation.
     * @return Document count. */
    virtual auto count() const -> size_t override;
    /** @brief Returns the HTTP-style status code reflecting the operation outcome.
     * @return Status code. */
    virtual auto status() const -> zpt::status override;
    /** @brief Returns a human-readable message describing the operation outcome.
     * @return Status message. */
    virtual auto message() const -> std::string override;
    /** @brief Serializes the result state to a JSON representation.
     * @return JSON representation of the result. */
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
