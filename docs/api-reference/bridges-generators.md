# Bridges & Generators API Reference

The bridges and generators modules provide language integration and code generation capabilities for Zapata.

## Module Overview

| Module | Header | Description |
|--------|--------|-------------|
| bridges/base | `<zapata/bridge.h>` | CRTP base template for language bridges |
| bridges/lua | `<zapata/lua.h>` | Lua scripting integration |
| generators/ast | `<zapata/ast.h>` | AST-based code generation |

---

## Bridges

### Base Bridge Template

**Header:** `<zapata/bridge.h>`

The bridge system uses CRTP (Curiously Recurring Template Pattern) to provide a common interface for integrating scripting languages.

#### zpt::programming::integration

Abstract base class for all language integrations.

```cpp
class integration {
  public:
    virtual auto name() const -> std::string = 0;
};
```

#### zpt::programming::bridge<C, O>

CRTP base template for language bridges.

```cpp
template<typename C, typename O>
class bridge : public integration {
  public:
    using class_type = C;   // Concrete bridge type
    using object_type = O;  // Native object type

    auto set_options(zpt::json _conf) -> bridge<C, O>&;
    auto options() const -> zpt::json;

    auto add_module(std::string _external_path, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    template<typename Callback>
    auto add_module(Callback _callback, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    template<typename Lambda>
    auto add_lambda(Lambda _lambda, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    auto init() -> bridge<C, O>&;

    auto locate(zpt::json _to_locate) -> object_type;
    auto json_to_object(zpt::json _to_convert) -> object_type;
    auto object_to_json(object_type _to_convert) -> zpt::json;

    template<typename Term, typename... Args>
    auto call(Term _to_call, Args... _arg) -> zpt::json;
};
```

**Type Parameters:**
- `C` - The concrete bridge class (for CRTP)
- `O` - The language's native object type

**Required Methods for Derived Classes:**

The derived class `C` must implement:
- `setup_module(zpt::json, std::string)` - Load external module
- `setup_module(zpt::json, Callback)` - Register callback module
- `setup_lambda(zpt::json, Lambda)` - Register lambda
- `initialize()` - Initialize the bridge
- `find(zpt::json)` - Locate an object by path
- `to_object(zpt::json)` - Convert JSON to native object
- `to_json(O)` - Convert native object to JSON
- `execute(Term, Args...)` - Execute a function

---

### Lua Bridge

**Header:** `<zapata/lua.h>`

Provides bidirectional integration between C++ and Lua.

#### zpt::lua_object

RAII wrapper for `lua_State*`.

```cpp
class lua_object {
  public:
    lua_object();
    lua_object(lua_State* _rhs);
    lua_object(lua_object const& _rhs);
    lua_object(lua_object&& _rhs);
    ~lua_object();

    auto operator=(lua_object const& _rhs) -> lua_object&;
    auto operator=(lua_object&& _rhs) -> lua_object&;
    auto operator=(lua_State* _rhs) -> lua_object&;
    auto operator->() -> lua_State*;
    auto operator*() -> lua_State&;
    operator lua_State*();

    auto get() -> lua_State*;
};
```

#### zpt::lua::bridge

Lua scripting language bridge implementation.

```cpp
class bridge : public zpt::programming::bridge<zpt::lua::bridge, zpt::lua_object> {
  public:
    using underlying_type = lua_State*;
    using callback_type = std::function<void(underlying_type)>;
    using lambda_type = std::function<int(underlying_type)>;

    bridge();
    ~bridge();

    auto name() const -> std::string;              // Returns "lua"
    auto state() -> lua_State*;                    // Raw Lua state
    auto thread_instance() -> bridge&;             // Thread-local instance

    // Module loading
    auto setup_module(zpt::json _conf, std::string _external_path, bool _persist = true)
      -> zpt::lua::bridge&;
    auto setup_module(zpt::json _conf, callback_type _callback, bool _persist = true)
      -> zpt::lua::bridge&;

    auto find(zpt::json _to_locate) -> object_type;
    auto clear_stack() -> zpt::lua::bridge&;

    // JSON/Lua conversion
    auto to_json(object_type _to_convert) -> zpt::json;
    auto to_json(object_type _to_convert, int _index) -> zpt::json;
    auto to_ref(object_type _to_convert, int _index = 1) -> zpt::json;
    auto to_object(zpt::json _to_convert) -> object_type;
    auto to_object(zpt::json _to_convert, object_type _return) -> object_type;
    auto from_ref(zpt::json _to_convert, object_type _return) -> object_type;

    // Execution
    auto execute(zpt::json _func, zpt::json _args) -> object_type;
    auto initialize() -> zpt::lua::bridge&;
};
```

#### zpt::LUA_BRIDGE()

Returns the global thread-local Lua bridge instance.

```cpp
auto LUA_BRIDGE() -> zpt::lua::bridge&;
```

---

#### Lua Bindings Functions

The Lua bridge provides utility functions for binding Zapata APIs to Lua scripts.

**Header:** `<zapata/lua/lua.h>`

**Namespace:** `zpt::lua::bindings`

##### `make_request`

Creates a new HTTP request from the Lua stack.

```cpp
auto make_request(lua_State* _state) -> int;
```

**Parameters:**
- `_state` - Lua state pointer

**Returns:** Pushes a new HTTP request object onto the Lua stack (returns 1)

**Raises:** `std::runtime_error` if the Lua stack doesn't contain a protocol atom

**Example:**
```lua
-- Call from Lua
zpt.make_request()  -- Creates HTTP request on stack
```

---

##### `send_request`

Sends an HTTP request and receives a response.

```cpp
auto send_request(lua_State* _state) -> int;
```

**Parameters:**
- `_state` - Lua state pointer

**Returns:** Pushes the response object onto the Lua stack (returns 1)

**Raises:** `std::runtime_error` if request parameters are missing or invalid

**Example:**
```lua
zpt.send_request()  -- Sends request, returns response on stack
```

---

##### `get_config`

Pushes the global Zapata configuration as a JSON table onto the Lua stack.

```cpp
auto get_config(lua_State* _state) -> int;
```

**Parameters:**
- `_state` - Lua state pointer

**Returns:** Pushes the global configuration JSON object onto the Lua stack (returns 1)

**Example:**
```lua
local config = zpt.get_config()  -- Get global config
local log_level = config.log.level
```

---

##### `log`

Logs the arguments from the Lua stack using the Zapata logging system.

```cpp
auto log(lua_State* _state) -> int;
```

**Parameters:**
- `_state` - Lua state pointer

**Returns:** 0 (no return values)

**Raises:** `std::runtime_error` if log level cannot be determined

**Example:**
```lua
-- Log at info level
zpt.log(zpt.levels.info, "Application started")

-- Log with format
zpt.log(zpt.levels.info, "User {}: logged in", "alice")
```

---

##### `to_json_str`

Parses a JSON string from the Lua stack and pushes it back as a JSON value.

```cpp
auto to_json_str(lua_State* _state) -> int;
```

**Parameters:**
- `_state` - Lua state pointer

**Returns:** Pushes the parsed JSON value onto the Lua stack (returns 1)

**Example:**
```lua
local json_str = '{"name": "Alice", "age": 30}'
local json_obj = zpt.to_json_str(json_str)
local name = json_obj.name
```

---

##### `sleep`

Sleeps for the given number of seconds.

```cpp
auto sleep(lua_State* _state) -> int;
```

**Parameters:**
- `_state` - Lua state pointer

**Returns:** 0 (no return values)

**Raises:** `std::runtime_error` if sleep duration is invalid

**Example:**
```lua
zpt.sleep(1.5)  -- Sleep for 1.5 seconds
```

---

##### `register_bindings`

Registers the `zpt` Lua module with bindings for HTTP requests, config access, logging, and JSON conversion.

```cpp
auto register_bindings(lua_State* _state) -> void;
```

**Parameters:**
- `_state` - Lua state pointer

**Effect:** Registers all `zpt` functions and constants to the global namespace

**Example:**
```lua
-- In C++ initialization
auto lua_state = luaL_newstate();
luaL_openlibs(lua_state);
zpt::lua::bindings::register_bindings(lua_state);

-- Now accessible from Lua
zpt.make_request()
zpt.log(zpt.levels.info, "Message")
zpt.sleep(1.0)
```

**Registered Functions:**
- `make_request` - Create HTTP request
- `send_request` - Send HTTP request
- `get_config` - Get global configuration
- `log` - Log message
- `to_json_str` - Parse JSON string
- `sleep` - Sleep for seconds

**Registered Constants:**
- `zpt.levels.debug`, `info`, `warn`, `error`, `critical`, `fatal`
- `zpt.levels.off` (0)
- `zpt.levels.debug` (1)
- `zpt.levels.info` (6)
- `zpt.levels.warn` (7)
- `zpt.levels.error` (8)
- `zpt.levels.critical` (9)
- `zpt.levels.fatal` (10)

---

### Lua Bridge Usage

```cpp
#include <zapata/lua.h>

// Local instance
zpt::lua::bridge lua;

// Configure and load a Lua module
lua.add_module("/path/to/script.lua")
   .init();

// Call a Lua function
auto result = lua.call(
    zpt::json{ "module", "mymodule", "function", "my_lua_func" },
    zpt::json{ zpt::array, "arg1", "arg2" }
);

// Register a C++ callback callable from Lua
lua.add_module([](lua_State* L) {
    // Register C++ functions here
    lua_register(L, "cpp_func", [](lua_State* L) -> int {
        // Implementation
        return 1;  // Number of return values
    });
}, zpt::json{ "module", "my_module" });

// Get thread-local instance
auto& local_lua = lua.thread_instance();
```

---

## Generators

### AST Code Generation

**Header:** `<zapata/ast.h>`

Provides an abstract syntax tree representation for generating source code programmatically.

### Configuration

```cpp
// Number of spaces per indentation level (default: 4)
inline std::uint16_t AST_INDENTATION_SPACES{ 4 };
```

### Visibility Constants

```cpp
namespace zpt::ast {
    static constexpr int PUBLIC{ 0 };
    static constexpr int PROTECTED{ 1 };
    static constexpr int PRIVATE{ 2 };
}
```

### Modifier Constants

```cpp
namespace zpt::ast {
    static constexpr int VIRTUAL{ 1 };
    static constexpr int FRIEND{ 2 };
    static constexpr int CONST{ 4 };
    static constexpr int OVERRIDE{ 8 };
    static constexpr int FINAL{ 16 };
    static constexpr int DEFAULT{ 32 };
    static constexpr int DELETE{ 64 };
    static constexpr int ABSTRACT{ 128 };
    static constexpr int PARAMETER{ 256 };
    static constexpr int EXTERN{ 512 };
    static constexpr int EXTERNC{ 1024 };
}
```

### AST Class Hierarchy

```
basic_element (abstract base)
├── basic_class
├── basic_code_block
├── basic_function
├── basic_variable
└── basic_instruction

basic_module (container for files)
basic_file (container for elements)
```

#### zpt::ast::basic_element

Abstract base class for all AST nodes.

```cpp
class basic_element : public std::enable_shared_from_this<basic_element> {
  public:
    std::shared_ptr<basic_element> __parent{ nullptr };

    virtual auto to_string() const -> std::string = 0;
    auto get_indentation() const -> std::string;
    auto new_line() const -> bool;
    auto set_new_line(bool _value) -> basic_element&;

    friend auto operator<<(std::ostream& _out, basic_element& _in) -> std::ostream&;
};
```

#### zpt::ast::basic_module

Container for generated source files.

```cpp
class basic_module {
  public:
    using allowed_type = std::shared_ptr<basic_file>;

    basic_module(std::string const& _module_name);

    auto name() const -> std::string const&;
    auto add(std::shared_ptr<basic_file> _to_add) -> basic_module&;
    template<typename... Args>
    auto add(Args... _args) -> basic_module&;
    auto dump() -> basic_module&;
    auto dump(std::ostream& _out) -> basic_module&;
    template<typename Callback>
    auto traverse_elements(Callback _callback) -> basic_module&;
};
```

#### zpt::ast::basic_file

Represents a source file containing classes, functions, and variables.

```cpp
class basic_file {
  public:
    using allowed_type = std::variant<
        std::shared_ptr<basic_class>,
        std::shared_ptr<basic_function>,
        std::shared_ptr<basic_code_block>,
        std::shared_ptr<basic_variable>,
        std::shared_ptr<basic_instruction>>;

    basic_file(std::filesystem::path const& _path);

    auto path() const -> std::filesystem::path const&;
    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_file&;
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_file&;
    auto dump() -> basic_file&;
    auto dump(std::ostream& _out) -> basic_file&;
    template<typename Callback>
    auto traverse_elements(Callback _callback) -> basic_file&;
};
```

#### zpt::ast::basic_class

Represents a class definition with visibility sections.

```cpp
class basic_class : public basic_element {
  public:
    using allowed_type = std::variant<
        std::shared_ptr<basic_class>,
        std::shared_ptr<basic_function>,
        std::shared_ptr<basic_variable>>;

    basic_class(std::string const& _name, std::string const& _extends = "");

    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add, int _visibility) -> basic_class&;
    template<BasicASTElement T, typename... Args>
    auto add(int _visibility, Args... _args) -> basic_class&;
    template<typename Callback>
    auto traverse_elements(Callback _callback) -> basic_class&;
};
```

#### zpt::ast::basic_function

Represents a function with parameters and optional body.

```cpp
class basic_function : public basic_element {
  public:
    basic_function(std::string const& _name,
                   std::string const& _return_type = "",
                   int _modifiers = 0);

    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_function&;
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_function&;
    auto set_modifiers(int _modifiers) -> basic_function&;
    auto body() -> std::shared_ptr<basic_code_block>;
};
```

#### zpt::ast::basic_variable

Represents a variable declaration with optional initialization.

```cpp
class basic_variable : public basic_element {
  public:
    basic_variable(std::string const& _name, std::string const& _type, int _modifiers = 0);

    auto add(std::shared_ptr<basic_code_block> _initialization) -> basic_variable&;
    template<typename... Args>
    auto add(Args... _args) -> basic_variable&;
    auto set_modifiers(int _modifiers) -> basic_variable&;
    auto initialization() -> std::shared_ptr<basic_code_block>;
};
```

#### zpt::ast::basic_instruction

Represents raw code instructions with optional body block.

```cpp
class basic_instruction : public basic_element {
  public:
    basic_instruction(std::string const& _code);

    auto add(std::shared_ptr<basic_code_block> _body) -> basic_instruction&;
    template<typename... Args>
    auto add(Args... _args) -> basic_instruction&;
    auto body() -> std::shared_ptr<basic_code_block>;
};
```

#### zpt::ast::basic_code_block

Represents a block of statements (e.g., function body, if block).

```cpp
class basic_code_block : public basic_element {
  public:
    using allowed_type = std::variant<
        std::shared_ptr<basic_class>,
        std::shared_ptr<basic_code_block>,
        std::shared_ptr<basic_variable>,
        std::shared_ptr<basic_instruction>>;

    basic_code_block(std::string const& _prefix = "");

    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_code_block&;
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_code_block&;
    template<typename Callback>
    auto traverse_elements(Callback _callback) -> basic_code_block&;
};
```

### C++ AST Classes

**Header:** `<zapata/ast/cpp.h>`

C++-specific AST implementations that generate proper C++ syntax.

#### zpt::ast::cpp_class

```cpp
class cpp_class : public basic_class {
  public:
    cpp_class(std::string const& _name, std::string const& _extends = "");
    auto to_string() const -> std::string override;
};
```

#### zpt::ast::cpp_function

```cpp
class cpp_function : public basic_function {
  public:
    cpp_function(std::string const& _name,
                 std::string const& _return_type = "",
                 int _modifiers = 0);
    auto to_string() const -> std::string override;
};
```

#### zpt::ast::cpp_variable

```cpp
class cpp_variable : public basic_variable {
  public:
    cpp_variable(std::string const& _name, std::string const& _type, int _modifiers = 0);
    auto to_string() const -> std::string override;
};
```

#### zpt::ast::cpp_instruction

```cpp
class cpp_instruction : public basic_instruction {
  public:
    cpp_instruction(std::string const& _code, bool _no_end_of_line = false);
    auto to_string() const -> std::string override;
};
```

#### zpt::ast::cpp_code_block

```cpp
class cpp_code_block : public basic_code_block {
  public:
    cpp_code_block(std::string const& _prefix = "");
    auto to_string() const -> std::string override;
};
```

#### zpt::ast::cmake_instruction

For generating CMakeLists.txt files.

```cpp
class cmake_instruction : public basic_instruction {
  public:
    cmake_instruction(std::string const& _code);
    auto to_string() const -> std::string override;
};
```

### Factory Functions

```cpp
namespace zpt {
    template<typename T, typename... Args>
    auto make_module(Args... _args) -> std::shared_ptr<ast::basic_module>;

    template<typename T, typename... Args>
    auto make_file(Args... _args) -> std::shared_ptr<ast::basic_file>;

    template<ast::BasicASTElement T, typename... Args>
    auto make_class(Args... _args) -> std::shared_ptr<ast::basic_class>;

    template<ast::BasicASTElement T, typename... Args>
    auto make_code_block(Args... _args) -> std::shared_ptr<ast::basic_code_block>;

    template<ast::BasicASTElement T, typename... Args>
    auto make_function(Args... _args) -> std::shared_ptr<ast::basic_function>;

    template<ast::BasicASTElement T, typename... Args>
    auto make_variable(Args... _args) -> std::shared_ptr<ast::basic_variable>;

    template<ast::BasicASTElement T, typename... Args>
    auto make_instruction(Args... _args) -> std::shared_ptr<ast::basic_instruction>;
}
```

### Concepts

#### zpt::ast::BasicASTElement

```cpp
template<typename T>
concept BasicASTElement = requires(T _t) {
    requires std::derived_from<T, basic_element>;
};
```

### Code Generation Example

```cpp
#include <zapata/ast.h>

// Create a module with a header file
auto file = zpt::allocate_shared<zpt::ast::basic_file>("my_class.h");

// Create a class
auto cls = zpt::allocate_shared<zpt::ast::cpp_class>("MyClass", "BaseClass");

// Add a public method
auto method = zpt::allocate_shared<zpt::ast::cpp_function>(
    "process",
    "void",
    zpt::ast::VIRTUAL | zpt::ast::OVERRIDE
);

// Add a parameter
method->add<zpt::ast::cpp_variable>("input", "std::string const&", zpt::ast::PARAMETER);

// Add method body
auto body = zpt::allocate_shared<zpt::ast::cpp_code_block>();
body->add<zpt::ast::cpp_instruction>("std::cout << input << std::endl");
method->add(body);

// Add method to class
cls->add(method, zpt::ast::PUBLIC);

// Add a private member
cls->add<zpt::ast::cpp_variable>(
    zpt::ast::PRIVATE,
    "data_",
    "std::string"
);

// Add class to file
file->add(cls);

// Generate output
file->dump(std::cout);
```

**Generated Output:**
```cpp
class MyClass : public BaseClass {
  public:
    virtual auto process(std::string const& input) -> void override {
        std::cout << input << std::endl;
    }
  private:
    std::string data_;
};
```

---

## See Also

- [REST Engine](rest.md) - Uses bridges for scripting support
- [Common Base](base.md) - JSON type used for bridge communication
- [Events](events.md) - Event handling patterns
