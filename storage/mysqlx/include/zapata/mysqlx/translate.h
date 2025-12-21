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

#include <mysql/mysql.h>
#include <zapata/json.h>

namespace zpt {
namespace storage {
namespace mysqlx {
class result_set_metadata {
  public:
    std::unique_ptr<MYSQL_BIND[]> __bind{ nullptr };
    size_t __column_count{ 0 };

    result_set_metadata(MYSQL_STMT* _statement);
    result_set_metadata(result_set_metadata&& _rhs);
    ~result_set_metadata();

    result_set_metadata(result_set_metadata const&) = delete;
    auto operator=(result_set_metadata const&) -> result_set_metadata& = delete;

    auto operator=(result_set_metadata&& _rhs) -> result_set_metadata&;
    auto name(size_t _column) const -> std::string;
    auto type(size_t _column) const -> enum_field_types;
    auto flags(size_t _column) const -> unsigned int;
    auto charset(size_t _column) const -> unsigned int;
    template<typename T>
    auto get(MYSQL_STMT* _statement, size_t _column) const -> T;

  private:
    MYSQL_RES* __metadata{ nullptr };
};
auto to_json(MYSQL_STMT* _statement) -> zpt::json;
auto to_query(zpt::json _fields, zpt::json _filter) -> std::string;
auto to_insert(zpt::json _to_insert) -> std::string;
auto to_update(zpt::json _to_update, zpt::json _pattern) -> std::string;
auto to_replace(zpt::json _to_replace) -> std::string;
auto to_delete(zpt::json _pattern) -> std::string;
auto to_assignment_list(zpt::json _to_convert, std::ostream& _out, std::string_view _separator)
  -> void;
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
