#include <zapata/mongodb.h>

auto main(int, char**) -> int {
    zpt::json _config{ { "storage",
                         { "mongodb", { "host", "zgul.me", "port", 27017, "db", "test" } } } };
    auto _connection = zpt::make_connection<zpt::storage::mongodb::connection>(_config);
    auto _session = _connection->session();

    auto _collection = _session->database("test")->collection("users");
    _collection //
      ->remove({})
      ->execute();

    auto _ids = _collection //
                  ->add({ "name", "John Smith", "address", "Upside Down" })
                  ->execute()
                  ->generated_id();

    std::cout << _ids << std::endl;
    std::cout << _collection //
                   ->find({ "_id", _ids(0)->string() })
                   ->execute()
                   ->fetch()
              << std::endl;

    _collection //
      ->modify({ "_id", _ids(0)->string() })
      ->set("name", "Zé Povinho")
      ->execute();

    _collection //
      ->modify({ "_id", _ids(0)->string() })
      ->patch({ "address", "Neverland" })
      ->execute();

    std::cout << zpt::pretty(_collection->find({})->execute()->fetch()) << std::endl;
    _session->commit();
}
