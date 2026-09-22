#include <zapata/mysqlx.h>

auto main(int, char**) -> int {
    zpt::log_lvl = 9;
    zpt::json _config{ { "storage", { "mysqlx", { "user", "zpt", "host", "127.0.0.1" } } } };
    auto _connection = zpt::make_connection<zpt::storage::mysqlx::connection>(_config);
    auto _session = _connection->session();
    _session->sql("drop schema if exists examples_storage_mysqlx");
    _session->sql("create schema examples_storage_mysqlx");
    _session->sql(
      "create table examples_storage_mysqlx.users (_id varchar(36) primary key, name TEXT, "
      "address TEXT, created TIMESTAMP(3) not null default now(3))");

    auto _database = _session->database("examples_storage_mysqlx");
    auto _collection = _database->collection("users");
    _collection //
      ->remove({})
      ->execute();
    auto _ids = _collection //
                  ->add({ "name",
                          "John Smith",
                          "address",
                          "Upside Down 'just for fun' with {}",
                          "created",
                          zpt::timestamp() })
                  ->add({ "name",
                          "John Smith 2",
                          "address",
                          "Upside Down 'just for fun' with {}",
                          "created",
                          zpt::timestamp() })
                  ->add({ "name",
                          "John Smith 3",
                          "address",
                          "Upside Down 'just for fun' with {}",
                          "created",
                          zpt::timestamp() })
                  ->execute()
                  ->generated_id();

    std::cout << _ids << std::endl;
    std::cout << _collection //
                   ->find(std::format("_id = \"{}\"", _ids(0)->string()))
                   ->execute()
                   ->fetch()
              << std::endl;

    std::cout << _session //
                   ->sql("select * from test.users where address like '%Down%'")
                   ->fetch()
              << std::endl;

    std::cout << _database //
                   ->sql("select * from users where address like '%Down%'")
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
