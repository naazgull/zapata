#include <zapata/catalogue.h>

auto
zpt::CATALOGUE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}
