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
class filter;
class result;

class connection {
  public:
    class type {
      public:
        virtual auto open(zpt::json _options) -> zpt::storage::connection::type* = 0;
        virtual auto close() -> zpt::storage::connection::type* = 0;
        virtual auto session() -> zpt::storage::session& = 0;
    };

    connection() = default;
    template<typename T, typename... Args>
    connection(Args... _args);
    connection(zpt::storage::connection const& _rhs);
    connection(zpt::storage::connection&& _rhs);
    virtual ~connection() = default;

    auto operator=(zpt::storage::connection const& _rhs) -> zpt::storage::connection&;
    auto operator=(zpt::storage::connection&& _rhs) -> zpt::storage::connection&;

    auto operator-> () -> zpt::storage::connection::type*;
    auto operator*() -> zpt::storage::connection::type&;

  private:
    std::shared_ptr<zpt::storage::connection::type> __underlying{ nullptr };
};
class session {
  public:
    class type {
      public:
        virtual auto is_open() -> bool = 0;
        virtual auto commit() -> zpt::storage::session::type* = 0;
        virtual auto rollback() -> zpt::storage::session::type* = 0;
        virtual auto database(std::string _db) -> zpt::storage::database& = 0;
        virtual auto connection() -> zpt::storage::connection& = 0;
    };

    session() = default;
    template<typename T, typename... Args>
    session(Args... _args);
    session(zpt::storage::session const& _rhs);
    session(zpt::storage::session&& _rhs);
    virtual ~session() = default;

    auto operator=(zpt::storage::session const& _rhs) -> zpt::storage::session&;
    auto operator=(zpt::storage::session&& _rhs) -> zpt::storage::session&;

    auto operator-> () -> zpt::storage::session::type*;
    auto operator*() -> zpt::storage::session::type&;

  private:
    std::shared_ptr<zpt::storage::session::type> __underlying{ nullptr };
};
class database {
  public:
    class type {
      public:
        virtual auto collection(std::string _name) -> zpt::storage::collection& = 0;
        virtual auto connection() -> zpt::storage::connection& = 0;
        virtual auto session() -> zpt::storage::session& = 0;
    };

    database() = default;
    template<typename T, typename... Args>
    database(Args... _args);
    database(zpt::storage::database const& _rhs);
    database(zpt::storage::database&& _rhs);
    virtual ~database() = default;

    auto operator=(zpt::storage::database const& _rhs) -> zpt::storage::database&;
    auto operator=(zpt::storage::database&& _rhs) -> zpt::storage::database&;

    auto operator-> () -> zpt::storage::database::type*;
    auto operator*() -> zpt::storage::database::type&;

  private:
    std::shared_ptr<zpt::storage::database::type> __underlying{ nullptr };
};
class collection {
  public:
    class type {
      public:
        virtual auto add(zpt::json _document) -> zpt::storage::filter& = 0;
        virtual auto modify(zpt::json _search) -> zpt::storage::filter& = 0;
        virtual auto remove(zpt::json _search) -> zpt::storage::filter& = 0;
        virtual auto replace(std::string _id, zpt::json _document) -> zpt::storage::filter& = 0;
        virtual auto find(zpt::json _search) -> zpt::storage::filter& = 0;
        virtual auto count() -> size_t = 0;
        virtual auto connection() -> zpt::storage::connection& = 0;
        virtual auto session() -> zpt::storage::session& = 0;
        virtual auto database() -> zpt::storage::database& = 0;
    };

    collection() = default;
    template<typename T, typename... Args>
    collection(Args... _args);
    collection(zpt::storage::collection const& _rhs);
    collection(zpt::storage::collection&& _rhs);
    virtual ~collection() = default;

    auto operator=(zpt::storage::collection const& _rhs) -> zpt::storage::collection&;
    auto operator=(zpt::storage::collection&& _rhs) -> zpt::storage::collection&;

    auto operator-> () -> zpt::storage::collection::type*;
    auto operator*() -> zpt::storage::collection::type&;

  private:
    std::shared_ptr<zpt::storage::collection::type> __underlying{ nullptr };
};
class filter {
  public:
    class type {
      public:
        virtual auto add(zpt::json _document) -> zpt::storage::filter::type* = 0;
        virtual auto modify(zpt::json _search) -> zpt::storage::filter::type* = 0;
        virtual auto remove(zpt::json _search) -> zpt::storage::filter::type* = 0;
        virtual auto replace(std::string _id, zpt::json _document)
          -> zpt::storage::filter::type* = 0;
        virtual auto find(zpt::json _search) -> zpt::storage::filter::type* = 0;
        virtual auto set(std::string _attribute, zpt::json _value)
          -> zpt::storage::filter::type* = 0;
        virtual auto unset(std::string _attribute) -> zpt::storage::filter::type* = 0;
        virtual auto patch(zpt::json _document) -> zpt::storage::filter::type* = 0;
        virtual auto sort(std::string _attribute) -> zpt::storage::filter::type* = 0;
        virtual auto fields(zpt::json _fields) -> zpt::storage::filter::type* = 0;
        virtual auto limit(size_t _number) -> zpt::storage::filter::type* = 0;
        virtual auto bind(std::string _attribute, zpt::json _Value)
          -> zpt::storage::filter::type* = 0;
        virtual auto execute() -> zpt::storage::result& = 0;
        virtual auto connection() -> zpt::storage::connection& = 0;
        virtual auto session() -> zpt::storage::session& = 0;
        virtual auto database() -> zpt::storage::database& = 0;
        virtual auto collection() -> zpt::storage::collection& = 0;
    };

    filter() = default;
    template<typename T, typename... Args>
    filter(Args... _args);
    filter(zpt::storage::filter const& _rhs);
    filter(zpt::storage::filter&& _rhs);
    virtual ~filter() = default;

    auto operator=(zpt::storage::filter const& _rhs) -> zpt::storage::filter&;
    auto operator=(zpt::storage::filter&& _rhs) -> zpt::storage::filter&;

    auto operator-> () -> zpt::storage::filter::type*;
    auto operator*() -> zpt::storage::filter::type&;

  private:
    std::shared_ptr<zpt::storage::filter::type> __underlying{ nullptr };
};
class result {
  public:
    class type {
      public:
        virtual auto fetch(size_t _amount = 1) -> zpt::json = 0;
        virtual auto all() -> zpt::json = 0;
    };

    result() = default;
    template<typename T, typename... Args>
    result(Args... _args);
    result(zpt::storage::result const& _rhs);
    result(zpt::storage::result&& _rhs);
    virtual ~result() = default;

    auto operator=(zpt::storage::result const& _rhs) -> zpt::storage::result&;
    auto operator=(zpt::storage::result&& _rhs) -> zpt::storage::result&;

    auto operator-> () -> zpt::storage::result::type*;
    auto operator*() -> zpt::storage::result::type&;

  private:
    std::shared_ptr<zpt::storage::result::type> __underlying{ nullptr };
};
} // namespace storage
} // namespace zpt

template<typename T, typename... Args>
zpt::storage::connection::connection(Args... _args)
  : __underlying{ std::make_shared<T>(_args...) } {}

template<typename T, typename... Args>
zpt::storage::session::session(Args... _args)
  : __underlying{ std::make_shared<T>(_args...) } {}

template<typename T, typename... Args>
zpt::storage::database::database(Args... _args)
  : __underlying{ std::make_shared<T>(_args...) } {}

template<typename T, typename... Args>
zpt::storage::collection::collection(Args... _args)
  : __underlying{ std::make_shared<T>(_args...) } {}

template<typename T, typename... Args>
zpt::storage::filter::filter(Args... _args)
  : __underlying{ std::make_shared<T>(_args...) } {}

template<typename T, typename... Args>
zpt::storage::result::result(Args... _args)
  : __underlying{ std::make_shared<T>(_args...) } {}
