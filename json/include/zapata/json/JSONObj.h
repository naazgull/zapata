/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
typedef unsigned long long timestamp_t;

class JSONElementT;
class JSONObj;
class JSONArr;
class JSONLambda;
class lambda;
class json;

class pretty : public std::string {
  public:
    inline pretty(std::string _rhs)
      : std::string(_rhs){};
    inline pretty(const char* _rhs)
      : std::string(_rhs){};
    template<typename T>
    inline pretty(T _rhs)
      : std::string() {
        _rhs->prettify(*this);
    };
    friend auto operator<<(std::ostream& _out, zpt::pretty& _in) -> std::ostream& {
        _out << std::string(_in.data());
        return _out;
    };
};

/**
 * \brief Smart shared pointer to a zpt::JSONElementT object.
 */
class JSONPtr : public std::shared_ptr<JSONElementT> {
  public:
    /**
     * \brief Creates a new JSONPtr instance, pointing to a *null* object.
     */
    JSONPtr();
    /**
     * \brief Creates a new JSONPtr instance, pointing to the *target* object.
     */
    JSONPtr(JSONElementT* _target);
    /**
     * \brief Creates a new JSONPtr instance, pointing to the *target* object.
     */
    JSONPtr(std::initializer_list<JSONElementT> _list);
    /**
     * \brief Destroys the current JSONPtr instance. It will only free the pointed
     * object if there
     * are no more
     * *shared_ptr* objects pointing to it.
     */
    virtual ~JSONPtr();

    /**
     * \brief Read-access method for retrieving the value pointed by *this*
     * instance.
     *
     * @return the value pointed by *this* instance
     */
    auto value() -> JSONElementT&;
    auto parse(std::istream& _in) -> void;

    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     */
    template<typename T>
    auto operator<<(T _in) -> JSONPtr&;
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
    auto operator>>(T _in) -> JSONPtr&;
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
    auto operator[](T _idx) -> JSONPtr&;

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
    auto operator+(T _in) -> JSONPtr&;
    template<typename T>
    auto operator-(T _in) -> JSONPtr&;
    template<typename T>
    auto operator/(T _in) -> JSONPtr&;
    template<typename T>
    auto operator|(T _in) -> JSONPtr&;

    /**
     * \brief Friendly '>>' std::istream operator override that parses the textual
     * representation
     * available on an
     * std::istream object into a of a zpt::JSONPtr object.
     */
    friend auto operator>>(std::istream& _in, zpt::JSONPtr& _out) -> std::istream& {
        _out.parse(_in);
        return _in;
    };

    template<typename T>
    static inline auto data(const T _delegate) -> zpt::JSONPtr {
        return _delegate->get_json();
    };
};

typedef JSONPtr JSONElement;

/**
 * \brief Class that represents the *object* JSON type. It inherits from the
 * std::map class and is
 * composed of
 * std::string and zpt::JSONPtr key-value pairs.
 */
class JSONObjT : public std::map<std::string, zpt::JSONPtr> {
  public:
    /**
     * \brief Creates a new JSONObjT instance.
     */
    JSONObjT();
    /**
     * \brief Destroys the current JSONObjT instance, freeing all allocated
     * memory. It will free the
     * objects pointed
     * by each zpt::JSONPtr smart pointer only if there aren't any more
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
     *     zpt::JSONPtr child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param  _path      the object path to search for
     * @param  _separator the object path separator
     *
     * @return            the pointer to the child object if it exists or
     * zpt::undefined otherwise
     */
    auto get_path(std::string _path, std::string _separator = ".") -> zpt::JSONPtr;

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
     *     zpt::JSONPtr child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param _path      the object path to search for
     * @param _value     the value to be assigned to the child element
     * @param _separator the object path separator
     */
    auto set_path(std::string _path, zpt::JSONPtr _value, std::string _separator = ".")
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
     *     zpt::JSONPtr child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param  _path      the object path to search for
     * @param  _separator the object path separator
     */
    auto del_path(std::string _path, std::string _separator = ".") -> void;

    /**
     * @brief Creates a full copy of the JSON representation stored in *this*
     * *zpt::JSONObjT*
     * @return a pointer to the copy of the underlying JSON representation
     */
    auto clone() -> zpt::JSONPtr;

    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator==(T _in) -> bool {
        return false;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    template<typename T>
    auto operator!=(T _in) -> bool {
        return true;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<(T _in) -> bool {
        return false;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>(T _in) -> bool {
        return false;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>=(T _in) -> bool {
        return false;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<=(T _in) -> bool {
        return false;
    };

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
    auto operator[](int _idx) -> zpt::JSONPtr&;
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
    auto operator[](size_t _idx) -> zpt::JSONPtr&;
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
    auto operator[](const char* _idx) -> zpt::JSONPtr&;
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
    auto operator[](std::string _idx) -> zpt::JSONPtr&;

    /**
     * \brief Friendly '<<' std::ostream operator override that outputs the
     * textual representation
     * of *this* object
     * instance into the *out* stream.
     */
    friend std::ostream& operator<<(std::ostream& _out, zpt::JSONObjT& _in) {
        _in.stringify(_out);
        return _out;
    };

  private:
    std::string __name;
};

/**
 * \brief Class that represents the *array* JSON type. It inherits from the
 * std::vector class and is
 * composed of
 * zpt::JSONPtr elements.
 */
class JSONArrT : public std::vector<zpt::JSONPtr> {
  public:
    /**
     * \brief Creates a new JSONArrT instance.
     */
    JSONArrT();
    /**
     * \brief Destroys the current JSONArrT instance, freeing all allocated
     * memory. It will free the
     * objects pointed
     * by each zpt::JSONPtr smart pointer only if there aren't any more
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
    virtual auto sort(std::function<bool(zpt::JSONPtr, zpt::JSONPtr)> _comparator)
      -> zpt::JSONArrT&;

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
     *     zpt::JSONPtr child =
     * my_json_array[1]["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::JSONPtr child =
     * my_json_array->get_path("1.some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::JSONPtr child =
     * my_json_array->get_path("1/some_array/0/first_field/name", "/");
     *
     * @param  _path      the object path to search for
     * @param  _separator the object path separator
     *
     * @return            the pointer to the child object if it exists or
     * zpt::undefined otherwise
     */
    auto get_path(std::string _path, std::string _separator = ".") -> zpt::JSONPtr;

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
     *     zpt::JSONPtr child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array/0/first_field/name", "/");
     *
     * @param _path      the object path to search for
     * @param _value     the value to be assigned to the child element
     * @param _separator the object path separator
     */
    auto set_path(std::string _path, zpt::JSONPtr _value, std::string _separator = ".")
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
     *     zpt::JSONPtr child =
     * my_json_object["some_array"][0]["first_field"]["name"];
     *
     * is analogue to
     *
     *     zpt::JSONPtr child =
     * my_json_object->get_path("some_array.0.first_field.name");
     *
     * or
     *
     *     zpt::JSONPtr child =
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
    auto clone() -> zpt::JSONPtr;

    /**
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator==(T _in) -> bool {
        return false;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    template<typename T>
    auto operator!=(T _in) -> bool {
        return true;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<(T _in) -> bool {
        return false;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>(T _in) -> bool {
        return false;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator<=(T _in) -> bool {
        return false;
    };
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
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    auto operator>=(T _in) -> bool {
        return false;
    };

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
    auto operator[](int _idx) -> zpt::JSONPtr&;
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
    auto operator[](size_t _idx) -> zpt::JSONPtr&;
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
    auto operator[](const char* _idx) -> zpt::JSONPtr&;
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
    auto operator[](std::string _idx) -> zpt::JSONPtr&;

    /**
     * \brief Friendly '<<' std::ostream operator override that outputs the
     * textual representation
     * of *this* array
     * instance into the *out* stream.
     */
    friend std::ostream& operator<<(std::ostream& _out, zpt::JSONArrT& _in) {
        _in.stringify(_out);
        return _out;
    };
};

/**
 * \brief Smart shared pointer to a zpt::JSONObjT object.
 */
class JSONObj : public std::shared_ptr<zpt::JSONObjT> {
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
    JSONObj(zpt::JSONObj& _rhs);
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
     * \brief Retrieves an iterator pointing to the beginning of *this* object
     * attribute list.
     *
     * @return the iterator pointing to the beginning of *this* object attribute
     * list
     */
    JSONObjT::iterator begin();
    /**
     * \brief Retrieves an iterator pointing to the end of *this* object attribute
     * list.
     *
     * @return the iterator pointing to the end of *this* object attribute list
     */
    JSONObjT::iterator end();

    /**
     * \brief Cast operation for the std::string class. This is a convenience
     * wrapper operator for
     * zpt::JSONObjT::stringify method.
     *
     * @return the textual representation of *this* object instance
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
     * \brief Operator '==' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    bool operator==(T _rhs) {
        return *(this->get()) == _rhs;
    };
    /**
     * \brief Operator '!=' override for comparing *this* instance with other JSON
     * typed argument.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are different, **false** otherwise
     */
    template<typename T>
    bool operator!=(T _rhs) {
        return *(this->get()) != _rhs;
    };
    /**
     * \brief Operator '<' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    bool operator<(T _rhs) {
        return *(this->get()) < _rhs;
    };
    /**
     * \brief Operator '>' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    bool operator>(T _rhs) {
        return *(this->get()) > _rhs;
    };
    /**
     * \brief Operator '<=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    bool operator<=(T _rhs) {
        return *(this->get()) <= _rhs;
    };
    /**
     * \brief Operator '>=' override for comparing *this* instance with other JSON
     * typed argument.
     * Type conversion
     * between JSON type is attempted in order to determine the objects equality.
     *
     * Allowed types for *T* are: zpt::JSONElementT, zpt::JSONPtr, zpt::JSONObj,
     * zpt::JSONArr,
     * std::string, const
     * char*, long long, double, bool, zpt::timestamp_t, int, size_t.
     *
     * @return **true** if the objects are equal, **false** otherwise
     */
    template<typename T>
    bool operator>=(T _rhs) {
        return *(this->get()) >= _rhs;
    };
    JSONObj& operator<<(string _in);
    JSONObj& operator<<(const char* _in);
    JSONObj& operator<<(JSONElementT& _in);
    JSONObj& operator<<(std::initializer_list<JSONElementT> _list);
    template<typename T>
    JSONObj& operator<<(T _in) {
        (*this)->push(new JSONElementT(_in));
        return *this;
    };
    template<typename T>
    JSONObj& operator>>(T _in) {
        (*this)->pop(_in);
        return *this;
    };
    template<typename T>
    JSONPtr& operator[](T _idx) {
        return (*(this->get()))[_idx];
    };

    friend ostream& operator<<(ostream& _out, JSONObj& _in) {
        _in->stringify(_out);
        return _out;
    };
};

class JSONArr : public std::shared_ptr<JSONArrT> {
  public:
    JSONArr();
    JSONArr(JSONArr& _rhs);
    JSONArr(JSONArrT* _target);
    virtual ~JSONArr();

    JSONArrT::iterator begin();
    JSONArrT::iterator end();

    operator string();

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

    template<typename T>
    bool operator==(T _rhs) {
        return *(this->get()) == _rhs;
    };
    template<typename T>
    bool operator!=(T _rhs) {
        return *(this->get()) != _rhs;
    };
    template<typename T>
    bool operator<(T _rhs) {
        return *(this->get()) < _rhs;
    };
    template<typename T>
    bool operator>(T _rhs) {
        return *(this->get()) > _rhs;
    };
    template<typename T>
    bool operator<=(T _rhs) {
        return *(this->get()) <= _rhs;
    };
    template<typename T>
    bool operator>=(T _rhs) {
        return *(this->get()) >= _rhs;
    };
    JSONArr& operator<<(JSONElementT& _in);
    JSONArr& operator<<(std::initializer_list<JSONElementT> _list);
    template<typename T>
    JSONArr& operator<<(T _in) {
        (*this)->push(new JSONElementT(_in));
        return *this;
    };
    template<typename T>
    JSONArr& operator>>(T _in) {
        (*this)->pop(_in);
        return *this;
    };
    template<typename T>
    JSONPtr& operator[](T _idx) {
        return (*(this->get()))[_idx];
    };

    friend ostream& operator<<(ostream& _out, JSONArr& _in) {
        _in->stringify(_out);
        return _out;
    };
};
typedef std::shared_ptr<std::string> JSONStr;

class JSONContext {
  public:
    JSONContext(void* _target);
    virtual ~JSONContext();

    virtual void* unpack();

  private:
    void* __target;
};

class context : public std::shared_ptr<zpt::JSONContext> {
  public:
    context(void* _target);
    virtual ~context();
};

typedef std::function<zpt::json(zpt::json, unsigned short, zpt::context)> symbol;
typedef std::shared_ptr<std::map<std::string, std::tuple<std::string, unsigned short, zpt::symbol>>>
  symbol_table;
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

    virtual zpt::json operator()(zpt::json _args, zpt::context _ctx);

    static void add(std::string _signature, zpt::symbol _lambda);
    static void add(std::string _name, unsigned short _n_args, zpt::symbol _lambda);

    static zpt::json call(std::string _name, zpt::json _args, zpt::context _ctx);

    static std::string stringify(std::string _name, unsigned short _n_args);
    static std::tuple<std::string, unsigned short> parse(std::string _signature);

  private:
    static zpt::symbol find(std::string _signature);
    static zpt::symbol find(std::string _name, unsigned short _1n_args);
};

class JSONLambda {
  public:
    JSONLambda();
    JSONLambda(std::string _signature);
    JSONLambda(std::string _name, unsigned short _n_args);
    virtual ~JSONLambda();

    virtual zpt::json call(zpt::json _args, zpt::context _ctx);

    virtual std::string name();
    virtual unsigned short n_args();
    virtual std::string signature();

  private:
    std::string __name;
    unsigned short __n_args;
};

typedef struct JSONStruct {
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
    };

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
} JSONUnion;

class JSONElementT {
  public:
    JSONElementT();
    JSONElementT(JSONElementT& _element);
    JSONElementT(std::initializer_list<JSONElementT> _init);
    JSONElementT(JSONPtr _value);
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

    virtual JSONType type();
    virtual string demangle();
    virtual void type(JSONType _in);
    virtual JSONUnion& value();
    virtual bool ok();
    virtual bool empty();
    virtual bool nil();

    virtual void assign(JSONElementT& _rhs);
    template<typename T>
    inline void assign(T _in) {
        zpt::JSONElementT _rhs(_in);
        this->assign(_rhs);
    }

    JSONElementT* parent();
    void parent(JSONElementT* _parent);

    /**
     * @brief Creates a full copy of the JSON representation pointed by this
     * *zpt::JSONPtr*
     * @return a pointer to the copy of the underlying JSON representation
     */
    JSONPtr clone();

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

    virtual JSONObj& obj();
    virtual JSONArr& arr();
    virtual std::string str();
    virtual long long intr();
    virtual double dbl();
    virtual bool bln();
    virtual zpt::timestamp_t date();
    virtual zpt::lambda& lbd();
    virtual double number();

    JSONElementT& operator<<(const char* _in);
    JSONElementT& operator<<(string _in);
    JSONElementT& operator<<(JSONElementT*);
    JSONElementT& operator<<(std::initializer_list<JSONElementT> _list);
    template<typename T>
    JSONElementT& operator<<(T _in) {
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
                assertz(this->__target.__type == zpt::JSObject ||
                          this->__target.__type == zpt::JSArray,
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
    };
    template<typename T>
    JSONElementT& operator>>(T _in) {
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
    };
    template<typename T>
    JSONPtr& operator[](T _idx) {
        if (this->__target.__type == zpt::JSObject) {
            return this->__target.__object[_idx];
        }
        else if (this->__target.__type == zpt::JSArray) {
            return this->__target.__array[_idx];
        }
        return zpt::undefined;
    };
    bool operator==(JSONElementT& _in);
    bool operator==(zpt::json _rhs);
    bool operator==(zpt::JSONPtr _rhs);
    template<typename T>
    bool operator==(T _in) {
        JSONElementT _rhs(_in);
        return (*this) == _rhs;
    };
    bool operator!=(JSONElementT& _in);
    bool operator!=(zpt::json _rhs);
    bool operator!=(zpt::JSONPtr _rhs);
    template<typename T>
    bool operator!=(T _in) {
        if (_in == nullptr) {
            return this->__target.__type == zpt::JSNil;
        }
        JSONElementT _rhs(_in);
        return (*this) == _rhs;
    };
    bool operator<(JSONElementT& _in);
    bool operator<(zpt::json _rhs);
    bool operator<(zpt::JSONPtr _rhs);
    template<typename T>
    bool operator<(T _in) {
        JSONElementT _rhs(_in);
        return (*this) < _rhs;
    };
    bool operator>(JSONElementT& _in);
    bool operator>(zpt::json _rhs);
    bool operator>(zpt::JSONPtr _rhs);
    template<typename T>
    bool operator>(T _in) {
        JSONElementT _rhs(_in);
        return (*this) > _rhs;
    };
    bool operator<=(JSONElementT& _in);
    bool operator<=(zpt::json _rhs);
    bool operator<=(zpt::JSONPtr _rhs);
    template<typename T>
    bool operator<=(T _in) {
        JSONElementT _rhs(_in);
        return (*this) <= _rhs;
    };
    bool operator>=(JSONElementT& _in);
    bool operator>=(zpt::json _rhs);
    bool operator>=(zpt::JSONPtr _rhs);
    template<typename T>
    bool operator>=(T _in) {
        JSONElementT _rhs(_in);
        return (*this) >= _rhs;
    };

    JSONPtr operator+(zpt::json _rhs);
    JSONPtr operator+(zpt::JSONPtr _rhs);
    JSONPtr operator+(zpt::JSONElementT& _rhs);
    JSONPtr operator-(zpt::json _rhs);
    JSONPtr operator-(zpt::JSONPtr _rhs);
    JSONPtr operator-(zpt::JSONElementT& _rhs);
    JSONPtr operator/(zpt::json _rhs);
    JSONPtr operator/(zpt::JSONPtr _rhs);
    JSONPtr operator/(zpt::JSONElementT& _rhs);
    JSONPtr operator|(zpt::json _rhs);
    JSONPtr operator|(zpt::JSONPtr _rhs);
    JSONPtr operator|(zpt::JSONElementT& _rhs);

    friend ostream& operator<<(ostream& _out, JSONElementT _in) {
        _in.stringify(_out);
        return _out;
    };

    zpt::JSONPtr get_path(std::string _path, std::string _separator = ".");
    void set_path(std::string _path, zpt::JSONPtr _value, std::string _separator = ".");
    void del_path(std::string _path, std::string _separator = ".");

    virtual zpt::JSONPtr flatten();
    virtual void inspect(
      zpt::JSONPtr _pattern,
      std::function<void(std::string, std::string, zpt::JSONElementT&)> _callback,
      zpt::JSONElementT* _parent = nullptr,
      std::string _key = "",
      std::string _parent_path = "");

    virtual void stringify(string& _out);
    virtual void stringify(ostream& _out);
    virtual string stringify();

    virtual void prettify(string& _out, uint _n_tabs = 0);
    virtual void prettify(ostream& _out, uint _n_tabs = 0);

  private:
    JSONUnion __target;
    JSONElementT* __parent;

    void init();
};

zpt::timestamp_t
timestamp(std::string _json_date = "");

class json : public zpt::JSONPtr {
  public:
    inline json()
      : zpt::JSONPtr(){};
    inline json(zpt::JSONPtr _target)
      : zpt::JSONPtr(_target){};
    inline json(JSONElementT* _target)
      : zpt::JSONPtr(_target){};
    inline json(std::initializer_list<JSONElementT> _init)
      : zpt::JSONPtr(_init){};
    inline json(std::string _rhs) {
        std::istringstream _iss;
        _iss.str(_rhs);
        _iss >> (*this);
    };
    inline json(const char* _rhs) {
        std::istringstream _iss;
        _iss.str(std::string(_rhs));
        _iss >> (*this);
    };
    inline json(zpt::pretty _rhs) {
        std::istringstream _iss;
        _iss.str(_rhs);
        _iss >> (*this);
    };
    template<typename T>
    zpt::json operator[](T _idx);

    static void stringify(std::string& _str);

    inline static zpt::json object() {
        zpt::JSONObj _empty;
        return zpt::json(new zpt::JSONElementT(_empty));
    };
    inline static zpt::json array() {
        zpt::JSONArr _empty;
        return zpt::json(new zpt::JSONElementT(_empty));
    };
    template<typename T>
    static std::string pretty(T _e);
    template<typename T>
    static zpt::json string(T _e);
    template<typename T>
    static zpt::json uinteger(T _e);
    template<typename T>
    static zpt::json integer(T _e);
    template<typename T>
    static zpt::json floating(T _e);
    template<typename T>
    static zpt::json ulong(T _e);
    template<typename T>
    static zpt::json boolean(T _e);
    inline static zpt::json date(std::string _e) {
        zpt::timestamp_t _v(zpt::timestamp(_e));
        return zpt::json(new zpt::JSONElementT(_v));
    };
    inline static zpt::json date() {
        zpt::timestamp_t _v((zpt::timestamp_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch())
                              .count());
        return zpt::json(new zpt::JSONElementT(_v));
    };
    template<typename T>
    inline static zpt::json date(T _e);
    template<typename T>
    static zpt::json lambda(T _e);
    inline static zpt::json lambda(std::string _name, unsigned short _n_args) {
        zpt::lambda _v(_name, _n_args);
        return zpt::json(new zpt::JSONElementT(_v));
    };
};

template<typename T>
zpt::JSONElementT*
mkelem(T& _e) {
    return new zpt::JSONElementT(_e);
}

template<typename T>
zpt::json
mkptr(T _v) {
    T _e(_v);
    return zpt::json{ new zpt::JSONElementT(_e) };
}

zpt::json
get(std::string _path, zpt::json _source);
template<typename T>
zpt::json
set(std::string _path, T _value, zpt::json _target = zpt::undefined);

zpt::timestamp_t
timestamp(zpt::json _json_date);
std::string
timestamp(zpt::timestamp_t _timestamp);

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
static inline JSONPtr undefined;
static inline JSONPtr nilptr = undefined;
static inline JSONPtr array{ zpt::mkptr("1b394520-2fed-4118-b622-940f25b8b35e" };

} // namespace zpt

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<typename T>
bool
zpt::JSONPtr::operator==(T _rhs) {
    return *(this->get()) == _rhs;
};
template<typename T>
bool
zpt::JSONPtr::operator!=(T _rhs) {
    return *(this->get()) != _rhs;
};
template<typename T>
bool
zpt::JSONPtr::operator<(T _rhs) {
    return *(this->get()) < _rhs;
};
template<typename T>
bool
zpt::JSONPtr::operator>(T _rhs) {
    return *(this->get()) > _rhs;
};
template<typename T>
bool
zpt::JSONPtr::operator<=(T _rhs) {
    return *(this->get()) <= _rhs;
};
template<typename T>
bool
zpt::JSONPtr::operator>=(T _rhs) {
    return *(this->get()) >= _rhs;
};
template<typename T>
zpt::JSONPtr&
zpt::JSONPtr::operator<<(T _in) {
    *(this->get()) << _in;
    return *this;
};
template<typename T>
zpt::JSONPtr&
zpt::JSONPtr::operator>>(T _in) {
    *(this->get()) >> _in;
    return *this;
};
template<typename T>
zpt::JSONPtr& zpt::JSONPtr::operator[](T _idx) {
    return (*(this->get()))[_idx];
};
template<typename T>
zpt::JSONPtr
zpt::JSONPtr::operator+(T _rhs) {
    return *(this->get()) + _rhs;
};
template<typename T>
zpt::JSONPtr
zpt::JSONPtr::operator-(T _rhs) {
    return *(this->get()) - _rhs;
};
template<typename T>
zpt::JSONPtr
zpt::JSONPtr::operator/(T _rhs) {
    return *(this->get()) / _rhs;
};
template<typename T>
zpt::JSONPtr
zpt::JSONPtr::operator|(T _rhs) {
    return *(this->get()) | _rhs;
};
template<typename T>
zpt::json zpt::json::operator[](T _idx) {
    return zpt::json((*(this->get()))[_idx]);
};
template<typename T>
std::string
zpt::json::pretty(T _e) {
    return zpt::pretty(_e);
}
template<typename T>
zpt::json
zpt::json::string(T _e) {
    std::string _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
zpt::json
zpt::json::integer(T _e) {
    long long int _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
zpt::json
zpt::json::uinteger(T _e) {
    unsigned int _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
zpt::json
zpt::json::floating(T _e) {
    double _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
zpt::json
zpt::json::ulong(T _e) {
    size_t _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
zpt::json
zpt::json::boolean(T _e) {
    bool _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
zpt::json
zpt::json::date(T _e) {
    zpt::timestamp_t _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
zpt::json
zpt::json::lambda(T _e) {
    zpt::lambda _v(_e);
    return zpt::json(new zpt::JSONElementT(_v));
}
template<typename T>
zpt::json
zpt::set(std::string _path, T _value, zpt::json _target) {
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

#endif
