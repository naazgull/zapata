/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <mutex>
#include <ossp/uuid++.hh>
#include <zapata/events.h>
#include <zapata/mongodb/convert_mongo.h>

namespace zpt {

namespace mongodb {

class Client : public zpt::Connector {
  public:
    Client(zpt::json _options, std::string const& _conf_path);
    virtual ~Client();

    virtual auto name() -> std::string;
    virtual auto options() -> zpt::json;
    virtual auto events(zpt::ev::emitter _emitter) -> void;
    virtual auto events() -> zpt::ev::emitter;

    virtual auto connect() -> void;
    virtual auto reconnect() -> void;
    virtual auto conn() -> mongo::ScopedDbConnection&;

    virtual auto insert(std::string const& _collection,
                        std::string _href_prefix,
                        zpt::json _record,
                        zpt::json _opts = zpt::undefined) -> std::string;
    virtual auto upsert(std::string const& _collection,
                        std::string _href_prefix,
                        zpt::json _record,
                        zpt::json _opts = zpt::undefined) -> std::string;
    virtual auto save(std::string const& _collection,
                      std::string _href,
                      zpt::json _record,
                      zpt::json _opts = zpt::undefined) -> int;
    virtual auto set(std::string const& _collection,
                     std::string _href,
                     zpt::json _record,
                     zpt::json _opts = zpt::undefined) -> int;
    virtual auto set(std::string const& _collection,
                     zpt::json _query,
                     zpt::json _record,
                     zpt::json _opts = zpt::undefined) -> int;
    virtual auto unset(std::string const& _collection,
                       std::string _href,
                       zpt::json _record,
                       zpt::json _opts = zpt::undefined) -> int;
    virtual auto unset(std::string const& _collection,
                       zpt::json _query,
                       zpt::json _record,
                       zpt::json _opts = zpt::undefined) -> int;
    virtual auto remove(std::string const& _collection,
                        std::string _href,
                        zpt::json _opts = zpt::undefined) -> int;
    virtual auto remove(std::string const& _collection,
                        zpt::json _query,
                        zpt::json _opts = zpt::undefined) -> int;
    virtual auto get(std::string const& _collection,
                     std::string const& _href,
                     zpt::json _opts = zpt::undefined) -> zpt::json;
    virtual auto query(std::string const& _collection,
                       std::string _query,
                       zpt::json _opts = zpt::undefined) -> zpt::json;
    virtual auto query(std::string const& _collection,
                       zpt::json _query,
                       zpt::json _opts = zpt::undefined) -> zpt::json;
    virtual auto all(std::string const& _collection, zpt::json _opts = zpt::undefined) -> zpt::json;

  private:
    zpt::json __options;
    std::mutex __mtx;
    std::unique_ptr<mongo::ScopedDbConnection> __conn;
    std::string _conn_str;
    zpt::ev::emitter __events;
};

class ClientPtr : public std::shared_ptr<zpt::mongodb::Client> {
  public:
    /**
     * @brief Creates an std::shared_ptr to an Self instance.
     *
     * @param _options the configuration object retrieved from the configuration
     * JSON file
     */
    ClientPtr(zpt::mongodb::Client* _target);
    ClientPtr(zpt::json _options, std::string const& _conf_path);

    /**
     * @brief Destroys the current Self instance, freeing all allocated memory.
     */
    virtual ~ClientPtr();
};

typedef zpt::mongodb::ClientPtr client;
} // namespace mongodb
} // namespace zpt
