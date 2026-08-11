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
 * @file convert.h
 * @brief Type conversion, encoding, and text utilities.
 *
 * Provides comprehensive utilities for:
 * - Type-to-string conversions (tostr, fromstr)
 * - Base64 encoding/decoding
 * - UTF-8 encoding/decoding
 * - URL encoding/decoding
 * - Unicode escaping
 * - Quoted-printable encoding
 * - UUID and key generation
 * - Input validation (email, phone, URI, timestamp)
 * - Timestamp conversion
 * - Backtrace and symbol demangling
 */

#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <errno.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory.h>
#include <mutex>
#include <regex>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <wchar.h>
#include <wctype.h>
#include <zapata/text/manip.h>

namespace zpt {

/** @name Type-to-String Conversions
 * Functions for converting various types to strings.
 * @{
 */
/**
 * @brief Converts an integer to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Integer value to convert.
 */
auto tostr(std::string& s, int i) -> void;
/**
 * @brief Converts a boolean to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Boolean value to convert.
 */
auto tostr(std::string& s, bool i) -> void;
/**
 * @brief Converts an integer with a custom stream manipulator to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param manip Stream manipulator function for formatting.
 */
auto tostr(std::string&, int, std::ios_base& (&)(std::ios_base&)) -> void;
#ifdef __LP64__
/**
 * @brief Converts an unsigned integer to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Unsigned integer value to convert.
 */
auto tostr(std::string& s, unsigned int i) -> void;
#endif
/**
 * @brief Converts a size_t to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Size value to convert.
 */
auto tostr(std::string& s, size_t i) -> void;
/**
 * @brief Converts a long to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Long integer value to convert.
 */
auto tostr(std::string& s, long i) -> void;
/**
 * @brief Converts a long long to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Long long integer value to convert.
 */
auto tostr(std::string& s, long long i) -> void;
/**
 * @brief Converts a float to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Float value to convert.
 * @param precision Number of decimal digits (default: 0).
 */
auto tostr(std::string& s, float i, int precision = 0) -> void;
/**
 * @brief Converts a double to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Double value to convert.
 * @param precision Number of decimal digits (default: 0).
 */
auto tostr(std::string& s, double i, int precision = 0) -> void;
/**
 * @brief Converts a char to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Character value to convert.
 */
auto tostr(std::string& s, char i) -> void;
/**
 * @brief Converts a time_t to a string (in-place).
 * @param s Output string receiving the converted value.
 * @param i Time value to convert.
 * @param f Format string passed to strftime.
 */
auto tostr(std::string& s, time_t i, const char* f) -> void;

/**
 * @brief Converts an integer to a string (returns copy).
 * @param i Integer value to convert.
 * @return String representation of the integer.
 */
auto tostr(int i) -> std::string;
/**
 * @brief Converts a boolean to a string (returns copy).
 * @param i Boolean value to convert.
 * @return String representation ("true" or "false").
 */
auto tostr(bool i) -> std::string;
/**
 * @brief Converts an integer with a custom stream manipulator to a string (returns copy).
 * @param i Integer value to convert.
 * @param manip Stream manipulator function for formatting.
 * @return String representation of the integer.
 */
auto tostr(int, std::ios_base& (&)(std::ios_base&)) -> std::string;
#ifdef __LP64__
/**
 * @brief Converts an unsigned integer to a string (returns copy).
 * @param i Unsigned integer value to convert.
 * @return String representation of the integer.
 */
auto tostr(unsigned int i) -> std::string;
#endif
/**
 * @brief Converts a size_t to a string (returns copy).
 * @param i Size value to convert.
 * @return String representation of the size.
 */
auto tostr(size_t i) -> std::string;
/**
 * @brief Converts a long to a string (returns copy).
 * @param i Long integer value to convert.
 * @return String representation of the integer.
 */
auto tostr(long i) -> std::string;
/**
 * @brief Converts a long long to a string (returns copy).
 * @param i Long long integer value to convert.
 * @return String representation of the integer.
 */
auto tostr(long long i) -> std::string;
/**
 * @brief Converts a float to a string (returns copy).
 * @param i Float value to convert.
 * @param precision Number of decimal digits (default: 0).
 * @return String representation of the float.
 */
auto tostr(float i, int precision = 0) -> std::string;
/**
 * @brief Converts a double to a string (returns copy).
 * @param i Double value to convert.
 * @param precision Number of decimal digits (default: 0).
 * @return String representation of the double.
 */
auto tostr(double i, int precision = 0) -> std::string;
/**
 * @brief Converts a char to a string (returns copy).
 * @param i Character value to convert.
 * @return String representation of the character.
 */
auto tostr(char i) -> std::string;
/**
 * @brief Converts a time_t to a string (returns copy).
 * @param i Time value to convert.
 * @param f Format string passed to strftime.
 * @return String representation of the time.
 */
auto tostr(time_t i, const char* f) -> std::string;

/** @} */

/** @name String-to-Type Conversions
 * Functions for parsing strings into various types.
 * @{
 */
/**
 * @brief Parses an integer from a string.
 * @param s String containing the integer to parse.
 * @param i Pointer to store the parsed integer.
 */
auto fromstr(std::string s, int* i) -> void;
#ifdef __LP64__
/**
 * @brief Parses an unsigned integer from a string.
 * @param s String containing the unsigned integer to parse.
 * @param i Pointer to store the parsed integer.
 */
auto fromstr(std::string s, unsigned int* i) -> void;
#endif
/**
 * @brief Parses a size_t from a string.
 * @param s String containing the size value to parse.
 * @param i Pointer to store the parsed size.
 */
auto fromstr(std::string s, size_t* i) -> void;
/**
 * @brief Parses a long from a string.
 * @param s String containing the long to parse.
 * @param i Pointer to store the parsed integer.
 */
auto fromstr(std::string s, long* i) -> void;
/**
 * @brief Parses a long long from a string.
 * @param s String containing the long long to parse.
 * @param i Pointer to store the parsed integer.
 */
auto fromstr(std::string s, long long* i) -> void;
/**
 * @brief Parses a float from a string.
 * @param s String containing the float to parse.
 * @param i Pointer to store the parsed value.
 */
auto fromstr(std::string s, float* i) -> void;
/**
 * @brief Parses a double from a string.
 * @param s String containing the double to parse.
 * @param i Pointer to store the parsed value.
 */
auto fromstr(std::string s, double* i) -> void;
/**
 * @brief Parses a char from a string.
 * @param s String containing the character to parse.
 * @param i Pointer to store the parsed character.
 */
auto fromstr(std::string s, char* i) -> void;
/**
 * @brief Parses a boolean from a string.
 * @param s String containing the boolean to parse.
 * @param i Pointer to store the parsed boolean.
 */
auto fromstr(std::string s, bool* i) -> void;
/**
 * @brief Parses a time_t from a string.
 * @param s String containing the time value to parse.
 * @param i Pointer to store the parsed time.
 * @param f Format string used for parsing.
 */
auto fromstr(std::string s, time_t* i, const char* f) -> void;
/**
 * @brief Converts a value of type T to its string representation.
 * @tparam T Type of the value to convert.
 * @param _in Value to convert.
 * @return String representation of the input value.
 */
template<typename T>
auto fromstr(T _in) -> std::string;
/** @} */

/** @brief Standard Base64 encoding lookup table (64 characters plus null terminator). */
const char encodeCharacterTable[65] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/** @brief Standard Base64 decoding lookup table (maps each byte to its 6-bit value, or -1 if
 * invalid). */
const signed char decodeCharacterTable[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
/** @brief URL-safe Base64 encoding lookup table (uses - and _ instead of + and /). */
const char encodeCharacterTableUrl[65] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
/** @brief URL-safe Base64 decoding lookup table. */
const signed char decodeCharacterTableUrl[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
/** @brief ISO-8859-1 Latin character wide string (accented vowels, cedilla, etc.). */
const std::wstring iso = L"\u00e1\u00e0\u00e2\u00e3\u00e4\u00e9\u00e8\u00ea\u1ebd\u00eb\u00ed\u00ec"
                         L"\u00ee\u0129\u00ef\u00f3"
                         L"\u00f2\u00f4\u00f5\u00f6\u00fa\u00f9\u00fb\u0169\u00fc\u00e7\u00c1\u00c0"
                         L"\u00c2\u00c3\u00c4\u00c9"
                         L"\u00c8\u00ca\u1ebc\u00cb\u00cd\u00cc\u00ce\u0128\u00cf\u00d3\u00d2\u00d4"
                         L"\u00d5\u00d6\u00da\u00d9"
                         L"\u00db\u0168\u00dc\u00c7";
/** @brief Plain ASCII equivalent wide string (unaccented vowels, no cedilla). */
const std::wstring plain = L"aaaaaeeeeeiiiiiooooouuuuucAAAAAEEEEEIIIIIOOOOOUUUUUC";

/**
 * @brief Returns the local timezone offset from UTC in seconds.
 * @return Timezone offset in seconds.
 */
time_t timezone_offset();

/**
 * @brief Demangles a C++ symbol name.
 * @param _mangled Mangled symbol name (e.g., from typeid).
 * @return Human-readable type name.
 */
auto demangle(std::string const& _mangled) -> std::string;

/**
 * @brief Captures the current call stack.
 * @param _skip Number of frames to skip from the top.
 * @return Formatted backtrace string.
 */
auto get_backtrace(int _skip = 0) -> std::string;

/**
 * @brief Base64 encoding and decoding functions.
 *
 * Provides both in-place and returning variants for Base64 operations.
 * Includes URL-safe Base64 variants that use - and _ instead of + and /.
 */
namespace base64 {
/**
 * @brief Encodes a string to Base64 (in-place).
 * @param _out String to encode; overwritten with the Base64-encoded result.
 */
void encode(std::string& _out);
/**
 * @brief Decodes a Base64 string (in-place).
 * @param _out String to decode; overwritten with the decoded bytes.
 */
void decode(std::string& _out);
/**
 * @brief Encodes stream data to Base64.
 * @param _in Input stream containing raw bytes to encode.
 * @param _out Output stream receiving the Base64-encoded result.
 */
void encode(std::istream& _in, std::ostream& _out);
/**
 * @brief Decodes Base64 stream data.
 * @param _in Input stream containing Base64-encoded text.
 * @param _out Output stream receiving the decoded bytes.
 */
void decode(std::istream& _in, std::ostream& _out);
/**
 * @brief Encodes a byte vector to Base64.
 * @param _in Byte vector to encode.
 * @param _out String receiving the Base64-encoded result.
 */
auto encode(std::vector<unsigned char> const& _in, std::string& _out) -> void;
/**
 * @brief Decodes a Base64 string into a byte vector.
 * @param _in Base64-encoded string.
 * @param _out Vector receiving the decoded bytes.
 */
auto decode(std::string const& _in, std::vector<unsigned char>& _out) -> void;
/**
 * @brief Encodes a byte vector to URL-safe Base64.
 * @param _in Byte vector to encode.
 * @param _out String receiving the URL-safe Base64-encoded result.
 * @param _pad Whether to include padding characters (`=`).
 */
auto url_encode(std::vector<unsigned char> const& _in, std::string& _out, bool _pad = true) -> void;
/**
 * @brief Decodes URL-safe Base64 into a byte vector.
 * @param _in URL-safe Base64-encoded string.
 * @param _out Vector receiving the decoded bytes.
 */
auto url_decode(std::string const& _in, std::vector<unsigned char>& _out) -> void;
/**
 * @brief Encodes to URL-safe Base64 (in-place).
 * @param _out String to encode; overwritten with the URL-safe Base64 result.
 */
void url_encode(std::string& _out);
/**
 * @brief Decodes URL-safe Base64 (in-place).
 * @param _out String to decode; overwritten with the decoded bytes.
 */
void url_decode(std::string& _out);

/**
 * @brief Returns Base64 encoded copy.
 * @param _in String to encode.
 * @return Base64-encoded string.
 */
auto r_encode(std::string const& _in) -> std::string;
/**
 * @brief Returns Base64 encoded copy of a byte vector.
 * @param _in Byte vector to encode.
 * @return Base64-encoded string.
 */
auto r_encode(std::vector<unsigned char> const& _in) -> std::string;
/**
 * @brief Returns Base64 decoded copy.
 * @param _in Base64-encoded string.
 * @return Decoded string.
 */
auto r_decode(std::string const& _in) -> std::string;
/**
 * @brief Returns URL-safe Base64 encoded copy.
 * @param _in String to encode.
 * @return URL-safe Base64-encoded string.
 */
auto r_url_encode(std::string const& _in) -> std::string;
/**
 * @brief Returns URL-safe Base64 decoded copy.
 * @param _in URL-safe Base64-encoded string.
 * @return Decoded string.
 */
auto r_url_decode(std::string const& _in) -> std::string;
} // namespace base64

using uchar = unsigned char; ///< Unsigned char alias.

/**
 * @brief UTF-8 encoding and decoding functions.
 */
namespace utf8 {
/**
 * @brief Converts a wide string to a UTF-8 encoded string.
 * @param ws Wide string to convert.
 * @return Dynamically allocated UTF-8 encoded string (caller must free with delete[]).
 */
char* wstring_to_utf8(std::wstring ws);
/**
 * @brief Converts a UTF-8 encoded string to a wide string.
 * @param s UTF-8 encoded string to convert.
 * @return Dynamically allocated wide string (caller must free with delete[]).
 */
wchar_t* utf8_to_wstring(std::string s);
/**
 * @brief Encodes a wide string as UTF-8.
 * @param s Wide string to encode.
 * @param _out Output string receiving the UTF-8 encoded result.
 * @param quote Optional quote character to wrap the output (default: none).
 */
void encode(std::wstring s, std::string& _out, char quote = '\0');
/**
 * @brief Encodes the current string as UTF-8 (in-place).
 * @param _out String to encode; overwritten with the UTF-8 encoded result.
 * @param quote Optional quote character to wrap the output (default: none).
 */
void encode(std::string& _out, char quote = '\0');
/**
 * @brief Decodes a UTF-8 encoded string in-place.
 * @param _out UTF-8 encoded string to decode; overwritten with the decoded wide string.
 */
void decode(std::string& _out);
} // namespace utf8

/**
 * @brief Unicode escape sequence functions.
 */
namespace unicode {
/**
 * @brief Escapes non-ASCII characters as \uXXXX sequences (in-place).
 * @param _out String to escape; modified in place.
 * @return void (input string modified in place).
 */
void escape(std::string& _out);
} // namespace unicode

/**
 * @brief Quoted-printable encoding functions.
 */
namespace quoted_printable {
/**
 * @brief Encodes a string in quoted-printable format.
 * @param _quote The input string to encode.
 * @param _charset The character set of the input string.
 * @param _out Output string receiving the quoted-printable encoded result.
 */
auto encode(std::string const& _quote, std::string const& _charset, std::string& _out) -> void;
/**
 * @brief Returns a quoted-printable encoded copy.
 * @param _quote The input string to encode.
 * @param _charset The character set of the input string.
 * @return Quoted-printable encoded string.
 */
auto r_encode(std::string const& _quote, std::string const& _charset) -> std::string;
} // namespace quoted_printable

/**
 * @brief URL encoding and decoding functions.
 *
 * Converts special characters to %XX format for safe inclusion in URLs.
 */
namespace url {
/**
 * @brief URL-encodes a wide string to a stream.
 * @param s Wide string to encode.
 * @param out Output stream to write the encoded result.
 */
auto encode(std::wstring s, std::ostream& out) -> void;
/**
 * @brief URL-encodes a string (in-place).
 * @param out String to encode; overwritten with URL-encoded result.
 */
auto encode(std::string& out) -> void;
/**
 * @brief URL-decodes a string (in-place).
 * @param out String to decode; overwritten with URL-decoded result.
 */
auto decode(std::string& out) -> void;
/**
 * @brief Returns a URL-encoded copy.
 * @param _out String to encode.
 * @return URL-encoded string.
 */
auto r_encode(std::string const& _out) -> std::string;
/**
 * @brief Returns a URL-decoded copy.
 * @param _out String to decode.
 * @return URL-decoded string.
 */
auto r_decode(std::string const& _out) -> std::string;
} // namespace url

/**
 * @brief ASCII encoding functions.
 */
namespace ascii {
/**
 * @brief Encodes a string for safe ASCII output.
 * @param out String to encode.
 * @param quote Whether to include surrounding quotes (default: true).
 */
void encode(std::string& out, bool quote = true);
}

/**
 * @brief Random key and identifier generation functions.
 */
namespace generate {
/**
 * @brief Generates a random key of specified size (in-place).
 * @param _out String receiving the random key.
 * @param _size Number of random bytes to generate.
 */
auto key(std::string& _out, size_t _size = 24) -> void;
/**
 * @brief Returns a random key of specified size.
 * @param _size Number of random bytes to generate.
 * @return Random key string.
 */
auto r_key(size_t _size) -> std::string;
/**
 * @brief Returns a random key (default size).
 * @return Random key string (24 bytes).
 */
auto r_key() -> std::string;
/**
 * @brief Generates a random pin of specified size (in-place).
 * @param _out String receiving the random pin.
 * @param _size Number of random digits to generate.
 */
auto pin(std::string& _out, size_t _size = 6) -> void;
/**
 * @brief Returns a random pin of specified size.
 * @param _size Number of random digits to generate.
 * @return Random pin string.
 */
auto r_pin(size_t _size) -> std::string;
/**
 * @brief Returns a random pin (default size).
 * @return Random pin string (6 digits).
 */
auto r_pin() -> std::string;
/**
 * @brief Generates a random hash (in-place).
 * @param _out String receiving the random hash.
 */
auto hash(std::string& _out) -> void;
/**
 * @brief Returns a random hash string.
 * @return Random hash string.
 */
auto r_hash() -> std::string;
} // namespace generate

/**
 * @brief Input validation functions.
 *
 * Functions to validate various string formats.
 */
namespace test {
/**
 * @brief Validates a UUID string format.
 * @param _uuid UUID string to validate.
 * @return True if the string matches a valid UUID format.
 */
auto uuid(std::string const& _uuid) -> bool;
/**
 * @brief Validates UTF-8 encoding.
 * @param _uri UTF-8 string to validate.
 * @return True if the string is valid UTF-8.
 */
auto utf8(std::string const& _uri) -> bool;
/**
 * @brief Validates ASCII-only string.
 * @param _ascii String to validate.
 * @return True if all bytes are in the ASCII range (0-127).
 */
auto ascii(std::string const& _ascii) -> bool;
/**
 * @brief Validates token format.
 * @param _token Token string to validate.
 * @return True if the token matches the expected format.
 */
auto token(std::string const& _token) -> bool;
/**
 * @brief Validates URI format.
 * @param _uri URI string to validate.
 * @return True if the string matches a valid URI format.
 */
auto uri(std::string _uri) -> bool;
/**
 * @brief Validates email address format.
 * @param _email Email address to validate.
 * @return True if the string matches a valid email format.
 */
auto email(std::string const& _email) -> bool;
/**
 * @brief Validates phone number format.
 * @param _phone Phone number to validate.
 * @return True if the string matches a valid phone number format.
 */
auto phone(std::string const& _phone) -> bool;
/**
 * @brief Tests if string matches a regex pattern.
 * @param _target String to test against the pattern.
 * @param _regex Regular expression pattern to match.
 * @return True if the target string matches the pattern.
 */
auto regex(std::string const& _target, std::string const& _regex) -> bool;
/**
 * @brief Validates ISO 8601 timestamp format.
 * @param _timestamp Timestamp string to validate.
 * @return True if the string matches ISO 8601 format.
 */
auto timestamp(std::string const& _timestamp) -> bool;
} // namespace test

/**
 * @brief Converts milliseconds since epoch to ISO 8601 timestamp.
 * @param _millis Milliseconds since Unix epoch.
 * @return ISO 8601 formatted timestamp string.
 */
auto timestamp_to_str(std::uint64_t _millis) -> std::string;

/**
 * @brief Returns current time in the specified type.
 * @tparam T Return type (std::string for ISO timestamp, numeric for millis).
 * @return Current time in the requested format.
 *
 * @par Example Usage
 * @code
 * auto ts = zpt::now<std::string>();     // "2024-01-15T10:30:00.000Z"
 * auto ms = zpt::now<uint64_t>();        // 1705315800000
 * auto sec = zpt::now<double>();         // 1705315800.0
 * @endcode
 */
template<typename T>
auto now() -> T;
} // namespace zpt

template<typename T>
auto zpt::now() -> T {
    auto _now = std::chrono::system_clock::now();
    auto _epoch = _now.time_since_epoch();
    auto _millis = std::chrono::duration_cast<std::chrono::milliseconds>(_epoch).count();
    if constexpr (std::is_same_v<T, std::string>) { return zpt::timestamp_to_str(_millis); }
    else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
        return static_cast<T>(static_cast<T>(_millis) / 1000);
    }
    else { return static_cast<T>(_millis); }
}
