#include <zapata/mysqlx.h>

auto main(int _argc, char* _argv[]) -> int {
    zpt::json _config{ "user", "root", "password", "", "host", "127.0.0.1" };
    auto _connection = zpt::storage::connection::alloc<zpt::storage::mysqlx::connection>(_config);
}
