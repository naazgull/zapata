/**
 * @file json.h
 * @brief JSON utility functions for parsing, manipulation, and configuration.
 *
 * Provides helper functions that work with zpt::json objects including
 * string splitting/joining, path manipulation, email parsing, configuration
 * loading, and HTTP cookie handling.
 */

#pragma once

#include <filesystem>
#include <string>
#include <unistd.h>
#include <zapata/json/JSONClass.h>
#include <zapata/json/JSONParser.h>

namespace zpt {

/**
 * @brief Converts a JSON value to its string representation.
 * @param _in JSON value to convert.
 * @return JSON string representation.
 */
auto to_string(zpt::json _in) -> std::string;
/**
 * @brief Splits a string by separator into a JSON array.
 * @param _to_split String to split.
 * @param _separator Delimiter string.
 * @param _trim Whether to trim whitespace from parts.
 * @return JSON array of string parts.
 */
auto split(std::string const& _to_split, std::string const& _separator, bool _trim = false)
  -> zpt::json;
/**
 * @brief Joins a JSON array into a string.
 * @param _to_join JSON array of strings.
 * @param _separator Delimiter to insert between parts.
 * @return Joined string.
 */
auto join(zpt::json _to_join, std::string const& _separator) -> std::string;
/** @brief Loads JSON from a file.
 * @param _file Path to JSON file.
 * @return The JSON content. */
auto parse_json_file(std::filesystem::path const& _file) -> zpt::json;

/**
 * @brief Path manipulation utilities.
 */
namespace path {
/** @brief Splits a file path into components.
 * @param _to_split File path to split.
 * @return JSON array of path components. */
auto split(std::string const& _to_split) -> zpt::json;
/** @brief Joins path components into a path string.
 * @param _to_join JSON array of path components.
 * @return Joined path string. */
auto join(zpt::json _to_join) -> std::string;
} // namespace path

/**
 * @brief Email parsing utilities.
 */
namespace email {
/**
 * @brief Parses an email address into components.
 * @param _email Email address string.
 * @return JSON object with "local" and "domain" fields.
 */
auto parse(std::string const& _email) -> zpt::json;
} // namespace email

/** @brief Converts a parsed URI JSON object back to a URI string.
 * @param _uri Parsed URI JSON object.
 * @param _opts Formatting options (default undefined).
 * @return URI string. */
auto to_str(zpt::json _uri, zpt::json _opts = zpt::undefined) -> std::string;

/**
 * @brief Configuration loading and management utilities.
 */
namespace conf {
/** @brief Parses command-line arguments into JSON.
 * @param _argc Argument count.
 * @param _argv Argument vector.
 * @return JSON object of parsed arguments. */
auto getopt(int _argc, char* _argv[]) -> zpt::json;
/** @brief Evaluates $ref references in configuration.
 * @param _options Configuration options.
 * @param _context Base filesystem path.
 * @param _root Root JSON document.
 * @return JSON object with resolved references. */
auto evaluate_ref(zpt::json _options, std::filesystem::path const& _context, zpt::json _root)
  -> zpt::json;
/** @brief Evaluates $ref references to entities in files other than the one being processed.
 * @param _options Configuration options.
 * @param _context Base filesystem path.
 * @param _root Root JSON document.
 * @return JSON object with resolved external references. */
auto evaluate_external_ref(zpt::json _options,
                           std::filesystem::path const& _context,
                           zpt::json _root) -> zpt::json;
/** @brief Evaluates $ref references in entities in the file being processed.
 * @param _options Configuration options.
 * @param _context Base filesystem path.
 * @param _root Root JSON document.
 * @return JSON object with resolved internal references. */
auto evaluate_internal_ref(zpt::json _options,
                           std::filesystem::path const& _context,
                           zpt::json _root) -> zpt::json;
/** @brief Loads configuration from a file.
 * @param _file Path to configuration file.
 * @param _options Configuration options (modified in place).
 * @param _root Root JSON document. */
auto file(std::filesystem::path const& _file, zpt::json& _options, zpt::json _root) -> void;
/** @brief Loads configuration from a directory.
 * @param _dir Directory path to scan.
 * @param _options Configuration options (modified in place). */
auto dirs(std::string const& _dir, zpt::json& _options) -> void;
/** @brief Loads configuration from environment variables.
 * @param _options Configuration options (modified in place). */
auto env(zpt::json& _options) -> void;
} // namespace conf

/**
 * @brief Command-line parameter parsing utilities.
 */
namespace parameters {
/** @brief Parses command-line arguments according to a config schema.
 * @param _argc Argument count.
 * @param _argv Argument vector.
 * @param _config Parameter configuration schema.
 * @return JSON object of parsed parameters. */
auto parse(int _argc, char* _argv[], zpt::json _config) -> zpt::json;
/** @brief Verifies parameters against validation rules.
 * @param _to_check JSON object of parameters to verify.
 * @param _rules Validation rules. */
auto verify(zpt::json _to_check, zpt::json _rules) -> void;
/** @brief Generates usage help text.
 * @param _config Parameter configuration schema.
 * @return Help text string. */
auto usage(zpt::json _config) -> std::string;
} // namespace parameters

namespace test {
/** @brief Validates a JSON object as a geographic location (lat/lng).
 * @param _location JSON object with lat/lng fields.
 * @return True if valid location. */
auto location(zpt::json _location) -> bool;
/** @brief Validates a JSON value as an ISO 8601 timestamp.
 * @param _timestamp JSON value to validate.
 * @return True if valid timestamp. */
auto timestamp(zpt::json _timestamp) -> bool;
} // namespace test

/**
 * @brief HTTP-related JSON utilities.
 */
namespace http {
/**
 * @brief HTTP cookie handling.
 */
namespace cookies {
/** @brief Parses a Cookie header into JSON.
 * @param _cookie_header Cookie header string.
 * @return JSON object of cookie name-value pairs. */
auto deserialize(std::string const& _cookie_header) -> zpt::json;
/** @brief Serializes JSON to a Set-Cookie header value.
 * @param _info JSON object of cookie properties.
 * @return Set-Cookie header string. */
auto serialize(zpt::json _info) -> std::string;
} // namespace cookies
} // namespace http
} // namespace zpt
