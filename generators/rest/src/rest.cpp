#include <set>
#include <zapata/generator/rest/rest.h>
#include <zapata/uri.h>

zpt::gen::rest::unit::unit(std::string const& _module_name,
                           std::filesystem::path const& _base_path_backend,
                           zpt::json _schema)
  : __base_path{ _base_path_backend }
  , __module{ _module_name }
  , __schema{ _schema } {
    this->__namespace = (this->__schema("info")("namespace")->ok()
                           ? this->__schema("info")("namespace")->string() + std::string{ "::" }
                           : std::string{ "" }) +
                        zpt::r_replace(this->__module.name(), "/", "::");

    if (this->__schema("info")("dbDriver")->ok()) {
        if (this->__schema("info")("database")->ok()) {
            this->__schema["info"]["database"] =
              std::format("\"{}\"", this->__schema("info")("database")->string());
        }
        else {
            this->__schema["info"]["database"] =
              "_config(\"storage\")(db_driver_type)(\"database\")->string()";
        }
    }
}

auto zpt::gen::rest::unit::generate_operations() -> unit& {
    for (auto const& [_, _path, _path_def] : this->__schema("paths")) {
        if (_path_def("resource")->string() == "collection") {
            this->generate_collection(_path_def, zpt::uri::parse(_path));
        }
        else if (_path_def("resource")->string() == "document") {
            this->generate_document(_path_def, zpt::uri::parse(_path));
        }
        else if (_path_def("resource")->string() == "controller") {
            this->generate_controller(_path_def, zpt::uri::parse(_path));
        }
        else if (_path_def("resource")->string() == "store") {
            this->generate_store(_path_def, zpt::uri::parse(_path));
        }
    }
    return (*this);
}

auto zpt::gen::rest::unit::generate_plugin() -> unit& {
    auto _directory = std::filesystem::absolute(this->__base_path) / this->__module.name() / "src";
    auto _file_path = _directory / "plugin.cpp";
    if (std::filesystem::exists(_file_path)) { return (*this); }

    std::cout << "> Generating " << _file_path << "." << std::endl;

    std::filesystem::create_directories(_directory);
    auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
    this->__module.add(_file);

    for (auto const& _include : this->__header_files) {
        _file->add<zpt::ast::cpp_instruction>(std::format("#include <{}>", _include));
    }

    _file->add<zpt::ast::cpp_instruction>("#include <iostream>\n#include <zapata/rest.h>\n");

    auto _load =
      zpt::make_function<zpt::ast::cpp_function>("_zpt_load_", "void", zpt::ast::EXTERNC);
    _load->add<zpt::ast::cpp_variable>("_plugin [[maybe_unused]]", "zpt::plugin&");
    auto _load_block = zpt::make_code_block<zpt::ast::cpp_code_block>();
    _load->add(_load_block);
    _file->add(_load);

    auto _unload =
      zpt::make_function<zpt::ast::cpp_function>("_zpt_unload_", "void", zpt::ast::EXTERNC);
    _unload->add<zpt::ast::cpp_variable>("_plugin [[maybe_unused]]", "zpt::plugin&");
    auto _unload_block = zpt::make_code_block<zpt::ast::cpp_code_block>();
    _unload->add(_unload_block);
    _file->add(_unload);

    _load_block //
      ->add<zpt::ast::cpp_instruction>(std::format(
        "zlog(\"Registering listeners for module '{}'\", zpt::info)", this->__module.name()))
      .add<zpt::ast::cpp_instruction>("auto _config = zpt::GLOBAL_CONFIG()")
      .add<zpt::ast::cpp_instruction>("auto _resolver = zpt::REST_RESOLVER()")
      .add<zpt::ast::cpp_instruction>("auto _prefix = _config(\"rest\")(\"prefix\")->ok() ? "
                                      "_config(\"rest\")(\"prefix\")->string() : \"\"");
    _unload_block //
      ->add<zpt::ast::cpp_instruction>(
        std::format("zlog(\"Unloading module '{}'\", zpt::info)", this->__module.name()))
      .add<zpt::ast::cpp_instruction>("auto _config = zpt::GLOBAL_CONFIG()")
      .add<zpt::ast::cpp_instruction>("auto _resolver = zpt::REST_RESOLVER()")
      .add<zpt::ast::cpp_instruction>("auto _prefix = _config(\"rest\")(\"prefix\")->ok() ? "
                                      "_config(\"rest\")(\"prefix\")->string() : \"\"");

    for (auto const& [_, _path, _path_def] : this->__schema("paths")) {
        std::string _method;
        if (_path_def("resource")->string() == "collection") { _method = "*"; }
        else if (_path_def("resource")->string() == "document") { _method = "*"; }
        else if (_path_def("resource")->string() == "controller") { _method = "post"; }
        else if (_path_def("resource")->string() == "store") { _method = "*"; }

        auto _operation =
          std::format("{}::{}", this->__namespace, _path_def(_method)("operationId")->string());
        auto _split = zpt::uri::parse(_path);
        std::string _ref;
        for (auto const& [_idx, _, _part] : _split("path")) {
            if (_part->string().find("{") == 0) { _ref += std::string{ "/{{}}" }; }
            else { _ref += std::format("/{}", _part->string()); }
        }
        if (_path_def("resource")->string() == "controller") {
            _load_block->add<zpt::ast::cpp_instruction>(std::format(
              "_resolver->add<{}>(zpt::Post, std::format(\"{{}}{}\", _prefix))", _operation, _ref));
            _unload_block->add<zpt::ast::cpp_instruction>(
              std::format("_resolver->remove<{}>(zpt::Post, std::format(\"{{}}{}\", _prefix))",
                          _operation,
                          _ref));
        }
        else {
            _load_block->add<zpt::ast::cpp_instruction>(std::format(
              "_resolver->add<{}>(std::format(\"{{}}{}\", _prefix))", _operation, _ref));
            _unload_block->add<zpt::ast::cpp_instruction>(std::format(
              "_resolver->remove<{}>(std::format(\"{{}}{}\", _prefix))", _operation, _ref));
        }
    }
    return (*this);
}

auto zpt::gen::rest::unit::generate_sql() -> unit& {
    for (auto const& [_, __, _schema] : this->__schema("components")("schemas")) {
        if (_schema("dbCollection")->ok()) { this->generate_sql_schemata_mysql(_schema); }
    }
    return (*this);
}

auto zpt::gen::rest::unit::generate_cmake() -> unit& {
    auto _base_path = std::filesystem::absolute(this->__base_path) / this->__module.name();
    auto _file_path = _base_path / "CMakeLists.txt";

    if (!std::filesystem::exists(_file_path)) {
        std::filesystem::create_directories(_base_path);
        auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
        this->__module.add(_file);
        std::cout << "> Generating " << _file_path << "." << std::endl;

        auto _lib = std::format(
          "{}-{}",
          this->__schema("info")("namespace")->ok() ? this->__schema("info")("namespace")->string()
                                                    : "",
          zpt::r_replace(zpt::r_replace(this->__module.name(), "_", "-"), "/", "-"));
        _file->add<zpt::ast::cmake_instruction>(std::format("add_library({} SHARED)", _lib));
        std::ostringstream _oss;
        _oss << std::format("target_sources({}\n"
                            "  PRIVATE\n",
                            _lib);
        this->__module.traverse_elements([&_oss, _base_path](auto const& _file) -> void {
            if (_file->path().string().find(".cpp") != std::string::npos) {
                _oss << "    ${CMAKE_CURRENT_SOURCE_DIR}"
                     << _file->path().string().replace(0, _base_path.string().length(), "") << "\n";
            }
        });
        _oss << "  INTERFACE\n";
        this->__module.traverse_elements([&_oss, _base_path](auto const& _file) -> void {
            if (_file->path().string().find(".h") != std::string::npos &&
                _file->path().string().find(".html") == std::string::npos) {
                _oss << "    ${CMAKE_CURRENT_SOURCE_DIR}"
                     << _file->path().string().replace(0, _base_path.string().length(), "") << "\n";
            }
        });
        _oss << ")";
        _file->add<zpt::ast::cmake_instruction>(_oss.str());
        _file->add<zpt::ast::cmake_instruction>(
          std::format("target_include_directories({}\n"
                      "  PRIVATE\n"
                      "    ${{CMAKE_CURRENT_SOURCE_DIR}}/include\n"
                      "  INTERFACE\n"
                      "    ${{CMAKE_CURRENT_SOURCE_DIR}}/include\n"
                      ")",
                      _lib));
        _file->add<zpt::ast::cmake_instruction>(std::format("target_link_libraries({}\n"
                                                            "  PRIVATE\n"
                                                            "    zapata-storage-mysqlx\n"
                                                            "    zapata-engine-transport\n"
                                                            "    zapata-engine-rest\n"
                                                            "    mysqlclient\n"
                                                            ")",
                                                            _lib));
        _file->add<zpt::ast::cmake_instruction>(
          std::format("set_target_properties({}\n"
                      "  PROPERTIES\n"
                      "    VERSION ${{PROJECT_VERSION}}\n"
                      "    SOVERSION ${{PROJECT_VERSION_MAJOR}}\n"
                      "    COMPILE_FLAGS -fPIC\n"
                      "    LINK_FLAGS -shared\n"
                      "    LIBRARIES ${{CMAKE_CURRENT_BINARY_DIR}}/lib{}.so\n"
                      ")",
                      _lib,
                      _lib));
        _file->add<zpt::ast::cmake_instruction>("include(GNUInstallDirs)");
        _file->add<zpt::ast::cmake_instruction>(
          std::format("install(TARGETS {}\n"
                      "  LIBRARY\n"
                      "    DESTINATION ${{CMAKE_INSTALL_LIBDIR}}\n"
                      ")",
                      _lib));
        _file->add<zpt::ast::cmake_instruction>(
          std::format("install(DIRECTORY ${{CMAKE_CURRENT_SOURCE_DIR}}/include/{}\n"
                      "  DESTINATION ${{CMAKE_INSTALL_INCLUDEDIR}}\n"
                      "  FILES_MATCHING PATTERN \"*.h\"\n"
                      ")",
                      this->__schema("info")("namespace")->string()));
    }
    return (*this);
}

auto zpt::gen::rest::unit::dump() -> unit& {
    this->__module.dump();
    return (*this);
}

auto zpt::gen::rest::unit::generate_operation_h_file(zpt::json _def, std::string const& _method)
  -> zpt::ast::basic_file::ptr {
    auto _directory = std::filesystem::absolute(this->__base_path) / this->__module.name() /
                      "include" / this->__schema("info")("namespace")->string() /
                      this->__module.name();
    auto _file_path = _directory / std::format("{}.h", _def(_method)("operationId")->string());
    this->__header_files.push_back(std::format("{}/{}/{}.h",
                                               this->__schema("info")("namespace")->string(),
                                               this->__module.name(),
                                               _def(_method)("operationId")->string()));
    if (!std::filesystem::exists(_file_path)) {
        std::filesystem::create_directories(_directory);
        auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
        this->__module.add(_file);
        _file //
          ->add<zpt::ast::cpp_instruction>("#pragma once\n")
          .add<zpt::ast::cpp_instruction>("#include <iostream>\n#include <zapata/rest.h>\n");
        if (_def("zpt:extends")->ok()) {
            _file //
              ->add<zpt::ast::cpp_instruction>(
                std::format("#include<{}.h>\n",
                            zpt::r_replace(_def("zpt:extends")("super")->string(), "::", "/")));
        }
        std::cout << "> Generating " << _file_path << "." << std::endl;
        return _file;
    }
    return nullptr;
}

auto zpt::gen::rest::unit::generate_operation_cpp_file(zpt::json _def, std::string const& _method)
  -> zpt::ast::basic_file::ptr {
    auto _directory = std::filesystem::absolute(this->__base_path) / this->__module.name() / "src";
    auto _file_path = _directory / std::format("{}.cpp", _def(_method)("operationId")->string());
    if (!std::filesystem::exists(_file_path)) {
        std::filesystem::create_directories(_directory);
        auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
        this->__module.add(_file);

        auto _include_path = std::format("{}/{}/{}.h",
                                         this->__schema("info")("namespace")->string(),
                                         this->__module.name(),
                                         _def(_method)("operationId")->string());
        _file->add<zpt::ast::cpp_instruction>(
          std::format("#include <{}>\n#include <zapata/uri.h>\n", _include_path));
        if (this->__schema("info")("dbDriver")->is_string()) {
            auto _db_driver = this->__schema("info")("dbDriver")->string();
            _file->add<zpt::ast::cpp_instruction>(std::format(
              "#include <zapata/connector.h>\n#include <zapata/{}.h>\n\nusing "
              "db_connection_type = zpt::storage::{}::connection;\nconstexpr char const* "
              "db_driver_type = \"{}\";\n",
              _db_driver,
              _db_driver,
              _db_driver));
        }

        std::cout << "> Generating " << _file_path << "." << std::endl;
        return _file;
    }
    return nullptr;
}

auto zpt::gen::rest::unit::generate_collection(zpt::json _def, zpt::json _path)
  -> zpt::ast::basic_file::ptr {
    auto _h_file = this->generate_operation_h_file(_def, "*");
    if (_h_file != nullptr) {
        auto _namespace = zpt::make_code_block<zpt::ast::cpp_code_block>(
          std::format("namespace {}", this->__namespace));
        _h_file->add(_namespace);

        auto _extends_from = _def("zpt:extends")->ok() ? _def("zpt:extends")("super")->string()
                                                       : "zpt::events::process";
        auto _constructor_name = zpt::split(_extends_from, "::");
        _constructor_name = _constructor_name(_constructor_name->size() - 1);
        auto _class = zpt::make_class<zpt::ast::cpp_class>(_def("*")("operationId")->string(),
                                                           std::format("public {}", _extends_from));
        auto _h_constructor = zpt::make_instruction<zpt::ast::cpp_instruction>(
          std::format("using {}::{}", _extends_from, _constructor_name->string()));
        _class->add(_h_constructor, zpt::ast::PUBLIC);

        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC,
                                        std::format("~{}", _def("*")("operationId")->string()),
                                        "",
                                        zpt::ast::DEFAULT);
        if (_def("zpt:extends")->ok() && !_def("zpt:extends")("no-override")->contains("blocked")) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "blocked", "bool", zpt::ast::CONST | zpt::ast::OVERRIDE);
        }
        if (_def("zpt:extends")->ok() &&
            !_def("zpt:extends")("no-override")->contains("authorized")) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "authorized", "bool", zpt::ast::CONST | zpt::ast::OVERRIDE);
        }

        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "process_request", "zpt::events::state");
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "list_elements", "zpt::events::state");
        }
        else {
            _class //
              ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "add_element", "zpt::events::state")
              .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "list_elements", "zpt::events::state")
              .add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "remove_elements", "zpt::events::state");
        }

        _namespace->add(_class);

        auto _h_operator = zpt::make_function<zpt::ast::cpp_function>(
          "operator()", "zpt::events::state", zpt::ast::OVERRIDE);
        _h_operator->add<zpt::ast::cpp_variable>("_dispatcher", "zpt::events::dispatcher::ptr");
        _class->add(_h_operator, zpt::ast::PUBLIC);
    }

    auto _cpp_file = this->generate_operation_cpp_file(_def, "*");
    if (_cpp_file != nullptr) {
        auto _class_method_prefix =
          std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

        if (_def("zpt:extends")->ok() && !_def("zpt:extends")("no-override")->contains("blocked")) {
            auto _cpp_blocked = zpt::make_function<zpt::ast::cpp_function>(
              std::format("{}blocked", _class_method_prefix), "bool", zpt::ast::CONST);
            auto _cpp_blocked_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
            if (_def("*")("requestBody")("zpt:redirect")->ok()) {
                _cpp_blocked_body->add<zpt::ast::cpp_instruction>(
                  "return this->context() != nullptr && !this->context()->is_replied()");
            }
            else { _cpp_blocked_body->add<zpt::ast::cpp_instruction>("return false"); }
            _cpp_blocked->add(_cpp_blocked_body);
            _cpp_file->add(_cpp_blocked);
        }

        if (_def("zpt:extends")->ok() &&
            !_def("zpt:extends")("no-override")->contains("authorized")) {
            auto _cpp_authorized = zpt::make_function<zpt::ast::cpp_function>(
              std::format("{}authorized", _class_method_prefix), "bool", zpt::ast::CONST);
            auto _cpp_authorized_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
            _cpp_authorized_body->add<zpt::ast::cpp_instruction>("return true");
            _cpp_authorized->add(_cpp_authorized_body);
            _cpp_file->add(_cpp_authorized);
        }

        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            this->generate_redirect(_cpp_file, _def, _path);
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            this->generate_list_elements(_cpp_file, _def, _path);
        }
        else {
            this->generate_add_element(_cpp_file, _def, _path);
            this->generate_list_elements(_cpp_file, _def, _path);
            this->generate_remove_elements(_cpp_file, _def, _path);
        }

        std::string _allowed{ "GET, POST, DELETE" };
        auto _cpp_operator = zpt::make_function<zpt::ast::cpp_function>(
          std::format("{}operator()", _class_method_prefix), "zpt::events::state");
        _cpp_operator->add<zpt::ast::cpp_variable>("_dispatcher [[maybe_unused]]",
                                                   "zpt::events::dispatcher::ptr");
        auto _cpp_operator_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        auto _cpp_operator_switch = zpt::make_code_block<zpt::ast::cpp_code_block>(
          "switch(this->received()->performative())");
        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("[[fallthrough]]");
            auto _cpp_operator_case_post =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Post :");
            _cpp_operator_case_post->add<zpt::ast::cpp_instruction>("[[fallthrough]]");
            auto _cpp_operator_case_delete =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
            _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>(
              "return this->process_request()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_post)
              .add(_cpp_operator_case_get)
              .add(_cpp_operator_case_delete);
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->list_elements()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_get);
            _allowed.assign("GET");
        }
        else {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->list_elements()");
            auto _cpp_operator_case_post =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Post :");
            _cpp_operator_case_post->add<zpt::ast::cpp_instruction>("return this->add_element()");
            auto _cpp_operator_case_delete =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
            _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>(
              "return this->remove_elements()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_post)
              .add(_cpp_operator_case_get)
              .add(_cpp_operator_case_delete);
        }

        _cpp_operator_body //
          ->add(_cpp_operator_switch)
          .add<zpt::ast::cpp_instruction>(
            std::format("this //\n->to_send()->status(405).body() = {{ \"message\", \"Only {} "
                        "allowed to use with a collection\" }}",
                        _allowed))
          .add<zpt::ast::cpp_instruction>("return zpt::events::abort");
        _cpp_operator->add(_cpp_operator_body);
        _cpp_file->add(_cpp_operator);
    }
    return _h_file;
}

auto zpt::gen::rest::unit::generate_document(zpt::json _def, zpt::json _path)
  -> zpt::ast::basic_file::ptr {
    auto _h_file = this->generate_operation_h_file(_def, "*");
    if (_h_file != nullptr) {
        auto _namespace = zpt::make_code_block<zpt::ast::cpp_code_block>(
          std::format("namespace {}", this->__namespace));
        _h_file->add(_namespace);

        auto _extends_from = _def("zpt:extends")->ok() ? _def("zpt:extends")("super")->string()
                                                       : "zpt::events::process";
        auto _constructor_name = zpt::split(_extends_from, "::");
        _constructor_name = _constructor_name(_constructor_name->size() - 1);
        auto _class = zpt::make_class<zpt::ast::cpp_class>(_def("*")("operationId")->string(),
                                                           std::format("public {}", _extends_from));
        auto _h_constructor = zpt::make_instruction<zpt::ast::cpp_instruction>(
          std::format("using {}::{}", _extends_from, _constructor_name->string()));
        _class->add(_h_constructor, zpt::ast::PUBLIC);

        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC,
                                        std::format("~{}", _def("*")("operationId")->string()),
                                        "",
                                        zpt::ast::DEFAULT);
        if (_def("zpt:extends")->ok() && !_def("zpt:extends")("no-override")->contains("blocked")) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "blocked", "bool", zpt::ast::CONST | zpt::ast::OVERRIDE);
        }
        if (_def("zpt:extends")->ok() &&
            !_def("zpt:extends")("no-override")->contains("authorized")) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "authorized", "bool", zpt::ast::CONST | zpt::ast::OVERRIDE);
        }

        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "process_request", "zpt::events::state");
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            _class //
              ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "get_element", "zpt::events::state");
        }
        else {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "update_element", "zpt::events::state")
              .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "get_element", "zpt::events::state")
              .add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "remove_element", "zpt::events::state");
            if (_def("*")("requestBody")("dbCollection")->is_string()) {
                auto _retrieve_element =
                  zpt::make_function<zpt::ast::cpp_function>("retrieve_element", "zpt::json");
                _retrieve_element //
                  ->add<zpt::ast::cpp_variable>("_session", "zpt::storage::session&")
                  .add<zpt::ast::cpp_variable>("_params", "zpt::json");
                _class->add(_retrieve_element, zpt::ast::PRIVATE);
            }
        }

        _namespace->add(_class);

        auto _h_operator = zpt::make_function<zpt::ast::cpp_function>(
          "operator()", "zpt::events::state", zpt::ast::OVERRIDE);
        _h_operator->add<zpt::ast::cpp_variable>("_dispatcher", "zpt::events::dispatcher::ptr");
        _class->add(_h_operator, zpt::ast::PUBLIC);
    }

    auto _cpp_file = this->generate_operation_cpp_file(_def, "*");
    if (_cpp_file != nullptr) {
        auto _class_method_prefix =
          std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

        if (_def("zpt:extends")->ok() && !_def("zpt:extends")("no-override")->contains("blocked")) {
            auto _cpp_blocked = zpt::make_function<zpt::ast::cpp_function>(
              std::format("{}blocked", _class_method_prefix), "bool", zpt::ast::CONST);
            auto _cpp_blocked_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
            if (_def("*")("requestBody")("zpt:redirect")->ok()) {
                _cpp_blocked_body->add<zpt::ast::cpp_instruction>(
                  "return this->context() != nullptr && !this->context()->is_replied()");
            }
            else { _cpp_blocked_body->add<zpt::ast::cpp_instruction>("return false"); }
            _cpp_blocked->add(_cpp_blocked_body);
            _cpp_file->add(_cpp_blocked);
        }

        if (_def("zpt:extends")->ok() &&
            !_def("zpt:extends")("no-override")->contains("authorized")) {
            auto _cpp_authorized = zpt::make_function<zpt::ast::cpp_function>(
              std::format("{}authorized", _class_method_prefix), "bool", zpt::ast::CONST);
            auto _cpp_authorized_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
            _cpp_authorized_body->add<zpt::ast::cpp_instruction>("return true");
            _cpp_authorized->add(_cpp_authorized_body);
            _cpp_file->add(_cpp_authorized);
        }

        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            this->generate_redirect(_cpp_file, _def, _path);
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            this->generate_get_element(_cpp_file, _def, _path);
        }
        else {
            this->generate_update_element(_cpp_file, _def, _path);
            this->generate_get_element(_cpp_file, _def, _path);
            this->generate_remove_element(_cpp_file, _def, _path);
        }

        std::string _allowed{ "GET, PATCH, DELETE" };
        auto _cpp_operator = zpt::make_function<zpt::ast::cpp_function>(
          std::format("{}operator()", _class_method_prefix), "zpt::events::state");
        _cpp_operator->add<zpt::ast::cpp_variable>("_dispatcher [[maybe_unused]]",
                                                   "zpt::events::dispatcher::ptr");
        auto _cpp_operator_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        auto _cpp_operator_switch = zpt::make_code_block<zpt::ast::cpp_code_block>(
          "switch(this->received()->performative())");
        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("[[fallthrough]]");
            auto _cpp_operator_case_post =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Patch :");
            _cpp_operator_case_post->add<zpt::ast::cpp_instruction>("[[fallthrough]]");
            auto _cpp_operator_case_delete =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
            _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>(
              "return this->process_request()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_post)
              .add(_cpp_operator_case_get)
              .add(_cpp_operator_case_delete);
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->get_element()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_get);

            this->generate_retrieve_element(_cpp_file, _def, _path);
            _allowed.assign("GET");
        }
        else {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->get_element()");
            auto _cpp_operator_case_put =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Patch :");
            _cpp_operator_case_put->add<zpt::ast::cpp_instruction>("return this->update_element()");
            auto _cpp_operator_case_delete =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
            _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>(
              "return this->remove_element()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_put)
              .add(_cpp_operator_case_get)
              .add(_cpp_operator_case_delete);

            if (_def("*")("requestBody")("dbCollection")->is_string()) {
                this->generate_retrieve_element(_cpp_file, _def, _path);
            }
        }
        _cpp_operator_body //
          ->add(_cpp_operator_switch)
          .add<zpt::ast::cpp_instruction>(
            std::format("this //\n->to_send()->status(405).body() = {{ \"message\", \"Only {} "
                        "allowed to use with a document\" }}",
                        _allowed))
          .add<zpt::ast::cpp_instruction>("return zpt::events::abort");
        _cpp_operator->add(_cpp_operator_body);
        _cpp_file->add(_cpp_operator);
    }
    return _h_file;
}

auto zpt::gen::rest::unit::generate_controller(zpt::json _def, zpt::json _path)
  -> zpt::ast::basic_file::ptr {
    auto _h_file = this->generate_operation_h_file(_def, "post");
    if (_h_file != nullptr) {
        auto _namespace = zpt::make_code_block<zpt::ast::cpp_code_block>(
          std::format("namespace {}", this->__namespace));
        _h_file->add(_namespace);

        auto _extends_from = _def("zpt:extends")->ok() ? _def("zpt:extends")("super")->string()
                                                       : "zpt::events::process";
        auto _constructor_name = zpt::split(_extends_from, "::");
        _constructor_name = _constructor_name(_constructor_name->size() - 1);
        auto _class = zpt::make_class<zpt::ast::cpp_class>(_def("post")("operationId")->string(),
                                                           std::format("public {}", _extends_from));
        auto _h_constructor = zpt::make_instruction<zpt::ast::cpp_instruction>(
          std::format("using {}::{}", _extends_from, _constructor_name->string()));
        _class->add(_h_constructor, zpt::ast::PUBLIC);

        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC,
                                        std::format("~{}", _def("post")("operationId")->string()),
                                        "",
                                        zpt::ast::DEFAULT);
        if (_def("zpt:extends")->ok() && !_def("zpt:extends")("no-override")->contains("blocked")) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "blocked", "bool", zpt::ast::CONST | zpt::ast::OVERRIDE);
        }
        if (_def("zpt:extends")->ok() &&
            !_def("zpt:extends")("no-override")->contains("authorized")) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "authorized", "bool", zpt::ast::CONST | zpt::ast::OVERRIDE);
        }
        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "process_request", "zpt::events::state");

        _namespace->add(_class);

        auto _h_operator = zpt::make_function<zpt::ast::cpp_function>(
          "operator()", "zpt::events::state", zpt::ast::OVERRIDE);
        _h_operator->add<zpt::ast::cpp_variable>("_dispatcher", "zpt::events::dispatcher::ptr");
        _class->add(_h_operator, zpt::ast::PUBLIC);
    }

    auto _cpp_file = this->generate_operation_cpp_file(_def, "post");
    if (_cpp_file != nullptr) {
        auto _class_method_prefix =
          std::format("{}::{}::", this->__namespace, _def("post")("operationId")->string());

        if (_def("zpt:extends")->ok() && !_def("zpt:extends")("no-override")->contains("blocked")) {
            auto _cpp_blocked = zpt::make_function<zpt::ast::cpp_function>(
              std::format("{}blocked", _class_method_prefix), "bool", zpt::ast::CONST);
            auto _cpp_blocked_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
            if (_def("post")("requestBody")("zpt:redirect")->ok()) {
                _cpp_blocked_body->add<zpt::ast::cpp_instruction>(
                  "return this->context() != nullptr && !this->context()->is_replied()");
            }
            else { _cpp_blocked_body->add<zpt::ast::cpp_instruction>("return false"); }
            _cpp_blocked->add(_cpp_blocked_body);
            _cpp_file->add(_cpp_blocked);
        }

        if (_def("zpt:extends")->ok() &&
            !_def("zpt:extends")("no-override")->contains("authorized")) {
            auto _cpp_authorized = zpt::make_function<zpt::ast::cpp_function>(
              std::format("{}authorized", _class_method_prefix), "bool", zpt::ast::CONST);
            auto _cpp_authorized_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
            _cpp_authorized_body->add<zpt::ast::cpp_instruction>("return true");
            _cpp_authorized->add(_cpp_authorized_body);
            _cpp_file->add(_cpp_authorized);
        }

        if (_def("post")("requestBody")("zpt:redirect")->ok()) {
            this->generate_redirect(_cpp_file, _def, _path);
        }
        else { this->generate_process_request(_cpp_file, _def, _path); }

        auto _cpp_operator = zpt::make_function<zpt::ast::cpp_function>(
          std::format("{}operator()", _class_method_prefix), "zpt::events::state");
        _cpp_operator->add<zpt::ast::cpp_variable>("_dispatcher [[maybe_unused]]",
                                                   "zpt::events::dispatcher::ptr");
        auto _cpp_operator_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        auto _cpp_operator_switch = zpt::make_code_block<zpt::ast::cpp_code_block>(
          "switch(this->received()->performative())");
        auto _cpp_operator_case_post =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Post :");
        _cpp_operator_case_post->add<zpt::ast::cpp_instruction>("return this->process_request()");
        _cpp_operator_switch //
          ->add(_cpp_operator_case_post);
        _cpp_operator_body //
          ->add(_cpp_operator_switch)
          .add<zpt::ast::cpp_instruction>(
            "this //\n->to_send()->status(405).body() = { \"message\", \"Only POST, "
            "allowed to use with a controller\" }")
          .add<zpt::ast::cpp_instruction>("return zpt::events::abort");
        _cpp_operator->add(_cpp_operator_body);
        _cpp_file->add(_cpp_operator);
    }
    return _h_file;
}

auto zpt::gen::rest::unit::generate_store(zpt::json _def, zpt::json _path)
  -> zpt::ast::basic_file::ptr {
    auto _h_file = this->generate_operation_h_file(_def, "*");
    if (_h_file != nullptr) {
        auto _namespace = zpt::make_code_block<zpt::ast::cpp_code_block>(
          std::format("namespace {}", this->__namespace));
        _h_file->add(_namespace);

        auto _extends_from = _def("zpt:extends")->ok() ? _def("zpt:extends")("super")->string()
                                                       : "zpt::events::process";
        auto _constructor_name = zpt::split(_extends_from, "::");
        _constructor_name = _constructor_name(_constructor_name->size() - 1);
        auto _class = zpt::make_class<zpt::ast::cpp_class>(_def("*")("operationId")->string(),
                                                           std::format("public {}", _extends_from));
        auto _h_constructor = zpt::make_instruction<zpt::ast::cpp_instruction>(
          std::format("using {}::{}", _extends_from, _constructor_name->string()));
        _class->add(_h_constructor, zpt::ast::PUBLIC);

        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC,
                                        std::format("~{}", _def("*")("operationId")->string()),
                                        "",
                                        zpt::ast::DEFAULT);
        if (_def("zpt:extends")->ok() && !_def("zpt:extends")("no-override")->contains("blocked")) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "blocked", "bool", zpt::ast::CONST | zpt::ast::OVERRIDE);
        }
        if (_def("zpt:extends")->ok() &&
            !_def("zpt:extends")("no-override")->contains("authorized")) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "authorized", "bool", zpt::ast::CONST | zpt::ast::OVERRIDE);
        }

        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "process_request", "zpt::events::state");
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            _class //
              ->add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "list_elements", "zpt::events::state");
        }
        else {
            _class //
              ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "add_element", "zpt::events::state")
              .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "list_elements", "zpt::events::state")
              .add<zpt::ast::cpp_function>(
                zpt::ast::PUBLIC, "remove_elements", "zpt::events::state");
        }

        _namespace->add(_class);

        auto _h_operator = zpt::make_function<zpt::ast::cpp_function>(
          "operator()", "zpt::events::state", zpt::ast::OVERRIDE);
        _h_operator->add<zpt::ast::cpp_variable>("_dispatcher", "zpt::events::dispatcher::ptr");
        _class->add(_h_operator, zpt::ast::PUBLIC);
    }

    auto _cpp_file = this->generate_operation_cpp_file(_def, "*");
    if (_cpp_file != nullptr) {
        auto _class_method_prefix =
          std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

        if (_def("zpt:extends")->ok() && !_def("zpt:extends")("no-override")->contains("blocked")) {
            auto _cpp_blocked = zpt::make_function<zpt::ast::cpp_function>(
              std::format("{}blocked", _class_method_prefix), "bool", zpt::ast::CONST);
            auto _cpp_blocked_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
            if (_def("*")("requestBody")("zpt:redirect")->ok()) {
                _cpp_blocked_body->add<zpt::ast::cpp_instruction>(
                  "return this->context() != nullptr && !this->context()->is_replied()");
            }
            else { _cpp_blocked_body->add<zpt::ast::cpp_instruction>("return false"); }
            _cpp_blocked->add(_cpp_blocked_body);
            _cpp_file->add(_cpp_blocked);
        }

        if (_def("zpt:extends")->ok() &&
            !_def("zpt:extends")("no-override")->contains("authorized")) {
            auto _cpp_authorized = zpt::make_function<zpt::ast::cpp_function>(
              std::format("{}authorized", _class_method_prefix), "bool", zpt::ast::CONST);
            auto _cpp_authorized_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
            _cpp_authorized_body->add<zpt::ast::cpp_instruction>("return true");
            _cpp_authorized->add(_cpp_authorized_body);
            _cpp_file->add(_cpp_authorized);
        }

        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            this->generate_redirect(_cpp_file, _def, _path);
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            this->generate_list_elements(_cpp_file, _def, _path);
        }
        else {
            this->generate_add_element(_cpp_file, _def, _path);
            this->generate_list_elements(_cpp_file, _def, _path);
            this->generate_remove_elements(_cpp_file, _def, _path);
        }

        std::string _allowed{ "GET, POST, DELETE" };
        auto _cpp_operator = zpt::make_function<zpt::ast::cpp_function>(
          std::format("{}operator()", _class_method_prefix), "zpt::events::state");
        _cpp_operator->add<zpt::ast::cpp_variable>("_dispatcher [[maybe_unused]]",
                                                   "zpt::events::dispatcher::ptr");
        auto _cpp_operator_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        auto _cpp_operator_switch = zpt::make_code_block<zpt::ast::cpp_code_block>(
          "switch(this->received()->performative())");
        if (_def("*")("requestBody")("zpt:redirect")->ok()) {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("[[fallthrough]]");
            auto _cpp_operator_case_post =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Put :");
            _cpp_operator_case_post->add<zpt::ast::cpp_instruction>("[[fallthrough]]");
            auto _cpp_operator_case_delete =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
            _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>(
              "return this->process_request()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_post)
              .add(_cpp_operator_case_get)
              .add(_cpp_operator_case_delete);
        }
        else if (_def("*")("requestBody")("zpt:view")->ok()) {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->list_elements()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_get);
            _allowed.assign("GET");
        }
        else {
            auto _cpp_operator_case_get =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
            _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->list_elements()");
            auto _cpp_operator_case_post =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Put :");
            _cpp_operator_case_post->add<zpt::ast::cpp_instruction>("return this->add_element()");
            auto _cpp_operator_case_delete =
              zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
            _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>(
              "return this->remove_elements()");
            _cpp_operator_switch //
              ->add(_cpp_operator_case_post)
              .add(_cpp_operator_case_get)
              .add(_cpp_operator_case_delete);
        }
        _cpp_operator_body //
          ->add(_cpp_operator_switch)
          .add<zpt::ast::cpp_instruction>(
            std::format("this //\n->to_send()->status(405).body() = {{ \"message\", \"Only {} "
                        "allowed to use with a store\" }}",
                        _allowed))
          .add<zpt::ast::cpp_instruction>("return zpt::events::abort");
        _cpp_operator->add(_cpp_operator_body);
        _cpp_file->add(_cpp_operator);
    }
    return _h_file;
}

auto zpt::gen::rest::unit::generate_add_element(zpt::ast::basic_file::ptr _cpp_file,
                                                zpt::json _def,
                                                zpt::json _path) -> void {
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}add_element", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);

    _method_body //
      ->add<zpt::ast::cpp_instruction>("auto _received = this->received()->body()")
      .add<zpt::ast::cpp_instruction>(
        "auto _params = this->received()->parameters()->is_object() ? "
        "this->received()->parameters()->clone() : zpt::json::object()");
    this->add_generated(_method_body, _def, "create");
    this->add_parameters_and_validation(_method_body, _def, _path);
    this->add_schema_validation(_method_body, _def);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    if (_def("parameters")->size() != 0) {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>(
            std::format("_received += {}", this->get_bind_expression(_def)));
    }

    if (_def("*")("requestBody")("dbCollection")->is_string()) {
        if (!this->has_id(_def)) {
            _method_try_body //
              ->add<zpt::ast::cpp_instruction>(
                "auto _id = _collection //\n->add(_received)->execute()->generated_id()(0)")
              .add<zpt::ast::cpp_instruction>("_session->commit()")
              .add<zpt::ast::cpp_instruction>(
                "this //\n->to_send()->status(201).body() = _received + zpt::json{ \"_id\", _id }");
        }
        else {
            _method_try_body //
              ->add<zpt::ast::cpp_instruction>("_collection //\n->add(_received)->execute()")
              .add<zpt::ast::cpp_instruction>("_session->commit()")
              .add<zpt::ast::cpp_instruction>(
                "this //\n->to_send()->status(201).body() = _received");
        }
    }
    else {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(201).body() = _received");
    }
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>(
        "this //\n->to_send()->status(500).body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::generate_list_elements(zpt::ast::basic_file::ptr _cpp_file,
                                                  zpt::json _def,
                                                  zpt::json _path) -> void {
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}list_elements", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>(
        "auto _params = this->received()->parameters()->is_object() ? "
        "this->received()->parameters()->clone() : zpt::json::object()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    if (_def("*")("requestBody")("dbCollection")->is_string() ||
        _def("*")("requestBody")("zpt:view")->ok()) {

        if (_def("*")("requestBody")("dbCollection")->is_string()) {
            _method_try_body //
              ->add<zpt::ast::cpp_instruction>(
                std::format("zpt::json _fields = {}", this->get_visible_fields(_def)))
              .add<zpt::ast::cpp_instruction>("_fields << \"_id\"")
              .add<zpt::ast::cpp_instruction>(this->remove_hidden_fields(_def));
            _method_try_body //
              ->add<zpt::ast::cpp_instruction>(
                "auto _result = zpt::storage::filter_find(_collection, "
                "_params) //\n->fields(_fields)->execute()->fetch()");
        }
        else if (_def("*")("requestBody")("zpt:view")->is_string()) {
            _method_try_body //
              ->add<zpt::ast::cpp_instruction>(
                "auto _result = zpt::storage::filter_find(_collection, "
                "_params) //\n->execute()->fetch()");
        }
        else {
            _method_try_body //
              ->add<zpt::ast::cpp_instruction>(
                "auto _result = _database //\n->sql(\"STATEMTENT HERE\")->fetch()");
        }

        auto _if_block =
          zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result->size() != 0)");
        _if_block //
          ->add<zpt::ast::cpp_instruction>(
            "this //\n->to_send()->status(200).body() = { \"items\", "
            "_result, \"size\", _collection->count(zpt::storage::extract_find(_params)) }")
          .add<zpt::ast::cpp_instruction>(
            "zpt::storage::reply_find(this->to_send()->body(), _params)");
        _method_try_body->add(_if_block);

        auto _else_block = zpt::make_code_block<zpt::ast::cpp_code_block>("else");
        _else_block->add<zpt::ast::cpp_instruction>("this->to_send()->status(204)");
        _method_try_body->add(_else_block);
    }
    else {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(200).body() = { "
                                           "\"items\", zpt::json::array(), \"size\", 0 }");
    }
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>(
        "this //\n->to_send()->status(500).body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::generate_remove_elements(zpt::ast::basic_file::ptr _cpp_file,
                                                    zpt::json _def,
                                                    zpt::json _path) -> void {
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}remove_elements", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>(
        "auto _params = this->received()->parameters()->is_object() ? "
        "this->received()->parameters()->clone() : zpt::json::object()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    if (_def("*")("requestBody")("dbCollection")->is_string()) {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>(
            "auto _result = zpt::storage::filter_remove(_collection, _params) "
            "//\n->execute()->count()")
          .add<zpt::ast::cpp_instruction>("_session->commit()")
          .add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(202).body() = { "
                                          "\"removed_for\", _params, \"removed_count\", _result }");
    }
    else {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(202).body() = { "
                                           "\"removed_for\", _params, \"removed_count\", 0 }");
    }
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>(
        "this //\n->to_send()->status(500).body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::generate_retrieve_element(zpt::ast::basic_file::ptr _cpp_file,
                                                     zpt::json _def,
                                                     zpt::json) -> void {
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}retrieve_element", _class_method_prefix), "zpt::json");
    _method //
      ->add<zpt::ast::cpp_variable>("_session", "zpt::storage::session&")
      .add<zpt::ast::cpp_variable>("_params", "zpt::json");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();

    if (_def("*")("requestBody")("dbCollection")->is_string()) {
        _method_body //
          ->add<zpt::ast::cpp_instruction>("auto _config = zpt::GLOBAL_CONFIG()")
          .add<zpt::ast::cpp_instruction>(
            std::format("auto _collection = _session->database({})->collection(\"{}\")",
                        this->__schema("info")("database")->string(),
                        _def("*")("requestBody")("dbCollection")->string()))
          .add<zpt::ast::cpp_instruction>(
            std::format("zpt::json _fields = {}", this->get_visible_fields(_def)))
          .add<zpt::ast::cpp_instruction>("_fields << \"_id\"")
          .add<zpt::ast::cpp_instruction>(this->remove_hidden_fields(_def))
          .add<zpt::ast::cpp_instruction>("auto _result = zpt::storage::filter_find(_collection, "
                                          "_params) //\n->fields(_fields)->execute()->fetch(1)");
    }
    else if (_def("*")("requestBody")("zpt:view")->is_string()) {
        _method_body //
          ->add<zpt::ast::cpp_instruction>("auto _config = zpt::GLOBAL_CONFIG()")
          .add<zpt::ast::cpp_instruction>(
            std::format("auto _collection = _session->database({})->collection(\"{}\")",
                        this->__schema("info")("database")->string(),
                        _def("*")("requestBody")("zpt:view")->string()))
          .add<zpt::ast::cpp_instruction>("auto _result = zpt::storage::filter_find(_collection, "
                                          "_params) //\n->execute()->fetch(1)");
    }
    else {
        _method_body //
          ->add<zpt::ast::cpp_instruction>("auto _config = zpt::GLOBAL_CONFIG()")
          .add<zpt::ast::cpp_instruction>(std::format("auto _database = _session->database({})",
                                                      this->__schema("info")("database")->string()))
          .add<zpt::ast::cpp_instruction>(
            "auto _result = _database //\n->sql(\"SQL STATEMENT HERE\")->fetch(1)");
    }

    auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result->size() != 0)");
    _if_block //
      ->add<zpt::ast::cpp_instruction>("return _result(0)");
    _method_body->add(_if_block);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::undefined");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::generate_update_element(zpt::ast::basic_file::ptr _cpp_file,
                                                   zpt::json _def,
                                                   zpt::json _path) -> void {
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}update_element", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>("auto _received = this->received()->body()")
      .add<zpt::ast::cpp_instruction>(
        "auto _params = this->received()->parameters()->is_object() ? "
        "this->received()->parameters()->clone() : zpt::json::object()");
    this->add_parameters_and_validation(_method_body, _def, _path);
    this->add_generated(_method_body, _def, "update");

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    if (_def("*")("requestBody")("dbCollection")->is_string()) {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>(
            "auto _result = zpt::storage::filter_modify(_collection, "
            "_params) //\n->patch(_received)->execute()->count()")
          .add<zpt::ast::cpp_instruction>("_session->commit()");

        auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result != 0)");
        _if_block //
          ->add<zpt::ast::cpp_instruction>(
            "this //\n->to_send()->status(202).body() = this->retrieve_element(_session, _params)");
        _method_try_body->add(_if_block);
        auto _else_block = zpt::make_code_block<zpt::ast::cpp_code_block>("else");
        _else_block->add<zpt::ast::cpp_instruction>("this->to_send()->status(404)");
        _method_try_body->add(_else_block);
    }
    else {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(202).body() = _received");
    }
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>(
        "this //\n->to_send()->status(500).body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::generate_get_element(zpt::ast::basic_file::ptr _cpp_file,
                                                zpt::json _def,
                                                zpt::json _path) -> void {
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}get_element", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def, false);
    _method_body //
      ->add<zpt::ast::cpp_instruction>(
        "auto _params = this->received()->parameters()->is_object() ? "
        "this->received()->parameters()->clone() : zpt::json::object()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    if (_def("*")("requestBody")("dbCollection")->is_string() ||
        _def("*")("requestBody")("zpt:view")->ok()) {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>(
            "auto _result = this->retrieve_element(_session, _params)");
        auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result->ok())");
        _if_block //
          ->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(200).body() = _result");
        _method_try_body->add(_if_block);
        auto _else_block = zpt::make_code_block<zpt::ast::cpp_code_block>("else");
        _else_block->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(404)");
        _method_try_body->add(_else_block);
    }
    else {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(200).body() = _received");
    }
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>(
        "this //\n->to_send()->status(500).body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::generate_remove_element(zpt::ast::basic_file::ptr _cpp_file,
                                                   zpt::json _def,
                                                   zpt::json _path) -> void {
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}remove_element", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>(
        "auto _params = this->received()->parameters()->is_object() ? "
        "this->received()->parameters()->clone() : zpt::json::object()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    if (_def("*")("requestBody")("dbCollection")->is_string()) {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>(
            "auto _result = zpt::storage::filter_remove(_collection, _params) "
            "//\n->execute()->count()")
          .add<zpt::ast::cpp_instruction>("_session->commit()");
        auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result != 0)");
        _if_block //
          ->add<zpt::ast::cpp_instruction>(
            "this //\n->to_send()->status(202).body() = { \"removed_count\", _result }");
        _method_try_body->add(_if_block);
        auto _else_block = zpt::make_code_block<zpt::ast::cpp_code_block>("else");
        _else_block->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(404)");
        _method_try_body->add(_else_block);
    }
    else {
        _method_try_body //
          ->add<zpt::ast::cpp_instruction>(
            "this //\n->to_send()->status(202).body() = { \"removed_count\", 0 }");
    }
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>(
        "this //\n->to_send()->status(500).body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::generate_process_request(zpt::ast::basic_file::ptr _cpp_file,
                                                    zpt::json _def,
                                                    zpt::json _path) -> void {
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def("post")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}process_request", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    _method_body //
      ->add<zpt::ast::cpp_instruction>("auto _received = this->received()->body()")
      .add<zpt::ast::cpp_instruction>(
        "auto _params = this->received()->parameters()->is_object() ? "
        "this->received()->parameters()->clone() : zpt::json::object()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _method_try_body //
      ->add<zpt::ast::cpp_instruction>("this //\n->to_send()->status(200).body() = { }");
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>(
        "this //\n->to_send()->status(500).body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::generate_redirect(zpt::ast::basic_file::ptr _cpp_file,
                                             zpt::json _def,
                                             zpt::json _path) -> void {
    auto _performative = _def("*")->ok() ? "*" : "post";
    auto _class_method_prefix =
      std::format("{}::{}::", this->__namespace, _def(_performative)("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      std::format("{}process_request", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    _method_body->add<zpt::ast::cpp_instruction>("auto _received = this->received()->body()");

    auto _if_block =
      zpt::make_code_block<zpt::ast::cpp_code_block>("if (this->context() == nullptr)");
    _if_block //
      ->add<zpt::ast::cpp_instruction>("auto _config = zpt::GLOBAL_CONFIG()")
      .add<zpt::ast::cpp_instruction>("auto _prefix = _config(\"rest\")(\"prefix\")->ok() ? "
                                      "_config(\"rest\")(\"prefix\")->string() : \"\"")
      .add<zpt::ast::cpp_instruction>(
        "auto _transport = _config(\"transport\")(\"default\")->ok() ? "
        "_config(\"transport\")(\"default\")->string() : \"tcp\"")
      .add<zpt::ast::cpp_instruction>(
        "auto _params = this->received()->parameters()->is_object() ? "
        "this->received()->parameters()->clone() : zpt::json::object()");
    this->add_parameters_and_validation(_if_block, _def, _path);

    auto _try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _try_body //
      ->add<zpt::ast::cpp_instruction>(
        std::format("auto _redirect_to{{ \"{}\" }}",
                    _def(_performative)("requestBody")("zpt:redirect")->string()))
      .add<zpt::ast::cpp_instruction>(
        "auto _request = zpt::TRANSPORT_LAYER() //\n.get(_transport)->make_request()")
      .add<zpt::ast::cpp_instruction>(
        "_request //\n->performative(this->received()->performative()).uri(std::format(\"{}{}{}\", "
        "_prefix, _redirect_to, zpt::uri::params::to_string(this->received()->uri()))).body() = "
        "_received")
      .add<zpt::ast::cpp_instruction>(
        "this->context(zpt::make_call(zpt::REST_RESOLVER(), _request))")
      .add<zpt::ast::cpp_instruction>("return zpt::events::retrigger");
    _if_block->add(_try_body);
    auto _catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _catch_body //
      ->add<zpt::ast::cpp_instruction>(
        "this //\n->to_send()->status(500).body() = { \"message\", _e.what() }");
    _if_block->add(_catch_body);

    auto _else_block =
      zpt::make_code_block<zpt::ast::cpp_code_block>("else if (this->context()->is_replied())");
    _else_block //
      ->add<zpt::ast::cpp_instruction>("auto _forwarded = this->context()->reply()")
      .add<zpt::ast::cpp_instruction>(
        "this->to_send() //\n->status(_forwarded->status()).body() = _forwarded->body()")
      .add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method_body //
      ->add(_if_block)
      .add(_else_block)
      .add<zpt::ast::cpp_instruction>("return zpt::events::abort");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::unit::add_db_configuration(zpt::ast::basic_code_block::ptr _block,
                                                zpt::json _def,
                                                bool _with_collection) -> void {
    _block //
      ->add<zpt::ast::cpp_instruction>("auto _config = zpt::GLOBAL_CONFIG()");
    if (this->__schema("info")("database")->is_string()) {
        _block->add<zpt::ast::cpp_instruction>(
          "auto _session = zpt::make_connection<db_connection_type>(_config)->session()");
        if (_with_collection) {
            if (_def("*")("requestBody")("dbCollection")->is_string()) {
                _block-> //
                  add<zpt::ast::cpp_instruction>(
                    std::format("auto _collection = _session->database({})->collection(\"{}\")",
                                this->__schema("info")("database")->string(),
                                _def("*")("requestBody")("dbCollection")->string()));
            }
            else if (_def("*")("requestBody")("zpt:view")->is_string()) {
                _block-> //
                  add<zpt::ast::cpp_instruction>(
                    std::format("auto _collection = _session->database({})->collection(\"{}\")",
                                this->__schema("info")("database")->string(),
                                _def("*")("requestBody")("zpt:view")->string()));
            }
            else {
                _block-> //
                  add<zpt::ast::cpp_instruction>(
                    std::format("auto _database = _session->database({})",
                                this->__schema("info")("database")->string()));
            }
        }
    }
}

auto zpt::gen::rest::unit::add_parameters_and_validation(zpt::ast::basic_code_block::ptr _block,
                                                         zpt::json _def,
                                                         zpt::json _path) -> void {
    bool _has_path{ false };
    for (auto const& [_, __, _param] : _def("parameters")) {
        if (_param("in")->string() == "path") {
            if (!_has_path) {
                _block
                  ->add<zpt::ast::cpp_instruction>("auto _path = this->received()->uri()(\"path\")")
                  .add<zpt::ast::cpp_instruction>(
                    "size_t _prefix_len = "
                    "zpt::GLOBAL_CONFIG()(\"rest\")(\"prefix_path_len\")->integer()");
                _has_path = true;
            }
        }
    }

    for (auto const& [_idx, _, _part] : _path("path")) {
        auto _variable = _part->string();
        if (_variable.find("{") == 0) {
            std::string _name = _variable.substr(1, _variable.length() - 2);
            if (_name == "id") { _name = "_id"; }
            _block //
              ->add<zpt::ast::cpp_instruction>(
                std::format("_params[\"{}\"] = _path(_prefix_len + {})", _name, _idx));
        }
    }
    for (auto const& [_, __, _param] : _def("parameters")) {
        if (_param("in")->string() == "path" ||
            (_param("in")->string() == "query" && _param("required")->boolean())) {
            auto _name = _param("name")->string();
            if (_name == "id") { _name = "_id"; }
            _block->add<zpt::ast::cpp_instruction>(
              std::format("expect(_params(\"{}\")->ok(), \"Required {} parameter '{}'\")",
                          _name,
                          _param("in")->string(),
                          _param("name")->string()));
        }
    }
}

auto zpt::gen::rest::unit::add_schema_validation(zpt::ast::basic_code_block::ptr _block,
                                                 zpt::json _def) -> void {
    for (auto const& [_, __, _object] : _def("*")("requestBody")("allOf")) {
        auto _required = _object("required");
        if (_required->is_array()) {
            for (auto const& [_, __, _name] : _required) {
                _block->add<zpt::ast::cpp_instruction>(std::format(
                  "expect(_received(\"{}\")->ok(), \"Required request member field '{}'\")",
                  _name->string(),
                  _name->string()));
            }
        }
    }
}

auto zpt::gen::rest::unit::add_generated(zpt::ast::basic_code_block::ptr _block,
                                         zpt::json _def,
                                         std::string const& _generate) -> void {
    for (auto const& [_, __, _object] : _def("*")("requestBody")("allOf")) {
        for (auto const& [___, _name, _prop] : _object("properties")) {
            if (_prop("rest:generation_expr")->ok() &&
                (_prop("rest:generate")->contains(_generate) ||
                 _prop("rest:generate")->contains("always"))) {
                _block->add<zpt::ast::cpp_instruction>(std::format(
                  "_received[\"{}\"] = {}", _name, _prop("rest:generation_expr")->string()));
            }
            else if (_prop("default")->ok() && _generate == "create") {
                auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>(
                  std::format("if (!_received(\"{}\")->ok())", _name));
                if (_prop("type")->string() == "object") {
                    std::string _value = _prop("default");
                    _value = _value.substr(1);
                    _value = _value.substr(0, _value.length() - 1);
                    zpt::replace(_value, "{", "zpt::json{");
                    zpt::replace(_value, "[]", "zpt::json::array()");
                    zpt::replace(_value, "[", "zpt::json{zpt::array,");
                    zpt::replace(_value, "]", "}");
                    zpt::replace(_value, ":", ",");
                    zpt::trim(_value);
                    if (_value.length() == 0) { _value = "zpt::json::object()"; }
                    else { _value = std::format("zpt::json{{ {} }}", _value); }
                    _if_block->add<zpt::ast::cpp_instruction>(
                      std::format("_received[\"{}\"] = {}", _name, _value));
                }
                else if (_prop("type")->string() == "array") {
                    std::string _value = _prop("default");
                    _value = _value.substr(1);
                    _value = _value.substr(0, _value.length() - 1);
                    zpt::replace(_value, "{", "zpt::json{");
                    zpt::replace(_value, "[]", "zpt::json::array()");
                    zpt::replace(_value, "[", "zpt::json{zpt::array,");
                    zpt::replace(_value, "]", "}");
                    zpt::replace(_value, ":", ",");
                    zpt::trim(_value);
                    if (_value.length() == 0) { _value = "zpt::json::array()"; }
                    else { _value = std::format("zpt::json{{ zpt::array, {} }}", _value); }
                    _if_block->add<zpt::ast::cpp_instruction>(
                      std::format("_received[\"{}\"] = {}", _name, _value));
                }
                else {
                    _if_block->add<zpt::ast::cpp_instruction>(
                      std::format("_received[\"{}\"] = {}",
                                  _name,
                                  _prop("default")->ok() ? _prop("default") : "zpt::undefined"));
                }
                _block->add(_if_block);
            }
        }
    }
}

auto zpt::gen::rest::unit::get_bind_expression(zpt::json _def) -> std::string {
    std::ostringstream _oss;
    _oss << "{ ";
    bool _first = true;
    for (auto const& [_, __, _param] : _def("parameters")) {
        auto _name = _param("name")->string();
        if (!_first) { _oss << ", "; }
        _first = false;
        _oss << "\"" << _name << "\", ";
        _oss << "_params(\"" << _name << "\")";
    }
    _oss << " }" << std::flush;
    return _oss.str();
}

auto zpt::gen::rest::unit::get_visible_fields(zpt::json _def) -> std::string {
    std::ostringstream _oss;
    _oss << R"((_params("fields")->ok() ? zpt::split(_params("fields")->string(), ",") : )";
    if (_def("*")("requestBody")("allOf")->ok()) {
        std::set<std::string> _visible;
        for (auto const& [_, __, _type] : _def("*")("requestBody")("allOf")) {
            for (auto const& [___, _name, _prop] : _type("properties")) { _visible.insert(_name); }
            for (auto const& [___, __, _prop] : _type("hidden")) { _visible.erase(_prop); }
        }
        _oss << "zpt::json{ zpt::array";
        for (auto const& _prop : _visible) { _oss << ", \"" << _prop << "\""; }
        _oss << " })" << std::flush;
    }
    return _oss.str();
}

auto zpt::gen::rest::unit::remove_hidden_fields(zpt::json _def) -> std::string {
    std::ostringstream _oss;
    if (_def("*")("requestBody")("allOf")->ok()) {
        _oss << "_fields -= zpt::json{ zpt::array";
        for (auto const& [_, __, _type] : _def("*")("requestBody")("allOf")) {
            for (auto const& [___, ____, _prop] : _type("hidden")) { _oss << ", " << _prop; }
        }
        _oss << " }" << std::flush;
    }
    return _oss.str();
}

auto zpt::gen::rest::unit::has_id(zpt::json _def) -> bool {
    if (_def("*")("requestBody")("allOf")->ok()) {
        for (auto const& [_, __, _object] : _def("*")("requestBody")("allOf")) {
            if (_object("properties")("_id")->ok()) { return true; }
        }
    }
    if (_def("allOf")->ok()) {
        for (auto const& [_, __, _object] : _def("allOf")) {
            if (_object("properties")("_id")->ok()) { return true; }
        }
    }
    return false;
}

auto zpt::gen::rest::unit::generate_sql_schemata_mysql(zpt::json _def)
  -> zpt::ast::basic_file::ptr {
    auto _collection = _def("dbCollection")->string();
    auto _directory = std::filesystem::absolute(this->__base_path) / this->__module.name() / "sql";
    auto _file_path = _directory / std::format("{}_mysql.sql", _collection);
    if (std::filesystem::exists(_file_path)) { return nullptr; }

    std::cout << "> Generating " << _file_path << "." << std::endl;

    std::filesystem::create_directories(_directory);
    auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
    this->__module.add(_file);

    std::ostringstream _oss;
    if (this->__schema("info")("database")->string().find("_config") != 0) {
        _oss << "create schema if not exists " << this->__schema("info")("database")->string()
             << ";" << std::endl
             << "use " << this->__schema("info")("database")->string() << ";" << std::endl;
    }
    _oss << "drop table if exists " << _collection << ";" << std::endl
         << "create table " << _collection << " (\n";
    if (!this->has_id(_def)) { _oss << "_id varchar(22) not null,\n"; }

    for (auto const& [_, __, _object] : _def("allOf")) {
        for (auto const& [_, _name, _field] : _object("properties")) {
            if (!_field("sql:add_to_table")->ok()) { continue; }

            std::string _type = zpt::gen::rest::unit::__sql_types[_field("type")->string()];
            if (_field("sql:type")->ok()) { _type = _field("sql:type")->string(); }
            else if (_field("type")->string() == "string") {
                _type = (_field("maximum")->ok()
                           ? std::format("varchar({})", _field("maximum")->integer())
                           : "text");
            }
            else if (_field("type")->string() == "uuid") { _type = "varchar(22)"; }
            else if (_field("type")->string() == "object") { _type = "json"; }
            else if (_field("type")->string() == "array") { _type = "json"; }

            _oss << _name << " " << _type
                 << (_object("required")->contains(_name) ? " not null" : "") << "," << std::endl;
            if (_field("sql:index")->ok()) {
                auto _index_type = _field("sql:index")->string();
                if (_index_type == "foreign") {
                    _oss << "foreign key " << _name << "_" << _index_type << "_idx(" << _name
                         << ") references " << _field("sql:references")->string()
                         << " on delete cascade on update cascade," << std::endl;
                }
                else {
                    _oss << (_index_type == "unique" ? "unique " : "") << "key " << _name << "_"
                         << _index_type << "_idx(" << _name << ")," << std::endl;
                }
            }
        }
    }
    _oss << "primary key (_id)\n);" << std::endl;
    _oss << "show create table " << _collection;

    _file->add<zpt::ast::cpp_instruction>(_oss.str());
    return _file;
}
