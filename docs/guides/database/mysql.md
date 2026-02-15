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
    "host", "localhost",
    "port", 3306,
    "database", "myapp",
    "user", "root",
    "password", ""
};

auto conn = zpt::storage::make_connection<zpt::storage::mysqlx::connection>(config);
```

## Executing Queries

```cpp
auto session = zpt::storage::make_session(conn);
auto db = zpt::storage::make_database(session, "myapp");
auto users = zpt::storage::make_collection(db, "users");

// Insert
users->insert({
    "name", "Bob",
    "email", "bob@example.com"
});

// Find
auto results = users->find({ "name", "Bob" })->execute();
for (auto row : results) {
    std::cout << row["email"] << std::endl;
}

// Update
users->update(
    { "name", "Bob" },
    { "email", "newemail@example.com" }
);

// Delete
users->remove({ "name", "Bob" });
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

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `host` | string | `"localhost"` | MySQL server hostname |
| `port` | int | `3306` | MySQL server port |
| `database` | string | — | Database name |
| `user` | string | — | Authentication user |
| `password` | string | — | Authentication password |

```json
{
    "storage": {
        "mysql": {
            "host": "localhost",
            "port": 3306,
            "database": "myapp",
            "user": "root",
            "password": ""
        }
    }
}
```

## See Also

- [Database Overview](overview.md) - Storage abstraction concepts
- [Storage API Reference](../../api-reference/storage.md) - Connector API details
- [SQLite Guide](sqlite.md) - Alternative backend
