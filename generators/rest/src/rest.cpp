#include <zapata/generator/rest/rest.h>
#include <zapata/uri.h>
#include <set>

zpt::gen::rest::module::module(std::string const& _module_name,
                               std::filesystem::path const& _base_path_backend,
                               std::filesystem::path const& _base_path_frontend,
                               zpt::json _schema)
  : __base_path{ _base_path_backend }
  , __ui_path{ _base_path_frontend }
  , __module{ _module_name }
  , __schema{ _schema } {
    this->__namespace = (this->__schema("info")("namespace")->ok()
                           ? this->__schema("info")("namespace")->string() + std::string{ "::" }
                           : std::string{ "" }) +
                        this->__module.name();
}

auto zpt::gen::rest::module::generate_operations() -> module& {
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

auto zpt::gen::rest::module::generate_plugin() -> module& {
    auto _directory = std::filesystem::absolute(this->__base_path) / this->__module.name() / "src";
    auto _file_path = _directory / "plugin.cpp";
    if (std::filesystem::exists(_file_path)) { return (*this); }

    std::cout << "> Generating " << _file_path << "." << std::endl;

    std::filesystem::create_directories(_directory);
    auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
    this->__module.add(_file);

    _file->add<zpt::ast::cpp_instruction>("#include <iostream>\n#include <zapata/rest.h>");

    for (auto const& _include : this->__header_files) {
        _file->add<zpt::ast::cpp_instruction>(zpt::format("#include <{}>", _include));
    }

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
    _unload_block //
      ->add<zpt::ast::cpp_instruction>(
        zpt::format("zlog(\"Unloading module '{}'\", zpt::info)", this->__module.name()));
    _unload->add(_unload_block);
    _file->add(_unload);

    _load_block //
      ->add<zpt::ast::cpp_instruction>(zpt::format(
        "zlog(\"Registering listeners for module '{}'\", zpt::info)", this->__module.name()))
      .add<zpt::ast::cpp_instruction>(
        "auto _config = zpt::global_cast<zpt::json>(zpt::GLOBAL_CONFIG())")
      .add<zpt::ast::cpp_instruction>(
        "auto& _resolver = zpt::global_cast<zpt::rest::resolver>(zpt::REST_RESOLVER())")
      .add<zpt::ast::cpp_instruction>("auto _prefix = _config(\"rest\")(\"prefix\")->ok() ? "
                                      "_config(\"rest\")(\"prefix\")->string() : \"\"");

    for (auto const& [_, _path, _path_def] : this->__schema("paths")) {
        std::string _method;
        if (_path_def("resource")->string() == "collection") { _method = "*"; }
        else if (_path_def("resource")->string() == "document") { _method = "*"; }
        else if (_path_def("resource")->string() == "controller") { _method = "post"; }
        else if (_path_def("resource")->string() == "store") { _method = "*"; }

        auto _operation =
          zpt::format("{}::{}", this->__namespace, _path_def(_method)("operationId")->string());
        auto _split = zpt::uri::parse(_path);
        std::string _ref;
        for (auto const& [_idx, _, _part] : _split("path")) {
            if (_part->string().find("{") == 0) { _ref += std::string{ "/{}" }; }
            else { _ref += zpt::format("/{}", _part->string()); }
        }
        if (_path_def("resource")->string() == "controller") {
            _load_block->add<zpt::ast::cpp_instruction>(
              zpt::format("_resolver->add<{}>(zpt::Post, zpt::format(\"{{}}{}\", _prefix))",
                          _operation,
                          "",
                          _ref));
        }
        else {
            _load_block->add<zpt::ast::cpp_instruction>(zpt::format(
              "_resolver->add<{}>(zpt::format(\"{{}}{}\", _prefix))", _operation, "", _ref));
        }
    }
    return (*this);
}

auto zpt::gen::rest::module::generate_sql() -> module& {
    for (auto const& [_, __, _schema] : this->__schema("components")("schemas")) {
        this->generate_sql_schemata_mysql(_schema);
    }
    return (*this);
}

auto zpt::gen::rest::module::generate_cmake() -> module& {
    auto _base_path = std::filesystem::absolute(this->__base_path) / this->__module.name();
    auto _file_path = _base_path / "CMakeLists.txt";

    if (!std::filesystem::exists(_file_path)) {
        std::filesystem::create_directories(_base_path);
        auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
        this->__module.add(_file);
        std::cout << "> Generating " << _file_path << "." << std::endl;

        auto _lib = zpt::format("{}-{}",
                                this->__schema("info")("namespace")->ok()
                                  ? this->__schema("info")("namespace")->string()
                                  : "",
                                zpt::r_replace(this->__module.name(), "_", "-"));
        _file->add<zpt::ast::cmake_instruction>(zpt::format("add_library({} SHARED)", _lib));
        std::ostringstream _oss;
        _oss << zpt::format("target_sources({}\n"
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
            if (_file->path().string().find(".h") != std::string::npos) {
                _oss << "    ${CMAKE_CURRENT_SOURCE_DIR}"
                     << _file->path().string().replace(0, _base_path.string().length(), "") << "\n";
            }
        });
        _oss << ")";
        _file->add<zpt::ast::cmake_instruction>(_oss.str());
        _file->add<zpt::ast::cmake_instruction>(
          zpt::format("target_include_directories({}\n"
                      "  PRIVATE\n"
                      "    ${CMAKE_CURRENT_SOURCE_DIR}/include\n"
                      "  INTERFACE\n"
                      "    ${CMAKE_CURRENT_SOURCE_DIR}/include\n"
                      ")",
                      _lib));
        _file->add<zpt::ast::cmake_instruction>(zpt::format("target_link_libraries({}\n"
                                                            "  PRIVATE\n"
                                                            "    zapata-storage-mysqlx\n"
                                                            "    zapata-engine-transport\n"
                                                            "    zapata-engine-rest\n"
                                                            "    mysqlcppconn8\n"
                                                            ")",
                                                            _lib));
        _file->add<zpt::ast::cmake_instruction>(
          zpt::format("set_target_properties({}\n"
                      "  PROPERTIES\n"
                      "    VERSION ${PROJECT_VERSION}\n"
                      "    SOVERSION ${PROJECT_VERSION_MAJOR}\n"
                      "    COMPILE_FLAGS -fPIC\n"
                      "    LINK_FLAGS -shared\n"
                      "    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/lib{}.so\n"
                      ")",
                      _lib,
                      _lib));
        _file->add<zpt::ast::cmake_instruction>("include(GNUInstallDirs)");
        _file->add<zpt::ast::cmake_instruction>(
          zpt::format("install(TARGETS {}\n"
                      "  LIBRARY\n"
                      "    DESTINATION ${CMAKE_INSTALL_LIBDIR}\n"
                      ")",
                      _lib));
        _file->add<zpt::ast::cmake_instruction>(
          zpt::format("install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/{}\n"
                      "  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}\n"
                      "  FILES_MATCHING PATTERN \"*.h\"\n"
                      ")",
                      this->__schema("info")("namespace")->string()));
    }
    else {
        std::cout << "> Skipping generation of " << _file_path
                  << ", file already exists, move it out of the way first." << std::endl;
    }
    return (*this);
}

auto zpt::gen::rest::module::generate_ui() -> module& {
    for (auto const& [_, _path, _path_def] : this->__schema("paths")) {
        if (_path_def("resource")->string() == "collection") {
            this->generate_collection_ui(_path_def, zpt::uri::parse(_path));
        }
        else if (_path_def("resource")->string() == "document") {
            this->generate_document_ui(_path_def, zpt::uri::parse(_path));
        }
        else if (_path_def("resource")->string() == "store") {
            this->generate_collection_ui(_path_def, zpt::uri::parse(_path));
        }
    }
    return (*this);
}

auto zpt::gen::rest::module::dump() -> module& {
    this->__module.dump();
    return (*this);
}

auto zpt::gen::rest::module::generate_operation_h_file(zpt::json _def, std::string const& _method)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _directory = std::filesystem::absolute(this->__base_path) / this->__module.name() /
                      "include" / this->__schema("info")("namespace")->string() /
                      this->__module.name();
    auto _file_path = _directory / zpt::format("{}.h", _def(_method)("operationId")->string());
    this->__header_files.push_back(zpt::format("{}/{}/{}.h",
                                               this->__schema("info")("namespace")->string(),
                                               this->__module.name(),
                                               _def(_method)("operationId")->string()));
    if (!std::filesystem::exists(_file_path)) {
        std::filesystem::create_directories(_directory);
        auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
        this->__module.add(_file);
        std::cout << "> Generating " << _file_path << "." << std::endl;
        return _file;
    }
    else {
        std::cout << "> Skipping generation of " << _file_path
                  << ", file already exists, move it out of the way first." << std::endl;
    }
    return nullptr;
}

auto zpt::gen::rest::module::generate_operation_cpp_file(zpt::json _def, std::string const& _method)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _directory = std::filesystem::absolute(this->__base_path) / this->__module.name() / "src";
    auto _file_path = _directory / zpt::format("{}.cpp", _def(_method)("operationId")->string());
    if (!std::filesystem::exists(_file_path)) {
        std::filesystem::create_directories(_directory);
        auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
        this->__module.add(_file);
        std::cout << "> Generating " << _file_path << "." << std::endl;
        return _file;
    }
    else {
        std::cout << "> Skipping generation of " << _file_path
                  << ", file already exists, move it out of the way first." << std::endl;
    }
    return nullptr;
}

auto zpt::gen::rest::module::generate_collection(zpt::json _def, zpt::json _path)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _h_file = this->generate_operation_h_file(_def, "*");
    if (_h_file != nullptr) {
        _h_file->add<zpt::ast::cpp_instruction>("#include <iostream>\n#include <zapata/rest.h>");
        auto _namespace = zpt::make_code_block<zpt::ast::cpp_code_block>(
          zpt::format("namespace {}", this->__namespace));
        _h_file->add(_namespace);

        auto _class = zpt::make_class<zpt::ast::cpp_class>(_def("*")("operationId")->string(),
                                                           "public zpt::events::process");
        auto _h_constructor =
          zpt::make_function<zpt::ast::cpp_function>(_def("*")("operationId")->string(), "");
        _h_constructor->add<zpt::ast::cpp_variable>("_received", "zpt::message");
        _class->add(_h_constructor, zpt::ast::PUBLIC);

        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC,
                                        zpt::format("~{}", _def("*")("operationId")->string()),
                                        "",
                                        zpt::ast::DEFAULT)
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "blocked", "bool", zpt::ast::CONST)
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "add_element", "zpt::events::state")
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "list_elements", "zpt::events::state")
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "remove_elements", "zpt::events::state");
        _namespace->add(_class);

        auto _h_operator =
          zpt::make_function<zpt::ast::cpp_function>("operator()", "zpt::events::state");
        _h_operator->add<zpt::ast::cpp_variable>("_dispatcher", "zpt::events::dispatcher&");
        _class->add(_h_operator, zpt::ast::PUBLIC);
    }

    auto _cpp_file = this->generate_operation_cpp_file(_def, "*");
    if (_cpp_file != nullptr) {
        auto _include_path = zpt::format("{}/{}/{}.h",
                                         this->__schema("info")("namespace")->string(),
                                         this->__module.name(),
                                         _def("*")("operationId")->string());
        auto _class_method_prefix =
          zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

        _cpp_file->add<zpt::ast::cpp_instruction>(zpt::format(
          "#include <{}>\n#include <zapata/connector.h>\n#include <zapata/{}/connector.h>",
          _include_path,
          this->__schema("info")("dbDriver")->string()));

        auto _cpp_constructor = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}{}", _class_method_prefix, _def("*")("operationId")->string()), "");
        _cpp_constructor->add<zpt::ast::cpp_variable>("_received", "zpt::message");
        auto _cpp_constructor_body =
          zpt::make_code_block<zpt::ast::cpp_code_block>(": zpt::events::process{ _received }");
        _cpp_constructor_body->add<zpt::ast::cpp_instruction>("");
        _cpp_constructor->add(_cpp_constructor_body);
        _cpp_file->add(_cpp_constructor);

        auto _cpp_blocked = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}blocked", _class_method_prefix), "bool", zpt::ast::CONST);
        auto _cpp_blocked_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        _cpp_blocked_body->add<zpt::ast::cpp_instruction>("return false");
        _cpp_blocked->add(_cpp_blocked_body);
        _cpp_file->add(_cpp_blocked);

        this->generate_add_element(_cpp_file, _def, _path);
        this->generate_list_elements(_cpp_file, _def, _path);
        this->generate_remove_elements(_cpp_file, _def, _path);

        auto _cpp_operator = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}operator()", _class_method_prefix), "zpt::events::state");
        _cpp_operator->add<zpt::ast::cpp_variable>("_dispatcher [[maybe_unused]]",
                                                   "zpt::events::dispatcher&");
        auto _cpp_operator_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        auto _cpp_operator_switch = zpt::make_code_block<zpt::ast::cpp_code_block>(
          "switch(this->received()->performative())");
        auto _cpp_operator_case_get =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
        _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->list_elements()");
        auto _cpp_operator_case_post =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Post :");
        _cpp_operator_case_post->add<zpt::ast::cpp_instruction>("return this->add_element()");
        auto _cpp_operator_case_delete =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
        _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>("return this->remove_elements()");
        _cpp_operator_switch //
          ->add(_cpp_operator_case_post)
          .add(_cpp_operator_case_get)
          .add(_cpp_operator_case_delete);
        _cpp_operator_body //
          ->add(_cpp_operator_switch)
          .add<zpt::ast::cpp_instruction>("this->to_send()->status(405)")
          .add<zpt::ast::cpp_instruction>(
            "this->to_send()->body() = { \"message\", \"Only GET, POST, "
            "DELETE allowed to use with a collection\" }")
          .add<zpt::ast::cpp_instruction>("return zpt::events::abort");
        _cpp_operator->add(_cpp_operator_body);
        _cpp_file->add(_cpp_operator);
    }
    return _h_file;
}

auto zpt::gen::rest::module::generate_document(zpt::json _def, zpt::json _path)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _h_file = this->generate_operation_h_file(_def, "*");
    if (_h_file != nullptr) {
        _h_file->add<zpt::ast::cpp_instruction>("#include <iostream>\n#include <zapata/rest.h>");
        auto _namespace = zpt::make_code_block<zpt::ast::cpp_code_block>(
          zpt::format("namespace {}", this->__namespace));
        _h_file->add(_namespace);

        auto _class = zpt::make_class<zpt::ast::cpp_class>(_def("*")("operationId")->string(),
                                                           "public zpt::events::process");
        auto _h_constructor =
          zpt::make_function<zpt::ast::cpp_function>(_def("*")("operationId")->string(), "");
        _h_constructor->add<zpt::ast::cpp_variable>("_received", "zpt::message");
        _class->add(_h_constructor, zpt::ast::PUBLIC);

        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC,
                                        zpt::format("~{}", _def("*")("operationId")->string()),
                                        "",
                                        zpt::ast::DEFAULT)
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "blocked", "bool", zpt::ast::CONST)
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "update_element", "zpt::events::state")
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "get_element", "zpt::events::state")
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "remove_element", "zpt::events::state");
        _namespace->add(_class);

        auto _h_operator =
          zpt::make_function<zpt::ast::cpp_function>("operator()", "zpt::events::state");
        _h_operator->add<zpt::ast::cpp_variable>("_dispatcher", "zpt::events::dispatcher&");
        _class->add(_h_operator, zpt::ast::PUBLIC);
    }

    auto _cpp_file = this->generate_operation_cpp_file(_def, "*");
    if (_cpp_file != nullptr) {
        auto _include_path = zpt::format("{}/{}/{}.h",
                                         this->__schema("info")("namespace")->string(),
                                         this->__module.name(),
                                         _def("*")("operationId")->string());
        auto _class_method_prefix =
          zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

        _cpp_file->add<zpt::ast::cpp_instruction>(zpt::format(
          "#include <{}>\n#include <zapata/connector.h>\n#include <zapata/{}/connector.h>",
          _include_path,
          this->__schema("info")("dbDriver")->string()));

        auto _cpp_constructor = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}{}", _class_method_prefix, _def("*")("operationId")->string()), "");
        _cpp_constructor->add<zpt::ast::cpp_variable>("_received", "zpt::message");
        auto _cpp_constructor_body =
          zpt::make_code_block<zpt::ast::cpp_code_block>(": zpt::events::process{ _received }");
        _cpp_constructor_body->add<zpt::ast::cpp_instruction>("");
        _cpp_constructor->add(_cpp_constructor_body);
        _cpp_file->add(_cpp_constructor);

        auto _cpp_blocked = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}blocked", _class_method_prefix), "bool", zpt::ast::CONST);
        auto _cpp_blocked_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        _cpp_blocked_body->add<zpt::ast::cpp_instruction>("return false");
        _cpp_blocked->add(_cpp_blocked_body);
        _cpp_file->add(_cpp_blocked);

        this->generate_update_element(_cpp_file, _def, _path);
        this->generate_get_element(_cpp_file, _def, _path);
        this->generate_remove_element(_cpp_file, _def, _path);

        auto _cpp_operator = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}operator()", _class_method_prefix), "zpt::events::state");
        _cpp_operator->add<zpt::ast::cpp_variable>("_dispatcher [[maybe_unused]]",
                                                   "zpt::events::dispatcher&");
        auto _cpp_operator_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        auto _cpp_operator_switch = zpt::make_code_block<zpt::ast::cpp_code_block>(
          "switch(this->received()->performative())");
        auto _cpp_operator_case_get =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
        _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->get_element()");
        auto _cpp_operator_case_put =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Patch :");
        _cpp_operator_case_put->add<zpt::ast::cpp_instruction>("return this->update_element()");
        auto _cpp_operator_case_delete =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
        _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>("return this->remove_element()");
        _cpp_operator_switch //
          ->add(_cpp_operator_case_put)
          .add(_cpp_operator_case_get)
          .add(_cpp_operator_case_delete);
        _cpp_operator_body //
          ->add(_cpp_operator_switch)
          .add<zpt::ast::cpp_instruction>("this->to_send()->status(405)")
          .add<zpt::ast::cpp_instruction>(
            "this->to_send()->body() = { \"message\", \"Only GET, PATCH, "
            "DELETE allowed to use with a document\" }")
          .add<zpt::ast::cpp_instruction>("return zpt::events::abort");
        _cpp_operator->add(_cpp_operator_body);
        _cpp_file->add(_cpp_operator);
    }
    return _h_file;
}

auto zpt::gen::rest::module::generate_controller(zpt::json _def, zpt::json _path)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _h_file = this->generate_operation_h_file(_def, "post");
    if (_h_file != nullptr) {
        _h_file->add<zpt::ast::cpp_instruction>("#include <iostream>\n#include <zapata/rest.h>");
        auto _namespace = zpt::make_code_block<zpt::ast::cpp_code_block>(
          zpt::format("namespace {}", this->__namespace));
        _h_file->add(_namespace);

        auto _class = zpt::make_class<zpt::ast::cpp_class>(_def("post")("operationId")->string(),
                                                           "public zpt::events::process");
        auto _h_constructor =
          zpt::make_function<zpt::ast::cpp_function>(_def("post")("operationId")->string(), "");
        _h_constructor->add<zpt::ast::cpp_variable>("_received", "zpt::message");
        _class->add(_h_constructor, zpt::ast::PUBLIC);

        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC,
                                        zpt::format("~{}", _def("post")("operationId")->string()),
                                        "",
                                        zpt::ast::DEFAULT)
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "blocked", "bool", zpt::ast::CONST)
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "process_request", "zpt::events::state");
        _namespace->add(_class);

        auto _h_operator =
          zpt::make_function<zpt::ast::cpp_function>("operator()", "zpt::events::state");
        _h_operator->add<zpt::ast::cpp_variable>("_dispatcher", "zpt::events::dispatcher&");
        _class->add(_h_operator, zpt::ast::PUBLIC);
    }

    auto _cpp_file = this->generate_operation_cpp_file(_def, "post");
    if (_cpp_file != nullptr) {
        auto _include_path = zpt::format("{}/{}/{}.h",
                                         this->__schema("info")("namespace")->string(),
                                         this->__module.name(),
                                         _def("post")("operationId")->string());
        auto _class_method_prefix =
          zpt::format("{}::{}::", this->__namespace, _def("post")("operationId")->string());

        _cpp_file->add<zpt::ast::cpp_instruction>(zpt::format(
          "#include <{}>\n#include <zapata/connector.h>\n#include <zapata/{}/connector.h>",
          _include_path,
          this->__schema("info")("dbDriver")->string()));

        auto _cpp_constructor = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}{}", _class_method_prefix, _def("post")("operationId")->string()), "");
        _cpp_constructor->add<zpt::ast::cpp_variable>("_received", "zpt::message");
        auto _cpp_constructor_body =
          zpt::make_code_block<zpt::ast::cpp_code_block>(": zpt::events::process{ _received }");
        _cpp_constructor_body->add<zpt::ast::cpp_instruction>("");
        _cpp_constructor->add(_cpp_constructor_body);
        _cpp_file->add(_cpp_constructor);

        auto _cpp_blocked = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}blocked", _class_method_prefix), "bool", zpt::ast::CONST);
        auto _cpp_blocked_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        _cpp_blocked_body->add<zpt::ast::cpp_instruction>("return false");
        _cpp_blocked->add(_cpp_blocked_body);
        _cpp_file->add(_cpp_blocked);

        this->generate_process_request(_cpp_file, _def, _path);

        auto _cpp_operator = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}operator()", _class_method_prefix), "zpt::events::state");
        _cpp_operator->add<zpt::ast::cpp_variable>("_dispatcher [[maybe_unused]]",
                                                   "zpt::events::dispatcher&");
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
          .add<zpt::ast::cpp_instruction>("this->to_send()->status(405)")
          .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"message\", \"Only POST "
                                          "allowed to use with a controller\" }")
          .add<zpt::ast::cpp_instruction>("return zpt::events::abort");
        _cpp_operator->add(_cpp_operator_body);
        _cpp_file->add(_cpp_operator);
    }
    return _h_file;
}

auto zpt::gen::rest::module::generate_store(zpt::json _def, zpt::json _path)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _h_file = this->generate_operation_h_file(_def, "*");
    if (_h_file != nullptr) {
        _h_file->add<zpt::ast::cpp_instruction>("#include <iostream>\n#include <zapata/rest.h>");
        auto _namespace = zpt::make_code_block<zpt::ast::cpp_code_block>(
          zpt::format("namespace {}", this->__namespace));
        _h_file->add(_namespace);

        auto _class = zpt::make_class<zpt::ast::cpp_class>(_def("*")("operationId")->string(),
                                                           "public zpt::events::process");
        auto _h_constructor =
          zpt::make_function<zpt::ast::cpp_function>(_def("*")("operationId")->string(), "");
        _h_constructor->add<zpt::ast::cpp_variable>("_received", "zpt::message");
        _class->add(_h_constructor, zpt::ast::PUBLIC);

        _class //
          ->add<zpt::ast::cpp_function>(zpt::ast::PUBLIC,
                                        zpt::format("~{}", _def("*")("operationId")->string()),
                                        "",
                                        zpt::ast::DEFAULT)
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "blocked", "bool", zpt::ast::CONST)
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "add_element", "zpt::events::state")
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "list_elements", "zpt::events::state")
          .add<zpt::ast::cpp_function>(zpt::ast::PUBLIC, "remove_elements", "zpt::events::state");
        _namespace->add(_class);

        auto _h_operator =
          zpt::make_function<zpt::ast::cpp_function>("operator()", "zpt::events::state");
        _h_operator->add<zpt::ast::cpp_variable>("_dispatcher", "zpt::events::dispatcher&");
        _class->add(_h_operator, zpt::ast::PUBLIC);
    }

    auto _cpp_file = this->generate_operation_cpp_file(_def, "*");
    if (_cpp_file != nullptr) {
        auto _include_path = zpt::format("{}/{}/{}.h",
                                         this->__schema("info")("namespace")->string(),
                                         this->__module.name(),
                                         _def("*")("operationId")->string());
        auto _class_method_prefix =
          zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

        _cpp_file->add<zpt::ast::cpp_instruction>(zpt::format(
          "#include <{}>\n#include <zapata/connector.h>\n#include <zapata/{}/connector.h>",
          _include_path,
          this->__schema("info")("dbDriver")->string()));

        auto _cpp_constructor = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}{}", _class_method_prefix, _def("*")("operationId")->string()), "");
        _cpp_constructor->add<zpt::ast::cpp_variable>("_received", "zpt::message");
        auto _cpp_constructor_body =
          zpt::make_code_block<zpt::ast::cpp_code_block>(": zpt::events::process{ _received }");
        _cpp_constructor_body->add<zpt::ast::cpp_instruction>("");
        _cpp_constructor->add(_cpp_constructor_body);
        _cpp_file->add(_cpp_constructor);

        auto _cpp_blocked = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}blocked", _class_method_prefix), "bool", zpt::ast::CONST);
        auto _cpp_blocked_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        _cpp_blocked_body->add<zpt::ast::cpp_instruction>("return false");
        _cpp_blocked->add(_cpp_blocked_body);
        _cpp_file->add(_cpp_blocked);

        this->generate_add_element(_cpp_file, _def, _path);
        this->generate_list_elements(_cpp_file, _def, _path);
        this->generate_remove_elements(_cpp_file, _def, _path);

        auto _cpp_operator = zpt::make_function<zpt::ast::cpp_function>(
          zpt::format("{}operator()", _class_method_prefix), "zpt::events::state");
        _cpp_operator->add<zpt::ast::cpp_variable>("_dispatcher [[maybe_unused]]",
                                                   "zpt::events::dispatcher&");
        auto _cpp_operator_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
        auto _cpp_operator_switch = zpt::make_code_block<zpt::ast::cpp_code_block>(
          "switch(this->received()->performative())");
        auto _cpp_operator_case_get =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Get :");
        _cpp_operator_case_get->add<zpt::ast::cpp_instruction>("return this->list_elements()");
        auto _cpp_operator_case_post =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Put :");
        _cpp_operator_case_post->add<zpt::ast::cpp_instruction>("return this->add_element()");
        auto _cpp_operator_case_delete =
          zpt::make_code_block<zpt::ast::cpp_code_block>("case zpt::Delete :");
        _cpp_operator_case_delete->add<zpt::ast::cpp_instruction>("return this->remove_elements()");
        _cpp_operator_switch //
          ->add(_cpp_operator_case_post)
          .add(_cpp_operator_case_get)
          .add(_cpp_operator_case_delete);
        _cpp_operator_body //
          ->add(_cpp_operator_switch)
          .add<zpt::ast::cpp_instruction>("this->to_send()->status(405)")
          .add<zpt::ast::cpp_instruction>(
            "this->to_send()->body() = { \"message\", \"Only GET, POST, "
            "DELETE allowed to use with a collection\" }")
          .add<zpt::ast::cpp_instruction>("return zpt::events::abort");
        _cpp_operator->add(_cpp_operator_body);
        _cpp_file->add(_cpp_operator);
    }
    return _h_file;
}

auto zpt::gen::rest::module::generate_add_element(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                                                  zpt::json _def,
                                                  zpt::json _path) -> void {
    auto _class_method_prefix =
      zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      zpt::format("{}add_element", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>("auto _received = this->received()->body()");
    this->add_generated(_method_body, _def, "create");
    this->add_parameters_and_validation(_method_body, _def, _path);
    this->add_schema_validation(_method_body, _def);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _method_try_body //
      ->add<zpt::ast::cpp_instruction>(
        "auto _id = _collection->add(_received)->execute()->generated_id()")
      .add<zpt::ast::cpp_instruction>("this->to_send()->status(201)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"element_id\", _id }");
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(500)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::module::generate_list_elements(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                                                    zpt::json _def,
                                                    zpt::json _path) -> void {
    auto _class_method_prefix =
      zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      zpt::format("{}list_elements", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>("auto _params = this->received()->parameters()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _method_try_body //
      ->add<zpt::ast::cpp_instruction>(
        "auto _find = zpt::storage::filter_find(_collection, _params)")
      .add<zpt::ast::cpp_instruction>(
        zpt::format("zpt::json _fields = {}", this->get_visible_fields(_def)))
      .add<zpt::ast::cpp_instruction>("_fields << \"_id\"")
      .add<zpt::ast::cpp_instruction>("auto _result = _find->fields(_fields)->execute()->fetch()");
    auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result->ok())");
    _if_block //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(200)")
      .add<zpt::ast::cpp_instruction>(
        "this->to_send()->body() = { \"items\", _result, \"size\", _result->size() }")
      .add<zpt::ast::cpp_instruction>("zpt::storage::reply_find(this->to_send()->body(), _params)");
    _method_try_body->add(_if_block);
    auto _else_block = zpt::make_code_block<zpt::ast::cpp_code_block>("else");
    _else_block->add<zpt::ast::cpp_instruction>("this->to_send()->status(204)");
    _method_try_body->add(_else_block);
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(500)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::module::generate_remove_elements(
  std::shared_ptr<zpt::ast::basic_file> _cpp_file,
  zpt::json _def,
  zpt::json _path) -> void {
    auto _class_method_prefix =
      zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      zpt::format("{}remove_elements", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>("auto _params = this->received()->parameters()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _method_try_body //
      ->add<zpt::ast::cpp_instruction>(
        "auto _result = zpt::storage::filter_remove(_collection, _params)->execute()->count()")
      .add<zpt::ast::cpp_instruction>("this->to_send()->status(202)")
      .add<zpt::ast::cpp_instruction>(
        "this->to_send()->body() = { \"removed_for\", _params, \"removed_count\", _result }");
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(500)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::module::generate_update_element(
  std::shared_ptr<zpt::ast::basic_file> _cpp_file,
  zpt::json _def,
  zpt::json _path) -> void {
    auto _class_method_prefix =
      zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      zpt::format("{}update_element", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>("auto _received = this->received()->body()");
    this->add_parameters_and_validation(_method_body, _def, _path);
    this->add_generated(_method_body, _def, "update");

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _method_try_body //
      ->add<zpt::ast::cpp_instruction>(
        std::string{ "auto _result = _collection->modify(\"_id = :id\")->bind({ \"id\", _id "
                     "})->patch(_received)->execute()->count()" });
    auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result != 0)");
    _if_block //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(202)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"updated_count\", _result }");
    _method_try_body->add(_if_block);
    auto _else_block = zpt::make_code_block<zpt::ast::cpp_code_block>("else");
    _else_block->add<zpt::ast::cpp_instruction>("this->to_send()->status(404)");
    _method_try_body->add(_else_block);
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(500)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::module::generate_get_element(std::shared_ptr<zpt::ast::basic_file> _cpp_file,
                                                  zpt::json _def,
                                                  zpt::json _path) -> void {
    auto _class_method_prefix =
      zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      zpt::format("{}get_element", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    _method_body //
      ->add<zpt::ast::cpp_instruction>("auto _params = this->received()->parameters()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _method_try_body //
      ->add<zpt::ast::cpp_instruction>(
        zpt::format("zpt::json _fields = {}", this->get_visible_fields(_def)))
      .add<zpt::ast::cpp_instruction>("_fields << \"_id\"")
      .add<zpt::ast::cpp_instruction>("auto _result = _collection->find(\"_id = :id\")->bind({ "
                                      "\"id\", _id })->fields(_fields)->execute()->fetch(1)");
    auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result->ok())");
    _if_block //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(200)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = _result");
    _method_try_body->add(_if_block);
    auto _else_block = zpt::make_code_block<zpt::ast::cpp_code_block>("else");
    _else_block->add<zpt::ast::cpp_instruction>("this->to_send()->status(404)");
    _method_try_body->add(_else_block);
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(500)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::module::generate_remove_element(
  std::shared_ptr<zpt::ast::basic_file> _cpp_file,
  zpt::json _def,
  zpt::json _path) -> void {
    auto _class_method_prefix =
      zpt::format("{}::{}::", this->__namespace, _def("*")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      zpt::format("{}remove_element", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    this->add_db_configuration(_method_body, _def);
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _method_try_body //
      ->add<zpt::ast::cpp_instruction>(
        std::string{ "auto _result = _collection->remove(\"_id = :id\")->bind({ \"id\", _id "
                     "})->execute()->count()" });
    auto _if_block = zpt::make_code_block<zpt::ast::cpp_code_block>("if (_result != 0)");
    _if_block //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(202)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"removed_count\", _result }");
    _method_try_body->add(_if_block);
    auto _else_block = zpt::make_code_block<zpt::ast::cpp_code_block>("else");
    _else_block->add<zpt::ast::cpp_instruction>("this->to_send()->status(404)");
    _method_try_body->add(_else_block);
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(500)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::module::generate_process_request(
  std::shared_ptr<zpt::ast::basic_file> _cpp_file,
  zpt::json _def,
  zpt::json _path) -> void {
    auto _class_method_prefix =
      zpt::format("{}::{}::", this->__namespace, _def("post")("operationId")->string());

    auto _method = zpt::make_function<zpt::ast::cpp_function>(
      zpt::format("{}process_request", _class_method_prefix), "zpt::events::state");
    auto _method_body = zpt::make_code_block<zpt::ast::cpp_code_block>();
    _method_body->add<zpt::ast::cpp_instruction>("auto _received = this->received()->body()");
    this->add_parameters_and_validation(_method_body, _def, _path);

    auto _method_try_body = zpt::make_code_block<zpt::ast::cpp_code_block>("try");
    _method_try_body //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(200)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { }");
    _method_body->add(_method_try_body);

    auto _method_catch_body =
      zpt::make_code_block<zpt::ast::cpp_code_block>("catch(std::exception const& _e)");
    _method_catch_body //
      ->add<zpt::ast::cpp_instruction>("this->to_send()->status(500)")
      .add<zpt::ast::cpp_instruction>("this->to_send()->body() = { \"message\", _e.what() }");
    _method_body->add(_method_catch_body);

    _method_body->add<zpt::ast::cpp_instruction>("return zpt::events::finish");

    _method->add(_method_body);
    _cpp_file->add(_method);
}

auto zpt::gen::rest::module::add_db_configuration(
  std::shared_ptr<zpt::ast::basic_code_block> _block,
  zpt::json _def) -> void {
    _block //
      ->add<zpt::ast::cpp_instruction>(
        "auto _config = zpt::global_cast<zpt::json>(zpt::GLOBAL_CONFIG())")
      .add<zpt::ast::cpp_instruction>(zpt::format(
        "auto _session = zpt::make_connection<zpt::storage::{}::connection>(_config)->session()",
        this->__schema("info")("dbDriver")->string()))
      .add<zpt::ast::cpp_instruction>(
        zpt::format("auto _collection = _session->database(\"{}\")->collection(\"{}\")",
                    this->__schema("info")("database")->string(),
                    _def("*")("requestBody")("dbCollection")->string()));
}

auto zpt::gen::rest::module::add_parameters_and_validation(
  std::shared_ptr<zpt::ast::basic_code_block> _block,
  zpt::json _def,
  zpt::json _path) -> void {
    bool _has_params{ false };
    bool _has_path{ false };
    for (auto const& [_, __, _param] : _def("parameters")) {
        if (_param("in")->string() == "query") {
            if (!_has_params) {
                _block->add<zpt::ast::cpp_instruction>(
                  "auto _params = this->received()->parameters()");
                _has_params = true;
            }
        }
        else if (_param("in")->string() == "path") {
            if (!_has_path) {
                _block
                  ->add<zpt::ast::cpp_instruction>("auto _path = this->received()->uri()(\"path\")")
                  .add<zpt::ast::cpp_instruction>("size_t _prefix_len = "
                                                  "zpt::global_cast<zpt::json>(zpt::GLOBAL_CONFIG()"
                                                  ")(\"rest\")(\"prefix_path_len\")->integer()");
                _has_path = true;
            }
        }
    }

    for (auto const& [_, __, _param] : _def("parameters")) {
        if (_param("in")->string() == "query") {
            _block->add<zpt::ast::cpp_instruction>(zpt::format(
              "auto _{} = _params(\"{}\")", _param("name")->string(), _param("name")->string()));
        }
    }
    for (auto const& [_idx, _, _part] : _path("path")) {
        auto _variable = _part->string();
        if (_variable.find("{") == 0) {
            std::string _name = _variable.substr(1, _variable.length() - 2);
            _block->add<zpt::ast::cpp_instruction>(
              zpt::format("auto _{} = _path(_prefix_len + {})", _name, _idx));
        }
    }
    for (auto const& [_, __, _param] : _def("parameters")) {
        if (_param("in")->string() == "path" ||
            (_param("in")->string() == "query" && _param("required")->boolean())) {
            _block->add<zpt::ast::cpp_instruction>(
              zpt::format("expect(_{}->ok(), \"Required {} parameter '{}'\")",
                          _param("name")->string(),
                          _param("in")->string(),
                          _param("name")->string()));
        }
    }
}

auto zpt::gen::rest::module::add_schema_validation(
  std::shared_ptr<zpt::ast::basic_code_block> _block,
  zpt::json _def) -> void {
    for (auto const& [_, __, _object] : _def("*")("requestBody")("allOf")) {
        auto _required = _object("required");
        if (_required->is_array()) {
            for (auto const& [_, __, _name] : _required) {
                _block->add<zpt::ast::cpp_instruction>(zpt::format(
                  "expect(_received(\"{}\")->ok(), \"Required request member field '{}'\")",
                  _name->string(),
                  _name->string()));
            }
        }
    }
}

auto zpt::gen::rest::module::add_generated(std::shared_ptr<zpt::ast::basic_code_block> _block,
                                           zpt::json _def,
                                           std::string const& _generate) -> void {
    for (auto const& [_, __, _object] : _def("*")("requestBody")("allOf")) {
        for (auto const& [___, _name, _prop] : _object("properties")) {
            if (_prop("generation_expr")->ok() && (_prop("generate")->string() == _generate ||
                                                   _prop("generate")->string() == "always")) {
                _block->add<zpt::ast::cpp_instruction>(
                  zpt::format("_received[\"{}\"] = {}", _name, _prop("generation_expr")->string()));
            }
        }
    }
}

auto zpt::gen::rest::module::get_visible_fields(zpt::json _def) -> std::string {
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

auto zpt::gen::rest::module::generate_sql_schemata_mysql(zpt::json _def)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _collection = _def("dbCollection")->string();
    auto _directory = std::filesystem::absolute(this->__base_path) / this->__module.name() / "sql";
    auto _file_path = _directory / zpt::format("{}_mysql.sql", _collection);
    if (std::filesystem::exists(_file_path)) { return nullptr; }

    std::cout << "> Generating " << _file_path << "." << std::endl;

    std::filesystem::create_directories(_directory);
    auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
    this->__module.add(_file);

    std::ostringstream _oss;
    _oss << "\\c zpt:@localhost:3306" << std::endl
         << "\\sql" << std::endl
         << "create database if not exists " << this->__schema("info")("database")->string() << ";"
         << std::endl
         << "use " << this->__schema("info")("database")->string() << ";" << std::endl
         << "drop table if exists " << _collection << ";" << std::endl
         << "\\py" << std::endl
         << "session = mysqlx.get_session('zpt:@localhost', '')" << std::endl
         << "db = session.get_schema('" << this->__schema("info")("database")->string() << "')"
         << std::endl
         << "db.create_collection('" << _collection << "')" << std::endl
         << "\\sql" << std::endl
         << "\\c zpt:@localhost:3306" << std::endl
         << "use " << this->__schema("info")("database")->string() << ";" << std::endl;

    for (auto const& [_, __, _object] : _def("allOf")) {
        std::set<std::string> _required;
        for (auto const& [_, __, _name] : _object("required")) { _required.insert(_name); }

        for (auto const& [_, _name, _field] : _object("properties")) {
            _oss << "alter table " << _collection << " add column " << _name << " "
                 << (_field("index")->ok() && _field("type")->string() == "string"
                       ? zpt::format("varchar({})",
                                     (_field("maximum")->ok() ? _field("maximum")->integer() : 512))
                       : zpt::gen::rest::module::__sql_types[_field("type")->string()])
                 << " generated always as (doc->>\"$." << _name << "\") stored"
                 << (_required.find(_name) != _required.end() ? " not null" : "") << ";"
                 << std::endl;
            if (_field("index")->ok()) {
                _oss << "alter table " << _collection << " add " << _field("index")->string()
                     << " index " << _name << "_" << _field("index")->string() << "_idx(" << _name
                     << ");" << std::endl;
            }
        }
    }
    _oss << "show create table " << _collection;

    _file->add<zpt::ast::cpp_instruction>(_oss.str());
    return _file;
}

auto zpt::gen::rest::module::generate_operation_lang_file(zpt::json _def,
                                                          std::string const& _method)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _directory = std::filesystem::absolute(this->__ui_path) / "lang" / this->__module.name();
    auto _file_path = _directory / zpt::format("{}.js", _def(_method)("operationId")->string());
    if (!std::filesystem::exists(_file_path)) {
        std::filesystem::create_directories(_directory);
        auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
        this->__module.add(_file);
        std::cout << "> Generating " << _file_path << "." << std::endl;
        return _file;
    }
    else {
        std::cout << "> Skipping generation of " << _file_path
                  << ", file already exists, move it out of the way first." << std::endl;
    }
    return nullptr;
}

auto zpt::gen::rest::module::generate_operation_html_file(zpt::json _def,
                                                          std::string const& _method)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _directory = std::filesystem::absolute(this->__ui_path) / this->__module.name();
    auto _file_path = _directory / zpt::format("{}.html", _def(_method)("operationId")->string());
    if (!std::filesystem::exists(_file_path)) {
        std::filesystem::create_directories(_directory);
        auto _file = std::make_shared<zpt::ast::basic_file>(_file_path);
        this->__module.add(_file);
        std::cout << "> Generating " << _file_path << "." << std::endl;
        return _file;
    }
    else {
        std::cout << "> Skipping generation of " << _file_path
                  << ", file already exists, move it out of the way first." << std::endl;
    }
    return nullptr;
}

auto zpt::gen::rest::module::generate_collection_ui(zpt::json _def, zpt::json _path)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _lang_file = this->generate_operation_lang_file(_def, "*");
    if (_lang_file != nullptr) { _lang_file->add<zpt::ast::cpp_instruction>(""); }

    auto _html_file = this->generate_operation_html_file(_def, "*");
    if (_html_file != nullptr) {
        auto _html = zpt::gen::rest::collection_html_template;
        auto _collection_name = _def("*")("operationId")->string();
        zpt::replace(_html,
                     "{{collection-dictionary}}",
                     this->__module.name() + std::string{ "/" } +
                       zpt::format("{}", _collection_name));
        zpt::replace(_html, "{{collection-name}}", _collection_name);
        zpt::replace(_html, "{{collection-uri}}", _path("raw_path")->string());
        zpt::replace(_html, "{{list-fields}}", this->extract_list_fields(_def));
        zpt::replace(_html, "{{form-fields}}", this->extract_form_fields(_def));
        _html_file->add<zpt::ast::cpp_instruction>(_html);
    }

    return _html_file;
}

auto zpt::gen::rest::module::generate_document_ui(zpt::json _def, zpt::json _path)
  -> std::shared_ptr<zpt::ast::basic_file> {
    auto _lang_file = this->generate_operation_lang_file(_def, "*");
    if (_lang_file != nullptr) { _lang_file->add<zpt::ast::cpp_instruction>(""); }

    auto _html_file = this->generate_operation_html_file(_def, "*");
    if (_html_file != nullptr) {
        auto _html = zpt::gen::rest::document_html_template;
        auto _document_name = _def("*")("operationId")->string();
        zpt::replace(_html,
                     "{{document-dictionary}}",
                     this->__module.name() + std::string{ "/" } +
                       zpt::format("{}", _document_name));
        zpt::replace(_html, "{{document-name}}", _document_name);
        zpt::replace(_html, "{{document-uri}}", _path("raw_path")->string());
        auto _form_fields = this->extract_form_fields(_def);
        zpt::replace(_html, "{{form-fields}}", static_cast<std::string>(_form_fields));
        _html_file->add<zpt::ast::cpp_instruction>(_html);
    }

    return _html_file;
}

auto zpt::gen::rest::module::extract_list_fields(zpt::json _def) -> std::string {
    std::ostringstream _oss;
    bool _first{ true };
    _oss << "{" << std::endl;
    for (auto const& [_, __, _object] : _def("*")("requestBody")("allOf")) {
        for (auto const& [___, _name, _prop] : _object("properties")) {
            if (!_first) { _oss << "," << std::endl; }
            _first = false;
            _oss << "                         " << _name << ": dictionary." << _name << "[lang]";
        }
    }
    _oss << "}" << std::flush;
    return _oss.str();
}

auto zpt::gen::rest::module::extract_form_fields(zpt::json _def) -> std::string { return ""; }
