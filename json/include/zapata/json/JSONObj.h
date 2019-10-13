/*
Author: n@zgul <n@zgul.me>
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
#include <vector>
#include <zapata/base/assertz.h>
#include <zapata/json/config.h>
#include <zapata/log/log.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

//
// #if !defined __APPLE__
//
// #endif

namespace zpt {

/**
 * \brief Type definition for representing a millisecond based timestamp.
 */
using timestamp_t = unsigned long long;

class JSONElementT;
class JSONObj;
class JSONArr;
class JSONLambda;
class lambda;
class json;

class pretty {
  public:
    pretty(const pretty& _rhs);
    pretty(pretty&& _rhs);
    pretty(std::string _rhs);
    pretty(const char* _rhs);
    template<typename T>
    pretty(T _rhs);
    virtual ~pretty() = default;

    operator std::string();

    auto operator=(const pretty& _rhs) -> pretty&;
    auto operator=(pretty&& _rhs) -> pretty&;

    auto operator-> () -> std::string&;
    auto operator*() -> std::string&;

    friend auto operator<<(std::ostream& _out, zpt::pretty _in) -> std::ostream& {
        _out << std::string(_in.__underlying.data());
        return _out;
    }

  private:
    std::string __underlying{ "" };
};

/**
 * \brief Smart shared pointer to a zpt::JSONElementT object.
 */
class json {
  public:
    // class element {
    //   public:
    //     element(size_t _index, zpt::json& _value);
    //     element(size_t _index, std::string _key, zpt::json& _value);
    //     element(const element& _rhs);
    //     element(element&& _rhs);
    //     virtual ~element() = default;
    //     auto index() -> size_t;
    //     auto key() -> std::string&;
    //     auto value() -> zpt::json;
    //     auto operator=(const element& _rhs) -> element&;
    //     auto operator=(element&& _rhs) -> element&;
    //   private:
    //     size_t __index{ 0 };
    //     std::string __key{ "" };
    //     zpt::json& __value;
    // };
    using element = std::tuple<size_t, std::string, zpt::json>;

    class iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = zpt::json::element;
        using pointer = zpt::json::element;
        using reference = zpt::json::element;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit iterator(zpt::json& _target, size_t _pos = 0);
        iterator(const iterator& _rhs);
        virtual ~iterator() = default;

        // BASIC ITERATOR METHODS //
        auto operator=(const iterator& _rhs) -> iterator&;
        auto operator++() -> iterator&;
        auto operator*() const -> reference;
        // END / BASIC ITERATOR METHODS //

        // INPUT ITERATOR METHODS //
        auto operator++(int) -> iterator;
        auto operator-> () const -> pointer;
        auto operator==(iterator _rhs) const -> bool;
        auto operator!=(iterator _rhs) const -> bool;
        // END / INPUT ITERATOR METHODS //

        // OUTPUT ITERATOR METHODS //
        // reference operator*() const; <- already defined
        // iterator operator++(int); <- already defined
        // END / OUTPUT ITERATOR METHODS //

        // FORWARD ITERATOR METHODS //
        // Enable support for both input and output iterator <- already enabled
        // END / FORWARD ITERATOR METHODS //

        // BIDIRECTIOANL ITERATOR METHODS //
        auto operator--() -> iterator&;
        auto operator--(int) -> iterator;
        // END / BIDIRECTIOANL ITERATOR METHODS //

      private:
        zpt::json& __target;
        size_t __pos{ 0 };
    };

    json();
    // json(zpt::json _target);
    json(zpt::JSONElementT* _target);
    json(std::initializer_list<JSONElementT> _init);
    json(std::string _rhs);
    json(const char* _rhs);
    json(zpt::pretty _rhs);
    json(const zpt::json& _rhs);
    json(zpt::json&& _rhs);
    virtual ~json();

    /**
     * \brief Read-access method for retrieving the value pointed by *this*
     * instance.
     *
     * @return the value pointed by *this* instance
     */
    auto size() -> size_t;
    auto value() -> JSONElementT&;
    auto parse(std::istream& _in) -> void;

    auto begin() -> zpt::json::iterator;
    auto end() -> zpt::json::iterator;

    auto operator=(const zpt::json& _rhs) -> zpt::json&;
    auto operator=(zpt::json&& _rhs) -> zpt::json&;

    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator-> () -> std::shared_ptr<JSONElementT>&;
    auto operator*() -> std::shared_ptr<JSONElementT>&;

    template<typename T>
    auto operator==(T _rhs) -> bool;
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    template<typename T>
    auto operator!=(T _rhs) -> bool;
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<(T _rhs) -> bool;
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>(T _rhs) -> bool;
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<=(T _rhs) -> bool;
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>=(T _rhs) -> bool;
    /**
     * \brief Operator '<<' override form injecting values into *this* instance
     * object. This is a
     * convenience
     * wrapper method for the zpt::JSONObjT or zpt::JSONArrT **push** methods.
     *
     * According to *this* object type, behaviors differ:
     *
     * to add attributes to your object:
     *
     *     zpt::JSONObj _o;
     *     _o << "name" << "Mr zapata";
     *     _o << "serial" << 123;
     *     _o << "sorting_field" << "name";
     *
     * or
     *
     *     zpt::JSONObj _o;
     *     _o <<
     *       "name" << "Mr zapata" <<
     *       "serial" << 123 <<
     *       "sorting_field" << "name";
     *     // this one is more JSON like
     *
     * (when *this* object is a zpt::JSONObj and *T* is std::string, it will
     * either be injected as
     * an attribute name
     * or as an attribute value, depending on whether or not you've already
     * injected an attribute
     * name)
     *
     * to add a JSON array, use the *zpt::JSONArr* class:
     *
     *     zpt::JSONArr _a;
     *     _a << 123 << 345 << 67 << 78;
     *
     *     zpt::JSONArr _b;
     *     _b << "lions" << 345 << "horses" << 78;
     *
     *     zpt::JSONObj _o;
     *     _o <<
     *       "name" << "Mr zapata" <<
     *       "serial" << 123 <<
     *       "sorting_field" << "name" <<
     *       "numbers" << _a <<
     *       "animal_numbers" << _b;
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     */
    template<typename T>
    auto operator<<(T _in) -> json&;
    /**
     * \brief Operator '>>' override for removing attributes or array elements
     * from *this* object
     * instance. This is
     * a convenience wrapper method for the zpt::JSONObjT or zpt::JSONArrT **pop**
     * methods.
     *
     * Allowed types for *T* are: std::string, const char*, int, size_t,.
     */
    template<typename T>
    auto operator>>(T _in) -> json&;
    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* object
     * instance. This is a
     * convenience wrapper method for the zpt::JSONObjT or zpt::JSONArrT '[]'
     * operators.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_obj["some_array"][2]["first_value"] ...
     *
     * Allowed types for *T* are: std::string, const char*, size_t.
     *
     * @return            the pointer to the child object if it exists or
     * zpt::undefined otherwise
     */
    template<typename T>
    auto operator[](T _idx) const -> json;

    /**
     * \brief Casting operator for the std::string class. **All** JSON types are
     * castable to an
     * std::string.
     *
     * @return the std::textual representation of *this* instance JSON typed
     * object
     */
    operator std::string();
    /**
     * \brief Casting operator to a pretty printed std::string class. **All** JSON
     * types are
     * castable to an
     * std::string.
     *
     * @return the pretty printed textual representation of *this* instance JSON
     * typed object
     */
    operator zpt::pretty();
    /**
     * \brief Casting operator for the *bool* basic type. **All** JSON types as
     * castable to a *bool*
     * value:
     *
     * - anything equals to a numerical zero or is empty, is castable to **false**
     * - anything instantiated and different from a numerical zero is castable to
     * **true**.
     *
     * @return the *bool* representation of *this* instance JSON typed object
     */
    operator bool();
    /**
     * \brief Casting operator for the *int* basic type. **All** JSON types as
     * castable to a *int*
     * value:
     *
     * - anything numerical is truncated to *int*
     * - if *this* object instance is a JSON string that an attempt is made to
     * parse the string
     * value as an *int*,
     * zero is returned if unsuccessful
     * - if *this* object instance is a JSON object, the number of attributes is
     * returned, truncated
     * to *int*
     * - if *this* object instance is a JSON array, the number of elements is
     * returned, truncated to
     * *int*
     * - if *this* object instance is a JSON date, the number of milliseconds
     * since *epoch* is
     * returned, truncated
     * to *int*
     *
     * @return the *int* representation of *this* instance JSON typed object
     */
    operator int();
    /**
     * \brief Casting operator for the *long* basic type. **All** JSON types as
     * castable to a *long*
     * value:
     *
     * - anything numerical is truncated to *long*
     * - if *this* object instance is a JSON string that an attempt is made to
     * parse the string
     * value as a *long*,
     * zero is returned if unsuccessful
     * - if *this* object instance is a JSON object, the number of attributes is
     * returned, truncated
     * to *long*
     * - if *this* object instance is a JSON array, the number of elements is
     * returned, truncated to
     * *long*
     * - if *this* object instance is a JSON date, the number of milliseconds
     * since *epoch* is
     * returned, truncated
     * to *long*
     *
     * @return the *long* representation of *this* instance JSON typed object
     */
    operator long();
    /**
     * \brief Casting operator for the *long long* basic type. **All** JSON types
     * as castable to a
     * *long long*
     * value:
     *
     * - anything numerical is truncated to *long long*
     * - if *this* object instance is a JSON string that an attempt is made to
     * parse the string
     * value as a *long
     * long*, zero is returned if unsuccessful
     * - if *this* object instance is a JSON object, the number of attributes is
     * returned, truncated
     * to *long long*
     * - if *this* object instance is a JSON array, the number of elements is
     * returned, truncated to
     * *long long*
     * - if *this* object instance is a JSON date, the number of milliseconds
     * since *epoch* is
     * returned, truncated
     * to *long long*
     *
     * @return the *long long* representation of *this* instance JSON typed object
     */
    operator long long();
#ifdef __LP64__
    /**
     * \brief Casting operator for the *unsigned int* basic type. **All** JSON
     * types as castable to
     * a *unsigned int*
     * value:
     *
     * - anything numerical is truncated to *unsigned int*
     * - if *this* object instance is a JSON string that an attempt is made to
     * parse the string
     * value as an
     * *unsigned int*, zero is returned if unsuccessful
     * - if *this* object instance is a JSON object, the number of attributes is
     * returned, truncated
     * to *unsigned
     * int*
     * - if *this* object instance is a JSON array, the number of elements is
     * returned, truncated to
     * *unsigned int*
     * - if *this* object instance is a JSON date, the number of milliseconds
     * since *epoch* is
     * returned, truncated
     * to *unsigned int*
     *
     * @return the *unsigned int* representation of *this* instance JSON typed
     * object
     */
    operator unsigned int();
#endif
    /**
     * \brief Casting operator for the *size_t* basic type. **All** JSON types as
     * castable to a
     * *size_t* value:
     *
     * - anything numerical is truncated to *size_t*
     * - if *this* object instance is a JSON string that an attempt is made to
     * parse the string
     * value as a *size_t*,
     * zero is returned if unsuccessful
     * - if *this* object instance is a JSON object, the number of attributes is
     * returned, truncated
     * to *size_t*
     * - if *this* object instance is a JSON array, the number of elements is
     * returned, truncated to
     * *size_t*
     * - if *this* object instance is a JSON date, the number of milliseconds
     * since *epoch* is
     * returned, truncated
     * to *size_t*
     *
     * @return the *size_t* representation of *this* instance JSON typed object
     */
    operator size_t();
    /**
     * \brief Casting operator for the *double* basic type. **All** JSON types as
     * castable to a
     * *double* value:
     *
     * - anything numerical is truncated to *double*
     * - if *this* object instance is a JSON string that an attempt is made to
     * parse the string
     * value as a *double*,
     * zero is returned if unsuccessful
     * - if *this* object instance is a JSON object, the number of attributes is
     * returned, truncated
     * to *double*
     * - if *this* object instance is a JSON array, the number of elements is
     * returned, truncated to
     * *double*
     * - if *this* object instance is a JSON date, the number of milliseconds
     * since *epoch* is
     * returned, truncated
     * to *double*
     *
     * @return the *double* representation of *this* instance JSON typed object
     */
    operator double();
    /**
     * \brief Casting operator for the *zpt::timestamp_t* basic type. **All** JSON
     * types as castable
     * to a
     * *zpt::timestamp_t* value:
     *
     * - anything numerical is truncated to *zpt::timestamp_t*
     * - if *this* object instance is a JSON string that an attempt is made to
     * parse the string
     * value as a
     * *zpt::timestamp_t*, zero is returned if unsuccessful
     * - if *this* object instance is a JSON object, the number of attributes is
     * returned, truncated
     * to
     * *zpt::timestamp_t*
     * - if *this* object instance is a JSON array, the number of elements is
     * returned, truncated to
     * *zpt::timestamp_t*
     * - if *this* object instance is a JSON date, the number of milliseconds
     * since *epoch* is
     * returned, truncated
     * to *zpt::timestamp_t*
     *
     * @return the *zpt::timestamp_t* representation of *this* instance JSON typed
     * object
     */
    operator timestamp_t();
    /**
     * \brief Casting operator for *zpt::JSONObj* class. If *this* instance object
     * is not of type
     * zpt::JSONType::JSObject or zpt::JSONType::JSNil, a zpt::assertion is
     * thrown.
     *
     * @return the *zpt::JSONObj* representation of *this* instance JSON typed
     * object
     */
    operator JSONObj();
    /**
     * \brief Casting operator for *zpt::JSONArr* class. If *this* instance object
     * is not of type
     * zpt::JSONType::JSArray or zpt::JSONType::JSNil, a zpt::assertion is thrown.
     *
     * @return the *zpt::JSONArr* representation of *this* instance JSON typed
     * object
     */
    operator JSONArr();
    /**
     * \brief Casting operator for *zpt::JSONObj* class. If *this* instance object
     * is not of type
     * zpt::JSONType::JSObject or zpt::JSONType::JSNil, a zpt::assertion is
     * thrown.
     *
     * @return the *zpt::JSONObj* representation of *this* instance JSON typed
     * object
     */
    operator JSONObj&();
    /**
     * \brief Casting operator for *zpt::JSONArr* class. If *this* instance object
     * is not of type
     * zpt::JSONType::JSArray or zpt::JSONType::JSNil, a zpt::assertion is thrown.
     *
     * @return the *zpt::JSONArr* representation of *this* instance JSON typed
     * object
     */
    operator JSONArr&();

    /**
     * \brief Casting operator for the zpt::lambda class.
     *
     * @return the *zpt::lambda* representation of *this* instance JSON typed
     * object
     */
    operator zpt::lambda();

    template<typename T>
    auto operator+(T _in) -> json;
    template<typename T>
    auto operator-(T _in) -> json;
    template<typename T>
    auto operator/(T _in) -> json;
    template<typename T>
    auto operator|(T _in) -> json;

    /**
     * \brief Friendly '>>' std::istream operator override that parses the textual
     * representation
     * available on an
     * std::istream object into a of a zpt::json object.
     */
    friend auto operator>>(std::istream& _in, zpt::json& _out) -> std::istream& {
        _out.parse(_in);
        return _in;
    }

    template<typename T>
    static auto data(const T _delegate) -> zpt::json;

    static auto stringify(std::string& _str) -> void;

    static auto object() -> zpt::json;
    static auto array() -> zpt::json;
    template<typename T>
    static auto pretty(T _e) -> std::string;
    template<typename T>
    static auto string(T _e) -> zpt::json;
    template<typename T>
    static auto uinteger(T _e) -> zpt::json;
    template<typename T>
    static auto integer(T _e) -> zpt::json;
    template<typename T>
    static auto floating(T _e) -> zpt::json;
    template<typename T>
    static auto ulong(T _e) -> zpt::json;
    template<typename T>
    static auto boolean(T _e) -> zpt::json;
    static auto date(std::string _e) -> zpt::json;
    static auto date() -> zpt::json;
    template<typename T>
    static auto date(T _e) -> zpt::json;
    template<typename T>
    static auto lambda(T _e) -> zpt::json;
    static auto lambda(std::string _name, unsigned short _n_args) -> zpt::json;

  private:
    std::shared_ptr<JSONElementT> __underlying{ nullptr };
};

using JSONElement = zpt::json;

/**
 * \brief Convenience global variable that represents the *undefined* JSON type,
 * to be used in
 * comparisons and default
 * return values.
 *
 * Example:
 *
 *     if (my_json_object["some_attribute"] == zpt::undefined) {
 *         ...
 *     }
 */
extern zpt::json undefined;
extern zpt::json nilptr;
extern zpt::json array;

/**
 * \brief Class that represents the *object* JSON type. It inherits from the
 * std::map class and is
 * composed of
 * std::string and zpt::json key-value pairs.
 */
class JSONObjT {
  public:
    /**
     * \brief Creates a new JSONObjT instance.
     */
    JSONObjT();
    /**
     * \brief Destroys the current JSONObjT instance, freeing all allocated
     * memory. It will free the
     * objects pointed
     * by each zpt::json smart pointer only if there aren't any more
     * std::shared_ptr pointing to
     * it.
     */
    virtual ~JSONObjT();

    /**
     * \brief Retrieves the textual representation of *this* JSON object instance.
     * The textual
     * representation has no
     * empty characters, that is, no spaces, tabs or new lines.
     *
     * @param _out the textual representation for the JSON object
     */
    virtual auto stringify(std::string& _out) -> zpt::JSONObjT&;
    /**
     * \brief Outputs to the std::ostring *out* the textual representation of
     * *this* JSON object
     * instance. The
     * textual representation has no empty characters, that is, no spaces, tabs or
     * new lines.
     *
     * @param _out the std::ostream to output the JSON object representation to
     */
    virtual auto stringify(std::ostream& _out) -> zpt::JSONObjT&;

    /**
     * \brief Retrieves a human readable textual representation of *this* JSON
     * object instance. The
     * textual
     * representation is multi-line indented.
     *
     * @param _out    the textual representation for the JSON object
     * @param _n_tabs the initial number of tabs to indent
     */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;
    /**
     * \brief Outputs to the std::ostring *out* a human readable textual
     * representation of *this*
     * JSON object
     * instance. The textual representation is multi-line indented.
     *
     * @param _out    the std::ostream to output the JSON object representation to
     * @param _n_tabs the initial number of tabs to indent
     */
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONObjT&;

    /**
     * \brief Write-access method that inserts an std::string into *this* object
     * map.
     *
     * If there isn't any std::string in context to be used as name/key the *name*
     * string will be
     * used as one. If
     * there is an std::string in context, then the *name* string is used as
     * value.
     *
     * @param _name the attribute name or attribute value
     */
    virtual auto push(std::string _name) -> zpt::JSONObjT&;
    /**
     * \brief Write-access method that inserts a zpt::JSONElementT object into
     * *this* object map.
     *
     * @param _value [description]
     */
    virtual auto push(JSONElementT& _value) -> zpt::JSONObjT&;
    /**
     * \brief Write-access method that inserts a zpt::JSONElementT object into
     * *this* object map.
     *
     * @param _value [description]
     */
    virtual auto push(JSONElementT* _value) -> zpt::JSONObjT&;

    /**
     * \brief Write-access method that removes the JSON element identified by
     * *idx*.
     *
     * @param _idx the identification of the element to remove
     */
    virtual auto pop(int _idx) -> zpt::JSONObjT&;
    /**
     * \brief Write-access method that removes the JSON element identified by
     * *idx*.
     *
     * @param _idx the identification of the element to remove
     */
    virtual auto pop(size_t _idx) -> zpt::JSONObjT&;
    /**
     * \brief Write-access method that removes the JSON element identified by
     * *idx*.
     *
     * @param _idx the identification of the element to remove
     */
    virtual auto pop(const char* _idx) -> zpt::JSONObjT&;
    /**
     * \brief Write-access method that removes the JSON element identified by
     * *idx*.
     *
     * @param _idx the identification of the element to remove
     */
    virtual auto pop(std::string _idx) -> zpt::JSONObjT&;

    virtual auto key_for(size_t _idx) -> std::string;
    virtual auto index_for(std::string _name) -> size_t;

    /**
     * \brief Read-access method for retrieving a child element represented by the
     * *path* object
     * path.
     *
     * An object path is sequence of child object identifiers, separated by a
     * given character. For
     * instance, the
     * following code
     *
     *     zpt::json child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param  _path      the object path to search for
     * @param  _separator the object path separator
     *
     * @return            the pointer to the child object if it exists or
     * zpt::undefined otherwise
     */
    auto get_path(std::string _path, std::string _separator = ".") -> zpt::json;

    /**
     * \brief Write-access method for adding a child element represented by the
     * *path* object path
     * and with *value*
     * value.
     *
     * An object path is sequence of child object identifiers, separated by a
     * given character. For
     * instance, the
     * following code
     *
     *     zpt::json child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param _path      the object path to search for
     * @param _value     the value to be assigned to the child element
     * @param _separator the object path separator
     */
    auto set_path(std::string _path, zpt::json _value, std::string _separator = ".")
      -> zpt::JSONObjT&;

    /**
     * \brief Write-access method for removing a child element represented by the
     * *path* object
     * path.
     *
     * An object path is sequence of child object identifiers, separated by a
     * given character. For
     * instance, the
     * following code
     *
     *     zpt::json child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param  _path      the object path to search for
     * @param  _separator the object path separator
     */
    auto del_path(std::string _path, std::string _separator = ".") -> JSONObjT&;

    /**
     * @brief Creates a full copy of the JSON representation stored in *this*
     * *zpt::JSONObjT*
     * @return a pointer to the copy of the underlying JSON representation
     */
    auto clone() -> zpt::json;

    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator-> () -> std::map<std::string, std::tuple<zpt::json, size_t>>&;
    auto operator*() -> std::map<std::string, std::tuple<zpt::json, size_t>>&;

    auto operator==(zpt::JSONObjT& _in) -> bool;
    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator==(zpt::JSONObj& _in) -> bool;
    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator==(T _in) -> bool {
        return false;
    }
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    auto operator!=(zpt::JSONObjT& _in) -> bool;
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    auto operator!=(zpt::JSONObj& _in) -> bool;
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    template<typename T>
    auto operator!=(T _in) -> bool {
        return true;
    }
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator<(zpt::JSONObjT& _in) -> bool;
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator<(zpt::JSONObj& _in) -> bool;
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<(T _in) -> bool {
        return false;
    }
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator>(zpt::JSONObjT& _in) -> bool;
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator>(zpt::JSONObj& _in) -> bool;
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>(T _in) -> bool {
        return false;
    }
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator>=(zpt::JSONObjT& _in) -> bool;
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator>=(zpt::JSONObj& _in) -> bool;
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>=(T _in) -> bool {
        return false;
    }
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator<=(zpt::JSONObjT& _in) -> bool;
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator<=(zpt::JSONObj& _in) -> bool;
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<=(T _in) -> bool {
        return false;
    }

    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* object
     * instance.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_obj["some_array"][2]["first_value"] ...
     *
     * @param  _idx the child object identifier
     *
     * @return  the pointer to the child object if it exists or zpt::undefined
     * otherwise
     */
    auto operator[](int _idx) -> zpt::json&;
    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* object
     * instance.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_obj["some_array"][2]["first_value"] ...
     *
     * @param  _idx the child object identifier
     *
     * @return  the pointer to the child object if it exists or zpt::undefined
     * otherwise
     */
    auto operator[](size_t _idx) -> zpt::json&;
    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* object
     * instance.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_obj["some_array"][2]["first_value"] ...
     *
     * @param  _idx the child object identifier
     *
     * @return  the pointer to the child object if it exists or zpt::undefined
     * otherwise
     */
    auto operator[](const char* _idx) -> zpt::json&;
    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* object
     * instance.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_obj["some_array"][2]["first_value"] ...
     *
     * @param  _idx the child object identifier
     *
     * @return  the pointer to the child object if it exists or zpt::undefined
     * otherwise
     */
    auto operator[](std::string _idx) -> zpt::json&;

    /**
     * \brief Friendly '<<' std::ostream operator override that outputs the
     * textual representation
     * of *this* object
     * instance into the *out* stream.
     */
    friend std::ostream& operator<<(std::ostream& _out, zpt::JSONObjT& _in) {
        _in.stringify(_out);
        return _out;
    }

  private:
    std::string __name{ "" };
    std::vector<std::string> __name_to_index;
    std::map<std::string, std::tuple<zpt::json, size_t>> __underlying;
};

/**
 * \brief Class that represents the *array* JSON type. It inherits from the
 * std::vector class and is
 * composed of
 * zpt::json elements.
 */
class JSONArrT {
  public:
    /**
     * \brief Creates a new JSONArrT instance.
     */
    JSONArrT();
    /**
     * \brief Destroys the current JSONArrT instance, freeing all allocated
     * memory. It will free the
     * objects pointed
     * by each zpt::json smart pointer only if there aren't any more
     * std::shared_ptr pointing to
     * it.
     */
    virtual ~JSONArrT();

    /**
     * \brief Retrieves the textual representation of *this* JSON array instance.
     * The textual
     * representation has no
     * empty characters, that is, no spaces, tabs or new lines.
     *
     * @param _out the textual representation for the JSON array
     */
    virtual auto stringify(std::string& _out) -> zpt::JSONArrT&;
    /**
     * \brief Outputs to the std::ostring *out* the textual representation of
     * *this* JSON array
     * instance. The
     * textual representation has no empty characters, that is, no spaces, tabs or
     * new lines.
     *
     * @param _out the std::ostream to output the JSON array representation to
     */
    virtual auto stringify(std::ostream& _out) -> zpt::JSONArrT&;

    /**
     * \brief Retrieves a human readable textual representation of *this* JSON
     * array instance. The
     * textual
     * representation is multi-line indented.
     *
     * @param _out    the textual representation for the JSON array
     * @param _n_tabs the initial number of tabs to indent
     */
    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;
    /**
     * \brief Outputs to the std::ostring *out* a human readable textual
     * representation of *this*
     * JSON array
     * instance. The textual representation is multi-line indented.
     *
     * @param _out    the std::ostream to output the JSON array representation to
     * @param _n_tabs the initial number of tabs to indent
     */
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> zpt::JSONArrT&;

    /**
     * \brief Write-access method that inserts a zpt::JSONElementT object into
     * *this* array vector.
     *
     * @param _value [description]
     */
    virtual auto push(zpt::JSONElementT& _value) -> zpt::JSONArrT&;
    /**
     * \brief Write-access method that inserts a zpt::JSONElementT object into
     * *this* array vector.
     *
     * @param _value [description]
     */
    virtual auto push(zpt::JSONElementT* _value) -> zpt::JSONArrT&;

    /**
     * \brief Write-access method that removes the JSON element identified by
     * *idx*.
     *
     * @param _idx the identification of the element to remove
     */
    virtual auto pop(int _idx) -> zpt::JSONArrT&;
    /**
     * \brief Write-access method that removes the JSON element identified by
     * *idx*.
     *
     * @param _idx the identification of the element to remove
     */
    virtual auto pop(size_t _idx) -> zpt::JSONArrT&;
    /**
     * \brief Write-access method that removes the JSON element identified by
     * *idx*.
     *
     * @param _idx the identification of the element to remove
     */
    virtual auto pop(const char* _idx) -> zpt::JSONArrT&;
    /**
     * \brief Write-access method that removes the JSON element identified by
     * *idx*.
     *
     * @param _idx the identification of the element to remove
     */
    virtual auto pop(std::string _idx) -> zpt::JSONArrT&;

    virtual auto sort() -> zpt::JSONArrT&;
    virtual auto sort(std::function<bool(zpt::json, zpt::json)> _comparator) -> zpt::JSONArrT&;

    /**
     * \brief Read-access method for retrieving a child element represented by the
     * *path* object
     * path.
     *
     * An object path is sequence of child object identifiers, separated by a
     * given character. For
     * instance, the
     * following code
     *
     *     zpt::json child =
     * my_json_array[1]["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::json child =
     * my_json_array->get_path("1.some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::json child =
     * my_json_array->get_path("1/some_array/0/first_field/name", "/");
     *
     * @param  _path      the object path to search for
     * @param  _separator the object path separator
     *
     * @return            the pointer to the child object if it exists or
     * zpt::undefined otherwise
     */
    auto get_path(std::string _path, std::string _separator = ".") -> zpt::json;

    /**
     * \brief Write-access method for adding a child element represented by the
     * *path* object path
     * and with *value*
     * value.
     *
     * An object path is sequence of child object identifiers, separated by a
     * given character. For
     * instance, the
     * following code
     *
     *     zpt::json child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param _path      the object path to search for
     * @param _value     the value to be assigned to the child element
     * @param _separator the object path separator
     */
    auto set_path(std::string _path, zpt::json _value, std::string _separator = ".")
      -> zpt::JSONArrT&;

    /**
     * \brief Write-access method for removing a child element represented by the
     * *path* object
     * path.
     *
     * An object path is sequence of child object identifiers, separated by a
     * given character. For
     * instance, the
     * following code
     *
     *     zpt::json child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::json child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param  _path      the object path to search for
     * @param  _separator the object path separator
     */
    auto del_path(std::string _path, std::string _separator = ".") -> zpt::JSONArrT&;

    /**
     * @brief Creates a full copy of the JSON representation stored in *this*
     * *zpt::JSONArrT*
     * @return a pointer to the copy of the underlying JSON representation
     */
    auto clone() -> zpt::json;

    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator-> () -> std::vector<zpt::json>&;
    auto operator*() -> std::vector<zpt::json>&;

    auto operator==(zpt::JSONArrT& _in) -> bool;
    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator==(zpt::JSONArr& _in) -> bool;
    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator==(T _in) -> bool {
        return false;
    }
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    auto operator!=(zpt::JSONArrT& _in) -> bool;
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    auto operator!=(zpt::JSONArr& _in) -> bool;
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    template<typename T>
    auto operator!=(T _in) -> bool {
        return true;
    }
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator<(zpt::JSONArrT& _in) -> bool;
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator<(zpt::JSONArr& _in) -> bool;
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<(T _in) -> bool {
        return false;
    }
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator>(zpt::JSONArrT& _in) -> bool;
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator>(zpt::JSONArr& _in) -> bool;
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>(T _in) -> bool {
        return false;
    }
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator<=(zpt::JSONArrT& _in) -> bool;
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator<=(zpt::JSONArr& _in) -> bool;
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<=(T _in) -> bool {
        return false;
    }
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator>=(zpt::JSONArrT& _in) -> bool;
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    auto operator>=(zpt::JSONArr& _in) -> bool;
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>=(T _in) -> bool {
        return false;
    }

    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* array
     * instance.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_arr[1]["some_array"][2]["first_value"] ...
     *
     * @param  _idx the child object identifier
     *
     * @return  the pointer to the child object if it exists or zpt::undefined
     * otherwise
     */
    auto operator[](int _idx) -> zpt::json&;
    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* array
     * instance.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_arr[1]["some_array"][2]["first_value"] ...
     *
     * @param  _idx the child object identifier
     *
     * @return  the pointer to the child object if it exists or zpt::undefined
     * otherwise
     */
    auto operator[](size_t _idx) -> zpt::json&;
    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* array
     * instance.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_arr[1]["some_array"][2]["first_value"] ...
     *
     * @param  _idx the child object identifier
     *
     * @return  the pointer to the child object if it exists or zpt::undefined
     * otherwise
     */
    auto operator[](const char* _idx) -> zpt::json&;
    /**
     * \brief Operator '[]' override for accessing attributes or array elements of
     * *this* array
     * instance.
     *
     * Returns the attribute or array element identified by *idx*. It allows
     * chaining, for instance:
     *
     *     my_json_arr[1]["some_array"][2]["first_value"] ...
     *
     * @param  _idx the child object identifier
     *
     * @return  the pointer to the child object if it exists or zpt::undefined
     * otherwise
     */
    auto operator[](std::string _idx) -> zpt::json&;

    /**
     * \brief Friendly '<<' std::ostream operator override that outputs the
     * textual representation
     * of *this* array
     * instance into the *out* stream.
     */
    friend auto operator<<(std::ostream& _out, zpt::JSONArrT& _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

  private:
    std::vector<zpt::json> __underlying;
};

/**
 * \brief Smart shared pointer to a zpt::JSONObjT object.
 */
class JSONObj {
  public:
    /**
     * \brief Creates a new JSONObj instance, pointing to a *null* object.
     */
    JSONObj();
    /**
     * \brief Creates a new JSONObj instance copying the target reference from
     * *rhs*.
     *
     * @param _rhs the smart pointer to copy the target from
     */
    JSONObj(const zpt::JSONObj& _rhs);
    JSONObj(zpt::JSONObj&& _rhs);
    /**
     * \brief Creates a new JSONObj instance, pointing to the *target* object.
     */
    JSONObj(zpt::JSONObjT* _target);
    /**
     * \brief Destroys the current JSONObj instance. It will only free the pointed
     * object if there
     * are no more
     * *shared_ptr* objects pointing to it.
     */
    virtual ~JSONObj();

    /**
     * \brief Cast operation for the std::string class. This is a convenience
     * wrapper operator for
     * zpt::JSONObjT::stringify method.
     *
     * @return the textual representation of *this* object instance
     */
    auto operator=(const zpt::JSONObj& _rhs) -> zpt::JSONObj&;
    auto operator=(zpt::JSONObj&& _rhs) -> zpt::JSONObj&;

    auto operator-> () -> std::shared_ptr<zpt::JSONObjT>&;
    auto operator*() -> std::shared_ptr<zpt::JSONObjT>&;

    operator std::string();
    /**
     * \brief Casting operator to a pretty printed std::string class. **All** JSON
     * types are
     * castable to an
     * std::string.
     *
     * @return the pretty printed textual representation of *this* instance JSON
     * typed object
     */
    operator zpt::pretty();
    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator==(T _rhs) -> bool;
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    template<typename T>
    auto operator!=(T _rhs) -> bool;
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<(T _rhs) -> bool;
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>(T _rhs) -> bool;
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<=(T _rhs) -> bool;
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::json, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>=(T _rhs) -> bool;
    auto operator<<(std::string _in) -> JSONObj&;
    auto operator<<(const char* _in) -> JSONObj&;
    auto operator<<(zpt::JSONElementT& _in) -> JSONObj&;
    auto operator<<(std::initializer_list<zpt::JSONElementT> _list) -> JSONObj&;
    template<typename T>
    auto operator<<(T _in) -> JSONObj&;
    template<typename T>
    auto operator>>(T _in) -> JSONObj&;
    template<typename T>
    auto operator[](T _idx) -> json&;

    friend auto operator<<(std::ostream& _out, JSONObj& _in) -> std::ostream& {
        _in.__underlying->stringify(_out);
        return _out;
    }

  private:
    std::shared_ptr<zpt::JSONObjT> __underlying{ nullptr };
};

class JSONArr {
  public:
    JSONArr();
    JSONArr(const JSONArr& _rhs);
    JSONArr(JSONArr&& _rhs);
    JSONArr(zpt::JSONArrT* _target);
    virtual ~JSONArr();

    // auto begin() -> JSONArrT::iterator;
    // auto end() -> JSONArrT::iterator;
    operator std::string();
    /**
     * \brief Casting operator to a pretty printed std::string class. **All** JSON
     * types are
     * castable to an
     * std::string.
     *
     * @return the pretty printed textual representation of *this* instance JSON
     * typed object
     */
    operator zpt::pretty();

    auto operator=(const zpt::JSONArr& _rhs) -> zpt::JSONArr&;
    auto operator=(zpt::JSONArr&& _rhs) -> zpt::JSONArr&;

    auto operator-> () -> std::shared_ptr<zpt::JSONArrT>&;
    auto operator*() -> std::shared_ptr<zpt::JSONArrT>&;

    template<typename T>
    auto operator==(T _rhs) -> bool;
    template<typename T>
    auto operator!=(T _rhs) -> bool;
    template<typename T>
    auto operator<(T _rhs) -> bool;
    template<typename T>
    auto operator>(T _rhs) -> bool;
    template<typename T>
    auto operator<=(T _rhs) -> bool;
    template<typename T>
    auto operator>=(T _rhs) -> bool;
    auto operator<<(JSONElementT& _in) -> JSONArr&;
    auto operator<<(std::initializer_list<JSONElementT> _list) -> JSONArr&;
    template<typename T>
    auto operator<<(T _in) -> JSONArr&;
    template<typename T>
    auto operator>>(T _in) -> JSONArr&;
    template<typename T>
    auto operator[](T _idx) -> json&;

    friend auto operator<<(std::ostream& _out, JSONArr& _in) -> std::ostream& {
        _in.__underlying->stringify(_out);
        return _out;
    }

  private:
    std::shared_ptr<zpt::JSONArrT> __underlying{ nullptr };
};

using JSONStr = std::shared_ptr<std::string>;

class JSONContext {
  public:
    JSONContext(void* _target);
    virtual ~JSONContext();

    virtual auto unpack() -> void*;

  private:
    void* __target{ nullptr };
};

class context {
  public:
    context(void* _target);
    context(const context& _rhs);
    context(context&& _rhs);
    virtual ~context();

    auto operator-> () -> std::shared_ptr<zpt::JSONContext>&;
    auto operator*() -> std::shared_ptr<zpt::JSONContext>&;

    auto operator=(const zpt::context& _rhs) -> zpt::context& ;
    auto operator=(zpt::context&& _rhs) -> zpt::context& ;

  private:
    std::shared_ptr<zpt::JSONContext> __underlying{ nullptr };
};

using symbol = std::function<zpt::json(zpt::json, unsigned short, zpt::context)>;
using symbol_table =
  std::shared_ptr<std::map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>>;
extern zpt::symbol_table __lambdas;

class lambda : public std::shared_ptr<zpt::JSONLambda> {
  public:
    lambda();
    lambda(std::shared_ptr<zpt::JSONLambda> _target);
    lambda(zpt::lambda& _target);
    lambda(zpt::JSONLambda* _target);
    lambda(std::string _signature);
    lambda(std::string _name, unsigned short _n_args);
    virtual ~lambda();

    virtual auto operator()(zpt::json _args, zpt::context _ctx) -> zpt::json;

    static auto add(std::string _signature, zpt::symbol _lambda) -> void;
    static auto add(std::string _name, unsigned short _n_args, zpt::symbol _lambda) -> void;

    static auto call(std::string _name, zpt::json _args, zpt::context _ctx) -> zpt::json;

    static auto stringify(std::string _name, unsigned short _n_args) -> std::string;
    static auto parse(std::string _signature) -> std::tuple<std::string, unsigned short>;

  private:
    static auto find(std::string _signature) -> zpt::symbol;
    static auto find(std::string _name, unsigned short _1n_args) -> zpt::symbol;
};

class JSONLambda {
  public:
    JSONLambda();
    JSONLambda(std::string _signature);
    JSONLambda(std::string _name, unsigned short _n_args);
    virtual ~JSONLambda();

    virtual auto call(zpt::json _args, zpt::context _ctx) -> zpt::json;

    virtual auto name() -> std::string;
    virtual auto n_args() -> unsigned short;
    virtual auto signature() -> std::string;

  private:
    std::string __name{ "" };
    unsigned short __n_args{ 0 };
};

using JSONUnion = struct JSONStruct {
    JSONStruct()
      : __type(JSNil){};
    ~JSONStruct() {
        switch (__type) {
            case zpt::JSObject: {
                __object.~JSONObj();
                break;
            }
            case zpt::JSArray: {
                __array.~JSONArr();
                break;
            }
            case zpt::JSString: {
                __string.~JSONStr();
                break;
            }
            case zpt::JSLambda: {
                __lambda.~lambda();
                break;
            }
            default: {
                break;
            }
        }
    }

    JSONStruct(JSONStruct const&) = delete;
    JSONStruct& operator=(JSONStruct const&) = delete;

    JSONStruct(JSONStruct&&) = delete;
    JSONStruct& operator=(JSONStruct&&) = delete;

    JSONType __type;
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    union {
#endif
        JSONObj __object;
        JSONArr __array;
        JSONStr __string;
        long long __integer;
        double __double;
        bool __boolean;
        void* __nil;
        zpt::timestamp_t __date;
        zpt::lambda __lambda;
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    };
#endif
};

class JSONElementT {
  public:
    JSONElementT();
    JSONElementT(JSONElementT& _element);
    JSONElementT(std::initializer_list<JSONElementT> _init);
    JSONElementT(json _value);
    JSONElementT(JSONObj& _value);
    JSONElementT(JSONArr& _value);
    JSONElementT(std::string _value);
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
    virtual ~JSONElementT();

    virtual auto type() -> JSONType;
    virtual auto demangle() -> std::string;
    virtual auto type(JSONType _in) -> void;
    virtual auto value() -> JSONUnion&;
    virtual auto ok() -> bool;
    virtual auto empty() -> bool;
    virtual auto nil() -> bool;

    virtual auto size() -> size_t;

    virtual auto assign(JSONElementT& _rhs) -> void;
    template<typename T>
    auto assign(T _in) -> void;

    auto parent() -> JSONElementT*;
    auto parent(JSONElementT* _parent) -> void;

    /**
     * @brief Creates a full copy of the JSON representation pointed by this
     * *zpt::json*
     * @return a pointer to the copy of the underlying JSON representation
     */
    auto clone() -> zpt::json;

    virtual auto is_object() -> bool;
    virtual auto is_array() -> bool;
    virtual auto is_string() -> bool;
    virtual auto is_integer() -> bool;
    virtual auto is_double() -> bool;
    virtual auto is_number() -> bool;
    virtual auto is_bool() -> bool;
    virtual auto is_date() -> bool;
    virtual auto is_lambda() -> bool;
    virtual auto is_nil() -> bool;
    virtual auto is_iterable() -> bool;

    virtual auto obj() -> JSONObj&;
    virtual auto arr() -> JSONArr&;
    virtual auto str() -> std::string;
    virtual auto intr() -> long long;
    virtual auto dbl() -> double;
    virtual auto bln() -> bool;
    virtual auto date() -> zpt::timestamp_t;
    virtual auto lbd() -> zpt::lambda&;
    virtual auto number() -> double;

    auto operator<<(const char* _in) -> JSONElementT&;
    auto operator<<(std::string _in) -> JSONElementT&;
    auto operator<<(JSONElementT*) -> JSONElementT&;
    auto operator<<(std::initializer_list<JSONElementT> _list) -> JSONElementT&;
    template<typename T>
    auto operator<<(T _in) -> JSONElementT&;
    template<typename T>
    auto operator>>(T _in) -> JSONElementT&;
    template<typename T>
    auto operator[](T _idx) -> json&;
    auto operator==(JSONElementT& _in) -> bool;
    auto operator==(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator==(T _in) -> bool;
    auto operator!=(JSONElementT& _in) -> bool;
    auto operator!=(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator!=(T _in) -> bool;
    auto operator<(JSONElementT& _in) -> bool;
    auto operator<(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator<(T _in) -> bool;
    auto operator>(JSONElementT& _in) -> bool;
    auto operator>(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator>(T _in) -> bool;
    auto operator<=(JSONElementT& _in) -> bool;
    auto operator<=(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator<=(T _in) -> bool;
    auto operator>=(JSONElementT& _in) -> bool;
    auto operator>=(zpt::json _rhs) -> bool;
    template<typename T>
    auto operator>=(T _in) -> bool;

    auto operator+(zpt::json _rhs) -> zpt::json;
    auto operator+(zpt::JSONElementT& _rhs) -> zpt::json;
    auto operator-(zpt::json _rhs) -> zpt::json;
    auto operator-(zpt::JSONElementT& _rhs) -> zpt::json;
    auto operator/(zpt::json _rhs) -> zpt::json;
    auto operator/(zpt::JSONElementT& _rhs) -> zpt::json;
    auto operator|(zpt::json _rhs) -> zpt::json;
    auto operator|(zpt::JSONElementT& _rhs) -> zpt::json;

    friend auto operator<<(std::ostream& _out, JSONElementT _in) -> std::ostream& {
        _in.stringify(_out);
        return _out;
    }

    auto get_path(std::string _path, std::string _separator = ".") -> zpt::json;
    auto set_path(std::string _path, zpt::json _value, std::string _separator = ".") -> void;
    auto del_path(std::string _path, std::string _separator = ".") -> void;

    virtual auto flatten() -> zpt::json;
    virtual auto inspect(
      zpt::json _pattern,
      std::function<void(std::string, std::string, zpt::JSONElementT&)> _callback,
      zpt::JSONElementT* _parent = nullptr,
      std::string _key = "",
      std::string _parent_path = "") -> void;

    virtual auto stringify(std::string& _out) -> void;
    virtual auto stringify(std::ostream& _out) -> void;
    virtual auto stringify() -> std::string;

    virtual auto prettify(std::string& _out, uint _n_tabs = 0) -> void;
    virtual auto prettify(std::ostream& _out, uint _n_tabs = 0) -> void;

    virtual auto element(size_t _pos) -> std::tuple<size_t, std::string, zpt::json>;

  private:
    JSONUnion __target;
    JSONElementT* __parent{ nullptr };

    auto init() -> void;
};

auto
timestamp(std::string _json_date = "") -> zpt::timestamp_t;

template<typename T>
auto
mkelem(T& _e) -> zpt::JSONElementT*;

template<typename T>
auto
mkptr(T _v) -> zpt::json;

auto
get(std::string _path, zpt::json _source) -> zpt::json;

template<typename T>
auto
set(std::string _path, T _value, zpt::json _target = zpt::undefined) -> zpt::json;

auto
timestamp(zpt::json _json_date) -> zpt::timestamp_t;

auto
timestamp(zpt::timestamp_t _timestamp) -> std::string;
} // namespace zpt

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<typename T>
zpt::pretty::pretty(T _rhs) {
    _rhs->prettify(this->__underlying);
}
template<typename T>
auto
zpt::json::operator==(T _rhs) -> bool {
    return (*this) == _rhs;
}
template<typename T>
auto
zpt::json::operator!=(T _rhs) -> bool {
    return (*this) != _rhs;
}
template<typename T>
auto
zpt::json::operator<(T _rhs) -> bool {
    return (*this) < _rhs;
}
template<typename T>
auto
zpt::json::operator>(T _rhs) -> bool {
    return (*this) > _rhs;
}
template<typename T>
auto
zpt::json::operator<=(T _rhs) -> bool {
    return (*this) <= _rhs;
}
template<typename T>
auto
zpt::json::operator>=(T _rhs) -> bool {
    return (*this) >= _rhs;
}
template<typename T>
auto
zpt::json::operator<<(T _in) -> zpt::json& {
    (*this) << _in;
    return *this;
}
template<typename T>
auto
zpt::json::operator>>(T _in) -> zpt::json& {
    (*this) >> _in;
    return *this;
}
template<typename T>
auto
zpt::json::operator+(T _rhs) -> zpt::json {
    return (*this) + _rhs;
}
template<typename T>
auto
zpt::json::operator-(T _rhs) -> zpt::json {
    return (*this) - _rhs;
}
template<typename T>
auto
zpt::json::operator/(T _rhs) -> zpt::json {
    return (*this) / _rhs;
}
template<typename T>
auto
zpt::json::operator|(T _rhs) -> zpt::json {
    return (*this) | _rhs;
}
template<typename T>
auto
zpt::json::data(const T _delegate) -> zpt::json {
    return _delegate->get_json();
}
template<typename T>
auto zpt::json::operator[](T _idx) const -> zpt::json {
    return zpt::json{ ((*this))[_idx] };
}
template<typename T>
auto
zpt::json::pretty(T _e) -> std::string {
    return zpt::pretty{ _e };
}
template<typename T>
auto
zpt::json::string(T _e) -> zpt::json {
    std::string _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
auto
zpt::json::integer(T _e) -> zpt::json {
    long long int _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
auto
zpt::json::uinteger(T _e) -> zpt::json {
    unsigned int _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
auto
zpt::json::floating(T _e) -> zpt::json {
    double _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
auto
zpt::json::ulong(T _e) -> zpt::json {
    size_t _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
auto
zpt::json::boolean(T _e) -> zpt::json {
    bool _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
auto
zpt::json::date(T _e) -> zpt::json {
    zpt::timestamp_t _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
auto
zpt::json::lambda(T _e) -> zpt::json {
    zpt::lambda _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
auto
zpt::JSONObj::operator==(T _rhs) -> bool {
    return (*this) == _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator!=(T _rhs) -> bool {
    return (*this) != _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator<(T _rhs) -> bool {
    return (*this) < _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator>(T _rhs) -> bool {
    return (*this) > _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator<=(T _rhs) -> bool {
    return (*this) <= _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator>=(T _rhs) -> bool {
    return (*this) >= _rhs;
}
template<typename T>
auto
zpt::JSONObj::operator<<(T _in) -> JSONObj& {
    (*this)->push(new JSONElementT(_in));
    return *this;
}
template<typename T>
auto
zpt::JSONObj::operator>>(T _in) -> JSONObj& {
    (*this)->pop(_in);
    return *this;
}
template<typename T>
auto zpt::JSONObj::operator[](T _idx) -> json& {
    return ((*this))[_idx];
}
template<typename T>
auto
zpt::JSONArr::operator==(T _rhs) -> bool {
    return (*this) == _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator!=(T _rhs) -> bool {
    return (*this) != _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator<(T _rhs) -> bool {
    return (*this) < _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator>(T _rhs) -> bool {
    return (*this) > _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator<=(T _rhs) -> bool {
    return (*this) <= _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator>=(T _rhs) -> bool {
    return (*this) >= _rhs;
}
template<typename T>
auto
zpt::JSONArr::operator<<(T _in) -> JSONArr& {
    (*this)->push(new JSONElementT(_in));
    return *this;
}
template<typename T>
auto
zpt::JSONArr::operator>>(T _in) -> JSONArr& {
    (*this)->pop(_in);
    return *this;
}
template<typename T>
auto zpt::JSONArr::operator[](T _idx) -> json& {
    return ((*this))[_idx];
}
template<typename T>
auto
zpt::JSONElementT::operator<<(T _in) -> JSONElementT& {
    assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
    JSONElementT _e(_in);
    if (this->__target.__type == _e.type() && _e.type() != zpt::JSObject &&
        _e.type() != zpt::JSArray) {
        this->assign(_e);
        return *this;
    }
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object->push(new JSONElementT(_in));
            break;
        }
        case zpt::JSArray: {
            this->__target.__array->push(new JSONElementT(_in));
            break;
        }
        default: {
            assertz(this->__target.__type == zpt::JSObject || this->__target.__type == zpt::JSArray,
                    "the type must be a JSObject, JSArray or the same type of the "
                    "target, in order "
                    "to push "
                    "JSONElementT*",
                    0,
                    0);
            break;
        }
    }
    return *this;
}
template<typename T>
auto
zpt::JSONElementT::operator>>(T _in) -> JSONElementT& {
    switch (this->__target.__type) {
        case zpt::JSObject: {
            this->__target.__object >> _in;
            break;
        }
        case zpt::JSArray: {
            this->__target.__array >> _in;
            break;
        }
        default: {
            break;
        }
    }
    return *this;
}
template<typename T>
auto zpt::JSONElementT::operator[](T _idx) -> json& {
    if (this->__target.__type == zpt::JSObject) {
        return this->__target.__object[_idx];
    }
    else if (this->__target.__type == zpt::JSArray) {
        return this->__target.__array[_idx];
    }
    return zpt::undefined;
}
template<typename T>
auto
zpt::JSONElementT::operator==(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) == _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator!=(T _in) -> bool {
    if (_in == nullptr) {
        return this->__target.__type == zpt::JSNil;
    }
    JSONElementT _rhs{ _in };
    return (*this) == _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator<(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) < _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator>(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) > _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator<=(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) <= _rhs;
}
template<typename T>
auto
zpt::JSONElementT::operator>=(T _in) -> bool {
    JSONElementT _rhs{ _in };
    return (*this) >= _rhs;
}
template<typename T>
auto
zpt::JSONElementT::assign(T _in) -> void {
    zpt::JSONElementT _rhs{ _in };
    this->assign(_rhs);
}
template<typename T>
auto
zpt::set(std::string _path, T _value, zpt::json _target) -> zpt::json {
    zpt::json _return;
    if (_target->ok()) {
        _return = _target;
    }
    else {
        _return = zpt::json::object();
    }
    _return->set_path(_path, zpt::mkptr(_value));
    return _return;
}

template<typename T>
auto
zpt::mkelem(T& _e) -> zpt::JSONElementT* {
    return new zpt::JSONElementT(_e);
}

template<typename T>
auto
zpt::mkptr(T _v) -> zpt::json {
    T _e(_v);
    return zpt::json{ new zpt::JSONElementT(_e) };
}

#endif
