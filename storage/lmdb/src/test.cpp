#include <zapata/lmdb.h>

auto main(int _argc, char* _argv[]) -> int {
    zpt::json _config{ "storage", { "lmdb", { "path", "/home/pf/Void/lmdb" } } };
    auto _connection = zpt::make_connection<zpt::storage::lmdb::connection>(_config);
    auto _session = _connection->session();
    auto _database = _session->database("zapata");
    auto _collection = _database->collection("users");
    auto _id1 = zpt::generate::r_key(64);
    std::string _id2;

    {
        _collection //
          ->add({ "_id", _id1, "nick", "fimber", "email", "address@host" })
          ->execute();
        auto _result = _collection //
                         ->add({ "nick", "jaster", "email", "address@kicker" })
                         ->execute();
        _id2.assign(_result->to_json()("generated")(0)("_id")->string());
        std::cout << "There are " << _collection->count() << " records in the collection."
                  << std::endl
                  << std::flush;
    }
    {
        auto _result = _collection //
                         ->find({})
                         ->execute();

        std::cout << "Collection elements: " << std::endl
                  << zpt::pretty(_result->to_json()("cursor")) << std::endl
                  << std::flush;
    }
    {
        auto _result = _collection //
                         ->find({ "_id", _id1 })
                         ->execute();

        std::cout << "First record: " << std::endl
                  << zpt::pretty(_result->to_json()) << std::endl
                  << std::flush;
    }
    {
        auto _result = _collection //
                         ->find({ "_id", _id2 })
                         ->execute();

        std::cout << "Second record: " << std::endl
                  << zpt::pretty(_result->to_json()) << std::endl
                  << std::flush;
    }
    {
        auto _result = _collection //
                         ->modify({ "_id", _id1 })
                         ->set("nick", "fin")
                         ->execute();

        std::cout << "First record modified: " << std::endl
                  << zpt::pretty(_result->to_json()) << std::endl
                  << std::flush;
    }
    {
        auto _result = _collection //
                         ->find({})
                         ->execute();

        std::cout << "Collection elements: " << std::endl
                  << zpt::pretty(_result->to_json()("cursor")) << std::endl
                  << std::flush;
    }
    {
        auto _result = _collection //
                         ->remove({})
                         ->execute();

        std::cout << "Removed elements: " << std::endl
                  << zpt::pretty(_result->to_json()) << std::endl
                  << std::flush;
    }
    {
        auto _result = _collection //
                         ->find({})
                         ->execute();

        std::cout << "Collection elements: " << std::endl
                  << zpt::pretty(_result->to_json()("cursor")) << std::endl
                  << std::flush;
    }
    return 0;
}
