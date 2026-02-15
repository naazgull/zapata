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
    connection(zpt::json _config) {
        // Connect to your database using config
    }

    auto open() -> connection& override {
        // Open the physical connection
        return *this;
    }

    auto close() -> connection& override {
        // Close the connection
        return *this;
    }
};

} // namespace mydb
```

## Step 2: Implement the Session

```cpp
namespace mydb {

class session : public zpt::storage::session {
public:
    session(std::shared_ptr<connection> _conn)
      : __connection{_conn} {}

    auto begin() -> session& override { /* start transaction */ return *this; }
    auto commit() -> session& override { /* commit */ return *this; }
    auto rollback() -> session& override { /* rollback */ return *this; }

private:
    std::shared_ptr<connection> __connection;
};

} // namespace mydb
```

## Step 3: Implement Database, Collection, Action, Result

Continue the pattern for each layer:

```cpp
namespace mydb {

class database : public zpt::storage::database {
public:
    database(std::shared_ptr<session> _session, std::string const& _name);
    auto collection(std::string const& _name) -> std::shared_ptr<zpt::storage::collection> override;
};

class collection : public zpt::storage::collection {
public:
    auto find(zpt::json _query) -> std::shared_ptr<zpt::storage::action> override;
    auto insert(zpt::json _document) -> std::shared_ptr<zpt::storage::action> override;
    auto update(zpt::json _query, zpt::json _document) -> std::shared_ptr<zpt::storage::action> override;
    auto remove(zpt::json _query) -> std::shared_ptr<zpt::storage::action> override;
};

class action : public zpt::storage::action {
public:
    auto sort(std::string const& _field) -> std::shared_ptr<zpt::storage::action> override;
    auto limit(size_t _n) -> std::shared_ptr<zpt::storage::action> override;
    auto execute() -> std::shared_ptr<zpt::storage::result> override;
};

class result : public zpt::storage::result {
public:
    auto next() -> bool override;
    auto current() -> zpt::json override;
};

} // namespace mydb
```

## Step 4: Register Factory Functions

```cpp
auto conn = zpt::storage::make_connection<mydb::connection>(config);
auto session = zpt::storage::make_session(conn);
auto db = zpt::storage::make_database(session, "mydb");
```

## See Also

- [Storage API Reference](../api-reference/storage.md) - Connector interface details
- [Database Overview](../guides/database/overview.md) - Storage concepts
- [Component Architecture](../architecture/components.md) - Module dependencies
