#include <zapata/json.h>
#include <zapata/generator/rest/rest.h>

auto main(int _argc, char* _argv[]) -> int {
    zpt::json _parameter_setup{ "--schema",
                                { "options",
                                  { zpt::array, "mandatory", "single" },
                                  "type",
                                  "path",
                                  "description",
                                  "schema definition file" },
                                "--output-dir",
                                { "options",
                                  { zpt::array, "mandatory", "single" },
                                  "type",
                                  "path",
                                  "description",
                                  "root directory for the generated files" },
                                "--help",
                                { "options",
                                  { zpt::array, "optional", "single" },
                                  "type",
                                  "void",
                                  "description",
                                  "show this help" } };

    // try {
        zpt::json _parameters = zpt::parameters::parse(_argc, _argv, _parameter_setup);

        if (_parameters("--help")->ok() || _argc < 2) {
            std::cout << zpt::parameters::usage(_parameter_setup) << std::flush;
            return 0;
        }

        std::filesystem::path _output =
          std::filesystem::absolute(_parameters("--output-dir")->string());
        std::filesystem::path _context =
          std::filesystem::absolute(_parameters("--schema")->string());
        _context.remove_filename();

        zpt::json _schema;
        std::ifstream _ifs{ _parameters("--schema")->string() };
        expect(_ifs.is_open(),
               "Couldn't open file at '" << _parameters("--schema")->string() << "'");
        _ifs >> _schema;
        zpt::conf::evaluate_ref(_schema, _schema, "", _context, _schema);

        zpt::gen::rest::module _module{ _schema("module")->string(), _output, _schema };
        _module //
          .generate_operations()
          .generate_sql()
          .generate_plugin()
          .dump();
    // }
    // catch (zpt::failed_expectation const& _e) {
    //     std::cout << "Error: " << _e.what() << std::endl
    //               << zpt::parameters::usage(_parameter_setup) << std::flush;
    //     return -1;
    // }
    return 0;
}
