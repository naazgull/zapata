#include <zapata/connector.h>

auto main(int, char**) -> int {
    zpt::json _find = { "a", "{.gt(+(1,2,*(2,4))).}",
                        "b", "{.lt(double(0.034)).}",
                        "c", "{.boolean(true).}",
                        "d", 0.1,
                        "e", true,
                        "f", "{.date(\"2022-02-20T16:28:01.000\").}" };
    zlog(zpt::storage::extract_find(_find), zpt::info);
}
