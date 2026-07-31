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

/**
 * @brief Path manipulation utilities.
 */
namespace path {
/** @brief Splits a file path into components. */
auto split(std::string const& _to_split) -> zpt::json;
/** @brief Joins path components into a path string. */
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

/** @brief Converts a parsed URI JSON object back to a URI string. */
auto to_str(zpt::json _uri, zpt::json _opts = zpt::undefined) -> std::string;

/**
 * @brief Configuration loading and management utilities.
 */
namespace conf {
/** @brief Parses command-line arguments into JSON. */
auto getopt(int _argc, char* _argv[]) -> zpt::json;
/** @brief Evaluates $ref references in configuration. */
auto evaluate_ref(zpt::json _options, std::filesystem::path const& _context, zpt::json _root)
  -> zpt::json;
/** @brief Loads configuration from a file. */
auto file(std::filesystem::path const& _file, zpt::json& _options, zpt::json _root) -> void;
/** @brief Loads configuration from a directory. */
auto dirs(std::string const& _dir, zpt::json& _options) -> void;
/** @brief Loads configuration from environment variables. */
auto env(zpt::json& _options) -> void;
} // namespace conf

/**
 * @brief Command-line parameter parsing utilities.
 */
namespace parameters {
/** @brief Parses command-line arguments according to a config schema. */
auto parse(int _argc, char* _argv[], zpt::json _config) -> zpt::json;
/** @brief Verifies parameters against validation rules. */
auto verify(zpt::json _to_check, zpt::json _rules) -> void;
/** @brief Generates usage help text. */
auto usage(zpt::json _config) -> std::string;
} // namespace parameters

namespace test {
/** @brief Validates a JSON object as a geographic location (lat/lng). */
auto location(zpt::json _location) -> bool;
/** @brief Validates a JSON value as an ISO 8601 timestamp. */
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
/** @brief Parses a Cookie header into JSON. */
auto deserialize(std::string const& _cookie_header) -> zpt::json;
/** @brief Serializes JSON to a Set-Cookie header value. */
auto serialize(zpt::json _info) -> std::string;
} // namespace cookies
} // namespace http
} // namespace zpt
