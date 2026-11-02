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

auto conn = zpt::make_connection<MyBackend>(config);
auto session = conn->session();
auto db = session->database("mydb");
auto coll = db->collection("users");
```

## Query Builder Pattern

The `action` class provides a fluent interface for building queries:

```cpp
auto users = db->collection("users");

// SELECT name, email FROM users WHERE age > 21 ORDER BY name LIMIT 10
auto results = users
    ->find({ "age", { "$gt", 21 } })
    ->fields({ "name", "email" })
    ->sort("name")
    ->limit(10)
    ->execute();

// Iterate results
for (auto&& [_, __, row] : results->fetch()) {
    auto name = std::string(row("name"));
}
```

## Pagination

Pagination is driven by `filter_find` / `filter_remove`, which accept JSON params
objects containing these special keys:

| Key | Type | Effect |
|-----|------|--------|
| `page_size` | int | Maximum rows to return (`->limit(n)`) |
| `page_start_index` | int | Rows to skip, 0-based (`->offset(n)`) |
| `order_by` | string | Comma-separated sort spec; `-` prefix for descending (e.g. `"-created_at,name"`) |

All other keys in the params object are treated as `col = value` WHERE conditions.

```cpp
// Server-side: extract params from the HTTP request
auto _params = this->received()->parameters()->is_object()
    ? this->received()->parameters()->clone()
    : zpt::json::object();

// filter_find applies page_size, page_start_index, and order_by automatically
auto _result = zpt::storage::filter_find(_collection, _params) //
    ->fields({ "name", "email", "created_at" }) //
    ->execute()->fetch();

this->to_send()->status(200).body() =
    { "items", _result, "size", _collection->count() };

// Echo page_size and page_start_index back so the client can track its position
zpt::storage::reply_find(this->to_send()->body(), _params);
```

The response body will include the page metadata alongside the items:

```json
{ "items": [ ... ], "size": 142, "page_size": 20, "page_start_index": 40 }
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
