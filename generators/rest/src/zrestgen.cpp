#include <zapata/json.h>
#include <zapata/generator/rest/rest.h>

auto main(int _argc, char* _argv[]) -> int {
    zpt::json _parameter_setup{ "--schema",
                                { "options",
                                  { zpt::array, "mandatory", "single" },
                                  "type",
                                  "string",
                                  "description",
                                  "schema definition file" },
                                "--backend-output-dir",
                                { "options",
                                  { zpt::array, "mandatory", "single" },
                                  "type",
                                  "string",
                                  "description",
                                  "root directory for the generated backend files" },
                                "--frontend-output-dir",
                                { "options",
                                  { zpt::array, "mandatory", "single" },
                                  "type",
                                  "string",
                                  "description",
                                  "root directory for the generated frontend files" },
                                "--with-cmake",
                                { "options",
                                  { zpt::array, "optional", "single" },
                                  "type",
                                  "bool",
                                  "description",
                                  "when passed, generates 'CMakeList.txt' files" } };

    zpt::json _parameters = zpt::parameters::parse(_argc, _argv, _parameter_setup);

    if (_parameters("--help")->ok() || _argc < 2) {
        std::cout << zpt::parameters::usage(_parameter_setup) << std::flush;
        return 0;
    }
    zpt::parameters::verify(_parameters, _parameter_setup);

    std::filesystem::path _output_backend =
      std::filesystem::absolute(_parameters("--backend-output-dir")->string());
    std::filesystem::path _output_frontend =
      std::filesystem::absolute(_parameters("--frontend-output-dir")->string());
    std::filesystem::path _context = std::filesystem::absolute(_parameters("--schema")->string());
    _context.remove_filename();

    zpt::json _schema;
    std::ifstream _ifs{ _parameters("--schema")->string() };
    expect(_ifs.is_open(), "Couldn't open file at '" << _parameters("--schema")->string() << "'");
    _ifs >> _schema;
    zpt::conf::evaluate_ref(_schema, _schema, "", _context, _schema);

    zpt::gen::rest::module _module{
        _schema("module")->string(), _output_backend, _output_frontend, _schema
    };
    _module //
      .generate_operations()
      .generate_sql()
      .generate_plugin()
      .generate_ui();

    if (_parameters("--with-cmake")->ok()) { _module.generate_cmake(); }

    _module.dump();
    return 0;
}
