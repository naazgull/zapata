# REST Engine API Reference

This document provides the API reference for Zapata's REST engine.

## Headers

```cpp
#include <zapata/rest.h>         // REST resolver
#include <zapata/startup.h>      // Application startup
#include <zapata/transport/engine.h> // Process class
```

---

## REST Resolver

### Class: `zpt::rest::resolver_t`

REST API request resolver implementing `zpt::events::resolver_t`.

Routes incoming HTTP requests to registered handlers based on URI patterns and HTTP methods.

### Constructor

```cpp
resolver_t(zpt::json _rest_config);
```

### Handler Registration

**Template-based (Operation classes):**
```cpp
auto add(zpt::json const& _id, zpt::json const& _metadata = zpt::undefined) -> resolver_t&;
auto add(zpt::performative _performative,
         zpt::json const& _id,
         zpt::json const& _metadata = zpt::undefined) -> resolver_t&;
```
Registers a handler Operation class. `zpt::events::Operation` types are registered with `resolver->add<MyHandler>("/path")`.

**Callback-based:**
```cpp
auto add(zpt::message _sent,
         zpt::events::resolver_callback callback) -> resolver_t&;
auto add(zpt::performative _performative,
         zpt::json const& _id,
         zpt::json const& _metadata,
         zpt::events::resolver_callback callback) -> resolver_t&;
```
Registers a callback function.

**Service descriptor:**
```cpp
auto add(zpt::json const& _service_description) -> resolver_t&;
```
Registers from a service descriptor JSON object.

### Handler Removal

```cpp
auto remove(zpt::message _sent) -> resolver_t&;
auto remove(zpt::performative _performative, zpt::json const& _id) -> resolver_t&;
```

### Resolution

```cpp
auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
    -> std::list<zpt::event>;
```
Resolves an incoming message to matching event handlers.

### Service Discovery

```cpp
auto search(zpt::json const& _id, std::string const& _provider_id = "") const -> zpt::json;
auto list(std::string const& _provider_id = "") const -> zpt::json;
auto register_provider(zpt::json const& _provider) -> resolver_t&;
auto unregister_provider(std::string const& _id) -> resolver_t&;
auto get_provider(std::string const& _id) const -> zpt::json;
auto clear() -> resolver_t&;
```

### `zpt::REST_RESOLVER`

```cpp
auto REST_RESOLVER(zpt::json _config = nullptr) -> zpt::events::resolver;
```
Returns the global REST resolver instance.

---

## Service Events

### Class: `zpt::rest::minion_boot`

Handles boot notifications from distributed nodes.

### Class: `zpt::rest::minion_shutdown`

Handles shutdown notifications from distributed nodes.

### Class: `zpt::rest::minion_hello`

Handles hello messages for node discovery.

### Class: `zpt::rest::services_list`

Handles service listing requests.

### `zpt::rest::services::broadcast`

```cpp
auto broadcast(std::string const& _path, zpt::json const& _config) -> void;
```
Broadcasts service availability to other nodes.

---

## Startup Engine

### Class: `zpt::plugin`

Dynamic plugin wrapper.

#### Type Alias

```cpp
using plugin_fn_type = std::function<bool(zpt::plugin& _plugin)>;
```

#### Constructor

```cpp
plugin(zpt::json _options, zpt::json _config);
```

#### Methods

```cpp
auto name() -> std::string&;
auto source() -> std::string&;
auto config() -> zpt::json&;
auto is_shutdown_ongoing() -> bool;
auto is_loaded() -> bool;
auto is_unloaded() -> bool;
auto add_thread(std::function<void()> _callback) -> plugin&;
```

| Method | Description |
|--------|-------------|
| `name()` | Plugin name |
| `source()` | Shared library path |
| `config()` | Plugin configuration |
| `is_shutdown_ongoing()` | True if shutdown in progress |
| `is_loaded()` | True if plugin is loaded |
| `is_unloaded()` | True if plugin is unloaded |
| `add_thread(callback)` | Register a worker thread |

### Plugin States

```cpp
inline constexpr std::uint64_t PLUGIN_STATE_UNLOADED{ 0 };
inline constexpr std::uint64_t PLUGIN_STATE_IN_SHUTDOWN{ 1 };
inline constexpr std::uint64_t PLUGIN_STATE_LOADED{ 2 };
```

---

### Class: `zpt::startup::boot`

Application boot manager.

#### Constructor

```cpp
boot(zpt::json _config);
```

#### Methods

```cpp
auto load() -> zpt::startup::boot&;
auto unload() -> zpt::startup::boot&;
auto to_string() -> std::string;
```

| Method | Description |
|--------|-------------|
| `load()` | Loads all plugins in dependency order |
| `unload()` | Unloads all plugins in reverse order |
| `to_string()` | String representation of loaded plugins |

---

## Global Functions

### `zpt::BOOT`

```cpp
auto BOOT(zpt::json _config = nullptr) -> zpt::startup::boot&;
```
Returns the global boot manager instance.

### `zpt::GLOBAL_CONFIG`

```cpp
auto GLOBAL_CONFIG() -> zpt::json;
```
Returns the global configuration.

### `zpt::IDENTITY`

```cpp
auto IDENTITY() -> zpt::json const&;
```
Returns the service identity JSON.

### `zpt::get_default_uri`

```cpp
auto get_default_uri() -> std::string;
```
Returns the default URI for this service instance.

---

## Configuration

### `zpt::startup::configuration::load`

```cpp
auto load(zpt::json _parameters, zpt::json& _output) -> void;
```
Loads configuration from files and environment.

---

## Usage Patterns

### Creating a REST Service

```cpp
#include <zapata/rest.h>
#include <zapata/transport/engine.h>

class GetUserHandler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        auto user_id = this->received()->uri()("params")("id");

        // Fetch user from database...
        this->to_send()->status(200)->body() = {
            "id", user_id,
            "name", "John Doe"
        };

        return zpt::events::finish;
    }
};

int main() {
    zpt::BOOT();

    auto& resolver = zpt::REST_RESOLVER();
    resolver->add<GetUserHandler>("/api/users/:id");

    // Start transport engine
    zpt::TRANSPORT_ENGINE();
    zpt::DISPATCHER()->trap();
}
```

### Writing a Plugin

```cpp
// my_plugin.cpp
#include <zapata/rest.h>

class MyHandler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        this->to_send()->status(200)->body() = zpt::json{ "plugin", "working" };
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> bool {
    auto& config = _plugin.config();

    // Register REST endpoints
    auto& resolver = zpt::REST_RESOLVER();
    resolver->add<MyHandler>("/api/my-endpoint");

    // Start a worker thread
    _plugin.add_thread([&]() {
        // Background work
    });

    return true;
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> bool {
    return true;
}
```

### Service Discovery

```cpp
auto& resolver = zpt::REST_RESOLVER();

// Register this node as a provider
resolver.register_provider({
    "_id", "node-1",
    "protocols", {
        "default", "http",
        "registered", {
            "http", {
                "bind", "192.168.1.10",
                "port", 8080
            }
        }
    }
});

// Search for services
auto services = resolver.search({ "path", "/api/users" });
for (auto&& [_, __, svc] : services) {
    std::cout << "Found: " << svc("provider_id") << std::endl;
}

// List all services
auto all = resolver.list();
```

### Making Outbound Calls

```cpp
#include <zapata/transport/engine.h>
#include <zapata/rest/services.h>

// Create request
auto request = zpt::make_message<zpt::json_message>();
request->performative(zpt::Post);
request->uri("/remote/endpoint");
request->body() = { "data", "value" };

// Make async call with response handler
class ResponseHandler : public zpt::events::process {
  public:
    using zpt::events::process::process;
    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        // Handle the response
        this->to_send()->status(200)
            ->body() = this->received()->body();
        return zpt::events::finish;
    }
};

zpt::make_call<ResponseHandler>(zpt::REST_RESOLVER(), request);
```

---

## See Also

- [Events API](events.md) - Event dispatcher
- [Transport API](transport.md) - Network transports
- [HTTP API](http.md) - HTTP message types
