# Writing Database Connectors

Implement a custom storage backend by subclassing the connector hierarchy.

## Overview

Storage connectors follow a layered pattern:

```
connection → session → database → collection → action → result
```

You must implement each layer for your database.

## Step 1: Implement the Connection

```cpp
#include <zapata/connector.h>

namespace mydb {

class connection : public zpt::storage::connection {
  public:
    using type = connection;

    connection() = default;

    auto open(zpt::json _options) -> type* override {
        // Connect to your database using _options
        // Return this on success
        return this;
    }

    auto close() -> type* override {
        // Close the connection
        return this;
    }

    auto session() -> zpt::storage::session override {
        // Return a new session
        return zpt::make_session<session>(std::make_shared<connection>(*this));
    }
};

} // namespace mydb
```

## Step 2: Implement the Session

```cpp
namespace mydb {

class session : public zpt::storage::session {
  public:
    using type = session;

    session(std::shared_ptr<connection> _conn)
      : __connection{_conn} {}

    auto is_open() const -> bool override {
        // Check if session is open
        return true;
    }

    auto sql(std::string const& _statement) -> type* override {
        // Execute raw SQL
        return this;
    }

    auto commit() -> type* override {
        // Commit transaction
        return this;
    }

    auto rollback() -> type* override {
        // Rollback transaction
        return this;
    }

    auto database(std::string const& _db) const -> zpt::storage::database override {
        return zpt::make_database<zpt::storage::database>(
            std::make_shared<database>(std::make_shared<session>(*this), _db));
    }

  private:
    std::shared_ptr<connection> __connection;
};

} // namespace mydb
```

## Step 3: Implement Database, Collection, Action, Result

```cpp
namespace mydb {

class database : public zpt::storage::database {
  public:
    using type = database;

    database(std::shared_ptr<session> _session, std::string const& _name);

    auto sql(std::string const& _statement) -> type* override;
    auto collection(std::string const& _name) const -> zpt::storage::collection override {
        return zpt::make_collection<collection>(
            std::make_shared<collection>(std::make_shared<database>(*this), _name));
    }
};

class collection : public zpt::storage::collection {
  public:
    using type = collection;

    auto add(zpt::json _document) const -> zpt::storage::action override;
    auto modify(zpt::json _search) const -> zpt::storage::action override;
    auto remove(zpt::json _search) const -> zpt::storage::action override;
    auto replace(std::string const& _id, zpt::json _document) const -> zpt::storage::action override;
    auto find(zpt::json _search) const -> zpt::storage::action override;
    auto count() -> size_t override;
};

class action : public zpt::storage::action {
  public:
    using type = action;

    auto add(zpt::json _document) -> type* override;
    auto modify(zpt::json _search) -> type* override;
    auto remove(zpt::json _search) -> type* override;
    auto replace(std::string const& _id, zpt::json _document) -> type* override;
    auto find(zpt::json _search) -> type* override;
    auto set(std::string const& _attribute, zpt::json _value) -> type* override;
    auto unset(std::string const& _attribute) -> type* override;
    auto patch(zpt::json _document) -> type* override;
    auto sort(std::string const& _attribute, bool asc = true) -> type* override;
    auto fields(zpt::json _fields) -> type* override;
    auto offset(size_t _rows) -> type* override;
    auto limit(size_t _number) -> type* override;
    auto bind(zpt::json _map) -> type* override;
    auto execute() -> zpt::storage::result override;
};

class result : public zpt::storage::result {
  public:
    using type = result;

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

```cpp
auto conn = zpt::storage::make_connection<mydb::connection>(config);
conn->open(config);
auto session = conn->session();
auto db = session->database("main");
auto coll = db->collection("items");
```

## See Also

- [Storage API Reference](../api-reference/storage.md) - Connector interface details
- [Database Overview](../guides/database/overview.md) - Storage concepts
- [Component Architecture](../architecture/components.md) - Module dependencies
- SQLite connector source: `storage/sqlite/include/zapata/sqlite.h`
