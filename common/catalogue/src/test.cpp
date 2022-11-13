#include <zapata/catalogue.h>

auto main(int argc, char* argv[]) -> int {
    zpt::catalogue<std::string, zpt::json> _catalogue;
    _catalogue.clear();

    _catalogue.add("/users", { "host", "localhost", "port", 8080 });
    _catalogue.add("/users/([^/]+)", { "host", "localhost", "port", 8080 });
    _catalogue.add("/users/([^/]+)/info", { "host", "localhost", "port", 8080 });

    for (auto& [_pattern, _record] : _catalogue.search("{.like(/users/%/info).}")) {
        zlog(_pattern << ": " << zpt::pretty{ _record }, zpt::info);
    }
    return 0;
}
