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

#include <algorithm>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/options/replace.hpp>
#include <zapata/mongodb/connector.h>
#include <zapata/uuid.h>

// ---- Library ----

zpt::storage::mongodb::library::library() {}
zpt::storage::mongodb::library::~library() {}

auto zpt::storage::mongodb::init() -> zpt::storage::mongodb::library& {
    static library _global;
    return _global;
}

// ---- Connection ----

zpt::storage::mongodb::connection::connection(zpt::json _options)
  : __options{ _options("storage")("mongodb") } {
    zpt::storage::mongodb::init();
    this->open(_options("storage")("mongodb"));
}

auto zpt::storage::mongodb::connection::open(zpt::json _options)
  -> zpt::storage::connection::type* {
    this->__options = _options;

    auto _host = this->__options("host")->ok() ? this->__options("host")->string() : "127.0.0.1";
    auto _port = this->__options("port")->ok() ? this->__options("port")->integer() : 27017LL;
    auto _user = this->__options("user")->ok() ? this->__options("user")->string() : "";
    auto _pass = this->__options("password")->ok() ? this->__options("password")->string() : "";
    auto _db = this->__options("db")->ok() ? this->__options("db")->string() : "";

    std::ostringstream _uri_ss;
    _uri_ss << "mongodb://";
    if (!_user.empty()) {
        _uri_ss << _user;
        if (!_pass.empty()) { _uri_ss << ":" << _pass; }
        _uri_ss << "@";
    }
    _uri_ss << _host << ":" << _port;
    if (!_db.empty()) { _uri_ss << "/" << _db; }

    try {
        auto _uri = mongocxx::uri{ _uri_ss.str() };
        this->__mongodb = std::make_shared<mongocxx::client>(_uri);
    }
    catch (mongocxx::exception const& _e) {
        expect(false, std::format("Unable to connect to MongoDB: {}", _e.what()));
    }

    return this;
}

auto zpt::storage::mongodb::connection::close() -> zpt::storage::connection::type* {
    this->__mongodb.reset();
    return this;
}

auto zpt::storage::mongodb::connection::session() -> zpt::storage::session {
    return zpt::make_session<zpt::storage::mongodb::session>(*this);
}

auto zpt::storage::mongodb::connection::options() const -> zpt::json { return this->__options; }

auto zpt::storage::mongodb::connection::mongodb() const -> mongodb_ptr { return this->__mongodb; }

// ---- Session ----

zpt::storage::mongodb::session::session(zpt::storage::mongodb::connection const& _connection)
  : __mongodb{ _connection.mongodb() } {}

auto zpt::storage::mongodb::session::is_open() const -> bool { return this->__mongodb != nullptr; }

auto zpt::storage::mongodb::session::commit() -> zpt::storage::session::type* { return this; }

auto zpt::storage::mongodb::session::rollback() -> zpt::storage::session::type* { return this; }

auto zpt::storage::mongodb::session::sql([[maybe_unused]] std::string const& _statement)
  -> zpt::storage::result {
    expect(false, "SQL is not supported for MongoDB");
    mongodb_cursor_ptr _cursor{ nullptr };
    return zpt::make_result<zpt::storage::mongodb::result>(_cursor);
}

auto zpt::storage::mongodb::session::database(std::string const& _db) const
  -> zpt::storage::database {
    return zpt::make_database<zpt::storage::mongodb::database>(*this, _db);
}

auto zpt::storage::mongodb::session::mongodb() const -> mongodb_ptr { return this->__mongodb; }

// ---- Database ----

zpt::storage::mongodb::database::database(zpt::storage::mongodb::session const& _session,
                                          std::string const& _db)
  : __mongodb{ _session.mongodb() }
  , __db{ _db } {}

auto zpt::storage::mongodb::database::sql([[maybe_unused]] std::string const& _statement)
  -> zpt::storage::result {
    expect(false, "SQL is not supported for MongoDB");
    mongodb_cursor_ptr _cursor{ nullptr };
    return zpt::make_result<zpt::storage::mongodb::result>(_cursor);
}

auto zpt::storage::mongodb::database::collection(std::string const& _collection) const
  -> zpt::storage::collection {
    return zpt::make_collection<zpt::storage::mongodb::collection>(*this, _collection);
}

auto zpt::storage::mongodb::database::db() const -> std::string const& { return this->__db; }

auto zpt::storage::mongodb::database::mongodb() const -> mongodb_ptr { return this->__mongodb; }

// ---- Collection ----

zpt::storage::mongodb::collection::collection(zpt::storage::mongodb::database const& _database,
                                              std::string const& _collection)
  : __mongodb{ _database.mongodb() }
  , __collection{ _collection }
  , __db{ _database.db() } {}

auto zpt::storage::mongodb::collection::add(zpt::json _document) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mongodb::action_add>(*this, _document);
}

auto zpt::storage::mongodb::collection::modify(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mongodb::action_modify>(*this, _search);
}

auto zpt::storage::mongodb::collection::remove(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mongodb::action_remove>(*this, _search);
}

auto zpt::storage::mongodb::collection::replace(std::string const& _id, zpt::json _document) const
  -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mongodb::action_replace>(*this, _id, _document);
}

auto zpt::storage::mongodb::collection::find(zpt::json _search) const -> zpt::storage::action {
    return zpt::make_action<zpt::storage::mongodb::action_find>(*this, _search);
}

auto zpt::storage::mongodb::collection::count(zpt::json _search) -> size_t {
    try {
        auto _coll = (*this->__mongodb)[this->__db][this->__collection];
        return static_cast<size_t>(
          _coll.count_documents(zpt::storage::mongodb::to_filter(_search).view()));
    }
    catch (mongocxx::exception const& _e) {
        expect(false, std::format("count failed: {}", _e.what()));
    }
    return 0;
}

auto zpt::storage::mongodb::collection::coll() const -> std::string const& {
    return this->__collection;
}

auto zpt::storage::mongodb::collection::db() const -> std::string const& { return this->__db; }

auto zpt::storage::mongodb::collection::mongodb() const -> mongodb_ptr { return this->__mongodb; }

// ---- Action (base) ----

zpt::storage::mongodb::action::action(zpt::storage::mongodb::collection const& _collection)
  : __mongodb{ _collection.mongodb() }
  , __collection{ _collection.coll() }
  , __db{ _collection.db() } {}

auto zpt::storage::mongodb::action::cursor() const -> mongodb_cursor_ptr { return this->__cursor; }

auto zpt::storage::mongodb::action::mongodb() const -> mongodb_ptr { return this->__mongodb; }

// ---- action_add ----

zpt::storage::mongodb::action_add::action_add(zpt::storage::mongodb::collection const& _collection,
                                              zpt::json _document)
  : zpt::storage::mongodb::action::action{ _collection }
  , __underlying{ zpt::json::array() }
  , __generated_ids{ zpt::json::array() } {
    this->__underlying << _document;
}

auto zpt::storage::mongodb::action_add::add(zpt::json _document) -> zpt::storage::action::type* {
    this->__underlying << _document;
    return this;
}

auto zpt::storage::mongodb::action_add::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from an 'add' action");
    return this;
}

auto zpt::storage::mongodb::action_add::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from an 'add' action");
    return this;
}

auto zpt::storage::mongodb::action_add::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from an 'add' action");
    return this;
}

auto zpt::storage::mongodb::action_add::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from an 'add' action");
    return this;
}

auto zpt::storage::mongodb::action_add::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_add::unset(std::string const&) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_add::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_add::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_add::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_add::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_add::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_add::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_add::execute() -> zpt::storage::result {
    try {
        auto _coll = (*this->__mongodb)[this->__db][this->__collection];
        for (auto&& [_, __, _record] : this->__underlying) {
            if (!_record("_id")->ok()) {
                auto _id = zpt::uuid{}.to_base64_string();
                _record << "_id" << _id;
                this->__generated_ids << _id;
            }
            _coll.insert_one(zpt::storage::mongodb::to_bson(_record).view());
        }
    }
    catch (mongocxx::exception const& _e) {
        expect(false, std::format("INSERT failed: {}", _e.what()));
    }
    return zpt::make_result<zpt::storage::mongodb::result>(*this);
}

auto zpt::storage::mongodb::action_add::get_generated_ids() const -> zpt::json {
    return this->__generated_ids;
}

// ---- action_modify ----

zpt::storage::mongodb::action_modify::action_modify(
  zpt::storage::mongodb::collection const& _collection,
  zpt::json _search)
  : zpt::storage::mongodb::action::action{ _collection }
  , __underlying{ zpt::json::object() }
  , __filter{ _search } {}

auto zpt::storage::mongodb::action_modify::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'modify' action");
    return this;
}

auto zpt::storage::mongodb::action_modify::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'modify' action");
    return this;
}

auto zpt::storage::mongodb::action_modify::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'modify' action");
    return this;
}

auto zpt::storage::mongodb::action_modify::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'modify' action");
    return this;
}

auto zpt::storage::mongodb::action_modify::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'modify' action");
    return this;
}

auto zpt::storage::mongodb::action_modify::set(std::string const& _attribute, zpt::json _value)
  -> zpt::storage::action::type* {
    this->__underlying << _attribute << _value;
    return this;
}

auto zpt::storage::mongodb::action_modify::unset(std::string const& _attribute)
  -> zpt::storage::action::type* {
    this->__underlying << _attribute << zpt::undefined;
    return this;
}

auto zpt::storage::mongodb::action_modify::patch(zpt::json _document)
  -> zpt::storage::action::type* {
    this->__underlying += _document;
    return this;
}

auto zpt::storage::mongodb::action_modify::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    expect(false, "can't sort from a 'modify' action");
    return this;
}

auto zpt::storage::mongodb::action_modify::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_modify::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_modify::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_modify::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_modify::execute() -> zpt::storage::result {
    try {
        auto _coll = (*this->__mongodb)[this->__db][this->__collection];
        auto _res =
          _coll.update_many(zpt::storage::mongodb::to_filter(this->__filter).view(),
                            zpt::storage::mongodb::to_update_doc(this->__underlying).view());
        this->__affected = _res ? static_cast<size_t>(_res->modified_count()) : 0;
    }
    catch (mongocxx::exception const& _e) {
        expect(false, std::format("UPDATE failed: {}", _e.what()));
    }
    return zpt::make_result<zpt::storage::mongodb::result>(*this);
}

auto zpt::storage::mongodb::action_modify::get_affected() const -> size_t {
    return this->__affected;
}

// ---- action_remove ----

zpt::storage::mongodb::action_remove::action_remove(
  zpt::storage::mongodb::collection const& _collection,
  zpt::json _search)
  : zpt::storage::mongodb::action::action{ _collection }
  , __filter{ _search } {}

auto zpt::storage::mongodb::action_remove::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'remove' action");
    return this;
}

auto zpt::storage::mongodb::action_remove::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'remove' action");
    return this;
}

auto zpt::storage::mongodb::action_remove::remove(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'remove' action");
    return this;
}

auto zpt::storage::mongodb::action_remove::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'remove' action");
    return this;
}

auto zpt::storage::mongodb::action_remove::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::unset(std::string const&)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_remove::execute() -> zpt::storage::result {
    try {
        auto _coll = (*this->__mongodb)[this->__db][this->__collection];
        auto _res = _coll.delete_many(zpt::storage::mongodb::to_filter(this->__filter).view());
        this->__affected = _res ? static_cast<size_t>(_res->deleted_count()) : 0;
    }
    catch (mongocxx::exception const& _e) {
        expect(false, std::format("DELETE failed: {}", _e.what()));
    }
    return zpt::make_result<zpt::storage::mongodb::result>(*this);
}

auto zpt::storage::mongodb::action_remove::get_affected() const -> size_t {
    return this->__affected;
}

// ---- action_replace ----

zpt::storage::mongodb::action_replace::action_replace(
  zpt::storage::mongodb::collection const& _collection,
  std::string _id,
  zpt::json _document)
  : zpt::storage::mongodb::action::action{ _collection }
  , __id{ std::move(_id) }
  , __underlying{ _document } {
    this->__underlying << "_id" << this->__id;
}

auto zpt::storage::mongodb::action_replace::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'replace' action");
    return this;
}

auto zpt::storage::mongodb::action_replace::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'replace' action");
    return this;
}

auto zpt::storage::mongodb::action_replace::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'replace' action");
    return this;
}

auto zpt::storage::mongodb::action_replace::replace(std::string const& _id, zpt::json _document)
  -> zpt::storage::action::type* {
    this->__id = _id;
    this->__underlying = _document;
    this->__underlying << "_id" << this->__id;
    return this;
}

auto zpt::storage::mongodb::action_replace::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'replace' action");
    return this;
}

auto zpt::storage::mongodb::action_replace::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_replace::unset(std::string const&)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_replace::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_replace::sort(std::string const&, bool)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_replace::fields(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_replace::offset(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_replace::limit(size_t) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_replace::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_replace::execute() -> zpt::storage::result {
    try {
        auto _coll = (*this->__mongodb)[this->__db][this->__collection];
        mongocxx::options::replace _opts;
        _opts.upsert(true);
        auto _filter = zpt::storage::mongodb::to_bson(zpt::json{ "_id", this->__id });
        _coll.replace_one(
          _filter.view(), zpt::storage::mongodb::to_bson(this->__underlying).view(), _opts);
    }
    catch (mongocxx::exception const& _e) {
        expect(false, std::format("REPLACE failed: {}", _e.what()));
    }
    return zpt::make_result<zpt::storage::mongodb::result>(*this);
}

// ---- action_find ----

zpt::storage::mongodb::action_find::action_find(
  zpt::storage::mongodb::collection const& _collection)
  : zpt::storage::mongodb::action::action{ _collection } {}

zpt::storage::mongodb::action_find::action_find(
  zpt::storage::mongodb::collection const& _collection,
  zpt::json _search)
  : zpt::storage::mongodb::action::action{ _collection }
  , __filter{ _search }
  , __fields{ zpt::json::array() }
  , __suffix{ zpt::json::object() } {}

auto zpt::storage::mongodb::action_find::add(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't add from a 'find' action");
    return this;
}

auto zpt::storage::mongodb::action_find::modify(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't modify from a 'find' action");
    return this;
}

auto zpt::storage::mongodb::action_find::remove(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't remove from a 'find' action");
    return this;
}

auto zpt::storage::mongodb::action_find::replace(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    expect(false, "can't replace from a 'find' action");
    return this;
}

auto zpt::storage::mongodb::action_find::find(zpt::json) -> zpt::storage::action::type* {
    expect(false, "can't find from a 'find' action");
    return this;
}

auto zpt::storage::mongodb::action_find::set(std::string const&, zpt::json)
  -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_find::unset(std::string const&) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_find::patch(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_find::sort(std::string const& _attribute, bool asc)
  -> zpt::storage::action::type* {
    this->__suffix["sort"][_attribute] = (asc ? "asc" : "desc");
    return this;
}

auto zpt::storage::mongodb::action_find::fields(zpt::json _fields) -> zpt::storage::action::type* {
    this->__fields += _fields;
    return this;
}

auto zpt::storage::mongodb::action_find::offset(size_t _rows) -> zpt::storage::action::type* {
    this->__suffix["skip"] = _rows;
    return this;
}

auto zpt::storage::mongodb::action_find::limit(size_t _number) -> zpt::storage::action::type* {
    this->__suffix["limit"] = _number;
    return this;
}

auto zpt::storage::mongodb::action_find::bind(zpt::json) -> zpt::storage::action::type* {
    return this;
}

auto zpt::storage::mongodb::action_find::execute() -> zpt::storage::result {
    try {
        auto _coll = (*this->__mongodb)[this->__db][this->__collection];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        mongocxx::options::find _opts{};

        if (this->__fields->ok() && this->__fields->size() != 0) {
            _opts.projection(zpt::storage::mongodb::to_projection(this->__fields).view());
        }
        if (this->__suffix["sort"]->ok()) { _opts.sort(to_sort(this->__suffix["sort"]).view()); }
        if (this->__suffix["limit"]->ok()) {
            _opts.limit(static_cast<int64_t>(this->__suffix["limit"]->integer()));
        }
        if (this->__suffix["skip"]->ok()) {
            _opts.skip(static_cast<int64_t>(this->__suffix["skip"]->integer()));
        }
#pragma GCC diagnostic pop

        auto _cursor = _coll.find(to_filter(this->__filter).view(), _opts);
        this->__cursor = std::make_shared<mongocxx::cursor>(std::move(_cursor));
    }
    catch (mongocxx::exception const& _e) {
        expect(false, std::format("FIND failed: {}", _e.what()));
    }
    return zpt::make_result<zpt::storage::mongodb::result>(*this);
}

// ---- result ----
zpt::storage::mongodb::result::result(mongodb_cursor_ptr _cursor)
  : __cursor{ _cursor } {}

zpt::storage::mongodb::result::result(zpt::storage::mongodb::action& _action)
  : zpt::storage::mongodb::result{ _action.cursor() } {}

zpt::storage::mongodb::result::result(zpt::storage::mongodb::action_add& _action)
  : zpt::storage::mongodb::result{ static_cast<zpt::storage::mongodb::action&>(_action) } {
    this->__generated_ids = _action.get_generated_ids();
}

zpt::storage::mongodb::result::result(zpt::storage::mongodb::action_modify& _action)
  : zpt::storage::mongodb::result{ static_cast<zpt::storage::mongodb::action&>(_action) } {
    this->__affected = _action.get_affected();
}

zpt::storage::mongodb::result::result(zpt::storage::mongodb::action_remove& _action)
  : zpt::storage::mongodb::result{ static_cast<zpt::storage::mongodb::action&>(_action) } {
    this->__affected = _action.get_affected();
}

zpt::storage::mongodb::result::result(zpt::storage::mongodb::action_replace& _action)
  : zpt::storage::mongodb::result{ static_cast<zpt::storage::mongodb::action&>(_action) } {}

zpt::storage::mongodb::result::result(zpt::storage::mongodb::action_find& _action)
  : zpt::storage::mongodb::result{ static_cast<zpt::storage::mongodb::action&>(_action) } {
    this->__is_cursor_result = true;
}

auto zpt::storage::mongodb::result::fetch(size_t _amount) -> zpt::json {
    zpt::json _result = zpt::json::array();
    if (!this->__cursor) { return _result; }
    if (_amount == 0) { _amount = std::numeric_limits<size_t>::max(); }

    size_t _fetched{ 0 };
    for (auto const& _doc : *this->__cursor) {
        if (_fetched >= _amount) { break; }
        _result << zpt::storage::mongodb::from_bson(_doc);
        ++_fetched;
    }
    return _result;
}

auto zpt::storage::mongodb::result::generated_id() -> zpt::json { return this->__generated_ids; }

auto zpt::storage::mongodb::result::count() const -> size_t { return this->__affected; }

auto zpt::storage::mongodb::result::status() const -> zpt::status { return 0; }

auto zpt::storage::mongodb::result::message() const -> std::string { return {}; }

auto zpt::storage::mongodb::result::to_json() const -> zpt::json {
    return { "state", { "code", this->status(), "message", this->message() } };
}
