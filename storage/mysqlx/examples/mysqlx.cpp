#include <zapata/mysqlx.h>

auto main(int, char**) -> int {
    zpt::json _config{ { "storage", { "mysqlx", { "user", "zpt", "host", "127.0.0.1" } } } };
    auto _connection = zpt::make_connection<zpt::storage::mysqlx::connection>(_config);
    auto _session = _connection->session();
    _session->sql("create schema if not exists test");
    _session->sql("create table if not exists test.users (_id varchar(36) primary key, name TEXT, "
                  "address TEXT)");

    auto _collection = _session->database("test")->collection("users");
    _collection //
      ->remove({})
      ->execute();
    auto _ids = _collection //
                  ->add({ "name", "John Smith", "address", "Upside Down 'just for fun' with {}" })
                  ->execute()
                  ->generated_id();

    std::cout << _ids << std::endl;
    std::cout << _collection //
                   ->find(std::format("_id = \"{}\"", _ids(0)->string()))
                   ->execute()
                   ->fetch()
              << std::endl;

    _collection //
      ->modify(std::format("_id = \"{}\"", _ids(0)->string()))
      ->set("name", "Zé Povinho 'or with {}'")
      ->execute();

    _collection //
      ->modify(std::format("_id = \"{}\"", _ids(0)->string()))
      ->patch({ "name", "Pixie", "address", "Neverland or a placeholder like '{}'" })
      ->execute();

    std::cout << zpt::pretty(_collection->find({})->execute()->fetch()) << std::endl;
    _session->commit();
}
