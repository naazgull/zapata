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
 * for (auto&& [idx, key, value] : obj) {
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
class JSONElementT;  ///< Internal JSON element implementation
class JSONObj;       ///< JSON object wrapper
class JSONArr;       ///< JSON array wrapper
class JSONLambda;    ///< Lambda function implementation
class JSONRegex;     ///< Regular expression implementation
class JSONIterator;  ///< Iterator for JSON containers
class lambda;        ///< Lambda function wrapper
class json;          ///< Main JSON value class
struct json_element; ///< JSON iterator target type

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
    /** @brief Copy constructor.
     * @param _rhs Source pretty-print wrapper to copy.
     * @return void (constructors implicitly initialize the object). */
    pretty(const pretty& _rhs);
    /** @brief Move constructor.
     * @param _rhs Source pretty-print wrapper to move.
     * @return void (constructors implicitly initialize the object). */
    pretty(pretty&& _rhs);
    /** @brief Constructs from a string.
     * @param _rhs String to wrap for pretty-printing.
     * @return void (constructors implicitly initialize the object). */
    pretty(std::string const& _rhs);
    /** @brief Constructs from a C-string.
     * @param _rhs C-string to wrap for pretty-printing.
     * @return void (constructors implicitly initialize the object). */
    pretty(const char* _rhs);
    /**
     * @brief Constructs a pretty-printable wrapper from a JSON-like object.
     * @tparam T Type with a `prettify()` method.
     * @param _rhs Object to wrap.
     */
    template<typename T>
    pretty(T _rhs);
    /** @brief Destroys the pretty-print wrapper.
     * @return void (destructors implicitly clean up the object). */
    virtual ~pretty() = default;

    /** @brief Converts to the pretty-printed string representation.
     * @return Pretty-printed string representation. */
    operator std::string();

    /** @brief Copy assignment.
     * @param _rhs Source pretty-print wrapper to copy.
     * @return Reference to this wrapper. */
    auto operator=(const pretty& _rhs) -> pretty&;
    /** @brief Move assignment.
     * @param _rhs Source pretty-print wrapper to move.
     * @return Reference to this wrapper. */
    auto operator=(pretty&& _rhs) -> pretty&;

    /** @brief Accesses the underlying string.
     * @return Pointer to the underlying string. */
    auto operator->() -> std::string*;
    /** @brief Dereferences to the underlying string.
     * @return Reference to the underlying string. */
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
 * for (auto&& [index, key, value] : obj) {
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
    using element = json_element;
    /** @brief Iterator type. */
    using iterator = zpt::JSONIterator;
    /** @brief Const iterator type. */
    using const_iterator = const zpt::JSONIterator;

    /** @brief Callback signature for traverse operations. */
    using traverse_callback =
      std::function<void(std::string const&, zpt::json, std::string const&)>;

    /** @brief Constructs a null JSON value.
     * @return void (constructors implicitly initialize the object). */
    json();
    /** @brief Constructs a null JSON value.
     * @param _rhs Null pointer to construct from.
     * @return void (constructors implicitly initialize the object). */
    json(std::nullptr_t _rhs);
    /** @brief Constructs from a unique pointer to a JSON element.
     * @param _target Shared pointer to JSON element to wrap.
     * @return void (constructors implicitly initialize the object). */
    json(zpt::allocator<zpt::JSONElementT>::shared_pointer _target);
    /**
     * @brief Constructs from an initializer list.
     *
     * If the list starts with `zpt::array`, creates an array.
     * Otherwise creates an object from alternating key-value pairs.
     * @return void (constructors implicitly initialize the object).
     */
    json(std::initializer_list<zpt::json> _init);
    /** @brief Copy constructor (shared pointer semantics).
     * @param _rhs JSON value to copy.
     * @return void (constructors implicitly initialize the object). */
    json(zpt::json const& _rhs);
    /** @brief Move constructor.
     * @param _rhs JSON value to move.
     * @return void (constructors implicitly initialize the object). */
    json(zpt::json&& _rhs);
    /**
     * @brief Constructs from any compatible type.
     * @tparam T Type convertible to a JSON value.
     * @param _rhs Value to construct from.
     * @return void (constructors implicitly initialize the object).
     */
    template<typename T>
    json(T const& _rhs);
    /** @brief Destroys the JSON value.
     * @return void (destructors implicitly clean up the object). */
    virtual ~json();

    /** @brief Returns the number of elements (for objects/arrays) or 1.
     * @return Number of elements. */
    auto size() const -> size_t;
    /** @brief Computes a hash value for use in containers.
     * @return Hash value. */
    auto hash() const -> size_t;
    /** @brief Returns reference to the underlying element.
     * @return Reference to the underlying JSONElementT. */
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
    /** @brief Serializes to an output stream.
     * @param _out Output stream to serialize to.
     * @return Reference to this JSON value. */
    auto stringify(std::ostream& _out) -> zpt::json&;
    /** @brief Serializes to a string.
     * @param _out String to serialize to.
     * @return Reference to this JSON value. */
    auto stringify(std::string& _out) -> zpt::json&;
    /** @brief Serializes to an output stream (const version).
     * @param _out Output stream to serialize to.
     * @return Const reference to this JSON value. */
    auto stringify(std::ostream& _out) const -> zpt::json const&;
    /** @brief Serializes to a string (const version).
     * @param _out String to serialize to.
     * @return Const reference to this JSON value. */
    auto stringify(std::string& _out) const -> zpt::json const&;
    /** @brief Returns the JSON string representation.
     * @return Compact JSON string. */
    auto stringify() const -> std::string;

    /** @brief Returns an iterator to the first element.
     * @return Iterator pointing to the first element. */
    auto begin() -> zpt::json::iterator;
    /** @brief Returns an iterator past the last element.
     * @return Iterator pointing past the last element. */
    auto end() -> zpt::json::iterator;
    /** @brief Returns a const iterator to the first element.
     * @return Const iterator pointing to the first element. */
    auto begin() const -> zpt::json::const_iterator;
    /** @brief Returns a const iterator past the last element.
     * @return Const iterator pointing past the last element. */
    auto end() const -> zpt::json::const_iterator;

    /** @name Assignment Operators */
    ///@{
    /** @brief Copy assignment (shared semantics).
     * @param _rhs JSON value to copy.
     * @return Reference to this JSON value. */
    auto operator=(zpt::json const& _rhs) -> zpt::json&;
    /** @brief Move assignment.
     * @param _rhs JSON value to move.
     * @return Reference to this JSON value. */
    auto operator=(zpt::json&& _rhs) -> zpt::json&;
    /** @brief Assignment from iterator element tuple.
     * @param _rhs Element tuple to assign.
     * @return Reference to this JSON value. */
    auto operator=(element _rhs) -> zpt::json&;
    /** @brief Assignment from initializer list.
     * @param _list Initializer list to assign.
     * @return Reference to this JSON value. */
    auto operator=(std::initializer_list<zpt::json> _list) -> zpt::json&;
    /** @brief Assignment from any compatible type.
     * @tparam T Type to assign.
     * @param _rhs Value to assign.
     * @return Reference to this JSON value. */
    template<typename T>
    auto operator=(T const& _rhs) -> zpt::json&;
    ///@}

    /** @name Element Access */
    ///@{
    /** @brief Accesses the underlying JSONElementT.
     * @return Pointer to the underlying JSONElementT. */
    auto operator->() -> zpt::JSONElementT*;
    /** @brief Dereferences to the underlying JSONElementT.
     * @return Reference to the underlying JSONElementT. */
    auto operator*() -> zpt::JSONElementT&;
    /** @brief Accesses the underlying JSONElementT (const).
     * @return Const pointer to the underlying JSONElementT. */
    auto operator->() const -> zpt::JSONElementT const*;
    /** @brief Dereferences to the underlying JSONElementT (const).
     * @return Const reference to the underlying JSONElementT. */
    auto operator*() const -> zpt::JSONElementT const&;
    ///@}

    /** @name Comparison Operators */
    ///@{
    /** @brief Equality with iterator element tuple.
     * @param _rhs Element tuple to compare with.
     * @return True if equal. */
    auto operator==(element _rhs) const -> bool;
    /** @brief Inequality with iterator element tuple.
     * @param _rhs Element tuple to compare with.
     * @return True if not equal. */
    auto operator!=(element _rhs) const -> bool;
    /** @brief Equality with nullptr (checks if null/undefined).
     * @param _rhs Null pointer to compare with.
     * @return True if null or undefined. */
    auto operator==(std::nullptr_t _rhs) const -> bool;
    /** @brief Inequality with nullptr.
     * @param _rhs Null pointer to compare with.
     * @return True if not null or undefined. */
    auto operator!=(std::nullptr_t _rhs) const -> bool;
    /** @brief Appends elements from initializer list.
     * @param _in Initializer list of elements to append.
     * @return Reference to this JSON value. */
    auto operator<<(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Equality comparison with any compatible type.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if equal. */
    template<typename T>
    auto operator==(T _rhs) const -> bool;
    /** @brief Inequality comparison with any compatible type.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if not equal. */
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    /** @brief Less-than comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is less than _rhs. */
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    /** @brief Greater-than comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is greater than _rhs. */
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    /** @brief Less-than-or-equal comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is less than or equal to _rhs. */
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    /** @brief Greater-than-or-equal comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is greater than or equal to _rhs. */
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    /** @brief Appends an element (stream insertion).
     * @tparam T Type of value to append.
     * @param _in Value to append.
     * @return Reference to this JSON value. */
    template<typename T>
    auto operator<<(T _in) -> json&;
    /** @brief Accesses element by index or key.
     * @tparam T Index or key type.
     * @param _idx Index (integer) or key (string/C-string) to access.
     * @return Reference to the element at the given index/key. */
    template<typename T>
    auto operator[](T _idx) -> json&;
    /** @brief Accesses element by index or key (const).
     * @tparam T Index or key type.
     * @param _idx Index (integer) or key (string/C-string) to access.
     * @return Const JSON value at the given index/key. */
    template<typename T>
    auto operator[](T _idx) const -> zpt::json const;
    /** @brief Returns element or undefined if not found (safe access).
     * @tparam T Index or key type.
     * @param _idx Index (integer) or key (string/C-string) to access.
     * @return Const JSON value at the given index/key, or undefined if not found. */
    template<typename T>
    auto operator()(T _idx) const -> zpt::json const;
    ///@}

    /** @name Type Conversion Operators */
    ///@{
    /** @brief Converts to string.
     * @return String representation of the JSON value. */
    operator std::string();
    /** @brief Converts to bool.
     * @return Boolean representation of the JSON value. */
    operator bool();
    /** @brief Converts to int.
     * @return Integer representation of the JSON value. */
    operator int();
    /** @brief Converts to long.
     * @return Long integer representation of the JSON value. */
    operator long();
    /** @brief Converts to long long.
     * @return Long long integer representation of the JSON value. */
    operator long long();
    /** @brief Converts to size_t.
     * @return Size_t representation of the JSON value. */
    operator size_t();
    /** @brief Converts to double.
     * @return Double-precision floating-point representation of the JSON value. */
    operator double();
#ifdef __LP64__
    /** @brief Converts to unsigned int.
     * @return Unsigned integer representation of the JSON value. */
    operator unsigned int();
#endif
    /** @brief Converts to timestamp.
     * @return Timestamp (milliseconds since Unix epoch) representation. */
    operator zpt::timestamp_t();
    /** @brief Converts to JSON object.
     * @return JSON object wrapper. */
    operator zpt::JSONObj();
    /** @brief Converts to JSON array.
     * @return JSON array wrapper. */
    operator zpt::JSONArr();
    /** @brief Converts to JSON object reference.
     * @return Reference to JSON object. */
    operator zpt::JSONObj&();
    /** @brief Converts to JSON array reference.
     * @return Reference to JSON array. */
    operator zpt::JSONArr&();
    /** @brief Converts to lambda.
     * @return Lambda function wrapper. */
    operator zpt::lambda();
    /** @brief Converts to regex.
     * @return Regex wrapper. */
    operator zpt::regex();
    /** @brief Converts to regex reference.
     * @return Reference to regex wrapper. */
    operator zpt::regex&();
    /** @brief Converts to std::regex reference.
     * @return Reference to underlying std::regex. */
    operator std::regex&();

    /** @brief Converts to string (const).
     * @return String representation of the JSON value. */
    operator std::string() const;
    /** @brief Converts to bool (const).
     * @return Boolean representation of the JSON value. */
    operator bool() const;
    /** @brief Converts to int (const).
     * @return Integer representation of the JSON value. */
    operator int() const;
    /** @brief Converts to long (const).
     * @return Long integer representation of the JSON value. */
    operator long() const;
    /** @brief Converts to long long (const).
     * @return Long long integer representation of the JSON value. */
    operator long long() const;
    /** @brief Converts to size_t (const).
     * @return Size_t representation of the JSON value. */
    operator size_t() const;
    /** @brief Converts to double (const).
     * @return Double-precision floating-point representation of the JSON value. */
    operator double() const;
#ifdef __LP64__
    /** @brief Converts to unsigned int (const).
     * @return Unsigned integer representation of the JSON value. */
    operator unsigned int() const;
#endif
    /** @brief Converts to timestamp (const).
     * @return Timestamp (milliseconds since Unix epoch) representation. */
    operator zpt::timestamp_t() const;
    /** @brief Converts to JSON object (const).
     * @return JSON object wrapper. */
    operator zpt::JSONObj() const;
    /** @brief Converts to JSON array (const).
     * @return JSON array wrapper. */
    operator zpt::JSONArr() const;
    /** @brief Converts to JSON object reference (const).
     * @return Const reference to JSON object. */
    operator zpt::JSONObj&() const;
    /** @brief Converts to JSON array reference (const).
     * @return Const reference to JSON array. */
    operator zpt::JSONArr&() const;
    /** @brief Converts to lambda (const).
     * @return Lambda function wrapper. */
    operator zpt::lambda() const;
    /** @brief Converts to regex (const).
     * @return Regex wrapper. */
    operator zpt::regex() const;
    /** @brief Converts to regex reference (const).
     * @return Const reference to regex wrapper. */
    operator zpt::regex&() const;
    /** @brief Converts to std::regex reference (const).
     * @return Const reference to underlying std::regex. */
    operator std::regex&() const;
    ///@}

    /** @name Set Operations */
    ///@{
    /** @brief Returns union of this and the initializer list.
     * @param _in Initializer list of elements to union.
     * @return Combined JSON value. */
    auto operator+(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief In-place union with initializer list.
     * @param _in Initializer list of elements to union.
     * @return Reference to this JSON value. */
    auto operator+=(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Returns difference (elements in this but not in the list).
     * @param _in Initializer list of elements to remove.
     * @return JSON value with matching elements removed. */
    auto operator-(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief In-place difference.
     * @param _in Initializer list of elements to remove.
     * @return Reference to this JSON value. */
    auto operator-=(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Returns strict union (non-overlapping merge).
     * @param _in Initializer list of elements to merge.
     * @return Combined JSON value. */
    auto operator/(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief Returns deep merge of this and the list.
     * @param _in Initializer list of values to merge.
     * @return Deeply merged JSON value. */
    auto operator|(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief In-place deep merge.
     * @param _in Initializer list of values to merge.
     * @return Reference to this JSON value. */
    auto operator|=(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Returns intersection of this and the list.
     * @param _in Initializer list of elements to intersect.
     * @return JSON value containing only common elements. */
    auto operator&(std::initializer_list<zpt::json> _in) const -> json;
    /** @brief In-place intersection.
     * @param _in Initializer list of elements to intersect.
     * @return Reference to this JSON value. */
    auto operator&=(std::initializer_list<zpt::json> _in) -> json&;
    /** @brief Returns union of this and another JSON value.
     * @param _rhs JSON value to union.
     * @return Combined JSON value. */
    auto operator+(zpt::json _rhs) const -> json;
    /** @brief In-place union.
     * @param _rhs JSON value to union.
     * @return Reference to this JSON value. */
    auto operator+=(zpt::json _rhs) -> json&;
    /** @brief Returns difference.
     * @param _rhs JSON value to subtract.
     * @return JSON value with matching elements removed. */
    auto operator-(zpt::json _rhs) const -> json;
    /** @brief In-place difference.
     * @param _rhs JSON value to subtract.
     * @return Reference to this JSON value. */
    auto operator-=(zpt::json _rhs) -> json&;
    /** @brief Returns strict union.
     * @param _rhs JSON value to merge.
     * @return Combined JSON value. */
    auto operator/(zpt::json _rhs) const -> json;
    /** @brief Returns deep merge.
     * @param _rhs JSON value to merge.
     * @return Deeply merged JSON value. */
    auto operator|(zpt::json _rhs) const -> json;
    /** @brief In-place deep merge.
     * @param _rhs JSON value to merge.
     * @return Reference to this JSON value. */
    auto operator|=(zpt::json _rhs) -> json&;
    /** @brief Returns intersection.
     * @param _rhs JSON value to intersect.
     * @return JSON value containing only common elements. */
    auto operator&(zpt::json _rhs) const -> json;
    /** @brief In-place intersection.
     * @param _rhs JSON value to intersect.
     * @return Reference to this JSON value. */
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
     * @param _delegate Object to extract JSON data from.
     * @return JSON value extracted from the delegate.
     */
    template<typename T>
    static auto data(const T _delegate) -> zpt::json;
    /** @brief Parses a JSON string (static version).
     * @param _in JSON string to parse.
     * @return Parsed JSON value. */
    static auto parse_json_str(std::string const& _in) -> zpt::json;
    /** @brief Converts escape sequences to Unicode in-place.
     * @param _str String to convert.
     * @return void. */
    static auto to_unicode(std::string& _str) -> void;
    /** @brief Creates an empty JSON object.
     * @return Empty JSON object. */
    static auto object() -> zpt::json;
    /** @brief Creates an empty JSON array.
     * @return Empty JSON array. */
    static auto array() -> zpt::json;
    /** @name Static Type Factories
     *  Create JSON values with explicit types.
     */
    ///@{
    /** @brief Returns pretty-printed string of a JSON-like object.
     * @tparam T Type with a `prettify()` method.
     * @param _e Object to pretty-print.
     * @return Pretty-printed JSON string. */
    template<typename T>
    static auto pretty(T _e) -> std::string;
    /** @brief Creates a JSON string value.
     * @tparam T Type convertible to string.
     * @param _e Value to convert.
     * @return JSON string value. */
    template<typename T>
    static auto string(T _e) -> zpt::json;
    /** @brief Creates a JSON unsigned integer value.
     * @tparam T Type convertible to unsigned int.
     * @param _e Value to convert.
     * @return JSON integer value. */
    template<typename T>
    static auto uinteger(T _e) -> zpt::json;
    /** @brief Creates a JSON integer value.
     * @tparam T Type convertible to long long.
     * @param _e Value to convert.
     * @return JSON integer value. */
    template<typename T>
    static auto integer(T _e) -> zpt::json;
    /** @brief Creates a JSON floating-point value.
     * @tparam T Type convertible to double.
     * @param _e Value to convert.
     * @return JSON floating-point value. */
    template<typename T>
    static auto floating(T _e) -> zpt::json;
    /** @brief Creates a JSON unsigned long value.
     * @tparam T Type convertible to size_t.
     * @param _e Value to convert.
     * @return JSON integer value. */
    template<typename T>
    static auto ulong(T _e) -> zpt::json;
    /** @brief Creates a JSON boolean value.
     * @tparam T Type convertible to bool.
     * @param _e Value to convert.
     * @return JSON boolean value. */
    template<typename T>
    static auto boolean(T _e) -> zpt::json;
    /** @brief Creates a JSON date from an ISO 8601 string.
     * @param _e ISO 8601 date string.
     * @return JSON date value. */
    static auto date(std::string const& _e) -> zpt::json;
    /** @brief Creates a JSON date with the current timestamp.
     * @return JSON date value. */
    static auto date() -> zpt::json;
    /** @brief Creates a JSON date from a numeric timestamp.
     * @tparam T Type convertible to timestamp.
     * @param _e Numeric timestamp value.
     * @return JSON date value. */
    template<typename T>
    static auto date(T _e) -> zpt::json;
    /** @brief Creates a JSON lambda from a callable.
     * @tparam T Type convertible to lambda.
     * @param _e Callable value.
     * @return JSON lambda value. */
    template<typename T>
    static auto lambda(T _e) -> zpt::json;
    /** @brief Creates a JSON lambda with name and argument count.
     * @param _name Lambda name.
     * @param _n_args Number of expected arguments.
     * @return JSON lambda value. */
    static auto lambda(std::string const& _name, unsigned short _n_args) -> zpt::json;
    /** @brief Creates a JSON regex value.
     * @tparam T Type convertible to regex.
     * @param _e Regex pattern string.
     * @return JSON regex value. */
    template<typename T>
    static auto regex(T _e) -> zpt::json;
    ///@}

    /** @name Type Introspection
     *  Returns the JSONType for a given C++ value.
     */
    ///@{
    /** @brief Returns JSString.
     * @param _value String value.
     * @return JSONType::JSString. */
    static auto type_of(std::string const& _value) -> zpt::JSONType;
    /** @brief Returns JSBoolean.
     * @param _value Boolean value.
     * @return JSONType::JSBoolean. */
    static auto type_of(bool _value) -> zpt::JSONType;
    /** @brief Returns JSInteger.
     * @param _value Integer value.
     * @return JSONType::JSInteger. */
    static auto type_of(int _value) -> zpt::JSONType;
    /** @brief Returns JSInteger.
     * @param _value Long value.
     * @return JSONType::JSInteger. */
    static auto type_of(long _value) -> zpt::JSONType;
    /** @brief Returns JSInteger.
     * @param _value Long long value.
     * @return JSONType::JSInteger. */
    static auto type_of(long long _value) -> zpt::JSONType;
    /** @brief Returns JSInteger.
     * @param _value Size_t value.
     * @return JSONType::JSInteger. */
    static auto type_of(size_t _value) -> zpt::JSONType;
    /** @brief Returns JSDouble.
     * @param _value Double value.
     * @return JSONType::JSDouble. */
    static auto type_of(double _value) -> zpt::JSONType;
#ifdef __LP64__
    /** @brief Returns JSInteger.
     * @param _value Unsigned int value.
     * @return JSONType::JSInteger. */
    static auto type_of(unsigned int _value) -> zpt::JSONType;
#endif
    /** @brief Returns the type stored in the element.
     * @param _value JSON element.
     * @return JSONType of the element's value. */
    static auto type_of(zpt::JSONElementT& _value) -> zpt::JSONType;
    /** @brief Returns JSDate.
     * @param _value Timestamp value.
     * @return JSONType::JSDate. */
    static auto type_of(zpt::timestamp_t _value) -> zpt::JSONType;
    /** @brief Returns JSString.
     * @param _value Pretty-print wrapper.
     * @return JSONType::JSString. */
    static auto type_of(zpt::pretty _value) -> zpt::JSONType;
    /** @brief Returns JSObject.
     * @param _value JSON object wrapper.
     * @return JSONType::JSObject. */
    static auto type_of(zpt::JSONObj _value) -> zpt::JSONType;
    /** @brief Returns JSArray.
     * @param _value JSON array wrapper.
     * @return JSONType::JSArray. */
    static auto type_of(zpt::JSONArr _value) -> zpt::JSONType;
    /** @brief Returns JSObject.
     * @param _value JSON object reference.
     * @return JSONType::JSObject. */
    static auto type_of(zpt::JSONObj& _value) -> zpt::JSONType;
    /** @brief Returns JSArray.
     * @param _value JSON array reference.
     * @return JSONType::JSArray. */
    static auto type_of(zpt::JSONArr& _value) -> zpt::JSONType;
    /** @brief Returns JSLambda.
     * @param _value Lambda wrapper.
     * @return JSONType::JSLambda. */
    static auto type_of(zpt::lambda _value) -> zpt::JSONType;
    /** @brief Returns JSRegex.
     * @param _value Regex reference.
     * @return JSONType::JSRegex. */
    static auto type_of(zpt::regex& _value) -> zpt::JSONType;
    /** @brief Returns the type of the underlying JSON value.
     * @param _value JSON value.
     * @return JSONType of the underlying value. */
    static auto type_of(zpt::json& _value) -> zpt::JSONType;
    ///@}

    /**
     * @brief Recursively traverses a JSON document.
     * @param _document Document to traverse.
     * @param _callback Called for each element with (path, value, key).
     * @return void.
     */
    static auto traverse(zpt::json _document, zpt::json::traverse_callback _callback) -> void;
    /**
     * @brief Flattens a nested document to a single-level object.
     * @param _document Document to flatten.
     * @return Object with dot-separated paths as keys.
     */
    static auto flatten(zpt::json _document) -> zpt::json;
    /** @brief Finds a value in an iterator range.
     * @param _begin Start iterator.
     * @param _end End iterator.
     * @param _to_find Value to find.
     * @return Iterator to found element, or _end. */
    static auto find(zpt::json::iterator _begin, zpt::json::iterator _end, zpt::json _to_find)
      -> zpt::json::iterator;
    /** @brief Finds a value in a JSON element.
     * @param _to_search Element to search.
     * @param _to_find Value to find.
     * @return Iterator to found element. */
    static auto find(zpt::JSONElementT const& _to_search, zpt::json _to_find)
      -> zpt::json::iterator;
    /** @brief Checks if an element contains a value.
     * @param _to_search Element to search.
     * @param _to_find Value to find.
     * @return True if value is found. */
    static auto contains(zpt::JSONElementT const& _to_search, zpt::json _to_find) -> bool;

  private:
    std::shared_ptr<zpt::JSONElementT> __underlying{ nullptr };

    json(element _rhs);
    auto strict_union(zpt::json _rhs) -> void;
    auto strict_intersection(zpt::json _rhs) -> void;

    static auto traverse(zpt::json _document,
                         zpt::json::traverse_callback _callback,
                         std::string _path) -> void;
};

struct json_element {
    size_t __index;
    std::string __name;
    zpt::json __value;
};
} // namespace zpt

static_assert(std::is_move_constructible<zpt::json>::value);

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
     * @return void (constructors implicitly initialize the object).
     */
    explicit JSONIterator(zpt::json const& _target, size_t _pos);
    /** @brief Copy constructor.
     * @param _rhs Iterator to copy.
     * @return void (constructors implicitly initialize the object). */
    JSONIterator(JSONIterator const& _rhs);
    /** @brief Move constructor.
     * @param _rhs Iterator to move.
     * @return void (constructors implicitly initialize the object). */
    JSONIterator(JSONIterator&& _rhs);
    /** @brief Destroys the iterator.
     * @return void (destructors implicitly clean up the object). */
    virtual ~JSONIterator() = default;

    /** @brief Copy assignment.
     * @param _rhs Iterator to copy.
     * @return Reference to this iterator. */
    auto operator=(JSONIterator const& _rhs) -> JSONIterator&;
    /** @brief Move assignment.
     * @param _rhs Iterator to move.
     * @return Reference to this iterator. */
    auto operator=(JSONIterator&& _rhs) -> JSONIterator&;
    /** @brief Pre-increment: advances to next element.
     * @return Reference to this iterator at new position. */
    auto operator++() -> JSONIterator&;
    /** @brief Dereference: returns (index, key, value) tuple.
     * @return Reference to the current element tuple. */
    auto operator*() -> reference;

    /** @brief Post-increment: advances and returns previous position.
     * @param _dummy Unused int parameter for postfix syntax.
     * @return Iterator at previous position. */
    auto operator++(int) -> JSONIterator;
    /** @brief Arrow operator: returns (index, key, value) tuple.
     * @return Pointer to the current element tuple. */
    auto operator->() -> pointer;
    /** @brief Equality comparison.
     * @param _rhs Iterator to compare with.
     * @return True if both iterators point to the same position. */
    auto operator==(JSONIterator const& _rhs) const -> bool;
    /** @brief Inequality comparison.
     * @param _rhs Iterator to compare with.
     * @return True if iterators point to different positions. */
    auto operator!=(JSONIterator const& _rhs) const -> bool;

    /** @brief Pre-decrement: moves to previous element.
     * @return Reference to this iterator at new position. */
    auto operator--() -> JSONIterator&;
    /** @brief Post-decrement: moves back and returns previous position.
     * @param _dummy Unused int parameter for postfix syntax.
     * @return Iterator at previous position. */
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
    /** @brief Constructs an empty JSON object.
     * @return void (constructors implicitly initialize the object). */
    JSONObjT();
    /** @brief Destroys the JSON object.
     * @return void (destructors implicitly clean up the object). */
    virtual ~JSONObjT();

    /** @name Serialization */
    ///@{
    /** @brief Serializes to compact JSON string.
     * @param _out String to append serialized JSON to.
     * @return Reference to this object. */
    virtual auto stringify(std::string& _out) -> zpt::JSONObjT&;
    /** @brief Serializes to compact JSON string via stream.
     * @param _out Output stream to write to.
     * @return Reference to this object. */
    virtual auto stringify(std::ostream& _out) -> zpt::JSONObjT&;
    /** @brief Serializes to compact JSON string (const).
     * @param _out String to append serialized JSON to.
     * @return Const reference to this object. */
    virtual auto stringify(std::string& _out) const -> zpt::JSONObjT const&;
    /** @brief Serializes to compact JSON string via stream (const).
     * @param _out Output stream to write to.
     * @return Const reference to this object. */
    virtual auto stringify(std::ostream& _out) const -> zpt::JSONObjT const&;

    /** @brief Serializes to formatted JSON with indentation.
     * @param _out String to append formatted JSON to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Reference to this object. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;
    /** @brief Serializes to formatted JSON with indentation via stream.
     * @param _out Output stream to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Reference to this object. */
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;
    /** @brief Serializes to formatted JSON with indentation (const).
     * @param _out String to append formatted JSON to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Const reference to this object. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> zpt::JSONObjT const&;
    /** @brief Serializes to formatted JSON with indentation via stream (const).
     * @param _out Output stream to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Const reference to this object. */
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> zpt::JSONObjT const&;
    ///@}

    /** @name Modification */
    ///@{
    /** @brief Pushes a key (next push must be a value).
     * @param _name Key name to push.
     * @return Reference to this object. */
    virtual auto push(std::string const& _name) -> zpt::JSONObjT&;
    /** @brief Pushes a value for the pending key.
     * @param _value Shared pointer to element to push.
     * @return Reference to this object. */
    virtual auto push(zpt::allocator<zpt::JSONElementT>::shared_pointer _value) -> JSONObjT&;
    /** @brief Pushes a value for the pending key.
     * @param _value JSON value to push.
     * @return Reference to this object. */
    virtual auto push(zpt::json const& _value) -> zpt::JSONObjT&;

    /** @brief Removes element by integer index.
     * @param _idx Index to remove.
     * @return Reference to this object. */
    virtual auto pop(int _idx) -> zpt::JSONObjT&;
    /** @brief Removes element by size_t index.
     * @param _idx Index to remove.
     * @return Reference to this object. */
    virtual auto pop(size_t _idx) -> zpt::JSONObjT&;
    /** @brief Removes element by C-string key.
     * @param _idx C-string key to remove.
     * @return Reference to this object. */
    virtual auto pop(const char* _idx) -> zpt::JSONObjT&;
    /** @brief Removes element by string key.
     * @param _idx String key to remove.
     * @return Reference to this object. */
    virtual auto pop(std::string const& _idx) -> zpt::JSONObjT&;
    ///@}

    /** @brief Returns key at given position.
     * @param _idx Position index.
     * @return Key string at the given position. */
    virtual auto key_for(size_t _idx) const -> std::string;
    /** @brief Returns true if a key was pushed without a value.
     * @return True if there is a pending key. */
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
     * @return Reference to this object.
     */
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> zpt::JSONObjT&;
    /** @brief Deletes value at path.
     * @param _path Dot-separated path.
     * @param _separator Path separator.
     * @return Reference to this object. */
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> JSONObjT&;
    ///@}

    /** @brief Creates a deep copy of this object.
     * @return Deep copy as JSON value. */
    auto clone() const -> zpt::json;

    /** @name Access Operators */
    ///@{
    /** @brief Accesses underlying map.
     * @return Pointer to underlying map. */
    auto operator->() -> zpt::json::map*;
    /** @brief Dereferences to underlying map.
     * @return Reference to underlying map. */
    auto operator*() -> zpt::json::map&;
    /** @brief Accesses underlying map (const).
     * @return Const pointer to underlying map. */
    auto operator->() const -> zpt::json::map const*;
    /** @brief Dereferences to underlying map (const).
     * @return Const reference to underlying map. */
    auto operator*() const -> zpt::json::map const&;
    ///@}

    /** @name Comparison Operators */
    ///@{
    /** @brief Equality comparison.
     * @param _in Other JSONObjT to compare with.
     * @return True if equal. */
    auto operator==(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Equality comparison with wrapper.
     * @param _in JSONObj wrapper to compare with.
     * @return True if equal. */
    auto operator==(zpt::JSONObj const& _in) const -> bool;
    /** @brief Equality comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if equal. */
    template<typename T>
    auto operator==(T _in) const -> bool;
    /** @brief Inequality comparison.
     * @param _in Other JSONObjT to compare with.
     * @return True if not equal. */
    auto operator!=(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Inequality comparison with wrapper.
     * @param _in JSONObj wrapper to compare with.
     * @return True if not equal. */
    auto operator!=(zpt::JSONObj const& _in) const -> bool;
    /** @brief Inequality comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if not equal. */
    template<typename T>
    auto operator!=(T _in) const -> bool;
    /** @brief Less-than comparison.
     * @param _in Other JSONObjT to compare with.
     * @return True if this is less than _in. */
    auto operator<(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Less-than comparison with wrapper.
     * @param _in JSONObj wrapper to compare with.
     * @return True if this is less than _in. */
    auto operator<(zpt::JSONObj const& _in) const -> bool;
    /** @brief Less-than comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is less than _in. */
    template<typename T>
    auto operator<(T _in) const -> bool;
    /** @brief Greater-than comparison.
     * @param _in Other JSONObjT to compare with.
     * @return True if this is greater than _in. */
    auto operator>(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Greater-than comparison with wrapper.
     * @param _in JSONObj wrapper to compare with.
     * @return True if this is greater than _in. */
    auto operator>(zpt::JSONObj const& _in) const -> bool;
    /** @brief Greater-than comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is greater than _in. */
    template<typename T>
    auto operator>(T _in) const -> bool;
    /** @brief Greater-than-or-equal comparison.
     * @param _in Other JSONObjT to compare with.
     * @return True if this is >= _in. */
    auto operator>=(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison with wrapper.
     * @param _in JSONObj wrapper to compare with.
     * @return True if this is >= _in. */
    auto operator>=(zpt::JSONObj const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is >= _in. */
    template<typename T>
    auto operator>=(T _in) const -> bool;
    /** @brief Less-than-or-equal comparison.
     * @param _in Other JSONObjT to compare with.
     * @return True if this is <= _in. */
    auto operator<=(zpt::JSONObjT const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison with wrapper.
     * @param _in JSONObj wrapper to compare with.
     * @return True if this is <= _in. */
    auto operator<=(zpt::JSONObj const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is <= _in. */
    template<typename T>
    auto operator<=(T _in) const -> bool;
    ///@}

    /** @name Subscript Operators */
    ///@{
    /** @brief Accesses value by integer index (non-const).
     * @param _idx Integer index.
     * @return Reference to the JSON value at that index. */
    auto operator[](int _idx) -> zpt::json&;
    /** @brief Accesses value by size_t index (non-const).
     * @param _idx Size_t index.
     * @return Reference to the JSON value at that index. */
    auto operator[](size_t _idx) -> zpt::json&;
    /** @brief Accesses value by C-string key (non-const).
     * @param _idx C-string key.
     * @return Reference to the JSON value associated with the key. */
    auto operator[](const char* _idx) -> zpt::json&;
    /** @brief Accesses value by string key (non-const).
     * @param _idx String key.
     * @return Reference to the JSON value associated with the key. */
    auto operator[](std::string const& _idx) -> zpt::json&;
    /** @brief Accesses value by integer index (const).
     * @param _idx Integer index.
     * @return Const JSON value at that index. */
    auto operator[](int _idx) const -> zpt::json const;
    /** @brief Accesses value by size_t index (const).
     * @param _idx Size_t index.
     * @return Const JSON value at that index. */
    auto operator[](size_t _idx) const -> zpt::json const;
    /** @brief Accesses value by C-string key (const).
     * @param _idx C-string key.
     * @return Const JSON value associated with the key. */
    auto operator[](const char* _idx) const -> zpt::json const;
    /** @brief Accesses value by string key (const).
     * @param _idx String key.
     * @return Const JSON value associated with the key. */
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
    /** @brief Constructs an empty JSON array.
     * @return void (constructors implicitly initialize the object). */
    JSONArrT();
    /** @brief Destroys the JSON array.
     * @return void (destructors implicitly clean up the object). */
    virtual ~JSONArrT();

    /** @name Serialization */
    ///@{
    /** @brief Serializes to compact JSON string (non-const string ref).
     * @param _out Output string to write to.
     * @return Reference to this array. */
    virtual auto stringify(std::string& _out) -> zpt::JSONArrT&;
    /** @brief Serializes to compact JSON string (ostream).
     * @param _out Output stream to write to.
     * @return Reference to this array. */
    virtual auto stringify(std::ostream& _out) -> zpt::JSONArrT&;
    /** @brief Serializes to compact JSON string (const, string ref).
     * @param _out Output string to write to.
     * @return Const reference to this array. */
    virtual auto stringify(std::string& _out) const -> zpt::JSONArrT const&;
    /** @brief Serializes to compact JSON string (const, ostream).
     * @param _out Output stream to write to.
     * @return Const reference to this array. */
    virtual auto stringify(std::ostream& _out) const -> zpt::JSONArrT const&;

    /** @brief Serializes to formatted JSON with indentation (non-const string ref).
     * @param _out Output string to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Reference to this array. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;
    /** @brief Serializes to formatted JSON with indentation (ostream).
     * @param _out Output stream to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Reference to this array. */
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;
    /** @brief Serializes to formatted JSON with indentation (const, string ref).
     * @param _out Output string to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Const reference to this array. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> zpt::JSONArrT const&;
    /** @brief Serializes to formatted JSON with indentation (const, ostream).
     * @param _out Output stream to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Const reference to this array. */
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> zpt::JSONArrT const&;
    ///@}

    /** @name Modification */
    ///@{
    /** @brief Appends a value to the array (shared_pointer).
     * @param _value Shared pointer to JSONElementT to append.
     * @return Reference to this array. */
    virtual auto push(zpt::allocator<zpt::JSONElementT>::shared_pointer _value) -> zpt::JSONArrT&;
    /** @brief Appends a value to the array (json).
     * @param _value JSON value to append.
     * @return Reference to this array. */
    virtual auto push(zpt::json const& _value) -> zpt::JSONArrT&;

    /** @brief Removes element by integer index.
     * @param _idx Integer index of element to remove.
     * @return Reference to this array. */
    virtual auto pop(int _idx) -> zpt::JSONArrT&;
    /** @brief Removes element by size_t index.
     * @param _idx Size_t index of element to remove.
     * @return Reference to this array. */
    virtual auto pop(size_t _idx) -> zpt::JSONArrT&;
    /** @brief Removes element by C-string key (for array of objects).
     * @param _idx C-string key to match against elements.
     * @return Reference to this array. */
    virtual auto pop(const char* _idx) -> zpt::JSONArrT&;
    /** @brief Removes element by string key (for array of objects).
     * @param _idx String key to match against elements.
     * @return Reference to this array. */
    virtual auto pop(std::string const& _idx) -> zpt::JSONArrT&;

    /** @brief Sorts array elements using default comparison.
     * @return Reference to this array. */
    virtual auto sort() -> zpt::JSONArrT&;
    /**
     * @brief Sorts array elements using custom comparator.
     * @param _comparator Returns true if first arg should come before second.
     * @return Reference to this array. */
    virtual auto sort(std::function<bool(zpt::json, zpt::json)> _comparator) -> zpt::JSONArrT&;
    ///@}

    /** @name Path-Based Access */
    ///@{
    /**
     * @brief Retrieves value at nested path.
     * @param _path Path with numeric indices (e.g., "0.name" for first element's name).
     * @param _separator Path separator character.
     * @return JSON value at the given path. */
    auto get_path(std::string const& _path, std::string const& _separator = ".") -> zpt::json;
    /** @brief Sets value at nested path, creating intermediate objects.
     * @param _path Dot-separated path.
     * @param _value Value to set.
     * @param _separator Path separator.
     * @return Reference to this array. */
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> zpt::JSONArrT&;
    /** @brief Deletes value at path.
     * @param _path Dot-separated path.
     * @param _separator Path separator.
     * @return Reference to this array. */
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> zpt::JSONArrT&;
    ///@}

    /** @brief Creates a deep copy of this array.
     * @return Deep copy as JSON value. */
    auto clone() const -> zpt::json;

    /** @name Access Operators */
    ///@{
    /** @brief Accesses underlying vector (non-const pointer).
     * @return Pointer to underlying vector. */
    auto operator->() -> std::vector<zpt::json>*;
    /** @brief Dereferences to underlying vector (non-const reference).
     * @return Reference to underlying vector. */
    auto operator*() -> std::vector<zpt::json>&;
    /** @brief Accesses underlying vector (const pointer).
     * @return Const pointer to underlying vector. */
    auto operator->() const -> std::vector<zpt::json> const*;
    /** @brief Dereferences to underlying vector (const reference).
     * @return Const reference to underlying vector. */
    auto operator*() const -> std::vector<zpt::json> const&;
    ///@}

    /** @name Comparison Operators */
    ///@{
    /** @brief Equality comparison.
     * @param _in Other JSONArrT to compare with.
     * @return True if equal. */
    auto operator==(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Equality comparison with wrapper.
     * @param _in JSONArr wrapper to compare with.
     * @return True if equal. */
    auto operator==(zpt::JSONArr const& _in) const -> bool;
    /** @brief Equality comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if equal. */
    template<typename T>
    auto operator==(T _in) const -> bool;
    /** @brief Inequality comparison.
     * @param _in Other JSONArrT to compare with.
     * @return True if not equal. */
    auto operator!=(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Inequality comparison with wrapper.
     * @param _in JSONArr wrapper to compare with.
     * @return True if not equal. */
    auto operator!=(zpt::JSONArr const& _in) const -> bool;
    /** @brief Inequality comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if not equal. */
    template<typename T>
    auto operator!=(T _in) const -> bool;
    /** @brief Less-than comparison.
     * @param _in Other JSONArrT to compare with.
     * @return True if this is less than _in. */
    auto operator<(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Less-than comparison with wrapper.
     * @param _in JSONArr wrapper to compare with.
     * @return True if this is less than _in. */
    auto operator<(zpt::JSONArr const& _in) const -> bool;
    /** @brief Less-than comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is less than _in. */
    template<typename T>
    auto operator<(T _in) const -> bool;
    /** @brief Greater-than comparison.
     * @param _in Other JSONArrT to compare with.
     * @return True if this is greater than _in. */
    auto operator>(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Greater-than comparison with wrapper.
     * @param _in JSONArr wrapper to compare with.
     * @return True if this is greater than _in. */
    auto operator>(zpt::JSONArr const& _in) const -> bool;
    /** @brief Greater-than comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is greater than _in. */
    template<typename T>
    auto operator>(T _in) const -> bool;
    /** @brief Less-than-or-equal comparison.
     * @param _in Other JSONArrT to compare with.
     * @return True if this is <= _in. */
    auto operator<=(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison with wrapper.
     * @param _in JSONArr wrapper to compare with.
     * @return True if this is <= _in. */
    auto operator<=(zpt::JSONArr const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is <= _in. */
    template<typename T>
    auto operator<=(T _in) const -> bool;
    /** @brief Greater-than-or-equal comparison.
     * @param _in Other JSONArrT to compare with.
     * @return True if this is >= _in. */
    auto operator>=(zpt::JSONArrT const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison with wrapper.
     * @param _in JSONArr wrapper to compare with.
     * @return True if this is >= _in. */
    auto operator>=(zpt::JSONArr const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison with any type.
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is >= _in. */
    template<typename T>
    auto operator>=(T _in) const -> bool;
    ///@}

    /** @name Subscript Operators */
    ///@{
    /** @brief Accesses element by integer index (non-const).
     * @param _idx Integer index.
     * @return Reference to the JSON value at that index. */
    auto operator[](int _idx) -> zpt::json&;
    /** @brief Accesses element by size_t index (non-const).
     * @param _idx Size_t index.
     * @return Reference to the JSON value at that index. */
    auto operator[](size_t _idx) -> zpt::json&;
    /** @brief Accesses element by C-string key (non-const, for array of objects).
     * @param _idx C-string key.
     * @return Reference to the first matching JSON value. */
    auto operator[](const char* _idx) -> zpt::json&;
    /** @brief Accesses element by string key (non-const, for array of objects).
     * @param _idx String key.
     * @return Reference to the first matching JSON value. */
    auto operator[](std::string const& _idx) -> zpt::json&;
    /** @brief Accesses element by integer index (const).
     * @param _idx Integer index.
     * @return Const JSON value at that index. */
    auto operator[](int _idx) const -> zpt::json const;
    /** @brief Accesses element by size_t index (const).
     * @param _idx Size_t index.
     * @return Const JSON value at that index. */
    auto operator[](size_t _idx) const -> zpt::json const;
    /** @brief Accesses element by C-string key (const, for array of objects).
     * @param _idx C-string key.
     * @return Const JSON value at the first matching key. */
    auto operator[](const char* _idx) const -> zpt::json const;
    /** @brief Accesses element by string key (const, for array of objects).
     * @param _idx String key.
     * @return Const JSON value at the first matching key. */
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
    /** @brief Constructs an empty JSON object wrapper.
     * @return void (constructors implicitly initialize the object). */
    JSONObj();
    /** @brief Copy constructor (shared semantics).
     * @param _rhs Another JSONObj wrapper to copy.
     * @return void (constructors implicitly initialize the object). */
    JSONObj(const zpt::JSONObj& _rhs);
    /** @brief Move constructor.
     * @param _rhs JSONObj wrapper to move from.
     * @return void (constructors implicitly initialize the object). */
    JSONObj(zpt::JSONObj&& _rhs);
    /** @brief Takes ownership of a raw JSONObjT pointer.
     * @param _target Raw JSONObjT pointer to adopt.
     * @return void (constructors implicitly initialize the object). */
    JSONObj(zpt::JSONObjT* _target);
    /** @brief Destroys the JSON object wrapper.
     * @return void (destructors implicitly clean up the object). */
    virtual ~JSONObj();

    /** @brief Computes hash value for use in containers.
     * @return Hash value of the underlying object. */
    auto hash() const -> size_t;

    /** @brief Copy assignment (shared semantics).
     * @param _rhs Another JSONObj wrapper to copy.
     * @return Reference to this wrapper. */
    auto operator=(const zpt::JSONObj& _rhs) -> zpt::JSONObj&;
    /** @brief Move assignment.
     * @param _rhs JSONObj wrapper to move from.
     * @return Reference to this wrapper. */
    auto operator=(zpt::JSONObj&& _rhs) -> zpt::JSONObj&;

    /** @brief Accesses underlying JSONObjT (non-const pointer).
     * @return Pointer to underlying JSONObjT. */
    auto operator->() -> zpt::JSONObjT*;
    /** @brief Dereferences to underlying JSONObjT (non-const reference).
     * @return Reference to underlying JSONObjT. */
    auto operator*() -> zpt::JSONObjT&;
    /** @brief Accesses underlying JSONObjT (const pointer).
     * @return Const pointer to underlying JSONObjT. */
    auto operator->() const -> zpt::JSONObjT const*;
    /** @brief Dereferences to underlying JSONObjT (const reference).
     * @return Const reference to underlying JSONObjT. */
    auto operator*() const -> zpt::JSONObjT const&;

    /** @brief Converts to JSON string.
     * @return String representation of the underlying object. */
    operator std::string();
    /** @brief Converts to pretty-printed string.
     * @return Pretty-printed wrapper of the underlying object. */
    operator zpt::pretty();
    /** @brief Equality comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if equal. */
    template<typename T>
    auto operator==(T _rhs) const -> bool;
    /** @brief Inequality comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if not equal. */
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    /** @brief Less-than comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is less than _rhs. */
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    /** @brief Greater-than comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is greater than _rhs. */
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    /** @brief Less-than-or-equal comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is <= _rhs. */
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    /** @brief Greater-than-or-equal comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is >= _rhs. */
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    /** @brief Pushes a key string.
     * @param _in Key string to push.
     * @return Reference to this wrapper. */
    auto operator<<(std::string const& _in) -> zpt::JSONObj&;
    /** @brief Pushes a C-string key.
     * @param _in C-string key to push.
     * @return Reference to this wrapper. */
    auto operator<<(const char* _in) -> zpt::JSONObj&;
    /** @brief Pushes key-value pairs from initializer list.
     * @param _list Initializer list of key-value pairs.
     * @return Reference to this wrapper. */
    auto operator<<(std::initializer_list<zpt::json> _list) -> zpt::JSONObj&;
    /** @brief Pushes a value for the pending key.
     * @tparam T Type of value to push.
     * @param _in Value to push.
     * @return Reference to this wrapper. */
    template<typename T>
    auto operator<<(T _in) -> zpt::JSONObj&;
    /** @brief Accesses element by index or key.
     * @tparam T Type of index/key.
     * @param _idx Index or key to access.
     * @return Reference to the JSON value at that index/key. */
    template<typename T>
    auto operator[](T _idx) -> zpt::json&;
    /** @brief Accesses element by index or key (const).
     * @tparam T Type of index/key.
     * @param _idx Index or key to access.
     * @return Const JSON value at that index/key. */
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
    /** @brief Constructs an empty JSON array wrapper.
     * @return void (constructors implicitly initialize the object). */
    JSONArr();
    /** @brief Copy constructor (shared semantics).
     * @param _rhs Another JSONArr wrapper to copy.
     * @return void (constructors implicitly initialize the object). */
    JSONArr(const JSONArr& _rhs);
    /** @brief Move constructor.
     * @param _rhs JSONArr wrapper to move from.
     * @return void (constructors implicitly initialize the object). */
    JSONArr(JSONArr&& _rhs);
    /** @brief Takes ownership of a raw JSONArrT pointer.
     * @param _target Raw JSONArrT pointer to adopt.
     * @return void (constructors implicitly initialize the object). */
    JSONArr(zpt::JSONArrT* _target);
    /** @brief Destroys the JSON array wrapper.
     * @return void (destructors implicitly clean up the object). */
    virtual ~JSONArr();

    /** @brief Computes hash value for use in containers.
     * @return Hash value of the underlying array. */
    auto hash() const -> size_t;

    /** @brief Converts to JSON string.
     * @return String representation of the underlying array. */
    operator std::string();
    /** @brief Converts to pretty-printed string.
     * @return Pretty-printed wrapper of the underlying array. */
    operator zpt::pretty();

    /** @brief Copy assignment (shared semantics).
     * @param _rhs Another JSONArr wrapper to copy.
     * @return Reference to this wrapper. */
    auto operator=(const zpt::JSONArr& _rhs) -> zpt::JSONArr&;
    /** @brief Move assignment.
     * @param _rhs JSONArr wrapper to move from.
     * @return Reference to this wrapper. */
    auto operator=(zpt::JSONArr&& _rhs) -> zpt::JSONArr&;

    /** @brief Accesses underlying JSONArrT (non-const pointer).
     * @return Pointer to underlying JSONArrT. */
    auto operator->() -> zpt::JSONArrT*;
    /** @brief Dereferences to underlying JSONArrT (non-const reference).
     * @return Reference to underlying JSONArrT. */
    auto operator*() -> zpt::JSONArrT&;
    /** @brief Accesses underlying JSONArrT (const pointer).
     * @return Const pointer to underlying JSONArrT. */
    auto operator->() const -> zpt::JSONArrT const*;
    /** @brief Dereferences to underlying JSONArrT (const reference).
     * @return Const reference to underlying JSONArrT. */
    auto operator*() const -> zpt::JSONArrT const&;

    /** @brief Equality comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if equal. */
    template<typename T>
    auto operator==(T _rhs) const -> bool;
    /** @brief Inequality comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if not equal. */
    template<typename T>
    auto operator!=(T _rhs) const -> bool;
    /** @brief Less-than comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is less than _rhs. */
    template<typename T>
    auto operator<(T _rhs) const -> bool;
    /** @brief Greater-than comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is greater than _rhs. */
    template<typename T>
    auto operator>(T _rhs) const -> bool;
    /** @brief Less-than-or-equal comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is <= _rhs. */
    template<typename T>
    auto operator<=(T _rhs) const -> bool;
    /** @brief Greater-than-or-equal comparison.
     * @tparam T Type to compare with.
     * @param _rhs Value to compare with.
     * @return True if this is >= _rhs. */
    template<typename T>
    auto operator>=(T _rhs) const -> bool;
    /** @brief Appends elements from initializer list.
     * @param _list Initializer list of values to append.
     * @return Reference to this wrapper. */
    auto operator<<(std::initializer_list<zpt::json> _list) -> JSONArr&;
    /** @brief Appends an element to the array.
     * @tparam T Type of value to append.
     * @param _in Value to append.
     * @return Reference to this wrapper. */
    template<typename T>
    auto operator<<(T _in) -> JSONArr&;
    /** @brief Accesses element by index.
     * @tparam T Type of index.
     * @param _idx Index of element to access.
     * @return Reference to the JSON value at that index. */
    template<typename T>
    auto operator[](T _idx) -> json&;
    /** @brief Accesses element by index (const).
     * @tparam T Type of index.
     * @param _idx Index of element to access.
     * @return Const JSON value at that index. */
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
    /** @brief Constructs an empty regex.
     * @return void (constructors implicitly initialize the object). */
    JSONRegex();
    /** @brief Copy constructor.
     * @param _rhs Another JSONRegex to copy.
     * @return void (constructors implicitly initialize the object). */
    JSONRegex(const zpt::JSONRegex& _rhs);
    /** @brief Move constructor.
     * @param _rhs JSONRegex to move from.
     * @return void (constructors implicitly initialize the object). */
    JSONRegex(zpt::JSONRegex&& _rhs);
    /**
     * @brief Constructs from a regex pattern string.
     * @param _target Regular expression pattern.
     * @throws std::regex_error If pattern is invalid.
     * @return void (constructors implicitly initialize the object).
     */
    JSONRegex(std::string const& _target);
    /** @brief Destroys the regex.
     * @return void (destructors implicitly clean up the object). */
    virtual ~JSONRegex();

    /** @brief Copy assignment.
     * @param _rhs Another JSONRegex to copy.
     * @return Reference to this regex. */
    auto operator=(const zpt::JSONRegex& _rhs) -> zpt::JSONRegex&;
    /** @brief Move assignment.
     * @param _rhs JSONRegex to move from.
     * @return Reference to this regex. */
    auto operator=(zpt::JSONRegex&& _rhs) -> zpt::JSONRegex&;

    /** @brief Access underlying std::regex (non-const pointer).
     * @return Pointer to underlying std::regex. */
    auto operator->() -> std::regex*;
    /** @brief Dereference to underlying std::regex (non-const reference).
     * @return Reference to underlying std::regex. */
    auto operator*() -> std::regex&;
    /** @brief Access underlying std::regex (const pointer).
     * @return Const pointer to underlying std::regex. */
    auto operator->() const -> std::regex const*;
    /** @brief Dereference to underlying std::regex (const reference).
     * @return Const reference to underlying std::regex. */
    auto operator*() const -> std::regex const&;

    /** @brief Converts to pretty-printed pattern string.
     * @return Pretty-printed wrapper of the pattern. */
    operator zpt::pretty();
    /** @brief Converts to std::regex reference.
     * @return Reference to underlying std::regex. */
    operator std::regex&();

    /** @brief Compares regex patterns (JSONRegex).
     * @param _rhs Another JSONRegex to compare with.
     * @return True if patterns are equal. */
    auto operator==(zpt::regex _rhs) const -> bool;
    /** @brief Compares regex patterns (json).
     * @param _rhs JSON value to compare with.
     * @return True if patterns are equal. */
    auto operator==(zpt::json _rhs) const -> bool;
    /** @brief Tests if string matches this regex.
     * @param _rhs String to test.
     * @return True if string matches the pattern. */
    auto operator==(std::string const& _rhs) const -> bool;
    /** @brief Compares regex patterns (JSONRegex, inequality).
     * @param _rhs Another JSONRegex to compare with.
     * @return True if patterns differ. */
    auto operator!=(zpt::regex _rhs) const -> bool;
    /** @brief Compares regex patterns (json, inequality).
     * @param _rhs JSON value to compare with.
     * @return True if patterns differ. */
    auto operator!=(zpt::json _rhs) const -> bool;
    /** @brief Tests if string does not match this regex.
     * @param _rhs String to test.
     * @return True if string does not match the pattern. */
    auto operator!=(std::string const& _rhs) const -> bool;

    friend auto operator<<(std::ostream& _out, JSONRegex& _in) -> std::ostream& {
        _out << "/" << _in.to_string() << "/" << std::flush;
        return _out;
    }

    /** @brief Returns the original pattern string.
     * @return The original regex pattern string. */
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
    /** @brief Constructs context wrapping a pointer.
     * @param _target Opaque pointer to wrap. */
    JSONContext(void* _target);
    virtual ~JSONContext();

    /** @brief Returns the wrapped pointer.
     * @return The opaque pointer passed at construction. */
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
    /** @brief Constructs context wrapping a pointer.
     * @param _target Opaque pointer to wrap. */
    context(void* _target);
    /** @brief Copy constructor (shared semantics).
     * @param _rhs Another context to copy. */
    context(const context& _rhs);
    /** @brief Move constructor.
     * @param _rhs Context to move from. */
    context(context&& _rhs);
    virtual ~context();

    /** @brief Accesses underlying JSONContext (non-const pointer).
     * @return Pointer to underlying JSONContext. */
    auto operator->() -> zpt::JSONContext*;
    /** @brief Dereferences to underlying JSONContext (non-const reference).
     * @return Reference to underlying JSONContext. */
    auto operator*() -> zpt::JSONContext&;
    /** @brief Accesses underlying JSONContext (const pointer).
     * @return Const pointer to underlying JSONContext. */
    auto operator->() const -> zpt::JSONContext const*;
    /** @brief Dereferences to underlying JSONContext (const reference).
     * @return Const reference to underlying JSONContext. */
    auto operator*() const -> zpt::JSONContext const&;

    /** @brief Copy assignment (shared semantics).
     * @param _rhs Another context to copy.
     * @return Reference to this context. */
    auto operator=(const zpt::context& _rhs) -> zpt::context&;
    /** @brief Move assignment.
     * @param _rhs Context to move from.
     * @return Reference to this context. */
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

    /** @brief Computes hash for use in containers.
     * @return Hash value of the lambda signature. */
    auto hash() const -> size_t;

    /**
     * @brief Invokes the lambda with arguments.
     * @param _args JSON array of arguments.
     * @param _ctx Execution context.
     * @return JSON result.
     */
    virtual auto operator()(zpt::json _args, zpt::context _ctx) -> zpt::json;
    /**
     * @brief Invokes the lambda with arguments (const).
     * @param _args JSON array of arguments.
     * @param _ctx Execution context.
     * @return JSON result.
     */
    virtual auto operator()(zpt::json _args, zpt::context _ctx) const -> zpt::json;

    /** @name Static Registration Methods */
    ///@{
    /** @brief Registers a lambda by signature string.
     * @param _signature Lambda signature in "name/n_args" format.
     * @param _lambda Function implementing the lambda. */
    static auto add(std::string const& _signature, zpt::symbol _lambda) -> void;
    /** @brief Registers a lambda by name and argument count.
     * @param _name Lambda name.
     * @param _n_args Number of expected arguments.
     * @param _lambda Function implementing the lambda. */
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

    /** @brief Creates signature string from name and argument count.
     * @param _name Lambda name.
     * @param _n_args Number of arguments.
     * @return Signature string in "name/n_args" format. */
    static auto stringify(std::string const& _name, unsigned short _n_args) -> std::string;
    /** @brief Parses signature string into (name, n_args) tuple.
     * @param _signature Signature string in "name/n_args" format.
     * @return Tuple of (name, n_args). */
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
    /** @brief Constructs from signature string "name/n_args".
     * @param _signature Lambda signature. */
    JSONLambda(std::string const& _signature);
    /** @brief Constructs from name and argument count.
     * @param _name Lambda name.
     * @param _n_args Number of expected arguments. */
    JSONLambda(std::string const& _name, unsigned short _n_args);
    virtual ~JSONLambda();

    /**
     * @brief Invokes the lambda function.
     * @param _args JSON array of arguments.
     * @param _ctx Execution context.
     * @return JSON result.
     */
    virtual auto call(zpt::json _args, zpt::context _ctx) -> zpt::json;

    /** @brief Returns the lambda name.
     * @return Lambda name string. */
    virtual auto name() const -> std::string;
    /** @brief Returns the expected argument count.
     * @return Number of expected arguments. */
    virtual auto n_args() const -> unsigned short;
    /** @brief Returns the signature string "name/n_args".
     * @return Signature string. */
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
    /** @brief Constructs a null element.
     * @return void (constructors implicitly initialize the object). */
    JSONElementT();
    /** @brief Constructs an element of the specified type.
     * @param _in JSON type to construct.
     * @return void (constructors implicitly initialize the object). */
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
    /** @brief Destroys the JSON element.
     * @return void (destructors implicitly clean up the object). */
    virtual ~JSONElementT();

    /** @name Type Introspection */
    ///@{
    /** @brief Returns the current value type.
     * @return JSONType enum value. */
    virtual auto type() const -> JSONType;
    /** @brief Returns human-readable type name.
     * @return Type name string. */
    virtual auto demangle() const -> std::string;
    /** @brief Changes the value type (resets value).
     * @param _in New JSONType to set.
     * @return Reference to this element. */
    virtual auto type(JSONType _in) -> JSONElementT&;
    /** @brief Returns true if value is not null or undefined.
     * @return True if value is present. */
    virtual auto ok() const -> bool;
    /** @brief Returns true if container is empty or value is null/undefined.
     * @return True if empty. */
    virtual auto empty() const -> bool;
    /** @brief Returns true if value is null.
     * @return True if null. */
    virtual auto nil() const -> bool;
    ///@}

    /** @brief Resets to null value.
     * @return Reference to this element. */
    virtual auto clear() -> void;
    /** @brief Returns element count (1 for scalars, size for containers).
     * @return Element count. */
    virtual auto size() const -> size_t;
    /** @brief Computes hash for use in containers.
     * @return Hash value. */
    virtual auto hash() const -> size_t;

    /** @brief Searches for a value in this container.
     * @param _to_find Value to search for.
     * @return Iterator to the found value, or end(). */
    auto find(zpt::json _to_find) const -> zpt::json::iterator;
    /** @brief Returns true if this container contains the value.
     * @param _to_find Value to search for.
     * @return True if found. */
    auto contains(zpt::json _to_find) const -> bool;

    /** @brief Returns parent element (for nested values).
     * @return Pointer to parent element, or nullptr. */
    auto parent() -> JSONElementT*;
    /** @brief Sets parent element.
     * @param _parent Pointer to parent element.
     * @return Reference to this element. */
    auto parent(JSONElementT* _parent) -> JSONElementT&;

    /** @brief Creates a deep copy of this element.
     * @return Deep copy as JSON value. */
    virtual auto clone() const -> zpt::json;

    /** @name Type Predicates */
    ///@{
    /** @brief Returns true if this element is a JSON object.
     * @return True if object type. */
    virtual auto is_object() const -> bool;
    /** @brief Returns true if this element is a JSON array.
     * @return True if array type. */
    virtual auto is_array() const -> bool;
    /** @brief Returns true if this element is a JSON string.
     * @return True if string type. */
    virtual auto is_string() const -> bool;
    /** @brief Returns true if this element is a JSON integer.
     * @return True if integer type. */
    virtual auto is_integer() const -> bool;
    /** @brief Returns true if this element is a JSON floating-point.
     * @return True if floating-point type. */
    virtual auto is_floating() const -> bool;
    /** @brief Returns true if integer or floating-point.
     * @return True if numeric type. */
    virtual auto is_number() const -> bool;
    /** @brief Returns true if this element is a JSON boolean.
     * @return True if boolean type. */
    virtual auto is_bool() const -> bool;
    /** @brief Returns true if this element is a date/timestamp.
     * @return True if date type. */
    virtual auto is_date() const -> bool;
    /** @brief Returns true if this element is a lambda function.
     * @return True if lambda type. */
    virtual auto is_lambda() const -> bool;
    /** @brief Returns true if this element is a regex.
     * @return True if regex type. */
    virtual auto is_regex() const -> bool;
    /** @brief Returns true if this element is JSON null.
     * @return True if null. */
    virtual auto is_nil() const -> bool;
    /** @brief Returns true if this element is undefined (void*).
     * @return True if undefined. */
    virtual auto is_undefined() const -> bool;
    ///@}

    /** @name Value Accessors (Mutable) */
    ///@{
    /** @brief Returns object value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the JSONObj value. */
    virtual auto object() -> JSONObj&;
    /** @brief Returns array value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the JSONArr value. */
    virtual auto array() -> JSONArr&;
    /** @brief Returns string value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the string value. */
    virtual auto string() -> std::string&;
    /** @brief Returns integer value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the integer value. */
    virtual auto integer() -> long long&;
    /** @brief Returns floating-point value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the double value. */
    virtual auto floating() -> double&;
    /** @brief Returns boolean value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the boolean value. */
    virtual auto boolean() -> bool&;
    /** @brief Returns date/timestamp value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the timestamp value. */
    virtual auto date() -> zpt::timestamp_t&;
    /** @brief Returns lambda value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the lambda value. */
    virtual auto lambda() -> zpt::lambda&;
    /** @brief Returns regex value.
     * @throws zpt::CastException if wrong type.
     * @return Reference to the regex value. */
    virtual auto regex() -> zpt::regex&;
    /** @brief Returns numeric value as double (works for int or float).
     * @throws zpt::CastException if wrong type.
     * @return Numeric value as double. */
    virtual auto number() -> double;
    ///@}

    /** @name Value Accessors (Const) */
    ///@{
    /** @brief Returns object value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the JSONObj value. */
    virtual auto object() const -> JSONObj const&;
    /** @brief Returns array value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the JSONArr value. */
    virtual auto array() const -> JSONArr const&;
    /** @brief Returns string value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the string value. */
    virtual auto string() const -> std::string const&;
    /** @brief Returns integer value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the integer value. */
    virtual auto integer() const -> long long const&;
    /** @brief Returns floating-point value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the double value. */
    virtual auto floating() const -> double const&;
    /** @brief Returns boolean value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the boolean value. */
    virtual auto boolean() const -> bool const&;
    /** @brief Returns date/timestamp value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the timestamp value. */
    virtual auto date() const -> zpt::timestamp_t const&;
    /** @brief Returns lambda value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the lambda value. */
    virtual auto lambda() const -> zpt::lambda const&;
    /** @brief Returns regex value (const).
     * @throws zpt::CastException if wrong type.
     * @return Const reference to the regex value. */
    virtual auto regex() const -> zpt::regex const&;
    /** @brief Returns numeric value as double (const).
     * @throws zpt::CastException if wrong type.
     * @return Numeric value as double. */
    virtual auto number() const -> double;
    ///@}

    /** @brief Copy assignment.
     * @param _rhs Another JSONElementT to copy.
     * @return Reference to this element. */
    auto operator=(const JSONElementT& _rhs) -> JSONElementT&;
    /** @brief Move assignment.
     * @param _rhs JSONElementT to move from.
     * @return Reference to this element. */
    auto operator=(JSONElementT&& _rhs) -> JSONElementT&;
    /** @brief Assigns a string value.
     * @param _rhs String value.
     * @return Reference to this element. */
    auto operator=(std::string const& _rhs) -> JSONElementT&;
    /** @brief Assigns a null value.
     * @param _rhs Null pointer.
     * @return Reference to this element. */
    auto operator=(std::nullptr_t) -> JSONElementT&;
    /** @brief Assigns a C-string value.
     * @param _rhs C-string value.
     * @return Reference to this element. */
    auto operator=(const char* _rhs) -> JSONElementT&;
    /** @brief Assigns an integer value.
     * @param _rhs Integer value.
     * @return Reference to this element. */
    auto operator=(long long _rhs) -> JSONElementT&;
    /** @brief Assigns a floating-point value.
     * @param _rhs Double value.
     * @return Reference to this element. */
    auto operator=(double _rhs) -> JSONElementT&;
    /** @brief Assigns a boolean value.
     * @param _rhs Boolean value.
     * @return Reference to this element. */
    auto operator=(bool _rhs) -> JSONElementT&;
    /** @brief Assigns an integer value.
     * @param _rhs Integer value.
     * @return Reference to this element. */
    auto operator=(int _rhs) -> JSONElementT&;
    /** @brief Assigns a size_t value.
     * @param _rhs Size_t value.
     * @return Reference to this element. */
    auto operator=(size_t _rhs) -> JSONElementT&;
#ifdef __LP64__
    /** @brief Assigns an unsigned int value.
     * @param _rhs Unsigned int value.
     * @return Reference to this element. */
    auto operator=(unsigned int _rhs) -> JSONElementT&;
#endif
    /** @brief Assigns a JSON value.
     * @param _rhs JSON value.
     * @return Reference to this element. */
    auto operator=(zpt::json _rhs) -> JSONElementT&;
    /** @brief Assigns a timestamp value.
     * @param _rhs Timestamp value.
     * @return Reference to this element. */
    auto operator=(zpt::timestamp_t _rhs) -> JSONElementT&;
    /** @brief Assigns a JSONObj value.
     * @param _rhs JSONObj reference.
     * @return Reference to this element. */
    auto operator=(zpt::JSONObj& _rhs) -> JSONElementT&;
    /** @brief Assigns a JSONArr value.
     * @param _rhs JSONArr reference.
     * @return Reference to this element. */
    auto operator=(zpt::JSONArr& _rhs) -> JSONElementT&;
    /** @brief Assigns a lambda value.
     * @param _rhs Lambda value.
     * @return Reference to this element. */
    auto operator=(zpt::lambda _rhs) -> JSONElementT&;
    /** @brief Assigns a regex value.
     * @param _rhs Regex value.
     * @return Reference to this element. */
    auto operator=(zpt::regex _rhs) -> JSONElementT&;
    /** @brief Assigns an undefined (void*) value.
     * @param _rhs Void pointer.
     * @return Reference to this element. */
    auto operator=(void*) -> JSONElementT&;

    /** @brief Converts to string.
     * @return String value. */
    operator std::string();
    /** @brief Converts to boolean.
     * @return Boolean value. */
    operator bool();
    /** @brief Converts to int.
     * @return Integer value. */
    operator int();
    /** @brief Converts to long.
     * @return Long value. */
    operator long();
    /** @brief Converts to long long.
     * @return Long long value. */
    operator long long();
    /** @brief Converts to size_t.
     * @return Size_t value. */
    operator size_t();
    /** @brief Converts to double.
     * @return Double value. */
    operator double();
#ifdef __LP64__
    /** @brief Converts to unsigned int.
     * @return Unsigned int value. */
    operator unsigned int();
#endif
    /** @brief Converts to timestamp.
     * @return Timestamp value. */
    operator zpt::timestamp_t();
    /** @brief Converts to JSONObj.
     * @return JSONObj value. */
    operator zpt::JSONObj();
    /** @brief Converts to JSONArr.
     * @return JSONArr value. */
    operator zpt::JSONArr();
    /** @brief Converts to JSONObj reference.
     * @return JSONObj reference. */
    operator zpt::JSONObj&();
    /** @brief Converts to JSONArr reference.
     * @return JSONArr reference. */
    operator zpt::JSONArr&();
    /** @brief Converts to lambda.
     * @return Lambda value. */
    operator zpt::lambda();
    /** @brief Converts to regex.
     * @return Regex value. */
    operator zpt::regex();
    /** @brief Converts to regex reference.
     * @return Regex reference. */
    operator zpt::regex&();

    /** @brief Appends a C-string value.
     * @param _in C-string to append.
     * @return Reference to this element. */
    auto operator<<(const char* _in) -> JSONElementT&;
    /** @brief Appends a string value.
     * @param _in String to append.
     * @return Reference to this element. */
    auto operator<<(std::string const& _in) -> JSONElementT&;
    /** @brief Appends a JSON value.
     * @param _in JSON value to append.
     * @return Reference to this element. */
    auto operator<<(zpt::json _in) -> JSONElementT&;
    /** @brief Appends a value of any type.
     * @tparam T Value type.
     * @param _in Value to append.
     * @return Reference to this element. */
    template<typename T>
    auto operator<<(T _in) -> JSONElementT&;
    /** @brief Accesses element by index or key (non-const).
     * @tparam T Index/key type.
     * @param _idx Index or key to access.
     * @return Reference to the JSON value at that index/key. */
    template<typename T>
    auto operator[](T _idx) -> json&;
    /** @brief Accesses element by index or key (const).
     * @tparam T Index/key type.
     * @param _idx Index or key to access.
     * @return Const JSON value at that index/key. */
    template<typename T>
    auto operator[](T _idx) const -> json const;
    /** @brief Equality comparison (JSONElementT).
     * @param _in Another JSONElementT to compare with.
     * @return True if equal. */
    auto operator==(JSONElementT const& _in) const -> bool;
    /** @brief Equality comparison (json).
     * @param _rhs JSON value to compare with.
     * @return True if equal. */
    auto operator==(zpt::json _rhs) const -> bool;
    /** @brief Equality comparison (any type).
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if equal. */
    template<typename T>
    auto operator==(T _in) const -> bool;
    /** @brief Inequality comparison (JSONElementT).
     * @param _in Another JSONElementT to compare with.
     * @return True if not equal. */
    auto operator!=(JSONElementT const& _in) const -> bool;
    /** @brief Inequality comparison (json).
     * @param _rhs JSON value to compare with.
     * @return True if not equal. */
    auto operator!=(zpt::json _rhs) const -> bool;
    /** @brief Inequality comparison (any type).
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if not equal. */
    template<typename T>
    auto operator!=(T _in) const -> bool;
    /** @brief Less-than comparison (JSONElementT).
     * @param _in Another JSONElementT to compare with.
     * @return True if this is less than _in. */
    auto operator<(JSONElementT const& _in) const -> bool;
    /** @brief Less-than comparison (json).
     * @param _rhs JSON value to compare with.
     * @return True if this is less than _rhs. */
    auto operator<(zpt::json _rhs) const -> bool;
    /** @brief Less-than comparison (any type).
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is less than _in. */
    template<typename T>
    auto operator<(T _in) const -> bool;
    /** @brief Greater-than comparison (JSONElementT).
     * @param _in Another JSONElementT to compare with.
     * @return True if this is greater than _in. */
    auto operator>(JSONElementT const& _in) const -> bool;
    /** @brief Greater-than comparison (json).
     * @param _rhs JSON value to compare with.
     * @return True if this is greater than _rhs. */
    auto operator>(zpt::json _rhs) const -> bool;
    /** @brief Greater-than comparison (any type).
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is greater than _in. */
    template<typename T>
    auto operator>(T _in) const -> bool;
    /** @brief Less-than-or-equal comparison (JSONElementT).
     * @param _in Another JSONElementT to compare with.
     * @return True if this is <= _in. */
    auto operator<=(JSONElementT const& _in) const -> bool;
    /** @brief Less-than-or-equal comparison (json).
     * @param _rhs JSON value to compare with.
     * @return True if this is <= _rhs. */
    auto operator<=(zpt::json _rhs) const -> bool;
    /** @brief Less-than-or-equal comparison (any type).
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is <= _in. */
    template<typename T>
    auto operator<=(T _in) const -> bool;
    /** @brief Greater-than-or-equal comparison (JSONElementT).
     * @param _in Another JSONElementT to compare with.
     * @return True if this is >= _in. */
    auto operator>=(JSONElementT const& _in) const -> bool;
    /** @brief Greater-than-or-equal comparison (json).
     * @param _rhs JSON value to compare with.
     * @return True if this is >= _rhs. */
    auto operator>=(zpt::json _rhs) const -> bool;
    /** @brief Greater-than-or-equal comparison (any type).
     * @tparam T Type to compare with.
     * @param _in Value to compare with.
     * @return True if this is >= _in. */
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
     * @return Reference to this element.
     */
    auto set_path(std::string const& _path, zpt::json _value, std::string const& _separator = ".")
      -> JSONElementT&;
    /** @brief Removes value at path.
     * @param _path Path string.
     * @param _separator Path separator.
     * @return Reference to this element.
     */
    auto del_path(std::string const& _path, std::string const& _separator = ".") -> JSONElementT&;
    ///@}

    /** @name Serialization */
    ///@{
    /** @brief Serializes to compact JSON string (non-const string ref).
     * @param _out Output string to write to.
     * @return Reference to this element. */
    virtual auto stringify(std::string& _out) -> JSONElementT&;
    /** @brief Serializes to compact JSON string (ostream).
     * @param _out Output stream to write to.
     * @return Reference to this element. */
    virtual auto stringify(std::ostream& _out) -> JSONElementT&;
    /** @brief Serializes to compact JSON string (const, string ref).
     * @param _out Output string to write to.
     * @return Const reference to this element. */
    virtual auto stringify(std::string& _out) const -> JSONElementT const&;
    /** @brief Serializes to compact JSON string (const, ostream).
     * @param _out Output stream to write to.
     * @return Const reference to this element. */
    virtual auto stringify(std::ostream& _out) const -> JSONElementT const&;
    /** @brief Returns compact JSON string representation.
     * @return JSON string. */
    virtual auto stringify() const -> std::string;

    /** @brief Serializes to formatted JSON with indentation (non-const string ref).
     * @param _out Output string to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Reference to this element. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> JSONElementT&;
    /** @brief Serializes to formatted JSON with indentation (ostream).
     * @param _out Output stream to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Reference to this element. */
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> JSONElementT&;
    /** @brief Serializes to formatted JSON with indentation (const, string ref).
     * @param _out Output string to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Const reference to this element. */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) const -> JSONElementT const&;
    /** @brief Serializes to formatted JSON with indentation (const, ostream).
     * @param _out Output stream to write to.
     * @param _n_tabs Number of indentation tabs (default 0).
     * @return Const reference to this element. */
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) const -> JSONElementT const&;
    /** @brief Returns formatted JSON string representation.
     * @return Pretty-printed JSON string. */
    virtual auto prettify() const -> std::string;
    ///@}

    /**
     * @brief Returns element at position as (index, key, value) tuple.
     * @param _pos Position index.
     * @return Tuple for iteration; key is empty for arrays.
     */
    virtual auto element(size_t _pos) -> zpt::json::element;

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
  : __underlying(zpt::allocate_shared<zpt::JSONElementT>(_rhs)) {}
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
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
}
template<typename T>
auto zpt::json::integer(T _e) -> zpt::json {
    long long int _v(_e);
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
}
template<typename T>
auto zpt::json::uinteger(T _e) -> zpt::json {
    unsigned int _v(_e);
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
}
template<typename T>
auto zpt::json::floating(T _e) -> zpt::json {
    double _v(_e);
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
}
template<typename T>
auto zpt::json::ulong(T _e) -> zpt::json {
    size_t _v(_e);
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
}
template<typename T>
auto zpt::json::boolean(T _e) -> zpt::json {
    bool _v(_e);
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
}
template<typename T>
auto zpt::json::date(T _e) -> zpt::json {
    zpt::timestamp_t _v(_e);
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
}
template<typename T>
auto zpt::json::lambda(T _e) -> zpt::json {
    zpt::lambda _v(_e);
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
}
template<typename T>
auto zpt::json::regex(T _e) -> zpt::json {
    zpt::regex _v(_e);
    return zpt::json(zpt::allocate_shared<zpt::JSONElementT>(_v));
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
