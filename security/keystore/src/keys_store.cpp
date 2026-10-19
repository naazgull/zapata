#include <zapata/keystore/keys_store.h>
#include <zapata/keystore/keystore.h>
#include <zapata/uri.h>

auto zpt::keystore::keys_store::add_element() -> zpt::events::state {
    auto _received = this->received()->body();
    expect(_received("key")->ok(), "Required request member field 'key'");
    expect(_received("secret")->ok(), "Required request member field 'secret'");
    try {
        zpt::KEY_STORE().create(
          _received("key")->string(), _received("secret")->string(), _received("metadata"));

        this //
          ->to_send()
          ->status(201)
          .body() = _received;
    }
    catch (std::exception const& _e) {
        this //
          ->to_send()
          ->status(500)
          .body() = { "message", _e.what() };
    }
    return zpt::events::finish;
}

auto zpt::keystore::keys_store::list_elements() -> zpt::events::state {
    try {
        auto _items = zpt::KEY_STORE().list();
        if (_items->size() == 0) {
            this //
              ->to_send()
              ->status(204);
        }
        else {
            this //
              ->to_send()
              ->status(200)
              .body() = { "items", _items, "size", _items->size() };
        }
    }
    catch (std::exception const& _e) {
        this //
          ->to_send()
          ->status(500)
          .body() = { "message", _e.what() };
    }
    return zpt::events::finish;
}

auto zpt::keystore::keys_store::remove_elements() -> zpt::events::state {
    try {
        auto _count = zpt::KEY_STORE().list()->size();
        zpt::KEY_STORE().clear();

        this //
          ->to_send()
          ->status(202)
          .body() = { "removed_count", _count };
    }
    catch (std::exception const& _e) {
        this //
          ->to_send()
          ->status(500)
          .body() = { "message", _e.what() };
    }
    return zpt::events::finish;
}

auto zpt::keystore::keys_store::operator()(zpt::events::dispatcher::ptr _dispatcher
                                           [[maybe_unused]]) -> zpt::events::state {
    switch (this->received()->performative()) {
        case zpt::Put: {
            return this->add_element();
        }
        case zpt::Get: {
            return this->list_elements();
        }
        case zpt::Delete: {
            return this->remove_elements();
        }
    }
    this //
      ->to_send()
      ->status(405)
      .body() = { "message", "Only GET, PUT, DELETE allowed to use with this store" };
    return zpt::events::abort;
}
