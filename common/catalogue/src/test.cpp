#include <zapata/catalogue.h>

auto
main(int argc, char* argv[]) -> int {
    zpt::catalogue<std::string, zpt::json> _catalogue{ "/home/pf/Void/sqlite" };
    _catalogue.clear();

    _catalogue.add("/users", { "host", "localhost", "port", 8080 });
    _catalogue.add("/users/{}", { "host", "localhost", "port", 8080 });

    for (auto _record : _catalogue.search("{.like(/users%).}")) { zlog(_record, zpt::info); }
    return 0;
}
