/**
 * @file uri.h
 * @brief URI parsing and serialization functions.
 */

#pragma once

#include <zapata/json/JSONClass.h>

namespace zpt {
/**
 * @brief URI parsing and manipulation namespace.
 *
 * Provides functions for parsing URIs into structured JSON and
 * serializing them back to string form.
 */
namespace uri {

/**
 * @brief Parses a URI string into a JSON structure.
 * @param _in URI string to parse.
 * @param _type Output format: JSObject for named fields, JSArray for path segments.
 * @return JSON object with URI components (scheme, host, port, path, params, etc.).
 *
 * @par Example
 * @code
 * auto uri = zpt::uri::parse("https://api.example.com:8080/v1/users?limit=10");
 * // Returns: {"scheme":"https","host":"api.example.com","port":8080,
 * //           "path":"/v1/users","params":{"limit":"10"}}
 * @endcode
 */
auto parse(std::string const& _in, zpt::JSONType _type = zpt::JSObject) -> zpt::json;

/**
 * @brief Parses a URI from an input stream.
 * @param _in Input stream containing URI.
 * @param _type Output format.
 * @return Parsed URI as JSON.
 */
auto parse(std::istream& _in, zpt::JSONType _type = zpt::JSObject) -> zpt::json;

/**
 * @brief Converts a parsed URI back to string form.
 * @param _uri JSON object containing URI components.
 * @return Reconstructed URI string.
 */
auto to_string(zpt::json const& _uri) -> std::string;

/**
 * @brief Converts URI path segments to regex patterns.
 * @param _in URI JSON with path containing placeholders like `:id`.
 * @return JSON with regex pattern for matching.
 */
auto to_regex(zpt::json const& _in) -> zpt::json;

/**
 * @brief Converts a URI to a regex pattern for object format.
 * @param _in URI JSON containing template variables.
 * @return JSON object with the generated regex pattern.
 */
auto to_regex_object(zpt::json const& _in) -> zpt::json;

/**
 * @brief Converts a URI to a regex pattern for array format.
 * @param _in URI JSON containing template variables.
 * @return JSON object with the generated regex pattern.
 */
auto to_regex_array(zpt::json const& _in) -> zpt::json;

/**
 * @brief Path-specific serialization functions.
 */
namespace path {
/**
 * @brief Converts path component to string.
 * @param _uri URI JSON object.
 * @return Path string (e.g., "/v1/users/123").
 */
auto to_string(zpt::json const& _uri) -> std::string;
} // namespace path

/**
 * @brief Address (host:port) serialization functions.
 */
namespace address {
/**
 * @brief Converts address components to string.
 * @param _uri URI JSON object.
 * @return Address string (e.g., "example.com:8080").
 */
auto to_string(zpt::json const& _uri) -> std::string;
} // namespace address

/**
 * @brief Query parameter serialization functions.
 */
namespace params {
/**
 * @brief Converts query parameters to string.
 * @param _uri URI JSON object.
 * @param _not_first Whether or not the URI has already parameters (include '?' or not).
 * @return Query string (e.g., "?foo=bar&baz=qux").
 */
auto to_string(zpt::json const& _uri, bool _not_first = false) -> std::string;
} // namespace params

} // namespace uri
} // namespace zpt
