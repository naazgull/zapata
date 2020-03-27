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

#include <pqxx/except>
#include <pqxx/pqxx>
#include <stddef.h>
#include <string>
#include <zapata/json.h>

namespace zpt {

namespace pgsql {

extern std::map<std::string, std::string> OPS;

auto
fromsql(pqxx::tuple _in, zpt::json _out) -> void;
auto
fromsql_r(pqxx::tuple _in) -> zpt::json;
auto
get_query(zpt::json _in, std::string& _queryr) -> void;
auto
get_opts(zpt::json _in, std::string& _queryr) -> void;
auto
get_column_values(zpt::json _document, zpt::json _opts) -> std::string;
auto
get_column_names(zpt::json _document, zpt::json _opts) -> std::string;
auto
get_column_sets(zpt::json _document, zpt::json _opts) -> std::string;
auto
escape_name(std::string const& _in) -> std::string;
auto
escape(zpt::json _in, std::string const& _str_delimiter = "'") -> std::string;
} // namespace pgsql
} // namespace zpt
