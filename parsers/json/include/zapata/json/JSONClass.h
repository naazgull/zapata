/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file JSONClass.h
 * @brief Core JSON type definitions and classes for the Zapata framework.
 *
 * This header defines the central `zpt::json` class and all supporting types
 * for representing JSON data in C++. The design uses a variant-based approach
 * with shared pointer semantics for efficient copying and reference counting.
 *
 * Key classes:
 * - `zpt::json` - Main dynamic JSON value type with automatic type conversion
 * - `zpt::JSONObj` - JSON object container (key-value pairs)
 * - `zpt::JSONArr` - JSON array container (ordered values)
 * - `zpt::JSONElementT` - Internal variant-based element storage
 * - `zpt::lambda` - Lambda function wrapper for JSON
 * - `zpt::regex` - Regular expression wrapper for JSON
 *
 * @par Example
 * @code
 * // Create JSON objects using initializer lists
 * zpt::json obj = { "name", "John", "age", 30 };
 * zpt::json arr = { zpt::array, 1, 2, 3, 4, 5 };
 *
 * // Access values with automatic type conversion
 * std::string name = obj["name"];
 * int age = obj["age"];
 *
 * // Iterate over elements
 * for (auto [idx, key, value] : obj) {
 *     std::cout << key << ": " << value << std::endl;
 * }
 *
 * // Path-based access
 * zpt::json nested = { "user", { "profile", { "email", "a@b.com" } } };
 * auto email = nested->get_path("user/profile/email", "/");
 * @endcode
 *
 * @see zpt::json
 * @see zpt::JSONObj
 * @see zpt::JSONArr
 */

#pragma once

#include <chrono>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <unordered_map>
#include <variant>
#include <vector>
#include <zapata/allocator.h>
#include <zapata/base/expect.h>
#include <zapata/log/log.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

namespace zpt {

/** @brief Timestamp type representing milliseconds since Unix epoch. */
using timestamp_t = unsigned long long;

/**
 * @brief Enumeration of JSON value types.
 *
 * Each value corresponds to a JSON data type or special Zapata extension type.
 */
enum JSONType {
    JSNil = 0,       ///< Null value (JSON `null`)
    JSBoolean = 1,   ///< Boolean value (`true` or `false`)
    JSInteger = 2,   ///< Integer number (stored as `long long`)
    JSDouble = 3,    ///< Floating-point number (stored as `double`)
    JSString = 4,    ///< String value
    JSDate = 5,      ///< Timestamp value (milliseconds since epoch)
    JSArray = 6,     ///< Array of JSON values
    JSObject = 7,    ///< Object with string keys and JSON values
    JSRegex = 8,     ///< Regular expression (Zapata extension)
    JSLambda = 9,    ///< Lambda function reference (Zapata extension)
    JSUndefined = 10 ///< Undefined value (distinct from null)
};

/**
 * @brief Converts a JSONType enum value to its string representation.
 * @param _type The JSON type to convert.
 * @return String name of the type (e.g., "object", "array", "string").
 */
auto to_string(zpt::JSONType _type) -> std::string;

// Forward declarations
class JSONElementT; ///< Internal JSON element implementation
class JSONObj;      ///< JSON object wrapper
class JSONArr;      ///< JSON array wrapper
class JSONLambda;   ///< Lambda function implementation
class JSONRegex;    ///< Regular expression implementation
class JSONIterator; ///< Iterator for JSON containers
class lambda;       ///< Lambda function wrapper
class json;         ///< Main JSON value class

/** @brief Alias for JSONRegex. */
using regex = JSONRegex;
} // namespace zpt

namespace zpt {

/**
 * @brief Wrapper class for pretty-printed JSON output.
 *
 * Wraps a JSON value and produces formatted, human-readable output with
 * indentation when serialized. Use `zpt::json::pretty()` to create instances.
 *
 * @par Example
 * @code
 * zpt::json obj = { "name", "John", "nested", { "a", 1, "b", 2 } };
 * std::cout << zpt::pretty(obj) << std::endl;
 * // Output:
 * // {
 * //     "name": "John",
 * //     "nested": {
 * //         "a": 1,
 * //         "b": 2
 * //     }
 * // }
 * @endcode
 */
class pretty {
  public:
    /** @brief Copy constructor. */
    pretty(const pretty& _rhs);
    /** @brief Move constructor. */
    pretty(pretty&& _rhs);
    /** @brief Constructs from a string. */
    pretty(std::string const& _rhs);
    /** @brief Constructs from a C-string. */
    pretty(const char* _rhs);
    /**
     * @brief Constructs a pretty-printable wrapper from a JSON-like object.
     * @tparam T Type with a `prettify()` method.
     * @param _rhs Object to wrap.
     */
    template<typename T>
    pretty(T _rhs);
    virtual ~pretty() = default;

    /** @brief Converts to the pretty-printed string representation. */
    operator std::string();

    /** @brief Copy assignment. */
    auto operator=(const pretty& _rhs) -> pretty&;
    /** @brief Move assignment. */
    auto operator=(pretty&& _rhs) -> pretty&;

    /** @brief Accesses the underlying string. */
    auto operator->() -> std::string*;
    /** @brief Dereferences to the underlying string. */
    auto operator*() -> std::string&;

    friend auto operator<<(std::ostream& _out, zpt::pretty _in) -> std::ostream& {
        _out << std::string(_in.__underlying.data());
        return _out;
    }

  private:
    std::string __underlying{ "" };
};
} // namespace zpt

namespace zpt {

/**
 * @brief Dynamic JSON value type with automatic type conversion.
 *
 * The `zpt::json` class is the central type for JSON manipulation in Zapata.
 * It uses shared pointer semantics internally, making copies cheap (reference
 * counting). Values can hold any JSON type including objects, arrays, strings,
 * numbers, booleans, null, and Zapata extensions (regex, lambda).
 *
 * @par Construction
 * @code
 * // From primitives
 * zpt::json s = "hello";
 * zpt::json n = 42;
 * zpt::json b = true;
 *
 * // Object using initializer list (alternating key-value pairs)
 * zpt::json obj = { "name", "John", "age", 30 };
 *
 * // Array using zpt::array marker
 * zpt::json arr = { zpt::array, 1, 2, 3, 4 };
 *
 * // Parse from string
 * zpt::json parsed;
 * parsed.load_from("{\"key\": \"value\"}");
 *
 * // User-defined literal
 * auto j = R"({"x": 10})"_JSON;
 * @endcode
 *
 * @par Type Conversion
 * Values automatically convert to C++ types:
 * @code
 * zpt::json obj = { "count", 42, "name", "test" };
 * int count = obj["count"];           // 42
 * std::string name = obj["name"];     // "test"
 * double d = obj["count"];            // 42.0
 * @endcode
 *
 * @par Iteration
 * @code
 * for (auto [index, key, value] : obj) {
 *     // index: position (size_t)
 *     // key: object key or empty for arrays
 *     // value: zpt::json element
 * }
 * @endcode
 *
 * @par Set Operations
 * - `+` / `+=` : Union of objects/arrays
 * - `-` / `-=` : Difference (remove matching elements)
 * - `|` / `|=` : Merge (deep merge for objects)
 * - `&` / `&=` : Intersection
 *
 * @see zpt::JSONObj
 * @see zpt::JSONArr
 */
class json {
  public:
    /** @brief Map type used internally for objects. */
    using map = std::map<std::string, zpt::json>;
    /** @brief Element tuple: (index, key, value). */
    using element = std::tuple<size_t, std::string, zpt::json>;
    /** @brief Iterator type. */
    using iterator = zpt::JSONIterator;
    /** @brief Const iterator type. */
    using const_iterator = const zpt::JSONIterator;

    /** @brief Callback signature for traverse operations. */
    using traverse_callback =
      std::function<void(std::string const&, zpt::json, std::string const&)>;

    /** @brief Constructs a null JSON value. */
    json();
    /** @brief Constructs a null JSON value. */
    json(std::nullptr_t _rhs);
    /** @brief Constructs from a unique pointer to a JSON element. */
    json(zpt::allocator<zpt::JSONElementT>::shared_pointer _target);
    /**
     * @brief Constructs from an initializer list.
     *
     * If the list starts with `zpt::array`, creates an array.
     * Otherwise creates an object from alternating key-value pairs.
     */
    json(std::initializer_list<zpt::json> _init);
    /** @brief Copy constructor (shared pointer semantics). */
    json(zpt::json const& _rhs);
    /** @brief Move constructor. */
    json(zpt::json&& _rhs);
    /**
     * @brief Constructs from any compatible type.
     * @tparam T Type convertible to a JSON value.
     */
    template<typename T>
    json(T const& _rhs);
    virtual ~json();

    /** @brief Returns the number of elements (for objects/arrays) or 1. */
    auto size() const -> size_t;
    /** @brief Computes a hash value for use in containers. */
    auto hash() const -> size_t;
    /** @brief Returns reference to the underlying element. */
    auto value() -> zpt::JSONElementT&;
    /**
     * @brief Parses JSON from a string.
     * @param _in JSON string to parse.
     * @return Reference to this object.
     * @throws zpt::SyntaxErrorException On parse error.
     */
    auto load_from(std::string const& _in) -> zpt::json&;
    /**
     * @brief Parses JSON from an input stream.
     * @param _in Input stream containing JSON.
     * @return Reference to this object.
     */
    auto load_from(std::istream& _in) -> zpt::json&;
    /** @brief Serializes to an output stream. */
    auto stringify(std::ostream& _out) -> zpt::json&;
    /** @brief Serializes to a string. */
    auto stringify(std::string& _out) -> zpt::json&;
    /** @brief Serializes to an output stream (const version). */
    auto stringify(std::ostream& _out) const -> zpt::json const&;
    /** @brief Serializes to a string (const version). */
    auto stringify(std::string& _out) const -> zpt::json const&;
    /** @brief Returns the JSON string representation. */
    auto stringify() const -> std::string;
    /** @brief Returns the length of the serialized string. */
    auto string_length() const -> size_t;

    /** @brief Returns an iterator to the first element. */
    auto begin() -> zpt::json::iterator;
    /** @brief Returns an iterator past the last element. */
    auto end() -> zpt::json::iterator;
    /** @brief Returns a const iterator to the first element. */
    auto begin() const -> zpt::json::const_iterator;
    /** @brief Returns a const iterator past the last element. */
    auto end() const -> zpt::json::const_iterator;

    /** @name Assignment Operators */
    ///@{
    /** @brief Copy assignment (shared semantics). */
    auto operator=(zpt::json const& _rhs) -> zpt::json&;
    /** @brief Move assignment. */
    auto operator=(zpt::json&& _rhs) -> zpt::json&;
    /** @brief Assignment from iterator element tuple. */
    auto operator=(std::tuple<size_t, std::string, zpt::json> _rhs) -> zpt::json&;
    /** @brief Assignment from initializer list. */
    auto operator=(std::initializer_list<zpt::json> _list) -> zpt::json&;
    /** @brief Assignment from any compatible type. */
    template<typename T>
    auto operator=(T const& _rhs) -> zpt::json&;
    ///@}

    /** @name Element Access */
    ///@{
    /** @brief Accesses the underlying JSONElementT. */
    auto operator->() -> zpt::JSONElementT*;
    /** @brief Dereferences to the underlying JSONElementT. */
    auto operator*() -> zpt::JSONElementT&;
    /** @brief Accesses the underlying JSONElementT (const). */
    auto operator->() const -> zpt::JSONElementT const*;
    /** @brief Dereferences to the underlying JSONElementT (const). */
    auto operator*() const -> zpt::JSONElementT const&;
    ///@}

    /** @name Comparison Operators */
    ///@{
    /** @brief Equality with iterator element tuple. */
    auto operator==(std::tuple<size_t, std::string, zpt::json> _rhs) const -> bool;
    /** @brief Inequality with iterator element tuple. */
    auto operator!=(std::tuple<size_t, std::string, zpt::json> _rhs) const -> bool;
    /** @brief Equality with nullptr (checks if null/undefined). */
    auto operator==(std::nullptr_t _rhs) const -> bool;
    /** @brief Inequality with nullptr. */
    auto operator!=(std::nullptr_t _rhs) const -> bool;
    /** @brief Appends elements from initializer list. */
    auto operator<<(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Equality comparison with any compatible type. */
    template<typename T>
    auto operator==(T _rhs) const -> bool;
    /** @brief Inequality comparison with any compatible type. */
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    /** @brief Less-than comparison. */
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    /** @brief Greater-than comparison. */
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    /** @brief Less-than-or-equal comparison. */
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    /** @brief Greater-than-or-equal comparison. */
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    /** @brief Appends an element (stream insertion). */
    template<typename T>
    auto operator<<(T _in) -> json&;
    /** @brief Accesses element by index or key. */
    template<typename T>
    auto operator[](T _idx) -> json&;
    /** @brief Accesses element by index or key (const). */
    template<typename T>
    auto operator[](T _idx) const -> zpt::json const;
    /** @brief Returns element or undefined if not found (safe access). */
    template<typename T>
    auto operator()(T _idx) const -> zpt::json const;
    ///@}

    /** @name Type Conversion Operators */
    ///@{
    /** @brief Converts to string. */
    operator std::string();
    /** @brief Converts to bool. */
    operator bool();
    /** @brief Converts to int. */
    operator int();
    /** @brief Converts to long. */
    operator long();
    /** @brief Converts to long long. */
    operator long long();
    /** @brief Converts to size_t. */
    operator size_t();
    /** @brief Converts to double. */
    operator double();
#ifdef __LP64__
    /** @brief Converts to unsigned int. */
    operator unsigned int();
#endif
    /** @brief Converts to timestamp. */
    operator zpt::timestamp_t();
    /** @brief Converts to JSON object. */
    operator zpt::JSONObj();
    /** @brief Converts to JSON array. */
    operator zpt::JSONArr();
    /** @brief Converts to JSON object reference. */
    operator zpt::JSONObj&();
    /** @brief Converts to JSON array reference. */
    operator zpt::JSONArr&();
    /** @brief Converts to lambda. */
    operator zpt::lambda();
    /** @brief Converts to regex. */
    operator zpt::regex();
    /** @brief Converts to regex reference. */
    operator zpt::regex&();
    /** @brief Converts to std::regex reference. */
    operator std::regex&();

    /** @brief Converts to string (const). */
    operator std::string() const;
    /** @brief Converts to bool (const). */
    operator bool() const;
    /** @brief Converts to int (const). */
    operator int() const;
    /** @brief Converts to long (const). */
    operator long() const;
    /** @brief Converts to long long (const). */
    operator long long() const;
    /** @brief Converts to size_t (const). */
    operator size_t() const;
    /** @brief Converts to double (const). */
    operator double() const;
#ifdef __LP64__
    /** @brief Converts to unsigned int (const). */
    operator unsigned int() const;
#endif
    /** @brief Converts to timestamp (const). */
    operator zpt::timestamp_t() const;
    /** @brief Converts to JSON object (const). */
    operator zpt::JSONObj() const;
    /** @brief Converts to JSON array (const). */
    operator zpt::JSONArr() const;
    /** @brief Converts to JSON object reference (const). */
    operator zpt::JSONObj&() const;
    /** @brief Converts to JSON array reference (const). */
    operator zpt::JSONArr&() const;
    /** @brief Converts to lambda (const). */
    operator zpt::lambda() const;
    /** @brief Converts to regex (const). */
    operator zpt::regex() const;
    /** @brief Converts to regex reference (const). */
    operator zpt::regex&() const;
    /** @brief Converts to std::regex reference (const). */
    operator std::regex&() const;
    ///@}

    /** @name Set Operations */
    ///@{
    /** @brief Returns union of this and the initializer list. */
    auto operator+(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief In-place union with initializer list. */
    auto operator+=(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Returns difference (elements in this but not in the list). */
    auto operator-(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief In-place difference. */
    auto operator-=(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Returns strict union (non-overlapping merge). */
    auto operator/(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief Returns deep merge of this and the list. */
    auto operator|(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief In-place deep merge. */
    auto operator|=(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Returns intersection of this and the list. */
    auto operator&(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief In-place intersection. */
    auto operator&=(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Returns union of this and another JSON value. */
    auto operator+(zpt::json _rhs) const -> json;
    /** @brief In-place union. */
    auto operator+=(zpt::json _rhs) -> json&;
    /** @brief Returns difference. */
    auto operator-(zpt::json _rhs) const -> json;
    /** @brief In-place difference. */
    auto operator-=(zpt::json _rhs) -> json&;
    /** @brief Returns strict union. */
    auto operator/(zpt::json _rhs) const -> json;
    /** @brief Returns deep merge. */
    auto operator|(zpt::json _rhs) const -> json;
    /** @brief In-place deep merge. */
    auto operator|=(zpt::json _rhs) -> json&;
    /** @brief Returns intersection. */
    auto operator&(zpt::json _rhs) const -> json;
    /** @brief In-place intersection. */
    auto operator&=(zpt::json _rhs) -> json&;
    ///@}

    friend auto operator>>(std::istream& _in, zpt::json& _out) -> std::istream& {
        _out.load_from(_in);
        return _in;
    }

    friend auto operator<<(std::ostream& _out, zpt::json _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

    /**
     * @brief Extracts JSON data from a delegate object.
     * @tparam T Type with `get_json()` method.
     */
    template<typename T>
    static auto data(const T _delegate) -> zpt::json;
    /** @brief Parses a JSON string (static version). */
    static auto parse_json_str(std::string const& _in) -> zpt::json;
    /** @brief Converts escape sequences to Unicode in-place. */
    static auto to_unicode(std::string& _str) -> void;
    /** @brief Creates an empty JSON object. */
    static auto object() -> zpt::json;
    /** @brief Creates an empty JSON array. */
    static auto array() -> zpt::json;
    /** @name Static Type Factories
     *  Create JSON values with explicit types.
     */
    ///@{
    /** @brief Returns pretty-printed string of a JSON-like object. */
    template<typename T>
    static auto pretty(T _e) -> std::string;
    /** @brief Creates a JSON string value. */
    template<typename T>
    static auto string(T _e) -> zpt::json;
    /** @brief Creates a JSON unsigned integer value. */
    template<typename T>
    static auto uinteger(T _e) -> zpt::json;
    /** @brief Creates a JSON integer value. */
    template<typename T>
    static auto integer(T _e) -> zpt::json;
    /** @brief Creates a JSON floating-point value. */
    template<typename T>
    static auto floating(T _e) -> zpt::json;
    /** @brief Creates a JSON unsigned long value. */
    template<typename T>
    static auto ulong(T _e) -> zpt::json;
    /** @brief Creates a JSON boolean value. */
    template<typename T>
    static auto boolean(T _e) -> zpt::json;
    /** @brief Creates a JSON date from an ISO 8601 string. */
    static auto date(std::string const& _e) -> zpt::json;
    /** @brief Creates a JSON date with the current timestamp. */
    static auto date() -> zpt::json;
    /** @brief Creates a JSON date from a numeric timestamp. */
    template<typename T>
    static auto date(T _e) -> zpt::json;
    /** @brief Creates a JSON lambda from a callable. */
    template<typename T>
    static auto lambda(T _e) -> zpt::json;
    /** @brief Creates a JSON lambda with name and argument count. */
    static auto lambda(std::string const& _name, unsigned short _n_args) -> zpt::json;
    /** @brief Creates a JSON regex value. */
    template<typename T>
    static auto regex(T _e) -> zpt::json;
    ///@}

    /** @name Type Introspection
     *  Returns the JSONType for a given C++ value.
     */
    ///@{
    /** @brief Returns JSString. */
    static auto type_of(std::string const& _value) -> zpt::JSONType;
    /** @brief Returns JSBoolean. */
    static auto type_of(bool _value) -> zpt::JSONType;
    /** @brief Returns JSInteger. */
    static auto type_of(int _value) -> zpt::JSONType;
    /** @brief Returns JSInteger. */
    static auto type_of(long _value) -> zpt::JSONType;
    /** @brief Returns JSInteger. */
    static auto type_of(long long _value) -> zpt::JSONType;
    /** @brief Returns JSInteger. */
    static auto type_of(size_t _value) -> zpt::JSONType;
    /** @brief Returns JSDouble. */
    static auto type_of(double _value) -> zpt::JSONType;
#ifdef __LP64__
    /** @brief Returns JSInteger. */
    static auto type_of(unsigned int _value) -> zpt::JSONType;
#endif
    /** @brief Returns the type stored in the element. */
    static auto type_of(zpt::JSONElementT& _value) -> zpt::JSONType;
    /** @brief Returns JSDate. */
    static auto type_of(zpt::timestamp_t _value) -> zpt::JSONType;
    /** @brief Returns JSString. */
    static auto type_of(zpt::pretty _value) -> zpt::JSONType;
    /** @brief Returns JSObject. */
    static auto type_of(zpt::JSONObj _value) -> zpt::JSONType;
    /** @brief Returns JSArray. */
    static auto type_of(zpt::JSONArr _value) -> zpt::JSONType;
    /** @brief Returns JSObject. */
    static auto type_of(zpt::JSONObj& _value) -> zpt::JSONType;
    /** @brief Returns JSArray. */
    static auto type_of(zpt::JSONArr& _value) -> zpt::JSONType;
    /** @brief Returns JSLambda. */
    static auto type_of(zpt::lambda _value) -> zpt::JSONType;
    /** @brief Returns JSRegex. */
    static auto type_of(zpt::regex& _value) -> zpt::JSONType;
    /** @brief Returns the type of the underlying JSON value. */
    static auto type_of(zpt::json& _value) -> zpt::JSONType;
    ///@}

    /**
     * @brief Recursively traverses a JSON document.
     * @param _document Document to traverse.
     * @param _callback Called for each element with (path, value, key).
     */
    static auto traverse(zpt::json _document, zpt::json::traverse_callback _callback) -> void;
    /**
     * @brief Flattens a nested document to a single-level object.
     * @param _document Document to flatten.
     * @return Object with dot-separated paths as keys.
     */
    static auto flatten(zpt::json _document) -> zpt::json;
    /** @brief Finds a value in an iterator range. */
    static auto find(zpt::json::iterator _begin, zpt::json::iterator _end, zpt::json _to_find)
      -> zpt::json::iterator;
    /** @brief Finds a value in a JSON element. */
    static auto find(zpt::JSONElementT const& _to_search, zpt::json _to_find)
      -> zpt::json::iterator;
    /** @brief Checks if an element contains a value. */
    static auto contains(zpt::JSONElementT const& _to_search, zpt::json _to_find) -> bool;

  private:
    std::shared_ptr<zpt::JSONElementT> __underlying{ nullptr };

    json(std::tuple<size_t, std::string, zpt::json> _rhs);
    auto strict_union(zpt::json _rhs) -> void;
    auto strict_intersection(zpt::json _rhs) -> void;

    static auto traverse(zpt::json _document,
                         zpt::json::traverse_callback _callback,
                         std::string _path) -> void;
};
} // namespace zpt

/**
 * @brief std::formatter specialization for zpt::json.
 *
 * Enables using JSON values with std::format and std::print (C++20/23).
 *
 * @par Example
 * @code
 * zpt::json obj = { "name", "John" };
 * std::cout << std::format("Data: {}", obj) << std::endl;
 * @endcode
 */
template<>
struct std::formatter<zpt::json> {
    constexpr auto parse(std::format_parse_context& _context) { return _context.begin(); }

    auto format(zpt::json const& _in, std::format_context& _context) const {
        std::ostringstream _out;
        _in.stringify(_out);
        return std::format_to(_context.out(), "{}", _out.str());
    }
};

namespace zpt {

/**
 * @brief Bidirectional iterator for JSON containers.
 *
 * Provides iteration over JSON objects and arrays. Each dereference yields
 * a tuple of (index, key, value) where key is empty for arrays.
 *
 * @par Example
 * @code
 * zpt::json obj = { "a", 1, "b", 2 };
 * for (auto it = obj.begin(); it != obj.end(); ++it) {
 *     auto [idx, key, value] = *it;
 *     std::cout << key << " = " << value << "\n";
 * }
 * @endcode
 */
class JSONIterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = zpt::json::element; ///< Tuple of (index, key, value)
    using pointer = zpt::json::element;
    using reference = zpt::json::element;
    using iterator_category = std::bidirectional_iterator_tag;

    /**
     * @brief Constructs an iterator at a specific position.
     * @param _target JSON value to iterate over.
     * @param _pos Starting position (0 for begin, size() for end).
     */
    explicit JSONIterator(zpt::json const& _target, size_t _pos);
    /** @brief Copy constructor. */
    JSONIterator(JSONIterator const& _rhs);
    /** @brief Move constructor. */
    JSONIterator(JSONIterator&& _rhs);
    /** @brief Destructor. */
    virtual ~JSONIterator() = default;

    /** @brief Copy assignment. */
    auto operator=(JSONIterator const& _rhs) -> JSONIterator&;
    /** @brief Move assignment. */
    auto operator=(JSONIterator&& _rhs) -> JSONIterator&;
    /** @brief Pre-increment: advances to next element. */
    auto operator++() -> JSONIterator&;
    /** @brief Dereference: returns (index, key, value) tuple. */
    auto operator*() -> reference;

    /** @brief Post-increment: advances and returns previous position. */
    auto operator++(int) -> JSONIterator;
    /** @brief Arrow operator: returns (index, key, value) tuple. */
    auto operator->() -> pointer;
    /** @brief Equality comparison. */
    auto operator==(JSONIterator const& _rhs) const -> bool;
    /** @brief Inequality comparison. */
    auto operator!=(JSONIterator const& _rhs) const -> bool;

    /** @brief Pre-decrement: moves to previous element. */
    auto operator--() -> JSONIterator&;
    /** @brief Post-decrement: moves back and returns previous position. */
    auto operator--(int) -> JSONIterator;
    // END / BIDIRECTIOANL ITERATOR METHODS //

    friend auto operator<<(std::ostream& _out, zpt::JSONIterator& _in) -> std::ostream& {
        _out << _in.__index << std::flush;
        return _out;
    }

  private:
    zpt::json __target;
    size_t __index;
    zpt::json::map::const_iterator __iterator;
};
} // namespace zpt

namespace zpt {
/** @brief Global undefined JSON value. Use to represent missing/unset values. */
inline zpt::json undefined{ zpt::JSUndefined };
/** @brief Marker for array construction in initializer lists. */
inline zpt::json array;
} // namespace zpt

namespace zpt {

/**
 * @brief Internal implementation class for JSON objects.
 *
 * Stores key-value pairs where keys are strings and values are `zpt::json`.
 * Keys are stored in insertion order. Use `zpt::JSONObj` wrapper for
 * reference-counted access.
 *
 * @note Prefer using `zpt::json` directly rather than this internal class.
 *
 * @see zpt::JSONObj
 * @see zpt::json
 */
class JSONObjT {
  public:
    /** @brief Constructs an empty JSON object. */
    JSONObjT();
    /** @brief Destructor. */
    virtual ~JSONObjT();

    /** @name Serialization */
    ///@{
    /** @brief Serializes to compact JSON string. */
    virtual auto stringify(std::string& _out) -> zpt::JSONObjT&;
    virtual auto stringify(std::ostream& _out) -> zpt::JSONObjT&;
    virtual auto stringify(std::string& _out) const -> zpt::JSONObjT const&;
    virtual auto stringify(std::ostream& _out) const -> zpt::JSONObjT const&;
    /** @brief Returns length of serialized string. */
    virtual auto string_length() const -> size_t;

    /** @brief Serializes to formatted JSON with indentation. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> zpt::JSONObjT const&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> zpt::JSONObjT const&;
    ///@}

    /** @name Modification */
    ///@{
    /** @brief Pushes a key (next push must be a value). */
    virtual auto push(std::string const& _name) -> zpt::JSONObjT&;
    /** @brief Pushes a value for the pending key. */
    virtual auto push(zpt::allocator<zpt::JSONElementT>::shared_pointer _value) -> JSONObjT&;
    virtual auto push(zpt::json const& _value) -> zpt::JSONObjT&;

    /** @brief Removes element by index or key. */
    virtual auto pop(int _idx) -> zpt::JSONObjT&;
    virtual auto pop(size_t _idx) -> zpt::JSONObjT&;
    virtual auto pop(const char* _idx) -> zpt::JSONObjT&;
    virtual auto pop(std::string const& _idx) -> zpt::JSONObjT&;
    ///@}

    /** @brief Returns key at given position. */
    virtual auto key_for(size_t _idx) const -> std::string;
    /** @brief Returns true if a key was pushed without a value. */
    virtual auto has_pending_key() const -> bool;

    /** @name Path-Based Access */
    ///@{
    /**
     * @brief Retrieves value at nested path.
     * @param _path Dot-separated path (e.g., "user.profile.name").
     * @param _separator Path separator character.
     * @return Value at path or undefined.
     */
    auto get_path(std::string const& _path, std::string const& _separator = ".") -> zpt::json;
    /**
     * @brief Sets value at nested path, creating intermediate objects.
     * @param _path Dot-separated path.
     * @param _value Value to set.
     * @param _separator Path separator.
     */
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> zpt::JSONObjT&;
    /** @brief Deletes value at path. */
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> JSONObjT&;
    ///@}

    /** @brief Creates a deep copy of this object. */
    auto clone() const -> zpt::json;

    /** @name Access Operators */
    ///@{
    /** @brief Accesses underlying map. */
    auto operator->() -> zpt::json::map*;
    /** @brief Dereferences to underlying map. */
    auto operator*() -> zpt::json::map&;
    /** @brief Accesses underlying map (const). */
    auto operator->() const -> zpt::json::map const*;
    /** @brief Dereferences to underlying map (const). */
    auto operator*() const -> zpt::json::map const&;
    ///@}

    /** @name Comparison Operators */
    ///@{
    /** @brief Equality comparison. */
    auto operator==(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Equality comparison with wrapper. */
    auto operator==(zpt::JSONObj const& _in) const -> bool;
    /** @brief Equality comparison with any type. */
    template<typename T>
    auto operator==(T _in) const -> bool;
    /** @brief Inequality comparison. */
    auto operator!=(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Inequality comparison with wrapper. */
    auto operator!=(zpt::JSONObj const& _in) const -> bool;
    /** @brief Inequality comparison with any type. */
    template<typename T>
    auto operator!=(T _in) const -> bool;
    /** @brief Less-than comparison. */
    auto operator<(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Less-than comparison with wrapper. */
    auto operator<(zpt::JSONObj const& _in) const -> bool;
    /** @brief Less-than comparison with any type. */
    template<typename T>
    auto operator<(T _in) const -> bool;
    /** @brief Greater-than comparison. */
    auto operator>(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Greater-than comparison with wrapper. */
    auto operator>(zpt::JSONObj const& _in) const -> bool;
    /** @brief Greater-than comparison with any type. */
    template<typename T>
    auto operator>(T _in) const -> bool;
    /** @brief Greater-than-or-equal comparison. */
    auto operator>=(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison with wrapper. */
    auto operator>=(zpt::JSONObj const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison with any type. */
    template<typename T>
    auto operator>=(T _in) const -> bool;
    /** @brief Less-than-or-equal comparison. */
    auto operator<=(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison with wrapper. */
    auto operator<=(zpt::JSONObj const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison with any type. */
    template<typename T>
    auto operator<=(T _in) const -> bool;
    ///@}

    /** @name Subscript Operators */
    ///@{
    /** @brief Access by integer index. */
    auto operator[](int _idx) -> zpt::json&;
    /** @brief Access by size_t index. */
    auto operator[](size_t _idx) -> zpt::json&;
    /** @brief Access by C-string key. */
    auto operator[](const char* _idx) -> zpt::json&;
    /** @brief Access by string key. */
    auto operator[](std::string const& _idx) -> zpt::json&;
    /** @brief Access by integer index (const). */
    auto operator[](int _idx) const -> zpt::json const;
    /** @brief Access by size_t index (const). */
    auto operator[](size_t _idx) const -> zpt::json const;
    /** @brief Access by C-string key (const). */
    auto operator[](const char* _idx) const -> zpt::json const;
    /** @brief Access by string key (const). */
    auto operator[](std::string const& _idx) const -> zpt::json const;
    ///@}

    friend auto operator<<(std::ostream& _out, zpt::JSONObjT& _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

  private:
    std::string __name{ "" };
    zpt::json::map __underlying;
};
} // namespace zpt

namespace zpt {

/**
 * @brief Internal implementation class for JSON arrays.
 *
 * Stores an ordered sequence of `zpt::json` values. Use `zpt::JSONArr`
 * wrapper for reference-counted access.
 *
 * @note Prefer using `zpt::json` directly rather than this internal class.
 *
 * @see zpt::JSONArr
 * @see zpt::json
 */
class JSONArrT {
  public:
    /** @brief Constructs an empty JSON array. */
    JSONArrT();
    /** @brief Destructor. */
    virtual ~JSONArrT();

    /** @name Serialization */
    ///@{
    /** @brief Serializes to compact JSON string. */
    virtual auto stringify(std::string& _out) -> zpt::JSONArrT&;
    virtual auto stringify(std::ostream& _out) -> zpt::JSONArrT&;
    virtual auto stringify(std::string& _out) const -> zpt::JSONArrT const&;
    virtual auto stringify(std::ostream& _out) const -> zpt::JSONArrT const&;
    /** @brief Returns length of serialized string. */
    virtual auto string_length() const -> size_t;

    /** @brief Serializes to formatted JSON with indentation. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> zpt::JSONArrT const&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> zpt::JSONArrT const&;
    ///@}

    /** @name Modification */
    ///@{
    /** @brief Appends a value to the array. */
    virtual auto push(zpt::allocator<zpt::JSONElementT>::shared_pointer _value) -> zpt::JSONArrT&;
    virtual auto push(zpt::json const& _value) -> zpt::JSONArrT&;

    /** @brief Removes element by index. */
    virtual auto pop(int _idx) -> zpt::JSONArrT&;
    virtual auto pop(size_t _idx) -> zpt::JSONArrT&;
    virtual auto pop(const char* _idx) -> zpt::JSONArrT&;
    virtual auto pop(std::string const& _idx) -> zpt::JSONArrT&;

    /** @brief Sorts array elements using default comparison. */
    virtual auto sort() -> zpt::JSONArrT&;
    /**
     * @brief Sorts array elements using custom comparator.
     * @param _comparator Returns true if first arg should come before second.
     */
    virtual auto sort(std::function<bool(zpt::json, zpt::json)> _comparator) -> zpt::JSONArrT&;
    ///@}

    /** @name Path-Based Access */
    ///@{
    /**
     * @brief Retrieves value at nested path.
     * @param _path Path with numeric indices (e.g., "0.name" for first element's name).
     * @param _separator Path separator character.
     */
    auto get_path(std::string const& _path, std::string const& _separator = ".") -> zpt::json;
    /** @brief Sets value at nested path. */
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> zpt::JSONArrT&;
    /** @brief Deletes value at path. */
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> zpt::JSONArrT&;
    ///@}

    /** @brief Creates a deep copy of this array. */
    auto clone() const -> zpt::json;

    /** @name Access Operators */
    ///@{
    /** @brief Accesses underlying vector. */
    auto operator->() -> std::vector<zpt::json>*;
    /** @brief Dereferences to underlying vector. */
    auto operator*() -> std::vector<zpt::json>&;
    /** @brief Accesses underlying vector (const). */
    auto operator->() const -> std::vector<zpt::json> const*;
    /** @brief Dereferences to underlying vector (const). */
    auto operator*() const -> std::vector<zpt::json> const&;
    ///@}

    /** @name Comparison Operators */
    ///@{
    /** @brief Equality comparison. */
    auto operator==(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Equality comparison with wrapper. */
    auto operator==(zpt::JSONArr const& _in) const -> bool;
    /** @brief Equality comparison with any type. */
    template<typename T>
    auto operator==(T _in) const -> bool;
    /** @brief Inequality comparison. */
    auto operator!=(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Inequality comparison with wrapper. */
    auto operator!=(zpt::JSONArr const& _in) const -> bool;
    /** @brief Inequality comparison with any type. */
    template<typename T>
    auto operator!=(T _in) const -> bool;
    /** @brief Less-than comparison. */
    auto operator<(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Less-than comparison with wrapper. */
    auto operator<(zpt::JSONArr const& _in) const -> bool;
    /** @brief Less-than comparison with any type. */
    template<typename T>
    auto operator<(T _in) const -> bool;
    /** @brief Greater-than comparison. */
    auto operator>(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Greater-than comparison with wrapper. */
    auto operator>(zpt::JSONArr const& _in) const -> bool;
    /** @brief Greater-than comparison with any type. */
    template<typename T>
    auto operator>(T _in) const -> bool;
    /** @brief Less-than-or-equal comparison. */
    auto operator<=(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison with wrapper. */
    auto operator<=(zpt::JSONArr const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison with any type. */
    template<typename T>
    auto operator<=(T _in) const -> bool;
    /** @brief Greater-than-or-equal comparison. */
    auto operator>=(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison with wrapper. */
    auto operator>=(zpt::JSONArr const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison with any type. */
    template<typename T>
    auto operator>=(T _in) const -> bool;
    ///@}

    /** @name Subscript Operators */
    ///@{
    /** @brief Access by integer index. */
    auto operator[](int _idx) -> zpt::json&;
    /** @brief Access by size_t index. */
    auto operator[](size_t _idx) -> zpt::json&;
    /** @brief Access by C-string key. */
    auto operator[](const char* _idx) -> zpt::json&;
    /** @brief Access by string key. */
    auto operator[](std::string const& _idx) -> zpt::json&;
    /** @brief Access by integer index (const). */
    auto operator[](int _idx) const -> zpt::json const;
    /** @brief Access by size_t index (const). */
    auto operator[](size_t _idx) const -> zpt::json const;
    /** @brief Access by C-string key (const). */
    auto operator[](const char* _idx) const -> zpt::json const;
    /** @brief Access by string key (const). */
    auto operator[](std::string const& _idx) const -> zpt::json const;
    ///@}

    friend auto operator<<(std::ostream& _out, zpt::JSONArrT& _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

  private:
    std::vector<zpt::json> __underlying;
};
} // namespace zpt

namespace zpt {

/**
 * @brief Reference-counted JSON object wrapper.
 *
 * Provides shared pointer semantics for JSON objects. Multiple `JSONObj`
 * instances can reference the same underlying object data. Use `operator->`
 * to access the underlying `JSONObjT` methods.
 *
 * @par Example
 * @code
 * zpt::JSONObj obj;
 * obj << "name" << "John" << "age" << 30;
 * std::cout << obj["name"] << std::endl;  // "John"
 * @endcode
 *
 * @see zpt::JSONObjT
 * @see zpt::json
 */
class JSONObj {
  public:
    /** @brief Constructs an empty JSON object wrapper. */
    JSONObj();
    /** @brief Copy constructor (shared semantics). */
    JSONObj(const zpt::JSONObj& _rhs);
    /** @brief Move constructor. */
    JSONObj(zpt::JSONObj&& _rhs);
    /** @brief Takes ownership of a raw JSONObjT pointer. */
    JSONObj(zpt::JSONObjT* _target);
    /** @brief Destructor. */
    virtual ~JSONObj();

    /** @brief Computes hash value for use in containers. */
    auto hash() const -> size_t;

    /** @brief Copy assignment. */
    auto operator=(const zpt::JSONObj& _rhs) -> zpt::JSONObj&;
    /** @brief Move assignment. */
    auto operator=(zpt::JSONObj&& _rhs) -> zpt::JSONObj&;

    /** @brief Accesses underlying JSONObjT. */
    auto operator->() -> zpt::JSONObjT*;
    /** @brief Dereferences to underlying JSONObjT. */
    auto operator*() -> zpt::JSONObjT&;
    /** @brief Accesses underlying JSONObjT (const). */
    auto operator->() const -> zpt::JSONObjT const*;
    /** @brief Dereferences to underlying JSONObjT (const). */
    auto operator*() const -> zpt::JSONObjT const&;

    /** @brief Converts to JSON string. */
    operator std::string();
    /** @brief Converts to pretty-printed string. */
    operator zpt::pretty();
    /** @brief Equality comparison. */
    template<typename T>
    auto operator==(T _rhs) const -> bool;
    /** @brief Inequality comparison. */
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    /** @brief Less-than comparison. */
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    /** @brief Greater-than comparison. */
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    /** @brief Less-than-or-equal comparison. */
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    /** @brief Greater-than-or-equal comparison. */
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    /** @brief Pushes a key string. */
    auto operator<<(std::string const& _in) -> zpt::JSONObj&;
    /** @brief Pushes a C-string key. */
    auto operator<<(const char* _in) -> zpt::JSONObj&;
    /** @brief Pushes key-value pairs from initializer list. */
    auto operator<<(std::initializer_list<zpt::json> _list) -> zpt::JSONObj&;
    /** @brief Pushes a value for the pending key. */
    template<typename T>
    auto operator<<(T _in) -> zpt::JSONObj&;
    /** @brief Accesses element by index or key. */
    template<typename T>
    auto operator[](T _idx) -> zpt::json&;
    /** @brief Accesses element by index or key (const). */
    template<typename T>
    auto operator[](T _idx) const -> zpt::json const;

    friend auto operator<<(std::ostream& _out, JSONObj& _in) -> std::ostream& {
        _in.__underlying->stringify(_out);
        return _out;
    }

  private:
    std::shared_ptr<zpt::JSONObjT> __underlying{ nullptr };
};
} // namespace zpt

namespace zpt {

/**
 * @brief Reference-counted JSON array wrapper.
 *
 * Provides shared pointer semantics for JSON arrays. Multiple `JSONArr`
 * instances can reference the same underlying array data. Use `operator->`
 * to access the underlying `JSONArrT` methods.
 *
 * @par Example
 * @code
 * zpt::JSONArr arr;
 * arr << 1 << 2 << 3 << "four";
 * std::cout << arr[0] << std::endl;  // 1
 * @endcode
 *
 * @see zpt::JSONArrT
 * @see zpt::json
 */
class JSONArr {
  public:
    /** @brief Constructs an empty JSON array wrapper. */
    JSONArr();
    /** @brief Copy constructor (shared semantics). */
    JSONArr(const JSONArr& _rhs);
    /** @brief Move constructor. */
    JSONArr(JSONArr&& _rhs);
    /** @brief Takes ownership of a raw JSONArrT pointer. */
    JSONArr(zpt::JSONArrT* _target);
    /** @brief Destructor. */
    virtual ~JSONArr();

    /** @brief Computes hash value for use in containers. */
    auto hash() const -> size_t;

    /** @brief Converts to JSON string. */
    operator std::string();
    /** @brief Converts to pretty-printed string. */
    operator zpt::pretty();

    /** @brief Copy assignment. */
    auto operator=(const zpt::JSONArr& _rhs) -> zpt::JSONArr&;
    /** @brief Move assignment. */
    auto operator=(zpt::JSONArr&& _rhs) -> zpt::JSONArr&;

    /** @brief Accesses underlying JSONArrT. */
    auto operator->() -> zpt::JSONArrT*;
    /** @brief Dereferences to underlying JSONArrT. */
    auto operator*() -> zpt::JSONArrT&;
    /** @brief Accesses underlying JSONArrT (const). */
    auto operator->() const -> zpt::JSONArrT const*;
    /** @brief Dereferences to underlying JSONArrT (const). */
    auto operator*() const -> zpt::JSONArrT const&;

    /** @brief Equality comparison. */
    template<typename T>
    auto operator==(T _rhs) const -> bool;
    /** @brief Inequality comparison. */
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    /** @brief Less-than comparison. */
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    /** @brief Greater-than comparison. */
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    /** @brief Less-than-or-equal comparison. */
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    /** @brief Greater-than-or-equal comparison. */
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    /** @brief Appends elements from initializer list. */
    auto operator<<(std::initializer_list<zpt::json> _list) -> JSONArr&;
    /** @brief Appends an element to the array. */
    template<typename T>
    auto operator<<(T _in) -> JSONArr&;
    /** @brief Accesses element by index. */
    template<typename T>
    auto operator[](T _idx) -> json&;
    /** @brief Accesses element by index (const). */
    template<typename T>
    auto operator[](T _idx) const -> json const;

    friend auto operator<<(std::ostream& _out, JSONArr& _in) -> std::ostream& {
        _in.__underlying->stringify(_out);
        return _out;
    }

  private:
    std::shared_ptr<zpt::JSONArrT> __underlying{ nullptr };
};
} // namespace zpt

namespace zpt {
/** @brief Shared pointer to string for internal use. */
using JSONStr = std::shared_ptr<std::string>;
} // namespace zpt

namespace zpt {

/**
 * @brief Regular expression wrapper for JSON values.
 *
 * Wraps `std::regex` for storage in JSON documents. The original regex
 * pattern string is preserved for serialization. Serialized format is
 * `/pattern/` (JavaScript-style).
 *
 * @par Example
 * @code
 * zpt::json pattern = zpt::json::regex("^[a-z]+$");
 * std::regex& rx = pattern;  // Implicit conversion
 * if (std::regex_match("hello", rx)) { ... }
 * @endcode
 *
 * @see zpt::json::regex()
 */
class JSONRegex {
  public:
    /** @brief Constructs an empty regex. */
    JSONRegex();
    /** @brief Copy constructor. */
    JSONRegex(const zpt::JSONRegex& _rhs);
    /** @brief Move constructor. */
    JSONRegex(zpt::JSONRegex&& _rhs);
    /**
     * @brief Constructs from a regex pattern string.
     * @param _target Regular expression pattern.
     * @throws std::regex_error If pattern is invalid.
     */
    JSONRegex(std::string const& _target);
    /** @brief Destructor. */
    virtual ~JSONRegex();

    /** @brief Copy assignment. */
    auto operator=(const zpt::JSONRegex& _rhs) -> zpt::JSONRegex&;
    /** @brief Move assignment. */
    auto operator=(zpt::JSONRegex&& _rhs) -> zpt::JSONRegex&;

    /** @brief Access underlying std::regex. */
    auto operator->() -> std::regex*;
    /** @brief Dereference to underlying std::regex. */
    auto operator*() -> std::regex&;
    /** @brief Access underlying std::regex (const). */
    auto operator->() const -> std::regex const*;
    /** @brief Dereference to underlying std::regex (const). */
    auto operator*() const -> std::regex const&;

    /** @brief Converts to pretty-printed pattern string. */
    operator zpt::pretty();
    /** @brief Converts to std::regex reference. */
    operator std::regex&();

    /** @brief Compares regex patterns. */
    auto operator==(zpt::regex _rhs) const -> bool;
    auto operator==(zpt::json _rhs) const -> bool;
    /** @brief Tests if string matches this regex. */
    auto operator==(std::string const& _rhs) const -> bool;
    auto operator!=(zpt::regex _rhs) const -> bool;
    auto operator!=(zpt::json _rhs) const -> bool;
    auto operator!=(std::string const& _rhs) const -> bool;

    friend auto operator<<(std::ostream& _out, JSONRegex& _in) -> std::ostream& {
        _out << "/" << _in.to_string() << "/" << std::flush;
        return _out;
    }

    /** @brief Returns the original pattern string. */
    auto to_string() const -> std::string const&;

  private:
    std::string __underlying_original{ "" };
    std::shared_ptr<std::regex> __underlying{ nullptr };
};
} // namespace zpt

namespace zpt {

/**
 * @brief Internal context storage for lambda execution.
 *
 * Wraps an opaque pointer to provide execution context when invoking
 * JSON lambda functions.
 *
 * @see zpt::context
 * @see zpt::lambda
 */
class JSONContext {
  public:
    /** @brief Constructs context wrapping a pointer. */
    JSONContext(void* _target);
    virtual ~JSONContext();

    /** @brief Returns the wrapped pointer. */
    virtual auto unpack() -> void*;

  private:
    void* __target{ nullptr };
};
} // namespace zpt

namespace zpt {

/**
 * @brief Reference-counted execution context for lambda functions.
 *
 * Provides shared pointer semantics for passing context to JSON lambda
 * function invocations. The context can wrap any pointer type for
 * application-specific use.
 *
 * @see zpt::lambda
 * @see zpt::JSONContext
 */
class context {
  public:
    /** @brief Constructs context wrapping a pointer. */
    context(void* _target);
    context(const context& _rhs);
    context(context&& _rhs);
    virtual ~context();

    auto operator->() -> zpt::JSONContext*;
    auto operator*() -> zpt::JSONContext&;
    auto operator->() const -> zpt::JSONContext const*;
    auto operator*() const -> zpt::JSONContext const&;

    auto operator=(const zpt::context& _rhs) -> zpt::context&;
    auto operator=(zpt::context&& _rhs) -> zpt::context&;

  private:
    std::shared_ptr<zpt::JSONContext> __underlying{ nullptr };
};
} // namespace zpt

namespace zpt {

/**
 * @brief Function signature for JSON lambda callbacks.
 * @param args JSON array of arguments.
 * @param n_args Number of expected arguments.
 * @param ctx Execution context.
 * @return JSON result value.
 */
using symbol = std::function<zpt::json(zpt::json, unsigned short, zpt::context)>;

/** @brief Global registry mapping lambda names to implementations. */
using symbol_table = std::shared_ptr<
  std::unordered_map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>>;

/** @brief Global lambda symbol table instance. */
inline symbol_table __lambdas{
    new std::unordered_map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>{}
};
} // namespace zpt

namespace zpt {

/**
 * @brief Reference-counted lambda function wrapper for JSON.
 *
 * Allows storing callable functions in JSON values. Lambdas are registered
 * globally by name and can be invoked with JSON arguments. Serialized
 * format is `"name/n_args"`.
 *
 * @par Registration and Invocation
 * @code
 * // Register a lambda
 * zpt::lambda::add("add", 2, [](zpt::json args, unsigned short, zpt::context) {
 *     return int(args[0]) + int(args[1]);
 * });
 *
 * // Store in JSON and invoke
 * zpt::json func = zpt::json::lambda("add", 2);
 * zpt::json result = zpt::lambda::call("add", { zpt::array, 1, 2 }, ctx);  // 3
 * @endcode
 *
 * @see zpt::json::lambda()
 * @see zpt::JSONLambda
 */
class lambda : public std::shared_ptr<zpt::JSONLambda> {
  public:
    lambda();
    lambda(std::shared_ptr<zpt::JSONLambda> _target);
    lambda(zpt::lambda& _target);
    lambda(zpt::JSONLambda* _target);
    /**
     * @brief Constructs from signature string "name/n_args".
     * @param _signature Lambda signature.
     */
    lambda(std::string const& _signature);
    /**
     * @brief Constructs from name and argument count.
     * @param _name Lambda name.
     * @param _n_args Number of arguments.
     */
    lambda(std::string const& _name, unsigned short _n_args);
    virtual ~lambda();

    /** @brief Computes hash for use in containers. */
    auto hash() const -> size_t;

    /**
     * @brief Invokes the lambda with arguments.
     * @param _args JSON array of arguments.
     * @param _ctx Execution context.
     * @return JSON result.
     */
    virtual auto operator()(zpt::json _args, zpt::context _ctx) -> zpt::json;
    virtual auto operator()(zpt::json _args, zpt::context _ctx) const -> zpt::json;

    /** @name Static Registration Methods */
    ///@{
    /** @brief Registers a lambda by signature string. */
    static auto add(std::string const& _signature, zpt::symbol _lambda) -> void;
    /** @brief Registers a lambda by name and argument count. */
    static auto add(std::string const& _name, unsigned short _n_args, zpt::symbol _lambda) -> void;
    ///@}

    /**
     * @brief Calls a registered lambda by name.
     * @param _name Lambda name.
     * @param _args JSON array of arguments.
     * @param _ctx Execution context.
     * @return JSON result.
     */
    static auto call(std::string const& _name, zpt::json _args, zpt::context _ctx) -> zpt::json;

    /** @brief Creates signature string from name and argument count. */
    static auto stringify(std::string const& _name, unsigned short _n_args) -> std::string;
    /** @brief Parses signature string into (name, n_args) tuple. */
    static auto parse(std::string const& _signature) -> std::tuple<std::string, unsigned short>;

  private:
    static auto find(std::string const& _signature) -> zpt::symbol;
    static auto find(std::string const& _name, unsigned short _1n_args) -> zpt::symbol;
};
} // namespace zpt

namespace zpt {

/**
 * @brief Internal implementation class for JSON lambdas.
 *
 * Stores lambda metadata (name, argument count) and provides invocation.
 * The actual function implementation is looked up in the global symbol table.
 *
 * @see zpt::lambda
 */
class JSONLambda {
  public:
    JSONLambda();
    /** @brief Constructs from signature string "name/n_args". */
    JSONLambda(std::string const& _signature);
    /** @brief Constructs from name and argument count. */
    JSONLambda(std::string const& _name, unsigned short _n_args);
    virtual ~JSONLambda();

    /**
     * @brief Invokes the lambda function.
     * @param _args JSON array of arguments.
     * @param _ctx Execution context.
     * @return JSON result.
     */
    virtual auto call(zpt::json _args, zpt::context _ctx) -> zpt::json;

    /** @brief Returns the lambda name. */
    virtual auto name() const -> std::string;
    /** @brief Returns the expected argument count. */
    virtual auto n_args() const -> unsigned short;
    /** @brief Returns the signature string "name/n_args". */
    virtual auto signature() const -> std::string;

  private:
    std::string __name{ "" };
    unsigned short __n_args{ 0 };
};
} // namespace zpt

namespace zpt {

/**
 * @brief Internal variant-based storage for JSON values.
 *
 * Stores any JSON value type using `std::variant`. This is the underlying
 * storage class used by `zpt::json`. Provides type introspection, value
 * access, serialization, and path-based navigation.
 *
 * @par Supported Types
 * - `nullptr_t` - JSON null
 * - `bool` - JSON boolean
 * - `long long int` - JSON integer
 * - `double` - JSON floating-point
 * - `std::string` - JSON string
 * - `zpt::timestamp_t` - Date/timestamp
 * - `JSONArr` - JSON array
 * - `JSONObj` - JSON object
 * - `JSONRegex` - Regular expression (extension)
 * - `zpt::lambda` - Lambda function (extension)
 * - `void*` - Undefined value
 *
 * @note Prefer using `zpt::json` wrapper rather than this class directly.
 *
 * @see zpt::json
 * @see zpt::JSONType
 */
class JSONElementT {
  public:
    /** @brief Constructs a null element. */
    JSONElementT();
    /** @brief Constructs an element of the specified type. */
    JSONElementT(JSONType _in);
    JSONElementT(const JSONElementT& _element);
    JSONElementT(JSONElementT&& _element);

    /** @name Value Constructors */
    ///@{
    JSONElementT(JSONObj& _value);
    JSONElementT(JSONArr& _value);
    JSONElementT(std::string const& _value);
    JSONElementT(const char* _value);
    JSONElementT(long long _value);
    JSONElementT(double _value);
    JSONElementT(bool _value);
    JSONElementT(zpt::timestamp_t _value);
    JSONElementT(int _value);
    JSONElementT(size_t _value);
#ifdef __LP64__
    JSONElementT(unsigned int _value);
#endif
    JSONElementT(zpt::lambda _value);
    JSONElementT(zpt::regex _value);
    JSONElementT(std::nullptr_t _rhs);
    JSONElementT(void* _rhs);
    ///@}
    virtual ~JSONElementT();

    /** @name Type Introspection */
    ///@{
    /** @brief Returns the current value type. */
    virtual auto type() const -> JSONType;
    /** @brief Returns human-readable type name. */
    virtual auto demangle() const -> std::string;
    /** @brief Changes the value type (resets value). */
    virtual auto type(JSONType _in) -> JSONElementT&;
    /** @brief Returns true if value is not null or undefined. */
    virtual auto ok() const -> bool;
    /** @brief Returns true if container is empty or value is null/undefined. */
    virtual auto empty() const -> bool;
    /** @brief Returns true if value is null. */
    virtual auto nil() const -> bool;
    ///@}

    /** @brief Resets to null value. */
    virtual auto clear() -> void;
    /** @brief Returns element count (1 for scalars, size for containers). */
    virtual auto size() const -> size_t;
    /** @brief Computes hash for use in containers. */
    virtual auto hash() const -> size_t;

    /** @brief Searches for a value in this container. */
    auto find(zpt::json _to_find) const -> zpt::json::iterator;
    /** @brief Returns true if this container contains the value. */
    auto contains(zpt::json _to_find) const -> bool;

    /** @brief Returns parent element (for nested values). */
    auto parent() -> JSONElementT*;
    /** @brief Sets parent element. */
    auto parent(JSONElementT* _parent) -> JSONElementT&;

    /** @brief Creates a deep copy of this element. */
    virtual auto clone() const -> zpt::json;

    /** @name Type Predicates */
    ///@{
    virtual auto is_object() const -> bool;
    virtual auto is_array() const -> bool;
    virtual auto is_string() const -> bool;
    virtual auto is_integer() const -> bool;
    virtual auto is_floating() const -> bool;
    /** @brief Returns true if integer or floating-point. */
    virtual auto is_number() const -> bool;
    virtual auto is_bool() const -> bool;
    virtual auto is_date() const -> bool;
    virtual auto is_lambda() const -> bool;
    virtual auto is_regex() const -> bool;
    virtual auto is_nil() const -> bool;
    virtual auto is_undefined() const -> bool;
    ///@}

    /** @name Value Accessors (Mutable) */
    ///@{
    /** @brief Returns object value. @throws zpt::CastException if wrong type. */
    virtual auto object() -> JSONObj&;
    virtual auto array() -> JSONArr&;
    virtual auto string() -> std::string&;
    virtual auto integer() -> long long&;
    virtual auto floating() -> double&;
    virtual auto boolean() -> bool&;
    virtual auto date() -> zpt::timestamp_t&;
    virtual auto lambda() -> zpt::lambda&;
    virtual auto regex() -> zpt::regex&;
    /** @brief Returns numeric value as double (works for int or float). */
    virtual auto number() -> double;
    ///@}

    /** @name Value Accessors (Const) */
    ///@{
    virtual auto object() const -> JSONObj const&;
    virtual auto array() const -> JSONArr const&;
    virtual auto string() const -> std::string const&;
    virtual auto integer() const -> long long const&;
    virtual auto floating() const -> double const&;
    virtual auto boolean() const -> bool const&;
    virtual auto date() const -> zpt::timestamp_t const&;
    virtual auto lambda() const -> zpt::lambda const&;
    virtual auto regex() const -> zpt::regex const&;
    virtual auto number() const -> double;
    ///@}

    auto operator=(const JSONElementT& _rhs) -> JSONElementT&;
    auto operator=(JSONElementT&& _rhs) -> JSONElementT&;
    auto operator=(std::string const& _rhs) -> JSONElementT&;
    auto operator=(std::nullptr_t) -> JSONElementT&;
    auto operator=(const char* _rhs) -> JSONElementT&;
    auto operator=(long long _rhs) -> JSONElementT&;
    auto operator=(double _rhs) -> JSONElementT&;
    auto operator=(bool _rhs) -> JSONElementT&;
    auto operator=(int _rhs) -> JSONElementT&;
    auto operator=(size_t _rhs) -> JSONElementT&;
#ifdef __LP64__
    auto operator=(unsigned int _rhs) -> JSONElementT&;
#endif
    auto operator=(zpt::json _rhs) -> JSONElementT&;
    auto operator=(zpt::timestamp_t _rhs) -> JSONElementT&;
    auto operator=(zpt::JSONObj& _rhs) -> JSONElementT&;
    auto operator=(zpt::JSONArr& _rhs) -> JSONElementT&;
    auto operator=(zpt::lambda _rhs) -> JSONElementT&;
    auto operator=(zpt::regex _rhs) -> JSONElementT&;
    auto operator=(void*) -> JSONElementT&;

    operator std::string();
    operator bool();
    operator int();
    operator long();
    operator long long();
    operator size_t();
    operator double();
#ifdef __LP64__
    operator unsigned int();
#endif
    operator zpt::timestamp_t();
    operator zpt::JSONObj();
    operator zpt::JSONArr();
    operator zpt::JSONObj&();
    operator zpt::JSONArr&();
    operator zpt::lambda();
    operator zpt::regex();
    operator zpt::regex&();

    auto operator<<(const char* _in) -> JSONElementT&;
    auto operator<<(std::string const& _in) -> JSONElementT&;
    auto operator<<(zpt::json _in) -> JSONElementT&;
    template<typename T>
    auto operator<<(T _in) -> JSONElementT&;
    template<typename T>
    auto operator[](T _idx) -> json&;
    template<typename T>
    auto operator[](T _idx) const -> json const;
    auto operator==(JSONElementT const& _in) const -> bool;
    auto operator==(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator==(T _in) const -> bool;
    auto operator!=(JSONElementT const& _in) const -> bool;
    auto operator!=(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator!=(T _in) const -> bool;
    auto operator<(JSONElementT const& _in) const -> bool;
    auto operator<(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator<(T _in) const -> bool;
    auto operator>(JSONElementT const& _in) const -> bool;
    auto operator>(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator>(T _in) const -> bool;
    auto operator<=(JSONElementT const& _in) const -> bool;
    auto operator<=(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator<=(T _in) const -> bool;
    auto operator>=(JSONElementT const& _in) const -> bool;
    auto operator>=(zpt::json _rhs) const -> bool;
    template<typename T>
    auto operator>=(T _in) const -> bool;

    friend auto operator<<(std::ostream& _out, JSONElementT _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

    /** @name Path-Based Access */
    ///@{
    /**
     * @brief Retrieves value at nested path.
     * @param _path Path string (e.g., "users.0.name" or "config/debug" with "/" separator).
     * @param _separator Path separator character.
     * @return Value at path, or undefined if not found.
     */
    auto get_path(std::string const& _path, std::string const& _separator = ".") -> zpt::json;
    /**
     * @brief Sets value at nested path, creating intermediate objects/arrays.
     * @param _path Path string.
     * @param _value Value to set.
     * @param _separator Path separator.
     */
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> JSONElementT&;
    /** @brief Removes value at path. */
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> JSONElementT&;
    ///@}

    /** @name Serialization */
    ///@{
    /** @brief Serializes to compact JSON string. */
    virtual auto stringify(std::string& _out) -> JSONElementT&;
    virtual auto stringify(std::ostream& _out) -> JSONElementT&;
    virtual auto stringify(std::string& _out) const -> JSONElementT const&;
    virtual auto stringify(std::ostream& _out) const -> JSONElementT const&;
    /** @brief Returns compact JSON string representation. */
    virtual auto stringify() const -> std::string;
    /** @brief Returns length of serialized JSON string. */
    virtual auto string_length() const -> size_t;

    /** @brief Serializes to formatted JSON with indentation. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> JSONElementT&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> JSONElementT&;
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> JSONElementT const&;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> JSONElementT const&;
    /** @brief Returns formatted JSON string representation. */
    virtual auto prettify() const -> std::string;
    ///@}

    /**
     * @brief Returns element at position as (index, key, value) tuple.
     * @param _pos Position index.
     * @return Tuple for iteration; key is empty for arrays.
     */
    virtual auto element(size_t _pos) -> std::tuple<size_t, std::string, zpt::json>;

  private:
    JSONElementT* __parent{ nullptr };
    std::variant<std::nullptr_t,   // JSNil
                 bool,             // JSBoolean
                 long long int,    // JSInteger
                 double,           // JSDouble
                 std::string,      // JSString
                 zpt::timestamp_t, // JSDate
                 JSONArr,          // JSArray
                 JSONObj,          // JSObject
                 JSONRegex,        // JSRegex
                 zpt::lambda,      // JSLambda
                 void*>            // JSUndefined
      __underlying;
};
} // namespace zpt

namespace zpt {

/**
 * @brief Parses ISO 8601 date string to timestamp.
 * @param _json_date Date string (e.g., "2024-01-15T10:30:00Z"). Empty for current time.
 * @return Milliseconds since Unix epoch.
 */
auto timestamp(std::string const& _json_date = "") -> zpt::timestamp_t;

/**
 * @brief Retrieves value at path from a JSON document.
 * @param _path Dot-separated path (e.g., "user.profile.name").
 * @param _source Source JSON document.
 * @return Value at path, or undefined if not found.
 */
auto get(std::string const& _path, zpt::json _source) -> zpt::json;

/**
 * @brief Sets value at path in a JSON document.
 * @tparam T Value type.
 * @param _path Dot-separated path.
 * @param _value Value to set.
 * @param _target Target document (creates new object if undefined).
 * @return Modified document.
 */
template<typename T>
auto set(std::string const& _path, T _value, zpt::json _target = zpt::undefined) -> zpt::json;

/**
 * @brief Converts timestamp to ISO 8601 date string.
 * @param _timestamp Milliseconds since Unix epoch.
 * @return ISO 8601 formatted string.
 */
auto timestamp(zpt::timestamp_t _timestamp) -> std::string;
} // namespace zpt

namespace std {
/**
 * @brief std::hash specialization for zpt::json.
 *
 * Enables using JSON values as keys in unordered containers.
 *
 * @par Example
 * @code
 * std::unordered_map<zpt::json, int> cache;
 * cache[zpt::json{"key"}] = 42;
 * @endcode
 */
template<>
struct hash<zpt::json> {
    auto operator()(zpt::json const& _json) const noexcept -> std::size_t;
};
} // namespace std

/// Class `zpt::pretty` methods
template<typename T>
zpt::pretty::pretty(T _rhs) {
    _rhs->prettify(this->__underlying);
}

/// Class `zpt::json` methods
template<typename T>
zpt::json::json(T const& _rhs)
  : __underlying{ zpt::allocate_shared<zpt::JSONElementT>(_rhs) } {}
template<typename T>
auto zpt::json::operator=(T const& _rhs) -> zpt::json& {
    (*this->__underlying.get()) = _rhs;
    return (*this);
}
template<typename T>
auto zpt::json::operator==(T _rhs) const -> bool {
    return (*this->__underlying.get()) == _rhs;
}
template<typename T>
auto zpt::json::operator!=(T _rhs) const -> bool {
    return (*this->__underlying.get()) != _rhs;
}
template<typename T>
auto zpt::json::operator<(T _rhs) const -> bool {
    return (*this->__underlying.get()) < _rhs;
}
template<typename T>
auto zpt::json::operator>(T _rhs) const -> bool {
    return (*this->__underlying.get()) > _rhs;
}
template<typename T>
auto zpt::json::operator<=(T _rhs) const -> bool {
    return (*this->__underlying.get()) <= _rhs;
}
template<typename T>
auto zpt::json::operator>=(T _rhs) const -> bool {
    return (*this->__underlying.get()) >= _rhs;
}
template<typename T>
auto zpt::json::operator<<(T _in) -> zpt::json& {
    (*this->__underlying.get()) << _in;
    return *this;
}
template<typename T>
auto zpt::json::data(const T _delegate) -> zpt::json {
    return _delegate->get_json();
}
template<typename T>
auto zpt::json::operator[](T _idx) -> zpt::json& {
    return (*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::json::operator[](T _idx) const -> zpt::json const {
    return const_cast<zpt::JSONElementT const&>(*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::json::operator()(T _idx) const -> zpt::json const {
    return const_cast<zpt::JSONElementT const&>(*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::json::pretty(T _e) -> std::string {
    return zpt::pretty{ _e };
}
template<typename T>
auto zpt::json::string(T _e) -> zpt::json {
    std::string _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::integer(T _e) -> zpt::json {
    long long int _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::uinteger(T _e) -> zpt::json {
    unsigned int _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::floating(T _e) -> zpt::json {
    double _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::ulong(T _e) -> zpt::json {
    size_t _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::boolean(T _e) -> zpt::json {
    bool _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::date(T _e) -> zpt::json {
    zpt::timestamp_t _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::lambda(T _e) -> zpt::json {
    zpt::lambda _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}
template<typename T>
auto zpt::json::regex(T _e) -> zpt::json {
    zpt::regex _v(_e);
    return zpt::json{ zpt::allocate_shared<zpt::JSONElementT>(_v) };
}

/// Class `zpt::JSONObjT` methods
template<typename T>
auto zpt::JSONObjT::operator==(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONObjT::operator!=(T _in) const -> bool {
    return true;
}
template<typename T>
auto zpt::JSONObjT::operator<(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONObjT::operator>(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONObjT::operator>=(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONObjT::operator<=(T _in) const -> bool {
    return false;
}

/// Class `zpt::JSONArrT` methods
template<typename T>
auto zpt::JSONArrT::operator==(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONArrT::operator!=(T _in) const -> bool {
    return true;
}
template<typename T>
auto zpt::JSONArrT::operator<(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONArrT::operator>(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONArrT::operator>=(T _in) const -> bool {
    return false;
}
template<typename T>
auto zpt::JSONArrT::operator<=(T _in) const -> bool {
    return false;
}

/// Class `zpt::JSONObj` methods
template<typename T>
auto zpt::JSONObj::operator==(T _rhs) const -> bool {
    return (*this->__underlying.get()) == _rhs;
}
template<typename T>
auto zpt::JSONObj::operator!=(T _rhs) const -> bool {
    return (*this->__underlying.get()) != _rhs;
}
template<typename T>
auto zpt::JSONObj::operator<(T _rhs) const -> bool {
    return (*this->__underlying.get()) < _rhs;
}
template<typename T>
auto zpt::JSONObj::operator>(T _rhs) const -> bool {
    return (*this->__underlying.get()) > _rhs;
}
template<typename T>
auto zpt::JSONObj::operator<=(T _rhs) const -> bool {
    return (*this->__underlying.get()) <= _rhs;
}
template<typename T>
auto zpt::JSONObj::operator>=(T _rhs) const -> bool {
    return (*this->__underlying.get()) >= _rhs;
}
template<typename T>
auto zpt::JSONObj::operator<<(T _in) -> JSONObj& {
    (*this)->push(_in);
    return *this;
}
template<typename T>
auto zpt::JSONObj::operator[](T _idx) -> json& {
    return (*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::JSONObj::operator[](T _idx) const -> json const {
    return static_cast<JSONObjT const&>(*this->__underlying.get())[_idx];
}

/// Class `zpt::JSONArr` methods
template<typename T>
auto zpt::JSONArr::operator==(T _rhs) const -> bool {
    return (*this->__underlying.get()) == _rhs;
}
template<typename T>
auto zpt::JSONArr::operator!=(T _rhs) const -> bool {
    return (*this->__underlying.get()) != _rhs;
}
template<typename T>
auto zpt::JSONArr::operator<(T _rhs) const -> bool {
    return (*this->__underlying.get()) < _rhs;
}
template<typename T>
auto zpt::JSONArr::operator>(T _rhs) const -> bool {
    return (*this->__underlying.get()) > _rhs;
}
template<typename T>
auto zpt::JSONArr::operator<=(T _rhs) const -> bool {
    return (*this->__underlying.get()) <= _rhs;
}
template<typename T>
auto zpt::JSONArr::operator>=(T _rhs) const -> bool {
    return (*this->__underlying.get()) >= _rhs;
}
template<typename T>
auto zpt::JSONArr::operator<<(T _in) -> JSONArr& {
    (*this)->push(_in);
    return *this;
}
template<typename T>
auto zpt::JSONArr::operator[](T _idx) -> json& {
    return (*this->__underlying.get())[_idx];
}
template<typename T>
auto zpt::JSONArr::operator[](T _idx) const -> json const {
    return static_cast<JSONArrT const&>(*this->__underlying.get())[_idx];
}

/// Class `zpt::JSONElementT` methods
template<typename T>
auto zpt::JSONElementT::operator<<(T _in) -> JSONElementT& {
    switch (this->__underlying.index()) {
        case zpt::JSObject: {
            this->object() << _in;
            break;
        }
        case zpt::JSArray: {
            this->array() << _in;
            break;
        }
        default: {
            this->__underlying = _in;
            break;
        }
    }
    return *this;
}
template<typename T>
auto zpt::JSONElementT::operator[](T _idx) -> json& {
    if (this->type() == zpt::JSObject) { return this->object()[_idx]; }
    else if (this->type() == zpt::JSArray) { return this->array()[_idx]; }
    else if (this->type() == zpt::JSNil) {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>) {
            this->__underlying = zpt::JSONObj();
            return this->object()[_idx];
        }
        else if constexpr (std::is_integral_v<T>) {
            this->__underlying = zpt::JSONArr();
            return this->array()[_idx];
        }
    }
    return zpt::undefined;
}

template<typename T>
auto zpt::JSONElementT::operator[](T _idx) const -> json const {
    if (this->type() == zpt::JSObject) { return static_cast<JSONObj const&>(this->object())[_idx]; }
    else if (this->type() == zpt::JSArray) {
        return static_cast<JSONArr const&>(this->array())[_idx];
    }
    return zpt::undefined;
}
template<typename T>
auto zpt::JSONElementT::operator==(T _in) const -> bool {
    if constexpr (std::is_same<T, std::nullptr_t>::value || std::is_pointer<T>::value) {
        if (_in == nullptr) {
            return this->type() == zpt::JSNil || this->type() == zpt::JSUndefined;
        }
    }
    JSONElementT _rhs{ _in };
    return (*this) == _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator!=(T _in) const -> bool {
    if constexpr (std::is_same<T, std::nullptr_t>::value || std::is_pointer<T>::value) {
        if (_in == nullptr) {
            return this->type() == zpt::JSNil || this->type() == zpt::JSUndefined;
        }
    }
    JSONElementT _rhs{ _in };
    return (*this) != _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator<(T _in) const -> bool {
    JSONElementT _rhs{ _in };
    return (*this) < _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator>(T _in) const -> bool {
    JSONElementT _rhs{ _in };
    return (*this) > _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator<=(T _in) const -> bool {
    JSONElementT _rhs{ _in };
    return (*this) <= _rhs;
}
template<typename T>
auto zpt::JSONElementT::operator>=(T _in) const -> bool {
    JSONElementT _rhs{ _in };
    return (*this) >= _rhs;
}

/// Namespace `zpt` methods
template<typename T>
auto zpt::set(std::string const& _path, T _value, zpt::json _target) -> zpt::json {
    zpt::json _return;
    if (_target->ok()) { _return = _target; }
    else { _return = zpt::json::object(); }
    _return->set_path(_path, zpt::json{ _value });
    return _return;
}

/**
 * @brief User-defined literal for parsing JSON strings.
 *
 * Allows inline JSON parsing using raw string literals.
 *
 * @par Example
 * @code
 * auto config = R"({"debug": true, "port": 8080})"_JSON;
 * bool debug = config["debug"];  // true
 * @endcode
 *
 * @param _string JSON string to parse.
 * @param _length String length.
 * @return Parsed JSON value.
 * @throws zpt::SyntaxErrorException On parse error.
 */
auto operator"" _JSON(const char* _string, size_t _length) -> zpt::json;
