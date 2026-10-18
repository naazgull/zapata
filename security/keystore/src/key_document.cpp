#include <zapata/keystore/key_document.h>
#include <zapata/keystore/keystore.h>
#include <zapata/uri.h>

auto zpt::keystore::key_document::update_element() -> zpt::events::state {
    auto _received = this->received()->body();
    auto _params = this->received()->parameters()->is_object()
                     ? this->received()->parameters()->clone()
                     : zpt::json::object();
    auto _path = this->received()->uri()("path");
    size_t _prefix_len = zpt::rest::default_prefix_len();
    _params["key"] = _path(_prefix_len + 1);
    expect(_params("key")->ok(), "Required path parameter 'key'");
    expect(_received("secret")->ok(), "Required request member field 'secret'");
    try {
        zpt::KEY_STORE().rotate(_params("key")->string(), _received("secret")->string());

        this //
          ->to_send()
          ->status(202)
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

auto zpt::keystore::key_document::get_element() -> zpt::events::state {
    auto _params = this->received()->parameters()->is_object()
                     ? this->received()->parameters()->clone()
                     : zpt::json::object();
    auto _path = this->received()->uri()("path");
    size_t _prefix_len = zpt::rest::default_prefix_len();
    _params["key"] = _path(_prefix_len + 1);
    expect(_params("key")->ok(), "Required path parameter 'key'");
    try {
        this //
          ->to_send()
          ->status(200)
          .body() = zpt::KEY_STORE().get(_params("key")->string());
    }
    catch (std::exception const& _e) {
        this //
          ->to_send()
          ->status(500)
          .body() = { "message", _e.what() };
    }
    return zpt::events::finish;
}

auto zpt::keystore::key_document::remove_element() -> zpt::events::state {
    auto _params = this->received()->parameters()->is_object()
                     ? this->received()->parameters()->clone()
                     : zpt::json::object();
    auto _path = this->received()->uri()("path");
    size_t _prefix_len = zpt::rest::default_prefix_len();
    _params["key"] = _path(_prefix_len + 1);
    expect(_params("key")->ok(), "Required path parameter 'key'");
    try {
        this //
          ->to_send()
          ->status(202)
          .body() = { "removed_count", zpt::KEY_STORE().remove(_params("key")->string()) };
    }
    catch (std::exception const& _e) {
        this //
          ->to_send()
          ->status(500)
          .body() = { "message", _e.what() };
    }
    return zpt::events::finish;
}

auto zpt::keystore::key_document::operator()(zpt::events::dispatcher::ptr _dispatcher
                                             [[maybe_unused]]) -> zpt::events::state {
    switch (this->received()->performative()) {
        case zpt::Patch: {
            return this->update_element();
        }
        case zpt::Get: {
            return this->get_element();
        }
        case zpt::Delete: {
            return this->remove_element();
        }
    }
    this //
      ->to_send()
      ->status(405)
      .body() = { "message", "Only GET, PATCH, DELETE allowed to use with this document" };
    return zpt::events::abort;
}
