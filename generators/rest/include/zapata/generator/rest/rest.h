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

    /** @brief Generates C++ operation handler files for all endpoints.
     * @return Reference to this unit. */
    auto generate_operations() -> unit&;
    /** @brief Generates the plugin registration file.
     * @return Reference to this unit. */
    auto generate_plugin() -> unit&;
    /** @brief Generates SQL DDL schemata for MySQL.
     * @return Reference to this unit. */
    auto generate_sql() -> unit&;
    /** @brief Generates CMakeLists.txt build configuration.
     * @return Reference to this unit. */
    auto generate_cmake() -> unit&;
    /** @brief Serializes all generated files (calls dump on the module).
     * @return Reference to this unit. */
    auto dump() -> unit&;

  private:
    std::filesystem::path __base_path;
    zpt::ast::basic_module __module;
    std::string __namespace;
    zpt::json __schema;
    std::vector<std::string> __header_files;
    std::vector<std::string> __source_files;
    std::map<std::string, zpt::json> __schema_components;

    static inline std::map<std::string, std::string> __sql_types{
        { "string", "text" },     { "integer", "bigint" }, { "double", "double" },
        { "boolean", "tinyint" }, { "date", "timestamp" }, { "object", "json" },
        { "array", "json" }
    };

    /** @brief Generates a C++ header file for an operation handler.
     * @param _def OpenAPI operation definition.
     * @param _method HTTP method string.
     * @return Shared pointer to the generated file AST. */
    auto generate_operation_h_file(zpt::json _def, std::string const& _method)
      -> zpt::ast::basic_file::ptr;
    /** @brief Generates a C++ source file for an operation handler.
     * @param _def OpenAPI operation definition.
     * @param _method HTTP method string.
     * @return Shared pointer to the generated file AST. */
    auto generate_operation_cpp_file(zpt::json _def, std::string const& _method)
      -> zpt::ast::basic_file::ptr;

    /** @brief Generates CRUD handlers for a resource collection.
     * @param _def OpenAPI schema definition.
     * @param _path OpenAPI path object.
     * @return Shared pointer to the generated file AST. */
    auto generate_collection(zpt::json _def, zpt::json _path) -> zpt::ast::basic_file::ptr;
    /** @brief Generates CRUD handlers for a single resource document.
     * @param _def OpenAPI schema definition.
     * @param _path OpenAPI path object.
     * @return Shared pointer to the generated file AST. */
    auto generate_document(zpt::json _def, zpt::json _path) -> zpt::ast::basic_file::ptr;
    /** @brief Generates controller class for an entity.
     * @param _def OpenAPI schema definition.
     * @param _path OpenAPI path object.
     * @return Shared pointer to the generated file AST. */
    auto generate_controller(zpt::json _def, zpt::json _path) -> zpt::ast::basic_file::ptr;
    /** @brief Generates store class for data access.
     * @param _def OpenAPI schema definition.
     * @param _path OpenAPI path object.
     * @return Shared pointer to the generated file AST. */
    auto generate_store(zpt::json _def, zpt::json _path) -> zpt::ast::basic_file::ptr;

    /** @brief Generates handler for adding a single element.
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_add_element(zpt::ast::basic_file::ptr _cpp_file, zpt::json _def, zpt::json _path)
      -> void;
    /** @brief Generates handlers for listing multiple elements.
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_list_elements(zpt::ast::basic_file::ptr _cpp_file,
                                zpt::json _def,
                                zpt::json _path) -> void;
    /** @brief Generates handlers for removing multiple elements.
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_remove_elements(zpt::ast::basic_file::ptr _cpp_file,
                                  zpt::json _def,
                                  zpt::json _path) -> void;
    /** @brief Generates handler for retrieving a single element.
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_retrieve_element(zpt::ast::basic_file::ptr _cpp_file,
                                   zpt::json _def,
                                   zpt::json _path) -> void;
    /** @brief Generates handler for updating a single element.
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_update_element(zpt::ast::basic_file::ptr _cpp_file,
                                 zpt::json _def,
                                 zpt::json _path) -> void;
    /** @brief Generates handler for getting a single element (alias).
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_get_element(zpt::ast::basic_file::ptr _cpp_file, zpt::json _def, zpt::json _path)
      -> void;
    /** @brief Generates handler for removing a single element.
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_remove_element(zpt::ast::basic_file::ptr _cpp_file,
                                 zpt::json _def,
                                 zpt::json _path) -> void;
    /** @brief Generates request processing logic (validation, binding, execution).
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_process_request(zpt::ast::basic_file::ptr _cpp_file,
                                  zpt::json _def,
                                  zpt::json _path) -> void;
    /** @brief Generates HTTP redirect logic.
     * @param _cpp_file Target file AST to append to.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto generate_redirect(zpt::ast::basic_file::ptr _cpp_file, zpt::json _def, zpt::json _path)
      -> void;

    /** @brief Adds database configuration block to a code block AST.
     * @param _block Target code block to populate.
     * @param _def OpenAPI schema definition.
     * @param _with_collection Whether to include collection-level config. */
    auto add_db_configuration(zpt::ast::basic_code_block::ptr _block,
                              zpt::json _def,
                              bool _with_collection = true) -> void;
    /** @brief Adds parameter extraction and validation logic.
     * @param _block Target code block to populate.
     * @param _def OpenAPI operation definition.
     * @param _path OpenAPI path object. */
    auto add_parameters_and_validation(zpt::ast::basic_code_block::ptr _block,
                                       zpt::json _def,
                                       zpt::json _path) -> void;
    /** @brief Adds JSON schema validation code.
     * @param _block Target code block to populate.
     * @param _def OpenAPI schema definition. */
    auto add_schema_validation(zpt::ast::basic_code_block::ptr _block, zpt::json _def) -> void;
    /** @brief Generates placeholder code for marked @generated sections.
     * @param _block Target code block to populate.
     * @param _def OpenAPI operation definition.
     * @param _generate The @generated directive value. */
    auto add_generated(zpt::ast::basic_code_block::ptr _block,
                       zpt::json _def,
                       std::string const& _generate) -> void;
    /** @brief Builds a bind expression string for a parameter.
     * @param _def OpenAPI parameter definition.
     * @return SQL bind expression string. */
    auto get_bind_expression(zpt::json _def) -> std::string;
    /** @brief Extracts visible field names from schema.
     * @param _def OpenAPI schema definition.
     * @return Comma-separated field names. */
    auto get_visible_fields(zpt::json _def) -> std::string;
    /** @brief Removes hidden fields from a schema definition.
     * @param _def OpenAPI schema definition.
     * @return Schema with hidden fields stripped. */
    auto remove_hidden_fields(zpt::json _def) -> std::string;
    /** @brief Checks whether the schema defines an ID field.
     * @param _def OpenAPI schema definition.
     * @return True if ID field exists. */
    auto has_id(zpt::json _def) -> bool;

    /** @brief Generates MySQL DDL schemata from schema definition.
     * @param _def OpenAPI schema definition.
     * @return Shared pointer to the generated SQL file AST. */
    auto generate_sql_schemata_mysql(zpt::json _def) -> zpt::ast::basic_file::ptr;
};
} // namespace rest
} // namespace gen
} // namespace zpt
