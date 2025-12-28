#include <zapata/mysqlx.h>

auto main(int, char**) -> int {
    zpt::json _config{
        { "storage", { "mysqlx", { "user", "root", "password", "d3bianer", "host", "127.0.0.1" } } }
    };
    auto _connection = zpt::make_connection<zpt::storage::mysqlx::connection>(_config);
    auto _session = _connection->session();
    auto _collection = _session->database("test")->collection("users");
    auto _ids = _collection->add({ "name", "John Smith", "address", "Upside Down" })
                  ->execute()
                  ->generated_id();
    std::cout << _ids << std::endl;
    std::cout
      << _collection->find(std::format("_id = \"{}\"", _ids(0)->string()))->execute()->fetch()
      << std::endl;
    std::cout << zpt::pretty(_collection->find({})->execute()->fetch()) << std::endl;
    _session->commit();
}
