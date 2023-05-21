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

#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/ast.h>
#include <filesystem>

namespace zpt {
namespace gen {
namespace rest {
class module {
  public:
    module(std::string const& _name, std::filesystem::path const& _base_path, zpt::json _schema);
    ~module() = default;

    auto generate_operations() -> module&;
    auto generate_plugin() -> module&;
    auto generate_sql() -> module&;
    auto generate_cmake() -> module&;
    auto dump() -> module&;

  private:
    std::filesystem::path __base_path;
    zpt::ast::basic_module __module;
    std::string __namespace;
    zpt::json __schema;
    std::vector<std::string> __header_files;
    std::map<std::string, zpt::json> __schema_components;

    static inline std::map<std::string, std::string> __sql_types{
        { "string", "text" },     { "integer", "bigint" }, { "double", "double" },
        { "boolean", "tinyint" }, { "date", "timestamp" },    { "object", "json" },
        { "array", "json" }
    };

    auto generate_operation_h_file(zpt::json _def, std::string const& _method)
      -> std::shared_ptr<zpt::ast::basic_file>;
    auto generate_operation_cpp_file(zpt::json _def, std::string const& _method)
      -> std::shared_ptr<zpt::ast::basic_file>;

    auto generate_collection(zpt::json _def, zpt::json _path)
      -> std::shared_ptr<zpt::ast::basic_file>;
    auto generate_document(zpt::json _def, zpt::json _path)
      -> std::shared_ptr<zpt::ast::basic_file>;
    auto generate_controller(zpt::json _def, zpt::json _path)
      -> std::shared_ptr<zpt::ast::basic_file>;
    auto generate_store(zpt::json _def, zpt::json _path) -> std::shared_ptr<zpt::ast::basic_file>;

    auto generate_add_element(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                              zpt::json _def,
                              zpt::json _path) -> void;
    auto generate_list_elements(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                                zpt::json _def,
                                zpt::json _path) -> void;
    auto generate_remove_elements(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                                  zpt::json _def,
                                  zpt::json _path) -> void;
    auto generate_update_element(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                                 zpt::json _def,
                                 zpt::json _path) -> void;
    auto generate_get_element(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                              zpt::json _def,
                              zpt::json _path) -> void;
    auto generate_remove_element(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                                 zpt::json _def,
                                 zpt::json _path) -> void;
    auto generate_process_request(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                                  zpt::json _def,
                                  zpt::json _path) -> void;

    auto add_db_configuration(std::shared_ptr<zpt::ast::basic_code_block> _block, zpt::json _def)
      -> void;
    auto add_parameters_and_validation(std::shared_ptr<zpt::ast::basic_code_block> _block,
                                       zpt::json _def,
                                       zpt::json _path) -> void;
    auto add_schema_validation(std::shared_ptr<zpt::ast::basic_code_block> _block, zpt::json _def)
      -> void;
    auto add_generated(std::shared_ptr<zpt::ast::basic_code_block> _block,
                       zpt::json _def,
                       std::string const& _generate) -> void;
    auto get_visible_fields(zpt::json _def) -> std::string;

    auto generate_sql_schemata_mysql(zpt::json _def) -> std::shared_ptr<zpt::ast::basic_file>;
};
} // namespace rest
} // namespace gen
} // namespace zpt
