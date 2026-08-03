# CRUD API Example

A RESTful CRUD API using SQLite for persistence. Shows how to wire up database-backed handlers with the Zapata REST engine.

## Files

- `main.cpp` - Application with CRUD endpoints
- `config.json` - Server configuration
- `CMakeLists.txt` - Build configuration

## main.cpp

```cpp
#include <zapata/rest.h>
#include <zapata/sqlite.h>

auto main(int _argc, char* _argv[]) -> int {
    zpt::BOOT(_argc, _argv);

    // Setup SQLite (in-memory database)
    auto db_config = zpt::json{ "storage", { "sqlite", { "memory", true } } };
    auto conn = zpt::storage::make_connection<zpt::storage::sqlite::connection>(db_config);
    auto session = conn->session();
    auto db = session->database("main");

    // Create table
    db->sql("CREATE TABLE IF NOT EXISTS items ("
            "  _id varchar PRIMARY KEY,"
            "  name TEXT NOT NULL,"
            "  price REAL"
            ")");

    auto& resolver = zpt::REST_RESOLVER();

    // Register handlers — the request body carries item data
    // POST /api/items → create_handler
    // GET  /api/items → list_handler
    // GET  /api/items/{id} → get_handler
    // PUT  /api/items/{id} → update_handler
    // DELETE /api/items/{id} → delete_handler

    std::cout << "CRUD API running on port 8080" << std::endl;

    // Start transport engine and block
    zpt::TRANSPORT_ENGINE();
    zpt::DISPATCHER()->trap();

    return 0;
}
```

## config.json

```json
{
    "identity": { "id": "crud-api-uuid", "name": "crud-api" },
    "log": { "level": 6, "format": 1 },
    "load": [
        { "name": "builtin:http" },
        { "name": "builtin:sqlite" },
        { "name": "builtin:rest" },
        { "name": "builtin:upnp" },
        { "name": "crud-api", "source": "libcrud-api.so", "requires": [ "builtin:rest" ] }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
    "sqlite": { "path": "./data/app.db" },
    "upnp": { "bind": "239.192.1.2", "port": 7979 },
    "transport": { "default": "http", "limits": { "max_workers": 16 } },
    "rest": { "prefix": "/api" }
}
```

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.18)
project(crud-api CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZAPATA REQUIRED
    zapata-base zapata-parser-json zapata-events zapata-ontology
    zapata-net-transport zapata-net-http zapata-storage-sqlite
    zapata-engine-startup zapata-engine-rest zapata-engine-transport
)

add_executable(crud-api main.cpp)
target_include_directories(crud-api PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(crud-api ${ZAPATA_LIBRARIES})
```

## Storage API Patterns Used

The storage layer follows a hierarchical model:

```
connection → session → database → collection → action → result
```

**Insert:**
```cpp
auto collection = db->collection("items");
collection->add({ "_id", "1", "name", "Widget", "price", 9.99 })->execute();
collection->add({ "_id", "2", "name", "Gadget", "price", 24.99 })->execute();
```

**Find:**
```cpp
auto results = collection->find({})->execute();          // all
auto results = collection->find({ "_id", "1" })->execute();  // by id
```

**Update:**
```cpp
collection->modify({ "_id", "1" })
    ->set("name", "Widget Pro")
    ->set("price", 19.99)
    ->execute();
```

**Delete:**
```cpp
collection->remove({ "_id", "1" })->execute();
```

**Iterate results:**
```cpp
auto results = collection->find({})->execute();
for (auto&& [_, __, doc] : results->fetch()) {
    std::cout << doc("name") << " - " << doc("price") << std::endl;
}
```

## Test

```bash
# List all items
curl http://localhost:8080/api/items

# Read one item
curl http://localhost:8080/api/items/1
```

## See Also

- [SQLite Guide](../../guides/database/sqlite.md) - Storage backend details
- [REST Engine API Reference](../../api-reference/rest.md) - Handler registration
- [Storage API Reference](../../api-reference/storage.md) - Connector API
- Actual implementation: `storage/sqlite/examples/sqlite.cpp`
