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

#include <zapata/json.h>

namespace zpt {
namespace storage {
class connection;
class session;
class database;
class collection;
class action;
class result;

class connection {
  public:
    class type {
      public:
        virtual auto add(std::string _name, zpt::storage::connection _connector)
            -> zpt::storage::connection::type* = 0;
        virtual auto open(zpt::json _options) -> zpt::storage::connection::type* = 0;
        virtual auto close() -> zpt::storage::connection::type* = 0;
        virtual auto session() -> zpt::storage::session = 0;
    };

    connection() = default;
    connection(zpt::storage::connection const& _rhs);
    connection(zpt::storage::connection&& _rhs);
    virtual ~connection() = default;

    auto operator=(zpt::storage::connection const& _rhs) -> zpt::storage::connection&;
    auto operator=(zpt::storage::connection&& _rhs) -> zpt::storage::connection&;

    auto operator-> () -> zpt::storage::connection::type*;
    auto operator*() -> zpt::storage::connection::type&;

    template<typename T, typename... Args>
    static auto alloc(Args&... _args) -> zpt::storage::connection;

  private:
    std::shared_ptr<zpt::storage::connection::type> __underlying{ nullptr };

    connection(zpt::storage::connection::type* _underlying);
};
class session {
  public:
    class type {
      public:
        virtual auto is_open() -> bool = 0;
        virtual auto commit() -> zpt::storage::session::type* = 0;
        virtual auto rollback() -> zpt::storage::session::type* = 0;
        virtual auto database(std::string _db) -> zpt::storage::database = 0;
    };

    session() = default;
    session(zpt::storage::session const& _rhs);
    session(zpt::storage::session&& _rhs);
    virtual ~session() = default;

    auto operator=(zpt::storage::session const& _rhs) -> zpt::storage::session&;
    auto operator=(zpt::storage::session&& _rhs) -> zpt::storage::session&;

    auto operator-> () -> zpt::storage::session::type*;
    auto operator*() -> zpt::storage::session::type&;

    template<typename T, typename... Args>
    static auto alloc(Args&... _args) -> zpt::storage::session;

  private:
    std::shared_ptr<zpt::storage::session::type> __underlying{ nullptr };

    session(zpt::storage::session::type* _underlying);
};
class database {
  public:
    class type {
      public:
        virtual auto collection(std::string _name) -> zpt::storage::collection = 0;
    };

    database() = default;
    database(zpt::storage::database const& _rhs);
    database(zpt::storage::database&& _rhs);
    virtual ~database() = default;

    auto operator=(zpt::storage::database const& _rhs) -> zpt::storage::database&;
    auto operator=(zpt::storage::database&& _rhs) -> zpt::storage::database&;

    auto operator-> () -> zpt::storage::database::type*;
    auto operator*() -> zpt::storage::database::type&;

    template<typename T, typename... Args>
    static auto alloc(Args&... _args) -> zpt::storage::database;

  private:
    std::shared_ptr<zpt::storage::database::type> __underlying{ nullptr };

    database(zpt::storage::database::type* _underlying);
};
class collection {
  public:
    class type {
      public:
        virtual auto add(zpt::json _document) -> zpt::storage::action = 0;
        virtual auto modify(zpt::json _search) -> zpt::storage::action = 0;
        virtual auto remove(zpt::json _search) -> zpt::storage::action = 0;
        virtual auto replace(std::string _id, zpt::json _document) -> zpt::storage::action = 0;
        virtual auto find(zpt::json _search) -> zpt::storage::action = 0;
        virtual auto count() -> size_t = 0;
    };

    collection() = default;
    collection(zpt::storage::collection const& _rhs);
    collection(zpt::storage::collection&& _rhs);
    virtual ~collection() = default;

    auto operator=(zpt::storage::collection const& _rhs) -> zpt::storage::collection&;
    auto operator=(zpt::storage::collection&& _rhs) -> zpt::storage::collection&;

    auto operator-> () -> zpt::storage::collection::type*;
    auto operator*() -> zpt::storage::collection::type&;

    template<typename T, typename... Args>
    static auto alloc(Args&... _args) -> zpt::storage::collection;

  private:
    std::shared_ptr<zpt::storage::collection::type> __underlying{ nullptr };

    collection(zpt::storage::collection::type* _underlying);
};
class action {
  public:
    class type {
      public:
        virtual auto add(zpt::json _document) -> zpt::storage::action::type* = 0;
        virtual auto modify(zpt::json _search) -> zpt::storage::action::type* = 0;
        virtual auto remove(zpt::json _search) -> zpt::storage::action::type* = 0;
        virtual auto replace(std::string _id, zpt::json _document)
          -> zpt::storage::action::type* = 0;
        virtual auto find(zpt::json _search) -> zpt::storage::action::type* = 0;
        virtual auto set(std::string _attribute, zpt::json _value)
          -> zpt::storage::action::type* = 0;
        virtual auto unset(std::string _attribute) -> zpt::storage::action::type* = 0;
        virtual auto patch(zpt::json _document) -> zpt::storage::action::type* = 0;
        virtual auto sort(std::string _attribute) -> zpt::storage::action::type* = 0;
        virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* = 0;
        virtual auto offset(size_t _rows) -> zpt::storage::action::type* = 0;
        virtual auto limit(size_t _number) -> zpt::storage::action::type* = 0;
        virtual auto bind(zpt::json _map) -> zpt::storage::action::type* = 0;
        virtual auto execute() -> zpt::storage::result = 0;
    };

    action() = default;
    action(zpt::storage::action const& _rhs);
    action(zpt::storage::action&& _rhs);
    virtual ~action() = default;

    auto operator=(zpt::storage::action const& _rhs) -> zpt::storage::action&;
    auto operator=(zpt::storage::action&& _rhs) -> zpt::storage::action&;

    auto operator-> () -> zpt::storage::action::type*;
    auto operator*() -> zpt::storage::action::type&;

    template<typename T, typename... Args>
    static auto alloc(Args&... _args) -> zpt::storage::action;

  private:
    std::shared_ptr<zpt::storage::action::type> __underlying{ nullptr };

    action(zpt::storage::action::type* _underlying);
};
class result {
  public:
    class type {
      public:
        virtual auto fetch(size_t _amount = 0) -> zpt::json = 0;
        virtual auto generated_id() -> zpt::json = 0;
        virtual auto count() -> size_t = 0;
        virtual auto status() -> zpt::status = 0;
        virtual auto message() -> std::string = 0;
    };

    result() = default;
    result(zpt::storage::result const& _rhs);
    result(zpt::storage::result&& _rhs);
    virtual ~result() = default;

    auto operator=(zpt::storage::result const& _rhs) -> zpt::storage::result&;
    auto operator=(zpt::storage::result&& _rhs) -> zpt::storage::result&;

    auto operator-> () -> zpt::storage::result::type*;
    auto operator*() -> zpt::storage::result::type&;

    template<typename T, typename... Args>
    static auto alloc(Args&... _args) -> zpt::storage::result;

  private:
    std::shared_ptr<zpt::storage::result::type> __underlying{ nullptr };

    result(zpt::storage::result::type* _underlying);
};
} // namespace storage
} // namespace zpt

template<typename T, typename... Args>
auto
zpt::storage::connection::alloc(Args&... _args) -> zpt::storage::connection {
    return zpt::storage::connection{ new T{ _args... } };
}

template<typename T, typename... Args>
auto
zpt::storage::session::alloc(Args&... _args) -> zpt::storage::session {
    return zpt::storage::session{ new T{ _args... } };
}

template<typename T, typename... Args>
auto
zpt::storage::database::alloc(Args&... _args) -> zpt::storage::database {
    return zpt::storage::database{ new T{ _args... } };
}

template<typename T, typename... Args>
auto
zpt::storage::collection::alloc(Args&... _args) -> zpt::storage::collection {
    return zpt::storage::collection{ new T{ _args... } };
}

template<typename T, typename... Args>
auto
zpt::storage::action::alloc(Args&... _args) -> zpt::storage::action {
    return zpt::storage::action{ new T{ _args... } };
}

template<typename T, typename... Args>
auto
zpt::storage::result::alloc(Args&... _args) -> zpt::storage::result {
    return zpt::storage::result{ new T{ _args... } };
}
