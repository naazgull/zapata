# JSON API Reference

This document provides the complete API reference for the Zapata JSON module.

## Headers

```cpp
#include <zapata/json.h>           // Main aggregate header
#include <zapata/json/JSONClass.h> // Core types only
```

---

## Types

### `zpt::JSONType`

Enumeration of JSON value types.

| Value | Description |
|-------|-------------|
| `JSNil` | Null value |
| `JSBoolean` | Boolean (`true`/`false`) |
| `JSInteger` | Integer number |
| `JSDouble` | Floating-point number |
| `JSString` | String value |
| `JSDate` | Timestamp (ms since epoch) |
| `JSArray` | Array of values |
| `JSObject` | Key-value object |
| `JSRegex` | Regular expression |
| `JSLambda` | Lambda reference |
| `JSUndefined` | Undefined value |

### `zpt::timestamp_t`

```cpp
using timestamp_t = unsigned long long;
```

Timestamp type representing milliseconds since Unix epoch.

---

## Class: `zpt::json`

The main dynamic JSON value type.

### Type Aliases

```cpp
using map = std::map<zpt::json, int>;  // JSON can be used as map key
using element = std::tuple<size_t, std::string, zpt::json>;
using iterator = zpt::JSONIterator;
using const_iterator = const zpt::JSONIterator;
using traverse_callback = std::function<void(std::string const&, zpt::json, std::string const&)>;
```

### Constructors

```cpp
json();                                           // Null value
json(std::nullptr_t);                             // Null value
json(std::initializer_list<zpt::json> _init);     // Object or array
json(zpt::json const& _rhs);                      // Copy (shared)
json(zpt::json&& _rhs);                           // Move
template<typename T> json(T const& _rhs);         // From any type
```

### Instance Methods

#### Size and State

| Method | Description |
|--------|-------------|
| `size() -> size_t` | Number of elements (1 for scalars) |
| `hash() -> size_t` | Hash value for containers |
| `value() -> JSONElementT&` | Access underlying element |

#### Parsing and Serialization

| Method | Description |
|--------|-------------|
| `load_from(string const&) -> json&` | Parse from string |
| `load_from(istream&) -> json&` | Parse from stream |
| `stringify(ostream&) -> json&` | Write compact JSON |
| `stringify(string&) -> json&` | Write to string |
| `stringify() -> string` | Return JSON string |

#### Iteration

| Method | Description |
|--------|-------------|
| `begin() -> iterator` | Iterator to first element |
| `end() -> iterator` | Iterator past last element |

#### Operators

| Operator | Description |
|----------|-------------|
| `operator=(T)` | Assign value |
| `operator->()` | Access element pointer |
| `operator*()` | Dereference element |
| `operator[](T)` | Access by key or index |
| `operator()(T)` | Access by key (const) |
| `operator==`, `!=`, `<`, `>`, `<=`, `>=` | Comparison |
| `operator<<(T)` | Append/push value |
| `operator+`, `+=` | Union |
| `operator-`, `-=` | Difference |
| `operator\|`, `\|=` | Merge (deep merge for objects) |
| `operator&`, `&=` | Intersection |

#### Type Conversion Operators

```cpp
operator std::string();
operator bool();
operator int();
operator long();
operator long long();
operator size_t();
operator double();
operator zpt::timestamp_t();
operator zpt::JSONObj();
operator zpt::JSONArr();
operator zpt::lambda();
operator zpt::regex();
```

### Static Methods

#### Factory Methods

| Method | Description |
|--------|-------------|
| `object() -> json` | Create empty object |
| `array() -> json` | Create empty array |
| `string(T) -> json` | Create string from value |
| `integer(T) -> json` | Create integer from value |
| `uinteger(T) -> json` | Create unsigned int |
| `floating(T) -> json` | Create double |
| `ulong(T) -> json` | Create size_t |
| `boolean(T) -> json` | Create boolean |
| `date() -> json` | Current timestamp |
| `date(string const&) -> json` | Parse ISO 8601 date |
| `date(T) -> json` | From timestamp value |
| `lambda(T) -> json` | Create lambda reference |
| `lambda(name, n_args) -> json` | Create lambda by signature |
| `regex(T) -> json` | Create regex pattern |

#### Utility Methods

| Method | Description |
|--------|-------------|
| `parse_json_str(string const&) -> json` | Parse JSON string |
| `to_unicode(string&) -> void` | Convert escapes to Unicode |
| `pretty(T) -> string` | Pretty-print value |
| `type_of(T) -> JSONType` | Get type of value |
| `traverse(json, callback) -> void` | Recursive traversal |
| `flatten(json) -> json` | Flatten to single level |
| `find(begin, end, value) -> iterator` | Find in range |
| `contains(element, value) -> bool` | Check containment |

---

## Class: `zpt::JSONElementT`

Internal variant-based element storage.

### Type Checking Methods

| Method | Returns `true` if... |
|--------|---------------------|
| `is_object()` | Value is an object |
| `is_array()` | Value is an array |
| `is_string()` | Value is a string |
| `is_integer()` | Value is an integer |
| `is_floating()` | Value is a double |
| `is_number()` | Value is numeric |
| `is_bool()` | Value is boolean |
| `is_date()` | Value is a timestamp |
| `is_lambda()` | Value is a lambda |
| `is_regex()` | Value is a regex |
| `is_nil()` | Value is null |
| `is_undefined()` | Value is undefined |

### State Methods

| Method | Description |
|--------|-------------|
| `type() -> JSONType` | Get value type |
| `ok() -> bool` | True if not null/undefined |
| `empty() -> bool` | True if empty container |
| `nil() -> bool` | True if null |
| `size() -> size_t` | Element count |
| `hash() -> size_t` | Hash value |

### Value Accessors

| Method | Description |
|--------|-------------|
| `object() -> JSONObj&` | Get as object |
| `array() -> JSONArr&` | Get as array |
| `string() -> string&` | Get as string |
| `integer() -> long long&` | Get as integer |
| `floating() -> double&` | Get as double |
| `boolean() -> bool&` | Get as boolean |
| `date() -> timestamp_t&` | Get as timestamp |
| `lambda() -> lambda&` | Get as lambda |
| `regex() -> regex&` | Get as regex |
| `number() -> double` | Get numeric value |

### Path Methods

| Method | Description |
|--------|-------------|
| `get_path(path, sep=".") -> json` | Get nested value |
| `set_path(path, value, sep=".") -> &` | Set nested value |
| `del_path(path, sep=".") -> &` | Delete nested value |

### Serialization

| Method | Description |
|--------|-------------|
| `stringify(out) -> &` | Compact JSON output |
| `stringify() -> string` | Return compact string |
| `prettify(out, tabs=0) -> &` | Formatted output |
| `prettify() -> string` | Return formatted string |

---

## Class: `zpt::JSONObj`

JSON object wrapper (shared pointer to `JSONObjT`).

### Methods

| Method | Description |
|--------|-------------|
| `hash() -> size_t` | Hash value |
| `operator->()` | Access `JSONObjT` |
| `operator*()` | Dereference `JSONObjT` |
| `operator[](T)` | Access element |
| `operator<<(T)` | Push key or value |

---

## Class: `zpt::JSONObjT`

Internal JSON object implementation.

### Methods

| Method | Description |
|--------|-------------|
| `stringify(out)` | Compact serialization |
| `prettify(out, tabs)` | Formatted serialization |
| `push(key)` | Add key (awaiting value) |
| `push(value)` | Add value for key |
| `pop(idx)` | Remove by index or key |
| `key_for(idx) -> string` | Get key at index |
| `clone() -> json` | Deep copy |
| `get_path(path, sep)` | Path-based access |
| `set_path(path, value, sep)` | Path-based mutation |
| `del_path(path, sep)` | Path-based deletion |

---

## Class: `zpt::JSONArr`

JSON array wrapper (shared pointer to `JSONArrT`).

### Methods

| Method | Description |
|--------|-------------|
| `hash() -> size_t` | Hash value |
| `operator->()` | Access `JSONArrT` |
| `operator*()` | Dereference `JSONArrT` |
| `operator[](T)` | Access element |
| `operator<<(T)` | Append value |

---

## Class: `zpt::JSONArrT`

Internal JSON array implementation.

### Methods

| Method | Description |
|--------|-------------|
| `stringify(out)` | Compact serialization |
| `prettify(out, tabs)` | Formatted serialization |
| `push(value)` | Append element |
| `pop(idx)` | Remove by index |
| `sort()` | Sort elements |
| `sort(comparator)` | Sort with custom comparator |
| `clone() -> json` | Deep copy |

---

## Class: `zpt::JSONIterator`

Bidirectional iterator for JSON containers.

### Types

```cpp
using difference_type = std::ptrdiff_t;
using value_type = zpt::json::element;  // tuple<size_t, string, json>
using iterator_category = std::bidirectional_iterator_tag;
```

### Operators

| Operator | Description |
|----------|-------------|
| `operator++()` | Pre-increment |
| `operator++(int)` | Post-increment |
| `operator--()` | Pre-decrement |
| `operator--(int)` | Post-decrement |
| `operator*()` | Dereference (returns element tuple) |
| `operator->()` | Arrow access |
| `operator==`, `!=` | Equality comparison |

---

## Class: `zpt::pretty`

Pretty-print wrapper for JSON values.

### Constructors

```cpp
pretty(const pretty&);
pretty(pretty&&);
pretty(string const&);
pretty(const char*);
template<typename T> pretty(T);  // From JSON-like object
```

### Operators

| Operator | Description |
|----------|-------------|
| `operator string()` | Convert to formatted string |
| `operator<<(ostream&, pretty)` | Stream output |

---

## Class: `zpt::lambda`

Lambda function wrapper for JSON-embedded callables.

### Constructors

```cpp
lambda();
lambda(string const& _signature);           // "name/arity"
lambda(string const& _name, unsigned short _n_args);
```

### Methods

| Method | Description |
|--------|-------------|
| `operator()(json args, context ctx)` | Invoke lambda |
| `hash() -> size_t` | Hash value |

### Static Methods

| Method | Description |
|--------|-------------|
| `add(signature, symbol)` | Register lambda |
| `add(name, n_args, symbol)` | Register with arity |
| `call(name, args, ctx) -> json` | Call by name |
| `stringify(name, n_args) -> string` | Create signature |
| `parse(signature) -> tuple` | Parse signature |

---

## Class: `zpt::regex`

Regular expression wrapper (alias for `JSONRegex`).

### Constructors

```cpp
JSONRegex();
JSONRegex(string const& pattern);
```

### Methods

| Method | Description |
|--------|-------------|
| `operator->()` | Access `std::regex*` |
| `operator*()` | Dereference `std::regex` |
| `to_string() -> string const&` | Get pattern string |
| `operator==(string)` | Match against string |

---

## Free Functions

### Namespace `zpt`

```cpp
// Timestamp handling
auto timestamp(string const& iso_date = "") -> timestamp_t;
auto timestamp(timestamp_t ts) -> string;

// Path-based access
auto get(string const& path, json source) -> json;

template<typename T>
auto set(string const& path, T value, json target = undefined) -> json;
```

### User-Defined Literals

```cpp
auto operator"" _JSON(const char* str, size_t len) -> zpt::json;
```

**Example:**
```cpp
auto data = R"({"key": "value", "count": 42})"_JSON;
```

---

## Global Constants

```cpp
namespace zpt {
    inline json undefined{ JSUndefined };  // Undefined value
    inline json array;                      // Array marker for initializers
}
```

---

## See Also

- [JSON User Guide](../guides/json.md)
- [URI API Reference](uri.md)
- [HTTP API Reference](http.md)
