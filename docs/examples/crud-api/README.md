# CRUD API Example

A RESTful CRUD API using SQLite for persistence. Shows how to wire up database-backed handlers with the Zapata REST engine.

## Files

- `plugin.cpp` - Plugin with CRUD handlers and database setup
- `config.json` - Server configuration
- `CMakeLists.txt` - Plugin build configuration

## plugin.cpp

```cpp
#include <zapata/rest.h>
#include <zapata/sqlite.h>
#include <zapata/startup.h>

class list_handler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto _db = zpt::GLOBAL_CONFIG()("storage")("sqlite")("session");
        auto items = _db->collection("items");
        auto results = items->find({})->execute();
        zpt::json items_list = { zpt::array };
        for (auto&& [_, __, doc] : results->fetch()) {
            items_list << doc;
        }
        this->to_send()->status(200)->body() = items_list;
        return zpt::events::finish;
    }
};

// GET  /api/items/{} → get_handler
// POST /api/items   → create_handler
// PUT  /api/items/{} → update_handler
// DELETE /api/items/{} → delete_handler
// ... (similar handlers)

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading crud-api plugin", zpt::info);

    // Setup SQLite database
    auto _config = zpt::GLOBAL_CONFIG();
    auto db_config = _config("storage");
    auto conn = zpt::storage::make_connection<zpt::storage::sqlite::connection>(db_config);
    auto session = conn->session();
    auto db = session->database("main");

    // Create table
    db->sql("CREATE TABLE IF NOT EXISTS items ("
            "  _id varchar PRIMARY KEY,"
            "  name TEXT NOT NULL,"
            "  price REAL"
            ")");

    auto _resolver = zpt::REST_RESOLVER();
    _resolver->add<list_handler>("/api/items")
             // ... register other handlers
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading crud-api plugin", zpt::info);
    auto _resolver = zpt::REST_RESOLVER();
    _resolver->remove<list_handler>("/api/items")
             // ... remove other handlers
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
        { "name": "crud-api", "source": "libcrud-api.so", "requires": [ "builtin:rest" ] }
    ],
    "resources": { "limits": { "max_heap_allocation": 0 } },
    "dispatcher": { "limits": { "max_workers": 4 } },
    "http": { "bind": "0.0.0.0", "port": 8080 },
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

add_library(crud-api SHARED plugin.cpp)
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

## Build and Run

The plugin builds as a shared library (`libcrud-api.so`). It is loaded by the Zapata host process at runtime via the `load` configuration.

```bash
mkdir build && cd build
cmake .. && make
zpt --config ../config.json
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
