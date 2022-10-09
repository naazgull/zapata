#include <zapata/sqlite.h>

auto
main(int _argc, char* _argv[]) -> int {
    zpt::json _config{ "storage", { "sqlite", { "path", "/home/pf/Void/sqlite" } } };
    auto _connection = zpt::storage::connection::alloc<zpt::storage::sqlite::connection>(_config);
    auto _session = _connection->session();
    auto _database = _session->database("zapata");
    auto _collection = _database->collection("users");
    auto _id1 = zpt::generate::r_key(16);
    std::string _id2;

    {
        _collection //
          ->add({ "_id", _id1, "nick", "fimber", "email", "address@host" })
          ->add({ "nick", "fizz", "email", "address@ping" })
          ->execute();
        auto _result = _collection //
                         ->add({ "nick", "jaster", "email", "address@kicker" })
                         ->execute();
        _id2.assign(_result->to_json()["generated"][0]->string());
        zlog("---- There are " << _collection->count() << " records in the collection.", zpt::info);
    }
    {
        auto _result = _collection //
                         ->find({})
                         ->execute();
        zlog("---- Collection elements: " << zpt::json::pretty(_result->fetch()), zpt::info);
    }
    {
        auto _result = _collection //
                         ->find({ "_id", _id1 })
                         ->execute();
        zlog("---- First record: " << zpt::pretty(_result->to_json()), zpt::info)
    }
    {
        auto _result = _collection //
                         ->find({ "_id", _id2 })
                         ->execute();
        zlog("---- Second record: " << zpt::pretty(_result->to_json()), zpt::info);
    }
    {

        auto _result = _collection //
                         ->modify("_id = :id")
                         ->set("nick", "#########")
                         ->bind({ "id", _id2 })
                         ->execute();
        zlog("---- First record modified: " << zpt::pretty(_result->to_json()), zpt::info);
    }
    {
        auto _result = _collection //
                         ->replace(_id1, { "nick", "fimber replace", "email", "address_replace@host" })
                         ->execute();
        zlog("----- Replaced elements: " << zpt::pretty(_result->to_json()), zpt::info);
    }
    {
        auto _result = _collection //
                         ->find({})
                         ->execute();
        zlog("---- Collection elements: " << zpt::json::pretty(_result->fetch()), zpt::info);
    }
    {
        zpt::json _search = { "nick", "{.lower(fizz).}" };
        auto _result = _collection //
                         ->find(_search)
                         ->execute();
        zlog("---- Collection elements matching 'fizz': " << zpt::json::pretty(_result->fetch()), zpt::info);
    }
    {
        auto _result = _collection //
                         ->remove("nick = :nick")
                         ->bind({ "nick", "fizz" })
                         ->execute();
        zlog("---- Removed elements: " << zpt::pretty(_result->to_json()), zpt::info);
    }
    {
        auto _result = _collection //
                         ->find("nick = :nick")
                         ->bind({ "nick", "fizz" })
                         ->execute();
        zlog("---- Collection elements matching 'fizz': " << zpt::json::pretty(_result->fetch()), zpt::info);
    }
    {
        auto _result = _collection //
                         ->remove({})
                         ->execute();
        zlog("---- Removed elements: " << zpt::pretty(_result->to_json()), zpt::info);
    }
    {
        auto _result = _collection //
                         ->find({})
                         ->execute();
        zlog("---- Collection elements: " << zpt::json::pretty(_result->fetch()), zpt::info);
    }
    return 0;
}
