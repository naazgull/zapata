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
auto config = zpt::json{
    "path", "/var/lib/myapp/data.db"
};

auto conn = zpt::storage::make_connection<zpt::storage::sqlite::connection>(config);
```

The database file is created if it doesn't exist. The connection uses `sqlite3_open_v2` internally with read-write-create flags.

## Database Bootstrapping

Create tables and initial schema:

```cpp
auto session = zpt::storage::make_session(conn);
auto db = zpt::storage::make_database(session, "main");

// Execute raw SQL for schema setup
db->execute("CREATE TABLE IF NOT EXISTS users ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL,"
            "  email TEXT UNIQUE,"
            "  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
            ")");
```

## CRUD Operations

```cpp
auto users = zpt::storage::make_collection(db, "users");

// Insert
users->insert({
    "name", "Alice",
    "email", "alice@example.com"
});

// Find
auto results = users->find({ "name", "Alice" })->execute();
for (auto row : results) {
    std::cout << row["email"] << std::endl;
}

// Update
users->update(
    { "name", "Alice" },           // WHERE
    { "email", "new@example.com" } // SET
);

// Delete
users->remove({ "name", "Alice" });
```

## Configuration

| Key | Type | Description |
|-----|------|-------------|
| `path` | string | Path to SQLite database file |

```json
{
    "storage": {
        "sqlite": {
            "path": "./data/app.db"
        }
    }
}
```

## See Also

- [Database Overview](overview.md) - Storage abstraction concepts
- [Storage API Reference](../../api-reference/storage.md) - Connector API details
- [MySQL Guide](mysql.md) - Alternative backend
