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

```cpp
// Global thread-local instance
auto& lua = zpt::LUA_BRIDGE();
```

Each thread gets its own Lua state to avoid synchronization issues.

## Loading Lua Modules

### From Files

```cpp
lua.add_module("/path/to/script.lua")
   .init();
```

### From C++ Callbacks

```cpp
lua.add_module([](lua_State* L) {
    // Register C++ functions callable from Lua
    lua_register(L, "greet", [](lua_State* L) -> int {
        const char* name = lua_tostring(L, 1);
        std::string result = std::string("Hello, ") + name + "!";
        lua_pushstring(L, result.c_str());
        return 1;  // Number of return values
    });
}, zpt::json{ "name", "my_module" });

lua.init();
```

## Calling Lua Functions

```cpp
// Call a Lua function with JSON arguments
auto result = lua.call(
    zpt::json{ "function", "my_lua_func" },
    zpt::json{ "arg1", "hello", "arg2", 42 }
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

## Configuration and Options

```cpp
auto& lua = zpt::LUA_BRIDGE();

// Set configuration options
lua.set_options(zpt::json{
    "script_path", "/usr/local/share/myapp/scripts",
    "preload", { zpt::array, "utils.lua", "config.lua" }
});
```

## Stack Management

The bridge handles Lua stack management automatically, but you can manually clear it:

```cpp
lua.clear_stack();
```

## Thread Safety

Each thread gets its own Lua state via `thread_instance()`. Access the current thread's bridge:

```cpp
auto& local_lua = zpt::LUA_BRIDGE().thread_instance();
```

## See Also

- [Bridges & Generators API Reference](../../api-reference/bridges-generators.md) - Bridge API details
- [Architecture Overview](../../architecture/overview.md) - Plugin architecture
