#include <zapata/mongodb.h>

auto main(int, char**) -> int {
    zpt::json _config{ { "storage", { "mongodb", { "user", "zpt", "host", "127.0.0.1" } } } };
    auto _connection = zpt::make_connection<zpt::storage::mongodb::connection>(_config);
}
