#include <zapata/mysqlx.h>

auto main(int, char**) -> int {
    zpt::json _config{ "user", "root", "password", "", "host", "127.0.0.1" };
    auto _connection = zpt::make_connection<zpt::storage::mysqlx::connection>(_config);
}
