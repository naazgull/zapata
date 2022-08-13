#include <zapata/base.h>
#include <zapata/globals.h>

auto f() -> int {
    auto uuid = zpt::generate::r_uuid();
    std::cout << uuid << std::endl << std::flush;
    return 0;
}
