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

#pragma once

#include <cstring>
#include <ctime>
#include <memory>
#include <iostream>
#include <sstream>
#include <zapata/base/performative.h>
#include <zapata/exceptions/ExpectationException.h>

/**
 * Compact form for throwing exceptions when validating logical requirements and
 * input/output
 * validation
 * @param x a boolean expression to be validated
 * @param y the error message
 * @param z the HTTP status code to be replied to the invoking HTTP client
 */
#define expect(x, y, z, c)                                                                         \
    if (!(x)) {                                                                                    \
        std::ostringstream __OSS__;                                                                \
        __OSS__ << y << std::flush;                                                                \
        throw zpt::ExpectationException(__OSS__.str(), z, c, #x, __LINE__, __FILE__);              \
    }

#define expect_mandatory(x, y, z)                                                                  \
    if (std::string(y).length() == 0) { expect(x->ok(), std::string(x), z, 1000) }                 \
    else { expect(x[y]->ok(), std::string(y), z, 1000) }
#define expect_string(x, y, z)                                                                     \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == zpt::JSString, std::string(x), z, 1001)                    \
    }                                                                                              \
    else { expect(!x[y]->ok() || x[y]->type() == zpt::JSString, std::string(y), z, 1001) }
#define expect_integer(x, y, z)                                                                    \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == zpt::JSInteger, std::string(x), z, 1002)                   \
    }                                                                                              \
    else { expect(!x[y]->ok() || x[y]->type() == zpt::JSInteger, std::string(y), z, 1002) }
#define expect_double(x, y, z)                                                                     \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == JSDouble, std::string(x), z, 1003)                         \
    }                                                                                              \
    else { expect(!x[y]->ok() || x[y]->type() == zpt::JSDouble, std::string(y), z, 1003) }
#define expect_timestamp(x, y, z)                                                                  \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == zpt::JSDate ||                                             \
                 (x->type() == zpt::JSString && zpt::test::timestamp(x)),                          \
               std::string(x),                                                                     \
               z,                                                                                  \
               1004)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || x[y]->type() == zpt::JSDate ||                                       \
                 (x[y]->type() == zpt::JSString && zpt::test::timestamp(x[y])),                    \
               std::string(y),                                                                     \
               z,                                                                                  \
               1004)                                                                               \
    }
#define expect_boolean(x, y, z)                                                                    \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == zpt::JSBoolean, std::string(x), z, 1005)                   \
    }                                                                                              \
    else { expect(!x[y]->ok() || x[y]->type() == zpt::JSBoolean, std::string(y), z, 1005) }
#define expect_complex(x, y, z)                                                                    \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == zpt::JSObject || x->type() == zpt::JSArray,                \
               std::string(x),                                                                     \
               z,                                                                                  \
               1006)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || x[y]->type() == zpt::JSObject || x[y]->type() == zpt::JSArray,       \
               std::string(y),                                                                     \
               z,                                                                                  \
               1006)                                                                               \
    }
#define expect_object(x, y, z)                                                                     \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == zpt::JSObject, std::string(x), z, 1007)                    \
    }                                                                                              \
    else { expect(!x[y]->ok() || x[y]->type() == zpt::JSObject, std::string(y), z, 1007) }
#define expect_array(x, y, z)                                                                      \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == zpt::JSArray, std::string(x), z, 1008)                     \
    }                                                                                              \
    else { expect(!x[y]->ok() || x[y]->type() == zpt::JSArray, std::string(y), z, 1008) }
#define expect_int(x, y, z)                                                                        \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || x->type() == zpt::JSInteger, std::string(x), z, 1009)                   \
    }                                                                                              \
    else { expect(!x[y]->ok() || x[y]->type() == zpt::JSInteger, std::string(y), z, 1009) }

#define expect_uuid(x, y, z)                                                                       \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || (x->type() == zpt::JSString && zpt::test::uuid(x->string())),           \
               std::string(x),                                                                     \
               z,                                                                                  \
               1010)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || (x[y]->type() == zpt::JSString && zpt::test::uuid(x[y]->string())),  \
               std::string(y),                                                                     \
               z,                                                                                  \
               1010)                                                                               \
    }
#define expect_utf8(x, y, z)                                                                       \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || (x->type() == zpt::JSString && zpt::test::utf8(x->string())),           \
               std::string(x),                                                                     \
               z,                                                                                  \
               1011)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || (x[y]->type() == zpt::JSString && zpt::test::utf8(x[y]->string())),  \
               std::string(y),                                                                     \
               z,                                                                                  \
               1011)                                                                               \
    }
#define expect_ascii(x, y, z)                                                                      \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || (x->type() == zpt::JSString && zpt::test::ascii(x->string())),          \
               std::string(x),                                                                     \
               z,                                                                                  \
               1012)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || (x[y]->type() == zpt::JSString && zpt::test::ascii(x[y]->string())), \
               std::string(y),                                                                     \
               z,                                                                                  \
               1012)                                                                               \
    }
#define expect_hash(x, y, z)                                                                       \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || (x->type() == zpt::JSString && zpt::test::token(x->string())),          \
               std::string(x),                                                                     \
               z,                                                                                  \
               1013)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || (x[y]->type() == zpt::JSString && zpt::test::token(x[y]->string())), \
               std::string(y),                                                                     \
               z,                                                                                  \
               1013)                                                                               \
    }
#define expect_token(x, y, z)                                                                      \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || (x->type() == zpt::JSString && zpt::test::token(x->string())),          \
               std::string(x),                                                                     \
               z,                                                                                  \
               1014)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || (x[y]->type() == zpt::JSString && zpt::test::token(x[y]->string())), \
               std::string(y),                                                                     \
               z,                                                                                  \
               1014)                                                                               \
    }
#define expect_uri(x, y, z)                                                                        \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || (x->type() == zpt::JSString && zpt::test::uri(x->string())),            \
               std::string(x),                                                                     \
               z,                                                                                  \
               1015)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || (x[y]->type() == zpt::JSString && zpt::test::uri(x[y]->string())),   \
               std::string(y),                                                                     \
               z,                                                                                  \
               1015)                                                                               \
    }
#define expect_email(x, y, z)                                                                      \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || (x->type() == zpt::JSString && zpt::test::email(x->string())),          \
               std::string(x),                                                                     \
               z,                                                                                  \
               1016)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || (x[y]->type() == zpt::JSString && zpt::test::email(x[y]->string())), \
               std::string(y),                                                                     \
               z,                                                                                  \
               1016);                                                                              \
        if (x[y]->is_string()) {                                                                   \
            std::string _email = std::string(x[y]);                                                \
            std::transform(_email.begin(), _email.end(), _email.begin(), ::tolower);               \
            x << y << _email;                                                                      \
        }                                                                                          \
    }
#define expect_location(x, y, z)                                                                   \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || ((x->type() == zpt::JSObject || x->type() == zpt::JSArray) &&           \
                            zpt::test::location(x)),                                               \
               std::string(x),                                                                     \
               z,                                                                                  \
               1017)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || ((x[y]->type() == zpt::JSObject || x[y]->type() == zpt::JSArray) &&  \
                               zpt::test::location(x[y])),                                         \
               std::string(y),                                                                     \
               z,                                                                                  \
               1017)                                                                               \
    }
#define expect_phone(x, y, z)                                                                      \
    if (std::string(y).length() == 0) {                                                            \
        expect(!x->ok() || (x->type() == zpt::JSString && zpt::test::phone(x->string())),          \
               std::string(x),                                                                     \
               z,                                                                                  \
               1018)                                                                               \
    }                                                                                              \
    else {                                                                                         \
        expect(!x[y]->ok() || (x[y]->type() == zpt::JSString && zpt::test::phone(x[y]->string())), \
               std::string(y),                                                                     \
               z,                                                                                  \
               1018)                                                                               \
    }

#define expect_intersects(x, y, z)                                                                 \
    {                                                                                              \
        std::vector<zpt::json> __result__;                                                         \
        std::set_intersection(                                                                     \
          std::begin(x->array()),                                                                  \
          std::end(x->array()),                                                                    \
          std::begin(y->array()),                                                                  \
          std::end(y->array()),                                                                    \
          std::back_inserter(__result__),                                                          \
          [](zpt::json _lhs, zpt::json _rhs) -> bool { return _lhs == _rhs; });                    \
        expect(__result__.size() != 0, std::string(y), z, 1018);                                   \
    }
#define expect_unauthorized(x) expect(x, std::string(#x), 401, 1019)
#define expect_valid_values(x, y, z) expect(x, std::string(y), z, 1020)

#define expect_reply(x, y, z, c, e, r)                                                             \
    if (!(x)) {                                                                                    \
        e->reply(                                                                                  \
          r,                                                                                       \
          { "status",                                                                              \
            z,                                                                                     \
            "payload",                                                                             \
            { "text",                                                                              \
              y,                                                                                   \
              "code",                                                                              \
              c,                                                                                   \
              "assertion_failed",                                                                  \
              (std::string("'") + std::string(#x) + std::string("' failed on file ") +             \
               std::string(__FILE__) + std::string(", line ") + std::to_string(__LINE__)) } });    \
    }

#define expect_mandatory_reply(x, y, z, e, r)                                                      \
    if (std::string(y).length() == 0) { expect_reply(x->ok(), std::string(x), z, 1000, e, r) }     \
    else { expect_reply(x[y]->ok(), std::string(y), z, 1000, e, r) }
#define expect_string_reply(x, y, z, e, r)                                                         \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == zpt::JSString, std::string(x), z, 1001, e, r)        \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSString, std::string(y), z, 1001, e, r)  \
    }
#define expect_integer_reply(x, y, z, e, r)                                                        \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == zpt::JSInteger, std::string(x), z, 1002, e, r)       \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSInteger, std::string(y), z, 1002, e, r) \
    }
#define expect_double_reply(x, y, z, e, r)                                                         \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == JSDouble, std::string(x), z, 1003, e, r)             \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSDouble, std::string(y), z, 1003, e, r)  \
    }
#define expect_timestamp_reply(x, y, z, e, r)                                                      \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == zpt::JSDate ||                                       \
                       (x->type() == zpt::JSString && zpt::test::timestamp(x)),                    \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1004,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSDate ||                                 \
                       (x[y]->type() == zpt::JSString && zpt::test::timestamp(x[y])),              \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1004,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_boolean_reply(x, y, z, e, r)                                                        \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == zpt::JSBoolean, std::string(x), z, 1005, e, r)       \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSBoolean, std::string(y), z, 1005, e, r) \
    }
#define expect_complex_reply(x, y, z, e, r)                                                        \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == zpt::JSObject || x->type() == zpt::JSArray,          \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1006,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSObject || x[y]->type() == zpt::JSArray, \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1006,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_object_reply(x, y, z, e, r)                                                         \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == zpt::JSObject, std::string(x), z, 1007, e, r)        \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSObject, std::string(y), z, 1007, e, r)  \
    }
#define expect_array_reply(x, y, z, e, r)                                                          \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == zpt::JSArray, std::string(x), z, 1008, e, r)         \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSArray, std::string(y), z, 1008, e, r)   \
    }
#define expect_int_reply(x, y, z, e, r)                                                            \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || x->type() == zpt::JSInteger, std::string(x), z, 1009, e, r)       \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() || x[y]->type() == zpt::JSInteger, std::string(y), z, 1009, e, r) \
    }

#define expect_uuid_reply(x, y, z, e, r)                                                           \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || (x->type() == zpt::JSString && zpt::test::uuid(x->string())),     \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1010,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       (x[y]->type() == zpt::JSString && zpt::test::uuid(x[y]->string())),         \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1010,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_utf8_reply(x, y, z, e, r)                                                           \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || (x->type() == zpt::JSString && zpt::test::utf8(x->string())),     \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1011,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       (x[y]->type() == zpt::JSString && zpt::test::utf8(x[y]->string())),         \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1011,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_ascii_reply(x, y, z, e, r)                                                          \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || (x->type() == zpt::JSString && zpt::test::ascii(x->string())),    \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1012,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       (x[y]->type() == zpt::JSString && zpt::test::ascii(x[y]->string())),        \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1012,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_hash_reply(x, y, z, e, r)                                                           \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || (x->type() == zpt::JSString && zpt::test::token(x->string())),    \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1013,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       (x[y]->type() == zpt::JSString && zpt::test::token(x[y]->string())),        \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1013,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_token_reply(x, y, z, e, r)                                                          \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || (x->type() == zpt::JSString && zpt::test::token(x->string())),    \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1014,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       (x[y]->type() == zpt::JSString && zpt::test::token(x[y]->string())),        \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1014,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_uri_reply(x, y, z, e, r)                                                            \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || (x->type() == zpt::JSString && zpt::test::uri(x->string())),      \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1015,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       (x[y]->type() == zpt::JSString && zpt::test::uri(x[y]->string())),          \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1015,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_email_reply(x, y, z, e, r)                                                          \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || (x->type() == zpt::JSString && zpt::test::email(x->string())),    \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1016,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       (x[y]->type() == zpt::JSString && zpt::test::email(x[y]->string())),        \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1016,                                                                         \
                     e,                                                                            \
                     r);                                                                           \
        if (x[y]->is_string()) {                                                                   \
            std::string _email = std::string(x[y]);                                                \
            std::transform(_email.begin(), _email.end(), _email.begin(), ::tolower);               \
            x << y << _email;                                                                      \
        }                                                                                          \
    }
#define expect_location_reply(x, y, z, e, r)                                                       \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || ((x->type() == zpt::JSObject || x->type() == zpt::JSArray) &&     \
                                  zpt::test::location(x)),                                         \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1017,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       ((x[y]->type() == zpt::JSObject || x[y]->type() == zpt::JSArray) &&         \
                        zpt::test::location(x[y])),                                                \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1017,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }
#define expect_phone_reply(x, y, z, e, r)                                                          \
    if (std::string(y).length() == 0) {                                                            \
        expect_reply(!x->ok() || (x->type() == zpt::JSString && zpt::test::phone(x->string())),    \
                     std::string(x),                                                               \
                     z,                                                                            \
                     1018,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }                                                                                              \
    else {                                                                                         \
        expect_reply(!x[y]->ok() ||                                                                \
                       (x[y]->type() == zpt::JSString && zpt::test::phone(x[y]->string())),        \
                     std::string(y),                                                               \
                     z,                                                                            \
                     1018,                                                                         \
                     e,                                                                            \
                     r)                                                                            \
    }

#define expect_intersects_reply(x, y, z, e, r)                                                     \
    {                                                                                              \
        std::vector<zpt::json> __result__;                                                         \
        std::set_intersection(                                                                     \
          std::begin(x->array()),                                                                  \
          std::end(x->array()),                                                                    \
          std::begin(y->array()),                                                                  \
          std::end(y->array()),                                                                    \
          std::back_inserter(__result__),                                                          \
          [](zpt::json _lhs, zpt::json _rhs) -> bool { return _lhs == _rhs; });                    \
        expect_reply(__result__.size() != 0, std::string(y), z, 1018, e, r);                       \
    }
#define expect_unauthorized_reply(x, e, r) expect_reply(x, std::string(#x), 401, 1019, e, r)
#define expect_valid_values_reply(x, y, z, e, r) expect_reply(x, std::string(y), z, 1020, e, r)

namespace zpt {
enum JSONType {
    JSObject,
    JSArray,
    JSString,
    JSInteger,
    JSDouble,
    JSBoolean,
    JSNil,
    JSDate,
    JSLambda,
    JSRegex
};
auto
to_string(zpt::JSONType _type) -> std::string;

auto
get_tz() -> std::string const&;

using tm_ptr = std::shared_ptr<std::tm>;

auto
get_time(time_t _t) -> zpt::tm_ptr;

} // namespace zpt
