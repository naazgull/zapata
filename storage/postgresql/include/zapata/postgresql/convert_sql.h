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

#include <pqxx/except>
#include <pqxx/pqxx>
#include <stddef.h>
#include <string>
#include <zapata/json.h>

namespace zpt {

namespace pgsql {

extern std::map<std::string, std::string> OPS;

auto fromsql(pqxx::tuple _in, zpt::json _out) -> void;
auto fromsql_r(pqxx::tuple _in) -> zpt::json;
auto get_query(zpt::json _in, std::string& _queryr) -> void;
auto get_opts(zpt::json _in, std::string& _queryr) -> void;
auto get_column_values(zpt::json _document, zpt::json _opts) -> std::string;
auto get_column_names(zpt::json _document, zpt::json _opts) -> std::string;
auto get_column_sets(zpt::json _document, zpt::json _opts) -> std::string;
auto escape_name(std::string const& _in) -> std::string;
auto escape(zpt::json _in, std::string const& _str_delimiter = "'") -> std::string;
} // namespace pgsql
} // namespace zpt
