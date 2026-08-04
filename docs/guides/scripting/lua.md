# Lua Integration

Zapata provides a Lua scripting bridge for extending applications with dynamic scripting.

## Prerequisites

Install Lua:

```bash
# Debian/Ubuntu
sudo apt install liblua-dev

# Arch Linux
sudo pacman -S lua
```

Link against `zapata-bridge-lua`:

```cmake
pkg_check_modules(ZAPATA REQUIRED zapata-bridge-lua)
```

## Setup

```cpp
#include <zapata/lua.h>
```

## The Bridge Pattern

Zapata's language bridges use CRTP (Curiously Recurring Template Pattern):

```
programming::integration (abstract base)
       │
programming::bridge<C, O> (CRTP template)
       │
lua::bridge (concrete implementation)
```

The Lua bridge converts between `zpt::json` and Lua values transparently.

## Getting the Bridge Instance

There are two ways to access the Lua bridge:

**Global accessor (thread-local):**
```cpp
auto& lua = zpt::LUA_BRIDGE();
```

**Local instance (typical in a plugin or module):**
```cpp
// Thread-local instance
static thread_local zpt::lua::bridge lua;

// Or per-module instance
zpt::lua::bridge lua;
```

Each thread should have its own Lua state to avoid synchronization issues.

## Loading Lua Modules

### From Files

Create a Lua script file and load it:

```cpp
lua.add_module("/path/to/script.lua", zpt::json{ "module", "mymodule" });
```

### From C++ Callbacks

Register C++ functions callable from Lua:

```cpp
lua.add_module([](lua_State* L) {
    // Register C++ functions here
    lua_register(L, "greet", [](lua_State* L) -> int {
        const char* name = lua_tostring(L, 1);
        std::string result = std::string("Hello, ") + name + "!";
        lua_pushstring(L, result.c_str());
        return 1;  // Number of return values
    });
}, zpt::json{ "module", "builtin" });
```

## Calling Lua Functions

```cpp
// Call a Lua function with arguments
auto result = lua.call(
    zpt::json{ "module", "mymodule", "function", "my_func" },
    zpt::json{ zpt::array, "arg1", "arg2" }
);

// Result is zpt::json
std::cout << result << std::endl;
```

## Type Conversion

### C++ (JSON) to Lua

| JSON Type | Lua Type |
|-----------|----------|
| `JSNil` | `nil` |
| `JSBoolean` | `boolean` |
| `JSInteger` | `integer` |
| `JSDouble` | `number` |
| `JSString` | `string` |
| `JSObject` | `table` (string keys) |
| `JSArray` | `table` (integer keys) |

### Lua to C++ (JSON)

| Lua Type | JSON Type |
|----------|-----------|
| `nil` | `JSNil` |
| `boolean` | `JSBoolean` |
| `integer` | `JSInteger` |
| `number` | `JSDouble` |
| `string` | `JSString` |
| `table` | `JSObject` or `JSArray` |

## Thread Safety

Each thread should use its own bridge instance via `thread_instance()`:

```cpp
// In each thread, get the thread-local bridge
auto& local_lua = lua.thread_instance();

// Use it for calls
auto result = local_lua.call(
    zpt::json{ "module", "mymodule", "function", "my_func" },
    zpt::json{ zpt::array, "arg" }
);
```

## Stack Management

The bridge handles Lua stack management automatically, but you can manually clear it:

```cpp
lua.clear_stack();
```

## Example: Lua Bridge in a Plugin

The Lua bridge is typically used within a plugin's load function. Each thread gets its own bridge instance via `thread_instance()`:

```cpp
#include <zapata/lua.h>
#include <zapata/startup.h>

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    static thread_local zpt::lua::bridge lua;

    // Register a C++ function
    lua.add_module([](lua_State* L) {
        lua_register(L, "add", [](lua_State* L) -> int {
            int a = lua_tointeger(L, 1);
            int b = lua_tointeger(L, 2);
            lua_pushinteger(L, a + b);
            return 1;
        });
    }, zpt::json{ "module", "mathlib" });

    // Call from multiple threads — each uses its own bridge instance
    std::thread t1([&]() {
        auto& lua1 = lua.thread_instance();
        auto result = lua1.call(
            zpt::json{ "module", "mathlib", "function", "add" },
            zpt::json{ zpt::array, 10, 20 }
        );
        std::cout << "Thread 1: " << int(result) << std::endl;
    });

    std::thread t2([&]() {
        auto& lua2 = lua.thread_instance();
        auto result = lua2.call(
            zpt::json{ "module", "mathlib", "function", "add" },
            zpt::json{ zpt::array, 30, 40 }
        );
        std::cout << "Thread 2: " << int(result) << std::endl;
    });

    t1.join();
    t2.join();
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    // Bridge cleanup is handled automatically
}
```

## See Also

- [Bridges & Generators API Reference](../../api-reference/bridges-generators.md) - Bridge API details
- [Architecture Overview](../../architecture/overview.md) - Plugin architecture
- [Creating Language Bridges](../../extending/bridges.md) - Custom bridge guide
