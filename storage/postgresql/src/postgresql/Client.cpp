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

#include <zapata/postgresql/Client.h>

zpt::pgsql::ClientPtr::ClientPtr(zpt::pgsql::Client* _target)
  : std::shared_ptr<zpt::pgsql::Client>(_target) {}

zpt::pgsql::ClientPtr::ClientPtr(zpt::json _options, std::string const& _conf_path)
  : std::shared_ptr<zpt::pgsql::Client>(new zpt::pgsql::Client(_options, _conf_path)) {}

zpt::pgsql::ClientPtr::~ClientPtr() {}

zpt::pgsql::Client::Client(zpt::json _options, std::string const& _conf_path)
  : __options(_options)
  , __conn(nullptr) {
    this->connection(_options->get_path(_conf_path));
}

zpt::pgsql::Client::~Client() {}

auto
zpt::pgsql::Client::conn() -> pqxx::connection& {
    return (*this->__conn.get());
}

auto
zpt::pgsql::Client::name() -> std::string {
    return std::string("pgsql://") + ((std::string)this->connection()["bind"]) + std::string("/") +
           ((std::string)this->connection()["db"]);
}

auto
zpt::pgsql::Client::options() -> zpt::json {
    return this->__options;
}

auto
zpt::pgsql::Client::events(zpt::ev::emitter _emitter) -> void {
    this->__events = _emitter;
}

auto
zpt::pgsql::Client::events() -> zpt::ev::emitter {
    return this->__events;
}

auto
zpt::pgsql::Client::connect() -> void {
    std::lock_guard<std::mutex> _lock(this->__mtx);
    std::string _s_conn =
      this->connection()["bind"]->string() + std::string(" dbname=") + this->connection()["db"]->string() +
      (this->connection()["user"]->ok() ? std::string(" user=") + this->connection()["user"]->string() +
                                            std::string(" password=") + this->connection()["passwd"]->string()
                                        : "") +
      std::string("");
    this->__conn.reset(new pqxx::connection(_s_conn));
    zpt::Connector::connect();
}

auto
zpt::pgsql::Client::reconnect() -> void {
    std::lock_guard<std::mutex> _lock(this->__mtx);
    expect(this->__conn.get() != nullptr,
           std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    this->__conn.release();
    this->__conn.reset(new pqxx::connection(
      this->connection()["bind"]->string() + std::string(" dbname=") + this->connection()["db"]->string() +
      (this->connection()["user"]->ok() ? std::string(" user=") + this->connection()["user"]->string() +
                                            std::string(" password=") + this->connection()["passwd"]->string()
                                        : "") +
      std::string("")));
    zpt::Connector::reconnect();
}

auto
zpt::pgsql::Client::insert(std::string const& _collection,
                           std::string _href_prefix,
                           zpt::json _document,
                           zpt::json _opts) -> std::string {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }
    expect(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject");

    if (!_document["id"]->ok()) { _document << "id" << zpt::generate::r_uuid(); }
    if (!_document["href"]->ok() && _href_prefix.length() != 0) {
        _document << "href"
                  << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                      _document["id"]->string());
    }

    std::string _expression("INSERT INTO ");
    _expression += zpt::pgsql::escape_name(_collection);
    _expression += std::string(" (");
    std::string _columns = zpt::pgsql::get_column_names(_document, _opts);
    std::string _values = zpt::pgsql::get_column_values(_document, _opts);
    _expression += _columns + std::string(") VALUES (") + _values + (")");
    int _size = 0;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _size = _stmt.exec(_expression).affected_rows();
            _stmt.commit();
        }
    }
    psql_catch_block(1200);
    if (_size != 0 && !bool(_opts["mutated-event"]))
        zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
    return _document["id"]->string();
}

auto
zpt::pgsql::Client::upsert(std::string const& _collection,
                           std::string _href_prefix,
                           zpt::json _document,
                           zpt::json _opts) -> std::string {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }
    expect(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject");

    {
        if (_document["href"]->ok() || _document["id"]->ok()) {
            if (!_document["href"]->ok()) {
                _document << "href"
                          << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                              _document["id"]->string());
            }
            if (!_document["id"]->ok()) {
                zpt::json _split = zpt::split(_document["href"]->string(), "/");
                _document << "id" << _split->array()->back();
            }
            std::string _href = std::string(_document["href"]);
            std::string _expression("UPDATE ");
            _expression += zpt::pgsql::escape_name(_collection);
            _expression += std::string(" SET ");
            std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
            _expression += _sets;

            zpt::json _splited = zpt::split(_href, "/");
            _expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->array()->back());

            int _size = 0;
            try {
                {
                    std::lock_guard<std::mutex> _lock(this->__mtx);
                    pqxx::work _stmt(this->conn());
                    _size = _stmt.exec(_expression).affected_rows();
                    _stmt.commit();
                }
            }
            psql_catch_block(1200);

            if (_size != 0) {
                if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
                return _document["id"]->string();
            }
        }
    }
    {
        if (!_document["id"]->ok()) { _document << "id" << zpt::generate::r_uuid(); }
        if (!_document["href"]->ok() && _href_prefix.length() != 0) {
            _document << "href"
                      << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) +
                          _document["id"]->string());
        }

        std::string _expression("INSERT INTO ");
        _expression += zpt::pgsql::escape_name(_collection);
        _expression += std::string(" (");
        std::string _columns = zpt::pgsql::get_column_names(_document, _opts);
        std::string _values = zpt::pgsql::get_column_values(_document, _opts);
        _expression += _columns + std::string(") VALUES (") + _values + (")");
        int _size = 0;
        try {
            {
                std::lock_guard<std::mutex> _lock(this->__mtx);
                pqxx::work _stmt(this->conn());
                _size = _stmt.exec(_expression).affected_rows();
                _stmt.commit();
            }
        }
        psql_catch_block(1200);
        if (_size != 0 && !bool(_opts["mutated-event"]))
            zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
    }
    return _document["id"]->string();
}

auto
zpt::pgsql::Client::save(std::string const& _collection, std::string _href, zpt::json _document, zpt::json _opts)
  -> int {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."),
               500,
               0);
    }
    expect(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject");

    std::string _expression("UPDATE ");
    _expression += zpt::pgsql::escape_name(_collection);
    _expression += std::string(" SET ");
    std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
    _expression += _sets;

    zpt::json _splited = zpt::split(_href, "/");
    _expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->array()->back());

    int _size = 0;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _size = _stmt.exec(_expression).affected_rows();
            _stmt.commit();
        }
    }
    psql_catch_block(1200);

    if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::save(_collection, _href, _document, _opts);
    return _size;
}

auto
zpt::pgsql::Client::set(std::string const& _collection, std::string _href, zpt::json _document, zpt::json _opts)
  -> int {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }
    expect(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject");

    std::string _expression("UPDATE ");
    _expression += zpt::pgsql::escape_name(_collection);
    _expression += std::string(" SET ");
    std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
    _expression += _sets;

    zpt::json _splited = zpt::split(_href, "/");
    _expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->array()->back());

    int _size = 0;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _size = _stmt.exec(_expression).affected_rows();
            _stmt.commit();
        }
    }
    psql_catch_block(1200);

    if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
    return _size;
}

auto
zpt::pgsql::Client::set(std::string const& _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts)
  -> int {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }
    expect(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject");

    std::string _expression("UPDATE ");
    _expression += zpt::pgsql::escape_name(_collection);
    _expression += std::string(" SET ");
    std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
    _expression += _sets;

    if (_pattern->ok() && _pattern->type() == zpt::JSObject) {
        std::string _where;
        zpt::pgsql::get_query(_pattern, _where);
        _expression += std::string(" WHERE ") + _where;
    }
    zpt::pgsql::get_opts(_opts, _expression);

    int _size = 0;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _size = _stmt.exec(_expression).affected_rows();
            _stmt.commit();
        }
    }
    psql_catch_block(1200);

    if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _pattern, _document, _opts);
    return _size;
}

auto
zpt::pgsql::Client::unset(std::string const& _collection, std::string _href, zpt::json _document, zpt::json _opts)
  -> int {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }
    expect(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject");

    std::string _expression("UPDATE ");
    _expression += zpt::pgsql::escape_name(_collection);
    _expression += std::string(" SET ");
    std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
    _expression += _sets;

    zpt::json _splited = zpt::split(_href, "/");
    _expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->array()->back());

    int _size = 0;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _size = _stmt.exec(_expression).affected_rows();
            _stmt.commit();
        }
    }
    psql_catch_block(1200);

    if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _href, _document, _opts);
    return _size;
}

auto
zpt::pgsql::Client::unset(std::string const& _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts)
  -> int {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }
    expect(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject");

    std::string _expression("UPDATE ");
    _expression += zpt::pgsql::escape_name(_collection);
    _expression += std::string(" SET ");
    std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
    _expression += _sets;

    if (_pattern->ok() && _pattern->type() == zpt::JSObject) {
        std::string _where;
        zpt::pgsql::get_query(_pattern, _where);
        _expression += std::string(" WHERE ") + _where;
    }
    zpt::pgsql::get_opts(_opts, _expression);

    int _size = 0;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _size = _stmt.exec(_expression).affected_rows();
            _stmt.commit();
        }
    }
    psql_catch_block(1200);

    if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _pattern, _document, _opts);
    return _size;
}

auto
zpt::pgsql::Client::remove(std::string const& _collection, std::string const& _href, zpt::json _opts) -> int {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }

    std::string _expression("DELETE FROM ");
    _expression += zpt::pgsql::escape_name(_collection);

    zpt::json _splited = zpt::split(_href, "/");
    _expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->array()->back());

    zpt::json _removed;
    if (!bool(_opts["mutated-event"])) _removed = this->get(_collection, _href);

    int _size = 0;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _size = _stmt.exec(_expression).affected_rows();
            _stmt.commit();
        }
    }
    psql_catch_block(1200);

    if (_size != 0 && !bool(_opts["mutated-event"]))
        zpt::Connector::remove(_collection, _href, _opts + zpt::json{ "removed", _removed });
    return _size;
}

auto
zpt::pgsql::Client::remove(std::string const& _collection, zpt::json _pattern, zpt::json _opts) -> int {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }

    zpt::json _selected = this->query(_collection, _pattern, _opts);
    if (!_selected->ok()) { return 0; }
    for (auto _record : _selected["elements"]->array()) {
        std::string _expression = std::string("DELETE FROM ") + zpt::pgsql::escape_name(_collection) +
                                  std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_record["id"]);
        int _size = 0;
        try {
            {
                std::lock_guard<std::mutex> _lock(this->__mtx);
                pqxx::work _stmt(this->conn());
                _size = _stmt.exec(_expression).affected_rows();
                _stmt.commit();
            }
        }
        psql_catch_block(1200);

        if (_size != 0 && !bool(_opts["mutated-event"]))
            zpt::Connector::remove(_collection, _record["href"]->string(), _opts + zpt::json{ "removed", _record });
    }

    return int(_selected["size"]);
}

auto
zpt::pgsql::Client::get(std::string const& _collection, std::string const& _href, zpt::json _opts) -> zpt::json {
    std::string _expression("SELECT ");
    _expression += zpt::pgsql::get_column_names(zpt::undefined, _opts);
    _expression += std::string(" FROM ");
    _expression += zpt::pgsql::escape_name(_collection);
    zpt::json _splited = zpt::split(_href, "/");
    _expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->array()->back());
    return this->query(_collection, _expression, _opts)["elements"][0];
}

auto
zpt::pgsql::Client::query(std::string const& _collection, std::string const& _pattern, zpt::json _opts) -> zpt::json {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn.get() != nullptr,
               std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."));
    }
    zpt::json _elements = zpt::json::array();

    try {
        pqxx::result _result;
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _result = _stmt.exec(_pattern);
        }
        for (auto _r : _result) { _elements << zpt::pgsql::fromsql_r(_r); }
    }
    psql_catch_block(1200);
    if (_elements->array()->size() == 0) { return zpt::undefined; }
    return { "size", _elements->array()->size(), "elements", _elements };
}

auto
zpt::pgsql::Client::query(std::string const& _collection, zpt::json _pattern, zpt::json _opts) -> zpt::json {
    std::string _expression("SELECT ");
    _expression += zpt::pgsql::get_column_names(zpt::undefined, _opts);
    _expression += std::string(" FROM ");
    std::string _count_expression("SELECT COUNT(1) FROM ");
    _expression += zpt::pgsql::escape_name(_collection);
    _count_expression += zpt::pgsql::escape_name(_collection);
    if (_pattern->is_object()) {
        std::string _where;
        zpt::pgsql::get_query(_pattern, _where);
        if (_where.length() != 0) {
            _expression += std::string(" WHERE ") + _where;
            _count_expression += std::string(" WHERE ") + _where;
        }
    }
    zpt::pgsql::get_opts(_opts, _expression);

    pqxx::result _result;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _result = _stmt.exec(_count_expression);
        }
    }
    psql_catch_block(1200);
    size_t _size = 0;
    for (auto _r : _result) { _size = size_t(zpt::pgsql::fromsql_r(_r)["count"]); }
    if (_size == 0) { return zpt::undefined; }

    zpt::json _return = this->query(_collection, _expression, _opts);
    if (_return->ok()) { _return << "size" << _size; }
    return _return;
}

auto
zpt::pgsql::Client::all(std::string const& _collection, zpt::json _opts) -> zpt::json {
    std::string _expression("SELECT ");
    _expression += zpt::pgsql::get_column_names(zpt::undefined, _opts);
    _expression += std::string(" FROM ");
    std::string _count_expression("SELECT COUNT(1) FROM ");
    _expression += zpt::pgsql::escape_name(_collection);
    _count_expression += zpt::pgsql::escape_name(_collection);
    zpt::pgsql::get_opts(_opts, _expression);

    pqxx::result _result;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            pqxx::work _stmt(this->conn());
            _result = _stmt.exec(_count_expression);
        }
    }
    psql_catch_block(1200);
    size_t _size = 0;
    for (auto _r : _result) { _size = size_t(zpt::pgsql::fromsql_r(_r)["count"]); }
    if (_size == 0) { return zpt::undefined; }

    zpt::json _return = this->query(_collection, _expression, _opts);
    if (_return->ok()) { _return << "size" << _size; }
    return _return;
}

extern "C" auto
zpt_postgresql() -> int {
    return 1;
}
