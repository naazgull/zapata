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
 * @file rest.h
 * @brief REST API scaffolding code generator.
 *
 * Generates C++ backend code, SQL schemata, CMake build files,
 * and Vue.js frontend UI from an OpenAPI-style JSON schema.
 *
 * @see zpt::gen::rest::unit
 */

#pragma once

#include <filesystem>
#include <zapata/ast.h>
#include <zapata/base.h>
#include <zapata/json.h>

namespace zpt {
namespace gen {
namespace rest {

/**
 * @brief Code generation unit for a REST API resource.
 *
 * Takes a JSON schema definition and generates:
 * - C++ operation handler files (CRUD endpoints)
 * - Plugin registration code
 * - SQL DDL for MySQL
 * - CMakeLists.txt build configuration
 * - Vue.js HTML/JS UI scaffolding
 *
 * @par Example
 * @code
 * zpt::gen::rest::unit gen("my_api", backend_path, frontend_path, schema, langs);
 * gen.generate_operations()
 *    .generate_plugin()
 *    .generate_sql()
 *    .generate_cmake()
 *    .generate_ui()
 *    .dump();
 * @endcode
 */
class unit {
  public:
    unit(std::string const& _name,
         std::filesystem::path const& _base_path_backend,
         zpt::json _schema);
    ~unit() = default;

    auto generate_operations() -> unit&;
    auto generate_plugin() -> unit&;
    auto generate_sql() -> unit&;
    auto generate_cmake() -> unit&;
    auto dump() -> unit&;

  private:
    std::filesystem::path __base_path;
    zpt::ast::basic_module __module;
    std::string __namespace;
    zpt::json __schema;
    std::vector<std::string> __header_files;
    std::map<std::string, zpt::json> __schema_components;

    static inline std::map<std::string, std::string> __sql_types{
        { "string", "text" },     { "integer", "bigint" }, { "double", "double" },
        { "boolean", "tinyint" }, { "date", "timestamp" }, { "object", "json" },
        { "array", "json" }
    };

    auto generate_operation_h_file(zpt::json _def, std::string const& _method)
      -> zpt::ast::basic_file::ptr;
    auto generate_operation_cpp_file(zpt::json _def, std::string const& _method)
      -> zpt::ast::basic_file::ptr;

    auto generate_collection(zpt::json _def, zpt::json _path) -> zpt::ast::basic_file::ptr;
    auto generate_document(zpt::json _def, zpt::json _path) -> zpt::ast::basic_file::ptr;
    auto generate_controller(zpt::json _def, zpt::json _path) -> zpt::ast::basic_file::ptr;
    auto generate_store(zpt::json _def, zpt::json _path) -> zpt::ast::basic_file::ptr;

    auto generate_add_element(zpt::ast::basic_file::ptr _cpp_file, zpt::json _def, zpt::json _path)
      -> void;
    auto generate_list_elements(zpt::ast::basic_file::ptr _cpp_file,
                                zpt::json _def,
                                zpt::json _path) -> void;
    auto generate_remove_elements(zpt::ast::basic_file::ptr _cpp_file,
                                  zpt::json _def,
                                  zpt::json _path) -> void;
    auto generate_retrieve_element(zpt::ast::basic_file::ptr _cpp_file,
                                   zpt::json _def,
                                   zpt::json _path) -> void;
    auto generate_update_element(zpt::ast::basic_file::ptr _cpp_file,
                                 zpt::json _def,
                                 zpt::json _path) -> void;
    auto generate_get_element(zpt::ast::basic_file::ptr _cpp_file, zpt::json _def, zpt::json _path)
      -> void;
    auto generate_remove_element(zpt::ast::basic_file::ptr _cpp_file,
                                 zpt::json _def,
                                 zpt::json _path) -> void;
    auto generate_process_request(zpt::ast::basic_file::ptr _cpp_file,
                                  zpt::json _def,
                                  zpt::json _path) -> void;
    auto generate_redirect(zpt::ast::basic_file::ptr _cpp_file, zpt::json _def, zpt::json _path)
      -> void;

    auto add_db_configuration(zpt::ast::basic_code_block::ptr _block,
                              zpt::json _def,
                              bool _with_collection = true) -> void;
    auto add_parameters_and_validation(zpt::ast::basic_code_block::ptr _block,
                                       zpt::json _def,
                                       zpt::json _path) -> void;
    auto add_schema_validation(zpt::ast::basic_code_block::ptr _block, zpt::json _def) -> void;
    auto add_generated(zpt::ast::basic_code_block::ptr _block,
                       zpt::json _def,
                       std::string const& _generate) -> void;
    auto get_filter_expression(zpt::json _def) -> std::string;
    auto get_bind_expression(zpt::json _def) -> std::string;
    auto get_visible_fields(zpt::json _def) -> std::string;
    auto remove_hidden_fields(zpt::json _def) -> std::string;

    auto generate_sql_schemata_mysql(zpt::json _def) -> zpt::ast::basic_file::ptr;
};
} // namespace rest
} // namespace gen
} // namespace zpt
