# JSON Handling Guide

The Zapata framework provides a powerful, type-safe JSON library centered around the `zpt::json` class. This guide covers common usage patterns and best practices.

## Quick Start

Include the JSON header:

```cpp
#include <zapata/json.h>
```

## Creating JSON Values

### Primitive Values

```cpp
// Strings
zpt::json str = "hello world";

// Numbers
zpt::json integer = 42;
zpt::json decimal = 3.14159;

// Booleans
zpt::json flag = true;

// Null
zpt::json null_value = nullptr;
```

### Objects

Objects are created using initializer lists with alternating key-value pairs:

```cpp
// Simple object
zpt::json user = {
    "name", "John Doe",
    "age", 30,
    "active", true
};

// Nested objects
zpt::json config = {
    "server", {
        "host", "localhost",
        "port", 8080
    },
    "database", {
        "connection", "postgresql://localhost/mydb"
    }
};
```

### Arrays

Arrays use the `zpt::array` marker as the first element:

```cpp
// Simple array
zpt::json numbers = { zpt::array, 1, 2, 3, 4, 5 };

// Mixed types
zpt::json mixed = { zpt::array, "hello", 42, true, nullptr };

// Array of objects
zpt::json users = { zpt::array,
    { "name", "Alice", "id", 1 },
    { "name", "Bob", "id", 2 }
};
```

### Factory Methods

```cpp
// Empty containers
zpt::json obj = zpt::json::object();
zpt::json arr = zpt::json::array();

// Explicit type conversion
zpt::json s = zpt::json::string(42);      // "42"
zpt::json i = zpt::json::integer("123");  // 123
zpt::json d = zpt::json::floating("3.14"); // 3.14
zpt::json b = zpt::json::boolean(1);       // true

// Timestamps
zpt::json now = zpt::json::date();                    // Current time
zpt::json ts = zpt::json::date("2024-01-15T10:30:00Z"); // ISO 8601
```

## Accessing Values

### Direct Access

```cpp
zpt::json obj = { "name", "John", "age", 30 };

// By key (for objects)
std::string name = obj("name");
int age = obj("age");

// By index (for arrays)
zpt::json arr = { zpt::array, "a", "b", "c" };
std::string first = arr(0);  // "a"
```

### Read vs Write Access

Understanding the difference between the `()` and `[]` operators is critical: they have fundamentally different semantics.

```cpp
zpt::json obj = { "name", "John" };

// Read-only access — returns the value or undefined if not found.
// Does NOT modify the object.
auto name = obj("name");
auto missing = obj("age");         // returns undefined, does not create "age"

// Create-or-access — returns a reference to the value.
// If the key does not exist, it creates the entry first (auto-vivification).
// If the index does not exist, it fills with undefined values.
obj["age"] = 30;                   // creates "age" if it didn't exist
obj["address"]["city"] = "Berlin"; // creates "address" (object) and "city" recursively

// Chained [] creates every missing level:
obj["a"]["b"]["c"] = 1;  // creates {"a": {"b": {"c": 1}}} from scratch
```

**Rule of thumb:** use `()` when reading, `[]` only when you intend to write or create entries.

```cpp
// Correct: read-only, safe to call multiple times
auto count = data("count");
auto label = data("label");

// Dangerous: accidental creation — the object grows with every call
// with a missing key. Use this only when you explicitly want to create:
auto count = data("count");   // obj is unchanged if "count" is missing
auto count2 = data("count");  // still unchanged
```

Using `[]` for read-only access silently mutates the JSON tree, which can cause subtle bugs:

```cpp
zpt::json config = { "server", { "host", "localhost" } };

// This reads "host" correctly, but does NOT mutate config.
auto host = config("server")("host");

// This ALSO reads "host" correctly, but creates "cache" inside
// the "server" object if it doesn't exist — mutating config unexpectedly.
auto host2 = config("server")["host"];  // creates "host" if it doesn't exist
auto cache = config("server")["cache"]; // creates "cache": undefined!
```

### Type Conversion

Values automatically convert to C++ types:

```cpp
zpt::json data = { "count", 42, "ratio", 0.5, "label", "test" };

// Numeric conversions
int i = data("count");           // 42
long l = data("count");          // 42L
double d = data("count");        // 42.0
size_t sz = data("count");       // 42

// String conversion
std::string s = data("label");   // "test"

// Boolean (non-zero/non-empty = true)
bool b = data("count");          // true
```

### Safe Access

Check if a value exists and has the expected type:

```cpp
zpt::json obj = { "name", "John" };

// Check for key existence
if (obj("name")->ok()) {
    // Key exists and is not null/undefined
}

// Check type
if (obj("name")->is_string()) {
    std::string name = obj("name");
}

// Type checking methods
obj("x")->is_object();
obj("x")->is_array();
obj("x")->is_string();
obj("x")->is_integer();
obj("x")->is_floating();
obj("x")->is_number();   // integer or floating
obj("x")->is_bool();
obj("x")->is_date();
obj("x")->is_nil();
obj("x")->is_undefined();
```

### Path-Based Access

Access deeply nested values using dot-notation paths:

```cpp
zpt::json config = {
    "database", {
        "primary", {
            "host", "db.example.com",
            "port", 5432
        }
    }
};

// Get nested value
zpt::json host = config->get_path("database.primary.host");
// or with custom separator
zpt::json port = config->get_path("database/primary/port", "/");

// Set nested value (creates intermediate objects)
config->set_path("database.primary.ssl", true);

// Delete nested value
config->del_path("database.primary.port");
```

## Iteration

### Range-Based For Loop

```cpp
zpt::json obj = { "a", 1, "b", 2, "c", 3 };

for (auto&& [index, key, value] : obj) {
    std::cout << index << ": " << key << " = " << value << std::endl;
}
// Output:
// 0: a = 1
// 1: b = 2
// 2: c = 3

zpt::json arr = { zpt::array, 10, 20, 30 };
for (auto&& [index, key, value] : arr) {
    std::cout << index << ": " << value << std::endl;
}
// Output:
// 0: 10
// 1: 20
// 2: 30
```

### Iterator Methods

```cpp
auto it = obj.begin();
auto end = obj.end();

while (it != end) {
    auto [idx, key, val] = *it;
    // process...
    ++it;
}
```

## Modifying JSON

### Adding Elements

```cpp
// Objects: use << operator with key-value pairs
zpt::json obj = zpt::json::object();
obj << "name" << "John";
obj << "age" << 30;

// Or with initializer list
obj << { "email", "john@example.com", "active", true };

// Arrays: use << to append
zpt::json arr = zpt::json::array();
arr << 1 << 2 << 3;
arr << { zpt::array, 4, 5, 6 };  // Append another array's contents
```

### Removing Elements

```cpp
zpt::json obj = { "a", 1, "b", 2, "c", 3 };

// Remove by key
obj->object()->pop("b");

// Arrays: remove by index
zpt::json arr = { zpt::array, 1, 2, 3 };
arr->array()->pop(1);  // Removes element at index 1
```

### The `+`, `-`, `|`, `&` Operators

These four operators each have a container meaning (for objects and arrays) and a scalar meaning. Every one also has an in-place compound form (`+=`, `-=`, `|=`, `&=`) that modifies the left-hand value and returns it. The binary forms return a new value and leave both operands untouched.

#### `+` — union

On **objects**, combines both key sets. Where a key exists on both sides, the right-hand value wins for scalars, while nested objects and arrays are merged recursively:

```cpp
zpt::json a = { "x", 1, "y", 2, "opts", { "keep", true } };
zpt::json b = { "y", 20, "z", 3, "opts", { "added", true } };

a + b;  // {"x":1, "y":20, "z":3, "opts":{"keep":true, "added":true}}
```

On **arrays**, appends the right-hand elements:

```cpp
{ zpt::array, 1, 2 } + { zpt::array, 3, 4 };  // [1, 2, 3, 4]
```

On **scalars** it is arithmetic:

```cpp
"foo" + "bar";           // "foobar" (concatenation)
3 + 4;                   // 7
3.5 + 0.25;              // 3.75
true + false;            // true (logical OR)
"2024-01-15T10:30:00Z" + 3600;  // date advanced by one hour
```

#### `-` — difference

On **objects**, removes every key that appears on the right-hand side (the right-hand values are ignored):

```cpp
zpt::json a = { "x", 1, "y", 2 };
zpt::json b = { "y", 20 };

a - b;  // {"x":1}
```

On **arrays**, subtracts element-wise by index. On **scalars** it is arithmetic:

```cpp
"hello world" - "world";  // "hello " (removes all occurrences of the substring)
10 - 4;                   // 6
0.5 - 0.25;               // 0.25
true - false;             // true (logical AND)
"2024-01-15T10:30:00Z" - 3600;  // date moved back by one hour
```

#### `|` — merge (strict union)

On **objects**, performs a deep merge that resolves in favor of the **left-hand side** at every level: for each right-hand key, if the left holds an object or array at that key the merge descends into it recursively, if the left entry is missing (or nil) the right-hand value is used, and otherwise the left value is kept. In the example below, `logging` collides as an object on both sides, so the merge goes one level deeper: `level` is kept from the left and `format` is added from the right:

```cpp
zpt::json defaults = { "port", 8080, "logging", { "level", "info" } };
zpt::json config   = { "port", 9090, "logging", { "format", "json" } };

defaults | config;
// {"port":8080, "logging":{"level":"info", "format":"json"}}
// "port" stays 8080 — existing left values are never overwritten
```

On **arrays**, appends (same as `+`). On **scalars** it is a bitwise/logical OR:

```cpp
1 | 2;        // 3
true | false; // true
"ab" | "cd";  // "abcd" (concatenation)
```

#### `&` — intersection

On **objects**, keeps only the keys present on **both** sides, using the left-hand values. Common nested objects and arrays are intersected recursively:

```cpp
zpt::json a = { "x", 1, "y", 2 };
zpt::json b = { "y", 20, "z", 3 };

a & b;  // {"y":2}
```

On **scalars** it is a bitwise/logical AND:

```cpp
6 & 3;        // 2
true & false; // false
```

### Operator Summary

| Operator | Objects | Arrays | Numbers | Strings | Booleans | Dates |
|----------|---------|--------|---------|---------|----------|-------|
| `+` | union, right wins at leaves | concatenate | add | concatenate | OR | add seconds |
| `-` | drop keys present on the right | subtract by index | subtract | remove substring occurrences | AND | subtract seconds |
| `\|` | deep merge, left wins | concatenate | bitwise OR | concatenate | OR | bitwise OR |
| `&` | common keys only (left values) | common elements only | bitwise AND | — | AND | bitwise AND |

## Parsing and Serialization

### Parsing JSON

```cpp
// From string
zpt::json data;
data.load_from(R"({"name": "John", "age": 30})");

// From stream
std::ifstream file("config.json");
zpt::json config;
file >> config;

// User-defined literal
auto json = R"({"key": "value"})"_JSON;
```

### Serializing JSON

```cpp
zpt::json obj = { "name", "John", "scores", { zpt::array, 95, 87, 92 } };

// To string
std::string compact = obj.stringify();
// {"name":"John","scores":[95,87,92]}

// To stream
std::cout << obj << std::endl;

// Pretty-printed
std::cout << zpt::pretty(obj) << std::endl;
// {
//     "name": "John",
//     "scores": [
//         95,
//         87,
//         92
//     ]
// }
```

## Advanced Features

### Cloning

JSON values use shared pointer semantics. To create an independent copy:

```cpp
zpt::json original = { "data", { zpt::array, 1, 2, 3 } };
zpt::json reference = original;  // Same underlying data
zpt::json copy = original->clone();  // Independent copy

reference["data"][0] = 100;  // Modifies original too!
copy["data"][0] = 200;       // Only modifies copy
```

### Traversal

Visit all elements recursively:

```cpp
zpt::json doc = {
    "users", { zpt::array,
        { "name", "Alice" },
        { "name", "Bob" }
    }
};

zpt::json::traverse(doc, [](std::string const& path,
                            zpt::json value,
                            std::string const& key) {
    std::cout << path << " (" << key << "): " << value << std::endl;
});
```

### Flattening

Convert nested structure to flat key-value pairs:

```cpp
zpt::json nested = {
    "server", {
        "host", "localhost",
        "port", 8080
    }
};

zpt::json flat = zpt::json::flatten(nested);
// {"server.host": "localhost", "server.port": 8080}
```

### Lambda Functions

Store callable references in JSON:

```cpp
// Register a lambda
zpt::lambda::add("greet/1", [](zpt::json args, zpt::context ctx) -> zpt::json {
    return zpt::json{ "Hello, " + std::string(args(0)) + "!" };
});

// Create a lambda reference
zpt::json fn = zpt::json::lambda("greet", 1);

// Call it (requires context)
zpt::context ctx{nullptr};
zpt::json result = fn->lambda()({"World"}, ctx);
```

### Regular Expressions

Store regex patterns in JSON:

```cpp
zpt::json pattern = zpt::json::regex("^user_[0-9]+$");

// Match against strings
if (pattern == "user_123") {
    // Matches!
}
```

## Configuration Files

Common pattern for loading configuration:

```cpp
// Load with defaults
zpt::json load_config(std::string const& path) {
    zpt::json defaults = {
        "server", {
            "host", "0.0.0.0",
            "port", 8080
        },
        "logging", {
            "level", "info"
        }
    };

    std::ifstream file(path);
    if (file) {
        zpt::json config;
        file >> config;
        // Deep merge, defaults winning at every level: colliding nested
        // objects are merged recursively, and config values only fill in
        // entries missing from the defaults.
        return defaults | config;
    }
    return defaults;
}
```

## Thread Safety

`zpt::json` uses shared pointers internally. While the reference counting is thread-safe, concurrent modifications to the same JSON value are not. Either:

1. Use external synchronization (mutex)
2. Clone values before passing to other threads
3. Use immutable patterns (create new values instead of modifying)

## See Also

- [API Reference: JSON](../api-reference/json.md)
- [Architecture Overview](../architecture/overview.md)
