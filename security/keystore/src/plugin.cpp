#include <iostream>
#include <zapata/keystore/key_document.h>
#include <zapata/keystore/keys_store.h>
#include <zapata/rest.h>

extern "C" auto _zpt_load_(zpt::plugin& _plugin [[maybe_unused]]) -> void {
    zlog("Registering handlers for key-store", zpt::info);
    auto const& _config = zpt::GLOBAL_CONFIG();
    auto _resolver = zpt::REST_RESOLVER();
    auto _prefix = zpt::rest::default_prefix();
    _resolver->add<zpt::keystore::keys_store>(_config("keystore")("url")("collection")->string());
    _resolver->add<zpt::keystore::key_document>(_config("keystore")("url")("document")->string());
}

extern "C" auto _zpt_unload_(zpt::plugin& _plugin [[maybe_unused]]) -> void {
    zlog("Unregistering handlers for key-store", zpt::info);
    auto const& _config = zpt::GLOBAL_CONFIG();
    auto _resolver = zpt::REST_RESOLVER();
    auto _prefix = zpt::rest::default_prefix();
    _resolver->remove<zpt::keystore::keys_store>(
      _config("keystore")("url")("collection")->string());
    _resolver->remove<zpt::keystore::key_document>(
      _config("keystore")("url")("document")->string());
}
