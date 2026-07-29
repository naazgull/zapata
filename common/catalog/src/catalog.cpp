#include <zapata/catalog.h>

auto zpt::catalog_id::split(std::string const& _pattern) -> zpt::json {
    return zpt::split(_pattern, zpt::catalog_id::separator<std::string>());
}
