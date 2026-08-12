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
 * @brief MySQL result set handling and SQL query generation.
 *
 * Provides utilities for converting between MySQL result sets and JSON,
 * and for generating SQL statements from JSON query descriptions.
 */

#pragma once

#include <mysql/mysql.h>
#include <zapata/json.h>

namespace zpt {
namespace storage {
namespace mysqlx {

/**
 * @brief Metadata for a MySQL result set, binding columns for fetch.
 *
 * Wraps MYSQL_BIND arrays and column metadata for a prepared statement
 * result set. Handles buffer allocation and column-level data retrieval.
 */
class result_set_metadata {
  public:
    /** @brief Array of bound column descriptors for fetching result data. */
    zpt::allocator<MYSQL_BIND>::array_pointer __bind{ nullptr };
    /** @brief Number of columns in the result set. */
    size_t __column_count{ 0 };

    /** @brief Constructs metadata for the given MySQL statement.
     * @param _statement The prepared MySQL statement to bind.
     * @return void (constructors implicitly initialize the object). */
    result_set_metadata(MYSQL_STMT* _statement);
    /** @brief Move constructor.
     * @param _rhs The result_set_metadata to move from.
     * @return void (constructors implicitly initialize the object). */
    result_set_metadata(result_set_metadata&& _rhs);
    /** @brief Destroys the metadata, freeing bound buffers.
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
    /** @brief Returns the column type by index.
     * @param _column Zero-based column index.
     * @return MySQL field type enum. */
    auto type(size_t _column) const -> enum_field_types;
    /** @brief Returns the column flags by index.
     * @param _column Zero-based column index.
     * @return Column flags bitmask. */
    auto flags(size_t _column) const -> unsigned int;
    /** @brief Returns the column character set by index.
     * @param _column Zero-based column index.
     * @return Character set ID. */
    auto charset(size_t _column) const -> unsigned int;
    template<typename T>
    auto get(MYSQL_STMT* _statement, size_t _column) const -> T;

  private:
    MYSQL_RES* __metadata{ nullptr };
};
/** @brief Converts a MySQL result row to JSON using column metadata.
 * @param _statement The prepared MySQL statement with fetched data.
 * @param _cols Reference to the result set metadata with column bindings.
 * @return JSON object representing the result row. */
auto to_json(MYSQL_STMT* _statement, zpt::storage::mysqlx::result_set_metadata& _cols) -> zpt::json;
/** @brief Generates a SELECT SQL query from JSON field/filter descriptions.
 * @param _fields JSON object specifying selected columns and options.
 * @param _filter JSON object specifying WHERE clause conditions.
 * @return Generated SELECT SQL query string. */
auto to_query(zpt::json _fields, zpt::json _filter) -> std::string;
/** @brief Generates an INSERT SQL statement from a JSON document.
 * @param _to_insert JSON array containing rows to insert.
 * @return Generated INSERT SQL statement string. */
auto to_insert(zpt::json _to_insert) -> std::string;
/** @brief Generates an UPDATE SQL statement from JSON update/pattern descriptions.
 * @param _to_update JSON object containing update fields and values.
 * @param _pattern JSON object specifying WHERE clause conditions.
 * @return Generated UPDATE SQL statement string. */
auto to_update(zpt::json _to_update, zpt::json _pattern) -> std::string;
/** @brief Generates a REPLACE SQL statement from a JSON document.
 * @param _to_replace JSON object containing column values to replace.
 * @return Generated REPLACE SQL statement string. */
auto to_replace(zpt::json _to_replace) -> std::string;
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
/** @brief Wraps a value in single quotes for SQL string literal output.
 * @param _to_quote JSON value to quote.
 * @return SQL string literal with single quotes. */
auto quote(zpt::json _to_quote) -> std::string;
} // namespace mysqlx
} // namespace storage
} // namespace zpt

template<typename T>
auto zpt::storage::mysqlx::result_set_metadata::get(MYSQL_STMT* _statement, size_t _column) const
  -> T {
    if (*this->__bind[_column].length > this->__bind[_column].buffer_length) {
        std::free(this->__bind[_column].buffer);
        this->__bind[_column].buffer = std::malloc(*this->__bind[_column].length);
        this->__bind[_column].buffer_length = *this->__bind[_column].length;

        _statement->bind[_column].buffer = this->__bind[_column].buffer;
        _statement->bind[_column].buffer_length = this->__bind[_column].buffer_length;
        mysql_stmt_fetch_column(_statement, &this->__bind[_column], _column, 0);
    }

    if constexpr (std::is_same_v<T, std::string>) {
        return std::string(reinterpret_cast<const char*>(this->__bind[_column].buffer),
                           *this->__bind[_column].length);
    }
    else if constexpr (std::is_same_v<T, std::string_view>) {
        return std::string_view(reinterpret_cast<const char*>(this->__bind[_column].buffer),
                                *this->__bind[_column].length);
    }
    else {
        if (this->__bind[_column].buffer_type == MYSQL_TYPE_BIT) {
            char* start = reinterpret_cast<char*>(this->__bind[_column].buffer);
            char* end = start + *this->__bind[_column].length;
            std::reverse(start, end);
            const size_t leftoverBytes =
              this->__bind[_column].buffer_length - *this->__bind[_column].length;
            if (leftoverBytes > 0) { memset(end, 0, leftoverBytes); }
        }

        T output{};
        std::memcpy(&output, this->__bind[_column].buffer, sizeof(T));
        return output;
    }
}
