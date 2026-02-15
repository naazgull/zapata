# Database Overview

Zapata provides a storage abstraction layer that works across different database backends through a unified interface.

## Connector Hierarchy

The storage system uses a layered approach:

```
connection → session → database → collection → action → result
```

| Class | Purpose |
|-------|---------|
| `connection` | Physical connection to a database server |
| `session` | Logical session within a connection |
| `database` | A specific database/schema |
| `collection` | A table or collection |
| `action` | A query builder for CRUD operations |
| `result` | Iteration over query results |

## Factory Functions

Create storage objects using factory functions:

```cpp
#include <zapata/connector.h>

auto conn = zpt::storage::make_connection<MyBackend>(config);
auto session = zpt::storage::make_session(conn);
auto db = zpt::storage::make_database(session, "mydb");
auto coll = zpt::storage::make_collection(db, "users");
```

## Query Builder Pattern

The `action` class provides a fluent interface for building queries:

```cpp
auto users = zpt::storage::make_collection(db, "users");

// SELECT * FROM users WHERE age > 21 ORDER BY name LIMIT 10
auto results = users
    ->find({ "age", { "$gt", 21 } })
    ->sort("name")
    ->limit(10)
    ->execute();

// Iterate results
for (auto row : results) {
    auto name = std::string(row["name"]);
    auto age = int(row["age"]);
}
```

## CRUD Operations

```cpp
// Create
coll->insert({ "name", "Alice", "age", 30 });

// Read
auto result = coll->find({ "id", 1 })->execute();

// Update
coll->update({ "id", 1 }, { "age", 31 });

// Delete
coll->remove({ "id", 1 });
```

## Available Backends

| Backend | Library | Header |
|---------|---------|--------|
| SQLite | `libzapata-storage-sqlite` | `<zapata/sqlite.h>` |
| MySQL | `libzapata-storage-mysqlx` | `<zapata/mysqlx.h>` |

## See Also

- [Storage API Reference](../../api-reference/storage.md) - Detailed connector API
- [SQLite Guide](sqlite.md) - SQLite-specific usage
- [MySQL Guide](mysql.md) - MySQL-specific usage
