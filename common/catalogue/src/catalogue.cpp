#include <zapata/catalogue.h>

auto zpt::CATALOGUE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

auto zpt::catalogue_id::split(std::string const& _pattern) -> zpt::json {
    return zpt::split(_pattern, zpt::catalogue_id::separator<std::string>());
}
