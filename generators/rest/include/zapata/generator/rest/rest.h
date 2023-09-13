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
    module(std::string const& _name,
           std::filesystem::path const& _base_path_backend,
           std::filesystem::path const& _base_path_frontend,
           zpt::json _schema,
           zpt::json languages);
    ~module() = default;

    auto generate_operations() -> module&;
    auto generate_plugin() -> module&;
    auto generate_sql() -> module&;
    auto generate_cmake() -> module&;
    auto generate_ui() -> module&;
    auto dump() -> module&;

  private:
    std::filesystem::path __base_path;
    std::filesystem::path __ui_path;
    zpt::ast::basic_module __module;
    std::string __namespace;
    zpt::json __schema;
    std::vector<std::string> __header_files;
    std::map<std::string, zpt::json> __schema_components;
    zpt::json __languages;

    static inline std::map<std::string, std::string> __sql_types{
        { "string", "text" },     { "integer", "bigint" }, { "double", "double" },
        { "boolean", "tinyint" }, { "date", "timestamp" }, { "object", "json" },
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

    auto generate_operation_lang_file(zpt::json _def, std::string const& _method)
      -> std::shared_ptr<zpt::ast::basic_file>;
    auto generate_operation_html_file(zpt::json _def, std::string const& _method)
      -> std::shared_ptr<zpt::ast::basic_file>;
    auto generate_collection_ui(zpt::json _def, zpt::json _path)
      -> std::shared_ptr<zpt::ast::basic_file>;
    auto generate_document_ui(zpt::json _def, zpt::json _path)
      -> std::shared_ptr<zpt::ast::basic_file>;
    auto extract_languages() -> std::string;
    auto extract_field_translations(zpt::json _def) -> std::string;
    auto extract_static_translations() -> std::string;
    auto extract_fields(zpt::json _def) -> std::string;
    auto extract_visible_fields(zpt::json _def, std::string const& _where) -> std::string;
    auto replace_additional_components(std::string& _html, zpt::json _def) -> void;
};

inline std::string collection_html_template = R"(
<!DOCTYPE html>
<html>
  <head>
    <link rel="preload stylesheet" href="/components/zpt/style.css" as="style">
    <script type="importmap">
      { "imports": {
          "data::menu": "/menu.js",
          "data::lang": "/lang/{{collection-dictionary}}.js",
          "vue": "https://unpkg.com/vue@3/dist/vue.esm-browser.js",
          "zpt": "/components/zpt/zpt.js",
          "zpt::menu": "/components/zpt/menu.js",
          "zpt::modal": "/components/zpt/modal.js",
          "zpt::side_tab": "/components/zpt/side_tab.js",
          "zpt::list": "/components/zpt/list.js",
          "zpt::form": "/components/zpt/form.js",
{{additional-imports}}
          "zpt::new_record": "/components/zpt/new_record.js"
      } }
    </script>
  </head>
  <body>
    <script type="module">
      import { default as config } from 'zpt'
      import { init_menu } from 'data::menu'
      import { init_dictionary } from 'data::lang'
      import { createApp } from 'vue'
      import { default as ZptMenu } from 'zpt::menu'
      import { default as ZptModal } from 'zpt::modal'
      import { default as ZptSideTab } from 'zpt::side_tab'
      import { default as ZptList } from 'zpt::list'
      import { default as ZptForm } from 'zpt::form'
      import { default as ZptNewRecord } from 'zpt::new_record'
{{additional-import-from}}

      init_menu(config)
      init_dictionary(config)

      createApp({
          components: {
              ZptMenu,
              ZptModal,
              ZptSideTab,
              ZptList,
              ZptForm,
{{additional-app-components}}
              ZptNewRecord
          },
          data() {
              let dictionary = config.pages.{{collection-name}}.dictionary
              let lang = config.get_lang()
              return {
                  config: config,
                  fields: {{fields}},
                  list: {
                      visible: {{list-fields}},
                      page_sizes: [ 10, 25, 50, 100 ]
                  },
                  form: {
                      visible: {{form-fields}},
                  }
              }
          }
      }).mount('#app')
    </script>
    <div id="app">
      <zpt-menu
        :config="config"
        :items="config.menu.items">
      </zpt-menu>
      <zpt-list
        :lang="config.get_lang() "
        :dictionary="config.pages.{{collection-name}}.dictionary"
        :id="'{{collection-name}}'"
        :url="config.get_url_prefix() + '{{collection-uri}}'"
        :fields="fields"
        :visible="list.visible"
        :sizes="list.page_sizes">
      </zpt-list>
      <zpt-new-record
        :trigger="'hashtag'"
        :label="'+'"
        :event="'?action=new'">
      </zpt-new-record>
      <teleport to="body">
        <zpt-side-tab
          :listen="'hashtag'">
          <template #content>
            <zpt-form
              :lang="config.get_lang() "
              :dictionary="config.pages.{{collection-name}}.dictionary"
              :id="'{{collection-name}}_form'"
              :target_url="config.get_url_prefix() + '{{collection-uri}}/{id}'"
              :fields="fields"
              :visible="form.visible"
              :clear_on_save="true">
              <template #components>
{{additional-components}}
              </template>
            </zpt-form>
          </template>
        </zpt-side-tab>
      </teleport>
    </div>
  </body>
</html>
)";

inline std::string document_html_template = R"(
<!DOCTYPE html>
<html>
  <head>
    <link rel="preload stylesheet" href="/components/zpt/style.css" as="style">
    <script type="importmap">
      { "imports": {
          "data::menu": "/menu.js",
          "data::lang": "/lang/{{document-dictionary}}.js",
          "vue": "https://unpkg.com/vue@3/dist/vue.esm-browser.js",
          "zpt": "/components/zpt/zpt.js",
          "zpt::menu": "/components/zpt/menu.js",
          "zpt::modal": "/components/zpt/modal.js",
          "zpt::form": "/components/zpt/form.js",
{{additional-imports}}
          "zpt::new_record": "/components/zpt/new_record.js"
      } }
    </script>
  </head>
  <body>
    <script type="module">
      import { default as config } from 'zpt'
      import { init_menu } from 'data::menu'
      import { init_dictionary } from 'data::lang'
      import { createApp } from 'vue'
      import { default as ZptMenu } from 'zpt::menu'
      import { default as ZptModal } from 'zpt::modal'
      import { default as ZptForm } from 'zpt::form'
      import { default as ZptNewRecord } from 'zpt::new_record'
{{additional-import-from}}

      init_menu(config)
      init_dictionary(config)

      createApp({
          components: {
              ZptMenu,
              ZptModal,
              ZptForm,
{{additional-app-components}}
              ZptNewRecord
          },
          data() {
              let dictionary = config.pages.{{document-name}}.dictionary
              let lang = config.get_lang()
              return {
                  config: config,
                  fields: {{fields}},
                  form: {
                      visible: {{form-fields}},
                  }
              }
          }
      }).mount('#app')
    </script>
    <div id="app">
      <zpt-menu
        :config="config"
        :items="config.menu.items">
      </zpt-menu>
      <zpt-new-record
        :trigger="'hashtag'"
        :label="'+'"
        :event="'?action=new'">
      </zpt-new-record>
      <zpt-form
        :lang="config.get_lang() "
        :dictionary="config.pages.{{document-name}}.dictionary"
        :id="'{{document-name}}'"
        :target_url="config.get_url_prefix() + '{{document-uri}}'"
        :fields="fields"
        :visible="form.visible"
        :clear_on_save="false">
        <template #components>
{{additional-components}}
        </template>
      </zpt-form>
    </div>
  </body>
</html>
)";

inline std::string dictionary_js_template = R"(
export const init_dictionary = (config) => {
    config.languages = { {{languages}} }
    config.pages.{{page-name}} = {
        dictionary: {
            {{field-translations}},
            {{static-translations}}
        }
    }
    config.get_url_prefix = () => {
        return '{{url-prefix}}'
    }
}
)";

} // namespace rest
} // namespace gen
} // namespace zpt
