# SQLite Integration

Using SQLite as a storage backend in Zapata.

## Prerequisites

Install SQLite3 development library:

```bash
# Debian/Ubuntu
sudo apt install libsqlite3-dev

# Arch Linux
sudo pacman -S sqlite
```

Link against `zapata-storage-sqlite` in your CMake:

```cmake
pkg_check_modules(ZAPATA REQUIRED zapata-storage-sqlite)
```

## Setup

```cpp
#include <zapata/sqlite.h>
```

## Creating a Connection

```cpp
// In-memory database
auto config = zpt::json{ "storage", { "sqlite", { "memory", true } } };

// File-based database
auto config = zpt::json{ "storage", { "sqlite", { "path", "./data/app.db" } } };

auto conn = zpt::storage::make_connection<zpt::storage::sqlite::connection>(config);
auto session = conn->session();
auto db = session->database("main");
```

The database file is created if it doesn't exist. The connection uses `sqlite3_open_v2` internally with read-write-create flags.

## Database Bootstrapping

Create tables and initial schema:

```cpp
// Execute raw SQL
db->sql("CREATE TABLE IF NOT EXISTS users ("
        "  _id varchar PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  email TEXT UNIQUE,"
        "  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")");
```

## CRUD Operations

```cpp
auto users = db->collection("users");

// Insert a single document
users->add({ "_id", "1", "name", "Alice", "email", "alice@example.com" })->execute();

// Insert multiple documents at once
users->add({ "_id", "1", "name", "Alice" })
     ->add({ "_id", "2", "name", "Bob" })
     ->add({ "_id", "3", "name", "Charlie" })
     ->execute();

// Find all records
auto results = users->find({})->execute();
for (auto&& [_, __, row] : results->fetch()) {
    std::cout << row["email"] << std::endl;
}

// Find by field
auto results = users->find({ "_id", "1" })->execute();

// Update using modify with SQL-like syntax
users->modify({ "_id", "1" })
    ->set("email", "new@example.com")
    ->execute();

// Update with parameter binding
users->modify("_id = :id")
    ->set("email", "updated@example.com")
    ->bind({ "id", "1" })
    ->execute();

// Replace by ID
users->replace("1", { "_id", "1", "name", "Alice Updated", "email", "new@example.com" })
    ->execute();

// Delete
users->remove({ "_id", "1" })->execute();

// Delete with parameter binding
users->remove("_id = :id")
    ->bind({ "id", "1" })
    ->execute();

// Delete all
users->remove({})->execute();

// Count records
auto count = users->count();
```

## Configuration

Storage configuration is passed directly to `make_connection`, not through the main JSON config file:

```cpp
// File-based database
auto config = zpt::json{ "path", "./data/app.db" };
auto conn = zpt::storage::make_connection<zpt::storage::sqlite::connection>(config);

// In-memory database
auto config = zpt::json{ "memory", true };
auto conn = zpt::storage::make_connection<zpt::storage::sqlite::connection>(config);
```

| Key | Type | Description |
|-----|------|-------------|
| `path` | string | Path to SQLite database file |
| `memory` | bool | Use in-memory database (default: `false`) |

## See Also

- [Database Overview](overview.md) - Storage abstraction concepts
- [Storage API Reference](../../api-reference/storage.md) - Connector API details
- [MySQL Guide](mysql.md) - Alternative backend
