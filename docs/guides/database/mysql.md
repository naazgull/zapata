# MySQL Integration

Using MySQL as a storage backend in Zapata.

## Prerequisites

Install the MySQL client library:

```bash
# Debian/Ubuntu
sudo apt install libmysqlclient-dev

# Arch Linux
sudo pacman -S mariadb-libs
```

Link against `zapata-storage-mysqlx` in your CMake:

```cmake
pkg_check_modules(ZAPATA REQUIRED zapata-storage-mysqlx)
```

## Setup

```cpp
#include <zapata/mysqlx.h>
```

> **Note:** Zapata uses the MySQL C API directly (migrated from the X DevAPI in a previous release).

## Creating a Connection

```cpp
auto config = zpt::json{
    "storage", { "mysqlx", {
        "host", "localhost",
        "port", 3306,
        "database", "myapp",
        "user", "root",
        "password", ""
    }}
};

auto conn = zpt::storage::make_connection<zpt::storage::mysqlx::connection>(config);
auto session = conn->session();
auto db = session->database("myapp");
```

## Database Bootstrapping

Create tables using raw SQL:

```cpp
// Create schema
session->sql("CREATE SCHEMA IF NOT EXISTS myapp");

// Create tables
db->sql("CREATE TABLE IF NOT EXISTS users ("
        "  _id varchar(36) PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  email TEXT UNIQUE"
        ")");
```

## CRUD Operations

```cpp
auto users = db->collection("users");

// Insert
users->add({ "_id", "1", "name", "Bob", "email", "bob@example.com" })->execute();

// Find
auto results = users->find("_id = \"1\"")->execute();
for (auto&& [_, __, row] : results->fetch()) {
    std::cout << row["email"] << std::endl;
}

// Update
users->modify("_id = \"1\"")
    ->set("email", "newemail@example.com")
    ->execute();

// Patch (partial update)
users->modify("_id = \"1\"")
    ->patch({ "email", "patched@example.com" })
    ->execute();

// Delete
users->remove("_id = \"1\"")->execute();

// Count
auto count = users->count();
```

## Column Type Handling

The MySQL connector maps MySQL column types to JSON types:

| MySQL Type | JSON Type |
|-----------|-----------|
| INT, BIGINT | `JSInteger` |
| FLOAT, DOUBLE, DECIMAL | `JSDouble` |
| VARCHAR, TEXT | `JSString` |
| DATETIME, TIMESTAMP | `JSDate` |
| TINYINT(1) | `JSBoolean` |
| NULL | `JSNil` |

Unrecognized column types are silently skipped with a warning.

## Configuration

Storage configuration is passed directly to `make_connection`, not through the main JSON config file:

```cpp
auto config = zpt::json{
    "host", "localhost",
    "port", 3306,
    "database", "myapp",
    "user", "root",
    "password", ""
};
auto conn = zpt::storage::make_connection<zpt::storage::mysqlx::connection>(config);
```

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `host` | string | `"localhost"` | MySQL server hostname |
| `port` | int | `3306` | MySQL server port |
| `database` | string | — | Database name |
| `user` | string | — | Authentication user |
| `password` | string | — | Authentication password |

## See Also

- [Database Overview](overview.md) - Storage abstraction concepts
- [Storage API Reference](../../api-reference/storage.md) - Connector API details
- [SQLite Guide](sqlite.md) - Alternative backend
