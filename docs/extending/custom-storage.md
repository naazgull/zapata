# Writing Database Connectors

Implement a custom storage backend by subclassing the connector hierarchy.

## Overview

Storage connectors follow a layered pattern using inner `::type` abstract classes:

```
connection::type → session::type → database::type → collection::type → action::type → result::type
```

Each layer has an outer wrapper class (e.g., `zpt::storage::connection`) that holds a `shared_ptr` to the inner implementation. You implement the inner `::type` classes and use the factory functions (`zpt::make_connection`, etc.) to create wrapped instances.

## Step 1: Implement the Connection

```cpp
#include <zapata/connector.h>

namespace mydb {

class connection : public zpt::storage::connection::type {
  public:
    explicit connection(zpt::json _options);
    virtual ~connection() override = default;

    auto open(zpt::json _options) -> zpt::storage::connection::type* override {
        // Connect to your database using _options
        return this;
    }

    auto close() -> zpt::storage::connection::type* override {
        // Close the connection
        return this;
    }

    auto session() -> zpt::storage::session override {
        // Return a new session backed by this connection
        return zpt::make_session<session>(*this);
    }
};

} // namespace mydb
```

## Step 2: Implement the Session

```cpp
namespace mydb {

class session : public zpt::storage::session::type {
  public:
    explicit session(connection const& _connection);
    virtual ~session() override = default;

    auto is_open() const -> bool override {
        // Check if session is open
        return true;
    }

    auto sql(std::string const& _statement) -> zpt::storage::result override {
        // Execute raw SQL and return a result wrapper
        return zpt::make_result<result>(/* ... */);
    }

    auto commit() -> zpt::storage::session::type* override {
        // Commit transaction
        return this;
    }

    auto rollback() -> zpt::storage::session::type* override {
        // Rollback transaction
        return this;
    }

    auto database(std::string const& _db) const -> zpt::storage::database override {
        return zpt::make_database<database>(*this, _db);
    }
};

} // namespace mydb
```

## Step 3: Implement Database, Collection, Action, Result

```cpp
namespace mydb {

class database : public zpt::storage::database::type {
  public:
    database(session const& _session, std::string const& _name);
    virtual ~database() override = default;

    auto sql(std::string const& _statement) -> zpt::storage::result override;
    auto collection(std::string const& _name) const -> zpt::storage::collection override {
        return zpt::make_collection<collection>(*this, _name);
    }
};

class collection : public zpt::storage::collection::type {
  public:
    collection(database const& _db, std::string const& _name);
    virtual ~collection() override = default;

    auto add(zpt::json _document) const -> zpt::storage::action override;
    auto modify(zpt::json _search) const -> zpt::storage::action override;
    auto remove(zpt::json _search) const -> zpt::storage::action override;
    auto replace(std::string const& _id, zpt::json _document) const -> zpt::storage::action override;
    auto find(zpt::json _search) const -> zpt::storage::action override;
    auto count(zpt::json _search = zpt::undefined) -> size_t override;
};

class action : public zpt::storage::action::type {
  public:
    virtual ~action() override = default;

    auto add(zpt::json _document) -> zpt::storage::action::type* override;
    auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    auto replace(std::string const& _id, zpt::json _document) -> zpt::storage::action::type* override;
    auto find(zpt::json _search) -> zpt::storage::action::type* override;
    auto set(std::string const& _attribute, zpt::json _value) -> zpt::storage::action::type* override;
    auto unset(std::string const& _attribute) -> zpt::storage::action::type* override;
    auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    auto sort(std::string const& _attribute, bool asc = true) -> zpt::storage::action::type* override;
    auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    auto offset(size_t _rows) -> zpt::storage::action::type* override;
    auto limit(size_t _number) -> zpt::storage::action::type* override;
    auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    auto execute() -> zpt::storage::result override;
};

class result : public zpt::storage::result::type {
  public:
    virtual ~result() override = default;

    auto fetch(size_t _amount = 0) -> zpt::json override;
    auto generated_id() -> zpt::json override;
    auto count() const -> size_t override;
    auto status() const -> zpt::status override;
    auto message() const -> std::string override;
    auto to_json() const -> zpt::json override;
};

} // namespace mydb
```

## Step 4: Use Your Connector

Factory functions are in the `zpt` namespace and create wrapped instances:

```cpp
auto conn = zpt::make_connection<mydb::connection>(config);
conn->open(config);
auto session = conn->session();
auto db = session->database("main");
auto coll = db->collection("items");

// Fluent query building
auto results = coll->find({ "status", "active" })
    ->fields({ "name", "email" })
    ->sort("created_at", false)
    ->limit(10)
    ->execute();
auto docs = results->fetch();
```

## See Also

- [Storage API Reference](../api-reference/storage.md) - Connector interface details
- [Database Overview](../guides/database/overview.md) - Storage concepts
- [Component Architecture](../architecture/components.md) - Module dependencies
- SQLite connector source: `storage/sqlite/include/zapata/sqlite/connector.h`
