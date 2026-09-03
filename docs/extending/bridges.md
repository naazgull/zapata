# Creating Language Bridges

Integrate a scripting language into Zapata using the CRTP bridge pattern.

## Overview

Language bridges provide bidirectional communication between C++ and scripting languages. The bridge system converts JSON values to native language objects and back.

## Architecture

```
programming::integration (abstract base)
       │
       │  name() → string
       │
programming::bridge<C, O> (CRTP template)
       │
       │  C = concrete class, O = native object type
       │  add_module(), init(), call(), json_to_object(), object_to_json()
       │
your_language::bridge (concrete implementation)
```

## Step 1: Define Your Bridge Class

```cpp
#include <zapata/bridge.h>

namespace mylang {

// The native object type for your language
using native_object = MyLangValue*;

class bridge : public zpt::programming::bridge<mylang::bridge, native_object> {
  public:
    bridge();
    ~bridge();

    // Required: return language name
    auto name() const -> std::string { return "mylang"; }

    // Required: load a module from a file path
    auto setup_module(zpt::json _conf, std::string _path) -> void;

    // Required: load a module from a C++ callback
    template<typename Callback>
    auto setup_module(zpt::json _conf, Callback _callback) -> void;

    // Required: register a lambda function
    template<typename Lambda>
    auto setup_lambda(zpt::json _conf, Lambda _lambda) -> void;

    // Required: initialize the language runtime
    auto initialize() -> void;

    // Required: locate a named object
    auto find(zpt::json _to_locate) -> native_object;

    // Required: convert JSON to native object
    auto to_object(zpt::json _to_convert) -> native_object;

    // Required: convert native object to JSON
    auto to_json(native_object _to_convert) -> zpt::json;

    // Required: execute a function (variadic template)
    template<typename Term, typename... Args>
    auto execute(Term _to_call, Args... _args) -> native_object;
};

} // namespace mylang
```

## Step 2: Implement Type Conversion

Map JSON types to your language's types:

```cpp
auto bridge::to_object(zpt::json _to_convert) -> native_object {
    switch (_to_convert->type()) {
        case zpt::JSNil:     return mylang_nil();
        case zpt::JSBoolean: return mylang_bool(bool(_to_convert));
        case zpt::JSInteger: return mylang_int(int(_to_convert));
        case zpt::JSDouble:  return mylang_float(double(_to_convert));
        case zpt::JSString:  return mylang_string(std::string(_to_convert));
        case zpt::JSObject:  /* convert object */ break;
        case zpt::JSArray:   /* convert array */ break;
        default: return mylang_nil();
    }
}

auto bridge::to_json(native_object _to_convert) -> zpt::json {
    // Convert your language's types back to JSON
    if (mylang_is_int(_to_convert)) return mylang_get_int(_to_convert);
    if (mylang_is_string(_to_convert)) return mylang_get_string(_to_convert);
    // ...
}
```

## Step 3: Implement Function Execution

```cpp
template<typename Term, typename... Args>
auto bridge::execute(Term _to_call, Args... _args) -> native_object {
    auto func_name = std::string(_to_call("function"));
    // Convert each argument and call the function
    return mylang_call(this->state(), func_name.c_str(), to_object(_args)...);
}
```

## Public API

The CRTP base class exposes a simplified public API that delegates to your implementation:

```cpp
auto& my = MYLANG_BRIDGE();
my.set_options({ "path", "/scripts" });
my.add_module("handlers.lua");        // → calls setup_module()
my.add_lambda([](zpt::json args) {   // → calls setup_lambda()
    return zpt::json{ 42 };
});
my.init();                            // → calls initialize()

auto result = my.call({ "function", "my_func" }, 1, 2, 3);  // → calls execute() + to_json()
auto native = my.json_to_object(zpt::json{ "key", "val" });  // → calls to_object()
auto json = my.object_to_json(native);                       // → calls to_json()
```

## Step 4: Provide a Global Accessor

```cpp
static thread_local bridge _instance;

auto MYLANG_BRIDGE() -> mylang::bridge& {
    return _instance;
}
```

## Reference: Lua Bridge

See the Lua bridge implementation for a complete working example:
- `bridges/lua/include/zapata/lua/lua.h` - Header
- `bridges/lua/src/lua.cpp` - Implementation

## See Also

- [Bridges & Generators API Reference](../api-reference/bridges-generators.md) - Bridge API details
- [Lua Integration Guide](../guides/scripting/lua.md) - Existing bridge example
- [Component Architecture](../architecture/components.md) - Module dependencies
