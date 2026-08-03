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
auto session = conn->session();
auto db = session->database("mydb");
auto coll = db->collection("users");
```

## Query Builder Pattern

The `action` class provides a fluent interface for building queries:

```cpp
auto users = db->collection("users");

// SELECT * FROM users WHERE age > 21 ORDER BY name LIMIT 10
auto results = users
    ->find({ "age", { "$gt", 21 } })
    ->sort("name")
    ->limit(10)
    ->execute();

// Iterate results
for (auto&& [_, __, row] : results->fetch()) {
    auto name = std::string(row["name"]);
    auto age = int(row["age"]);
}
```

## CRUD Operations

```cpp
auto users = db->collection("users");

// Create (insert)
users->add({ "name", "Alice", "age", 30 })->execute();

// Read (find)
auto result = users->find({ "id", 1 })->execute();

// Update
users->modify({ "id", 1 })->set("age", 31)->execute();

// Delete
users->remove({ "id", 1 })->execute();
```

### Inserting Multiple Documents

```cpp
// Add multiple documents in one query
users->add({ "name", "Alice" })
     ->add({ "name", "Bob" })
     ->add({ "name", "Charlie" })
     ->execute();
```

### Raw SQL

```cpp
// Execute raw SQL on session or database
session->sql("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)");
db->sql("CREATE INDEX idx_users_name ON users(name)");
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
