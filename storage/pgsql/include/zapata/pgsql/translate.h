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
 * @file translate.h
 * @brief PostgreSQL result set handling and SQL query generation.
 *
 * Provides utilities for converting between PostgreSQL result sets and JSON,
 * and for generating SQL statements from JSON query descriptions.
 */

#pragma once

#include <libpq-fe.h>
#include <zapata/json.h>

namespace zpt {
namespace storage {
namespace pgsql {

/**
 * @brief Metadata for a PostgreSQL result set.
 *
 * Wraps column metadata for a query result set. Handles
 * column-level data retrieval and JSON conversion.
 */
class result_set_metadata {
  public:
    /** @brief Constructs metadata for the given PostgreSQL result.
     * @param _result The PostgreSQL result to extract metadata from.
     * @return void (constructors implicitly initialize the object). */
    result_set_metadata(PGresult* _result);
    /** @brief Move constructor.
     * @param _rhs The result_set_metadata to move from.
     * @return void (constructors implicitly initialize the object). */
    result_set_metadata(result_set_metadata&& _rhs);
    /** @brief Destroys the metadata, freeing the PGresult handle.
     * @return void (destructors implicitly clean up the object). */
    ~result_set_metadata();

    result_set_metadata(result_set_metadata const&) = delete;
    auto operator=(result_set_metadata const&) -> result_set_metadata& = delete;

    /** @brief Move assignment operator.
     * @param _rhs The result_set_metadata to move from.
     * @return Reference to this result_set_metadata. */
    auto operator=(result_set_metadata&& _rhs) -> result_set_metadata&;
    /** @brief Returns the column name by index.
     * @param _column Zero-based column index.
     * @return Column name string. */
    auto name(size_t _column) const -> std::string;
    /** @brief Returns the column type OID by index.
     * @param _column Zero-based column index.
     * @return PostgreSQL type OID. */
    auto type(size_t _column) const -> Oid;
    /** @brief Returns the table OID by index.
     * @param _column Zero-based column index.
     * @return PostgreSQL table OID. */
    auto tableoid(size_t _column) const -> Oid;
    /** @brief Returns the column display size by index.
     * @param _column Zero-based column index.
     * @return Column display size in characters. */
    auto column_size(size_t _column) const -> int16_t;
    /** @brief Returns whether the column is binary by index.
     * @param _column Zero-based column index.
     * @return True if the column type is binary. */
    auto is_binary(size_t _column) const -> bool;
    auto column_count() const -> int { return this->__column_count; }

    /** @brief Retrieves a string value from the result at the given column.
     * @param _result PostgreSQL result set to read from
     * @param _row Row index within the result set
     * @param _column Column index to read from
     * @return String value at the specified row and column
     */
    auto get_string(PGresult* _result, int _row, int _column) const -> std::string;
    /** @brief Retrieves an integer value from the result at the given column.
     * @tparam T Integer type to return.
     * @param _result PostgreSQL result set to read from.
     * @param _row Row index within the result set.
     * @param _column Column index to read from.
     * @return Integer value at the specified row and column, or 0 if null. */
    template<typename T>
    auto get_integer(PGresult* _result, int _row, int _column) const -> T;

  private:
    /** @brief Cached PGresult handle for retrieving column metadata. */
    PGresult* __metadata{ nullptr };
    /** @brief Number of columns in the result set. */
    int __column_count{ 0 };
};

/** @brief Converts a PostgreSQL result row to JSON using column metadata.
 * @param _result PostgreSQL result set to read from.
 * @param _cols Reference to the result set metadata with column info.
 * @param _row Row index to convert (default: 0).
 * @return JSON object representing the result row. */
auto to_json(PGresult* _result, zpt::storage::pgsql::result_set_metadata const& _cols, int _row = 0)
  -> zpt::json;
/** @brief Converts a PostgreSQL result row to JSON (convenience overload).
 * @param _result PostgreSQL result set to read from.
 * @param _row Row index to convert (default: 0).
 * @return JSON object representing the result row. */
auto to_json(PGresult* _result, int _row = 0) -> zpt::json;
/** @brief Generates a SELECT SQL query from JSON field/filter descriptions.
 * @param _fields JSON object specifying selected columns and options.
 * @param _filter JSON object specifying WHERE clause conditions.
 * @return Generated SELECT SQL query string. */
auto to_query(zpt::json _fields, zpt::json _filter) -> std::string;
/** @brief Generates an INSERT SQL statement from a JSON document.
 * @param _to_insert JSON object containing column values to insert.
 * @return Generated INSERT SQL statement string. */
auto to_insert(zpt::json _to_insert) -> std::string;
/** @brief Generates an UPDATE SQL statement from JSON update/pattern descriptions.
 * @param _to_update JSON object containing update fields and values.
 * @param _pattern JSON object specifying WHERE clause conditions.
 * @return Generated UPDATE SQL statement string. */
auto to_update(zpt::json _to_update, zpt::json _pattern) -> std::string;
/** @brief Generates a PostgreSQL UPSERT (REPLACE) statement from a JSON document.
 * @param _to_upsert JSON object containing column values to upsert.
 * @return Generated INSERT ... ON CONFLICT UPDATE SQL statement string. */
auto to_upsert(zpt::json _to_upsert) -> std::string;
/** @brief Generates a DELETE SQL statement from a JSON filter pattern.
 * @param _pattern JSON object specifying WHERE clause conditions.
 * @return Generated DELETE SQL statement string. */
auto to_delete(zpt::json _pattern) -> std::string;
/** @brief Writes a comma-separated assignment list (col=val) to the stream.
 * @param _to_convert JSON object containing columns and values.
 * @param _out Output stream to write assignments to.
 * @param _separator String separator between assignments.
 * @return void */
auto to_assignment_list(zpt::json _to_convert, std::ostream& _out, std::string_view _separator)
  -> void;
/** @brief Escapes a value for use in a PostgreSQL SQL string literal.
 * @param _conn PostgreSQL connection handle for encoding-aware escaping.
 * @param _to_quote JSON value to escape.
 * @return Escaped SQL string literal. */
auto quote(PGconn* _conn, zpt::json _to_quote) -> std::string;
/** @brief Escapes a value for use in a PostgreSQL SQL string literal (no conn).
 * @param _to_quote JSON value to escape.
 * @return Escaped SQL string literal. */
auto quote(zpt::json _to_quote) -> std::string;
} // namespace pgsql
} // namespace storage
} // namespace zpt

template<typename T>
auto zpt::storage::pgsql::result_set_metadata::get_integer(PGresult* _result,
                                                           int _row,
                                                           int _column) const -> T {
    auto* str = PQgetvalue(_result, _row, _column);
    if (!str || PQgetisnull(_result, _row, _column)) { return T{ 0 }; }
    return static_cast<T>(std::stoll(std::string{ str }));
}
