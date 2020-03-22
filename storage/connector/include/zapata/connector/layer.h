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
#include <zapata/connector.h>

namespace zpt {
namespace storage {
namespace layer {
class connection;
class session;
class database;
class collection;
class action;
class result;

class connection : public zpt::storage::connection::type {
  public:
    connection(zpt::json _config);
    virtual auto open(zpt::json _options) -> zpt::storage::connection::type* override;
    virtual auto close() -> zpt::storage::connection::type* override;
    virtual auto session() -> zpt::storage::session override;
    virtual auto config() -> zpt::json& final;

    static auto connectors() -> std::map<std::string, zpt::storage::connection>&;
    static auto add(std::string _name, zpt::storage::connection _connector) -> void;

  private:
    zpt::json __config;
    static inline std::map<std::string, zpt::storage::connection> __connectors;
};
class session : public zpt::storage::session::type {
  public:
    session(zpt::storage::layer::connection& _connection);
    virtual auto is_open() -> bool override;
    virtual auto commit() -> zpt::storage::session::type* override;
    virtual auto rollback() -> zpt::storage::session::type* override;
    virtual auto database(std::string _db) -> zpt::storage::database override;

    virtual auto connection() -> zpt::storage::layer::connection& final;
    virtual auto sessions() -> std::map<std::string, zpt::storage::session>& final;

  private:
    zpt::storage::layer::connection& __connection;
    std::map<std::string, zpt::storage::session> __sessions;
};
class database : public zpt::storage::database::type {
  public:
    database(zpt::storage::layer::session& _session, std::string _db);
    virtual auto collection(std::string _name) -> zpt::storage::collection override;

    virtual auto session() -> zpt::storage::layer::session& final;
    virtual auto databases() -> std::map<std::string, zpt::storage::database>& final;

  private:
    zpt::storage::layer::session& __session;
    std::map<std::string, zpt::storage::database> __databases;
};
class collection : public zpt::storage::collection::type {
  public:
    collection(zpt::storage::layer::database& _database, std::string _collection);
    virtual auto add(zpt::json _document) -> zpt::storage::action override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action override;
    virtual auto replace(std::string _id, zpt::json _document) -> zpt::storage::action override;
    virtual auto find(zpt::json _search) -> zpt::storage::action override;
    virtual auto count() -> size_t override;

    virtual auto database() -> zpt::storage::layer::database& final;
    virtual auto collections() -> std::map<std::string, zpt::storage::collection>& final;

  private:
    zpt::storage::layer::database& __database;
    std::map<std::string, zpt::storage::collection> __collections;
};
class action : public zpt::storage::action::type {
  public:
    action(zpt::storage::layer::collection& _collection);

    virtual auto collection() -> zpt::storage::layer::collection& final;
    virtual auto actions() -> std::map<std::string, zpt::storage::action>& final;
    virtual auto config() -> zpt::json& final;

  protected:
    zpt::storage::layer::collection& __collection;
    std::map<std::string, zpt::storage::action> __actions;
    zpt::json __config;
};
class action_add : public zpt::storage::layer::action {
  public:
    action_add(zpt::storage::layer::collection& _collection, zpt::json _document);
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;
};
class action_modify : public zpt::storage::layer::action {
  public:
    action_modify(zpt::storage::layer::collection& _collection, zpt::json _search);
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;
};
class action_remove : public zpt::storage::layer::action {
  public:
    action_remove(zpt::storage::layer::collection& _collection, zpt::json _search);
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;
};
class action_replace : public zpt::storage::layer::action {
  public:
    action_replace(zpt::storage::layer::collection& _collection,
                   std::string _id,
                   zpt::json _document);
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;
};
class action_find : public zpt::storage::layer::action {
  public:
    action_find(zpt::storage::layer::collection& _collection, zpt::json _search);
    virtual auto add(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto modify(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto remove(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto replace(std::string _id, zpt::json _document)
      -> zpt::storage::action::type* override;
    virtual auto find(zpt::json _search) -> zpt::storage::action::type* override;
    virtual auto set(std::string _attribute, zpt::json _value)
      -> zpt::storage::action::type* override;
    virtual auto unset(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto patch(zpt::json _document) -> zpt::storage::action::type* override;
    virtual auto sort(std::string _attribute) -> zpt::storage::action::type* override;
    virtual auto fields(zpt::json _fields) -> zpt::storage::action::type* override;
    virtual auto offset(size_t _rows) -> zpt::storage::action::type* override;
    virtual auto limit(size_t _number) -> zpt::storage::action::type* override;
    virtual auto bind(zpt::json _map) -> zpt::storage::action::type* override;
    virtual auto execute() -> zpt::storage::result override;
};
class result : public zpt::storage::result::type {
  public:
    result(zpt::storage::layer::action& _action);
    virtual auto fetch(size_t _amount = 0) -> zpt::json override;
    virtual auto generated_id() -> zpt::json override;
    virtual auto count() -> size_t override;
    virtual auto status() -> zpt::status override;
    virtual auto message() -> std::string override;

    virtual auto action() -> zpt::storage::layer::action& final;
    virtual auto results() -> std::map<std::string, zpt::storage::result>& final;

  private:
    zpt::storage::layer::action& __action;
    std::map<std::string, zpt::storage::result> __results;
};
} // namespace layer
} // namespace storage
} // namespace zpt
