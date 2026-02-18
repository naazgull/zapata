# CRUD API Example

A RESTful CRUD API using SQLite for persistence.

## Files

- `main.cpp` - Application with CRUD endpoints
- `CMakeLists.txt` - Build configuration

## main.cpp

```cpp
#include <zapata/rest.h>
#include <zapata/sqlite.h>

auto main(int _argc, char* _argv[]) -> int {
    zpt::BOOT(_argc, _argv);
    auto& boot = zpt::BOOT_ENGINE();

    // Setup SQLite
    auto db_config = zpt::json{ "path", "./data.db" };
    auto conn = zpt::storage::make_connection<zpt::storage::sqlite::connection>(db_config);
    auto session = zpt::storage::make_session(conn);
    auto db = zpt::storage::make_database(session, "main");

    // Create table
    db->execute(
        "CREATE TABLE IF NOT EXISTS items ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL,"
        "  price REAL"
        ")"
    );

    auto items = zpt::storage::make_collection(db, "items");

    // LIST: GET /api/items
    boot.add_handler(zpt::Get, "/api/items",
        [&items](zpt::performative, zpt::json, zpt::json) -> zpt::json {
            auto results = items->find({})->execute();
            auto list = zpt::json::array();
            for (auto row : results) { list << row; }
            return { "status", 200, "body", list };
        });

    // CREATE: POST /api/items
    boot.add_handler(zpt::Post, "/api/items",
        [&items](zpt::performative, zpt::json _envelope, zpt::json) -> zpt::json {
            items->insert(_envelope["body"]);
            return { "status", 201, "body", _envelope["body"] };
        });

    // READ: GET /api/items/{id}
    boot.add_handler(zpt::Get, "/api/items/{id}",
        [&items](zpt::performative, zpt::json _envelope, zpt::json) -> zpt::json {
            auto id = int(_envelope["params"]["id"]);
            auto results = items->find({ "id", id })->execute();
            if (!results->next()) {
                return { "status", 404, "body", { "error", "Not found" } };
            }
            return { "status", 200, "body", results->current() };
        });

    // UPDATE: PUT /api/items/{id}
    boot.add_handler(zpt::Put, "/api/items/{id}",
        [&items](zpt::performative, zpt::json _envelope, zpt::json) -> zpt::json {
            auto id = int(_envelope["params"]["id"]);
            items->update({ "id", id }, _envelope["body"]);
            return { "status", 200, "body", _envelope["body"] };
        });

    // DELETE: DELETE /api/items/{id}
    boot.add_handler(zpt::Delete, "/api/items/{id}",
        [&items](zpt::performative, zpt::json _envelope, zpt::json) -> zpt::json {
            auto id = int(_envelope["params"]["id"]);
            items->remove({ "id", id });
            return { "status", 204 };
        });

    boot.start();
    return 0;
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

## Test

```bash
# Create
curl -X POST http://localhost:8080/api/items \
     -H "Content-Type: application/json" \
     -d '{"name": "Widget", "price": 9.99}'

# List
curl http://localhost:8080/api/items

# Read
curl http://localhost:8080/api/items/1

# Update
curl -X PUT http://localhost:8080/api/items/1 \
     -H "Content-Type: application/json" \
     -d '{"name": "Widget Pro", "price": 19.99}'

# Delete
curl -X DELETE http://localhost:8080/api/items/1
```
