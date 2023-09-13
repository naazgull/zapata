#include <zapata/catalog.h>

auto zpt::CATALOG() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

auto zpt::catalog_id::split(std::string const& _pattern) -> zpt::json {
    return zpt::split(_pattern, zpt::catalog_id::separator<std::string>());
}
