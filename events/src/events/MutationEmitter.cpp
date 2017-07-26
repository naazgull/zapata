/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <zapata/events/EventEmitter.h>

zpt::Connector::Connector() {
}

zpt::Connector::~Connector() {
}

auto zpt::Connector::connection() -> zpt::json {
	return this->__connection;
}

auto zpt::Connector::connection(zpt::json _conn_conf) -> void {
	this->__connection = _conn_conf;
}

auto zpt::Connector::connect() -> void {
	this->mutations()->route(zpt::mutation::Connect, std::string("/") + zpt::r_replace(this->name(), "://", "/"), { "id", this->name(), "node", this->connection() });	
}

auto zpt::Connector::reconnect() -> void {
	this->mutations()->route(zpt::mutation::Reconnect, std::string("/") + zpt::r_replace(this->name(), "://", "/"), { "id", this->name(), "node", this->connection() });	
}

auto zpt::Connector::insert(std::string _collection, std::string _href_prefix, zpt::json _record, zpt::json _opts) -> std::string {
	if (bool(_opts["mutated-event"])) return "";
	assertz(_record["href"]->ok(), "required fields: 'href'", 412, 0);
	this->mutations()->route(zpt::mutation::Insert, std::string("/") + _collection, { "headers", _opts["headers"], "performative", "insert", "href", _record["href"], "new", _record });
	return std::string(_record["href"]);
}

auto zpt::Connector::upsert(std::string _collection, std::string _href_prefix, zpt::json _record, zpt::json _opts) -> std::string {
	return std::string(_record["href"]);
}

auto zpt::Connector::save(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts) -> int {
	if (bool(_opts["mutated-event"])) return 0;
	this->mutations()->route(zpt::mutation::Replace, std::string("/") + _collection, { "headers", _opts["headers"], "performative", "save", "href", _href, "new", _record });
	return 0;
}

auto zpt::Connector::set(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts) -> int {
	if (bool(_opts["mutated-event"])) return 0;
	this->mutations()->route(zpt::mutation::Update, std::string("/") + _collection, { "headers", _opts["headers"], "performative", "set", "href", _href, "changes", _record });
	return 0;
}

auto zpt::Connector::set(std::string _collection, zpt::json _pattern, zpt::json _record, zpt::json _opts) -> int {
	if (bool(_opts["mutated-event"])) return 0;
	this->mutations()->route(zpt::mutation::Update, std::string("/") + _collection, { "headers", _opts["headers"], "performative", "set", "href", _opts["href"], "changes", _record, "filter", _pattern });
	return 0;
}

auto zpt::Connector::unset(std::string _collection, std::string _href, zpt::json _record, zpt::json _opts) -> int {
	if (bool(_opts["mutated-event"])) return 0;
	this->mutations()->route(zpt::mutation::Update, std::string("/") + _collection, { "headers", _opts["headers"], "performative", "unset", "href", _href, "changes", _record });
	return 0;
}

auto zpt::Connector::unset(std::string _collection, zpt::json _pattern, zpt::json _record, zpt::json _opts) -> int {
	if (bool(_opts["mutated-event"])) return 0;
	this->mutations()->route(zpt::mutation::Update, std::string("/") + _collection, { "headers", _opts["headers"], "performative", "unset", "href", _opts["href"], "changes", _record, "filter", _pattern });
	return 0;
}

auto zpt::Connector::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {
	if (bool(_opts["mutated-event"])) return 0;
	this->mutations()->route(zpt::mutation::Remove, std::string("/") + _collection, { "headers", _opts["headers"], "performative", "remove", "href", _href, "removed", _opts["removed"] });
	return 0;
}

auto zpt::Connector::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
	if (bool(_opts["mutated-event"])) return 0;
	this->mutations()->route(zpt::mutation::Remove, std::string("/") + _collection, { "headers", _opts["headers"], "performative", "remove", "href", _opts["href"], "filter", _pattern, "removed", _opts["removed"] });
	return 0;
}

auto zpt::Connector::get(std::string _collection, std::string _href, zpt::json _opts) -> zpt::json {
	return zpt::undefined;
}
	 
auto zpt::Connector::query(std::string _collection, std::string _query, zpt::json _opts) -> zpt::json {
	return zpt::undefined;
}
		     
auto zpt::Connector::query(std::string _collection, zpt::json _query, zpt::json _opts) -> zpt::json {
	return zpt::undefined;
}
		     
auto zpt::Connector::all(std::string _collection, zpt::json _opts) -> zpt::json {
	return zpt::undefined;
}

zpt::MutationEmitter::MutationEmitter() : __self(this) {
}

zpt::MutationEmitter::MutationEmitter(zpt::json _options) :  __options(_options), __self(this), __events(nullptr) {
}

zpt::MutationEmitter::~MutationEmitter() {
}

auto zpt::MutationEmitter::options() -> zpt::json {
	return this->__options;
}
					 
auto zpt::MutationEmitter::self() const -> zpt::mutation::emitter {
	return this->__self;
}

auto zpt::MutationEmitter::events() -> zpt::ev::emitter {
	return this->__events;
}

auto zpt::MutationEmitter::events(zpt::ev::emitter _emitter) -> void {
	this->__events = _emitter;
}

auto zpt::MutationEmitter::unbind() -> void {
	this->__self.reset();
}

auto zpt::MutationEmitter::connector(std::string _name, zpt::connector _connector) -> void {
	auto _found = this->__connector.find(_name);
	if (_found == this->__connector.end()) {
		ztrace(std::string("registering connector ") + _name + std::string("@") + _connector->name());
		_connector->mutations(this->__self);
		try {
			_connector->connect();
		}
		catch(std::exception& _e) {
			zlog(_e.what(), zpt::error);
			return;
		}
		this->__connector.insert(make_pair(_name, _connector));
	}
}

auto zpt::MutationEmitter::connector(std::string _name) -> zpt::connector {
	auto _found = this->__connector.find(_name);
	assertz(_found != this->__connector.end(), std::string("theres isn't any connector by the name '") + _name + std::string("'"), 500, 0);
	return _found->second;
}

zpt::DefaultMutationEmitter::DefaultMutationEmitter(zpt::json _options) : zpt::MutationEmitter(_options) {
}

zpt::DefaultMutationEmitter::~DefaultMutationEmitter() {
}

auto zpt::DefaultMutationEmitter::version() -> std::string {
	return this->options()["rest"]["version"]->str();
}

auto zpt::DefaultMutationEmitter::on(zpt::mutation::operation _operation, std::string _data_class_ns,  zpt::mutation::Handler _handler, zpt::json _opts) -> std::string {
	return "";
}

auto zpt::DefaultMutationEmitter::on(std::string _data_class_ns,  std::map< zpt::mutation::operation, zpt::mutation::Handler > _handler_set, zpt::json _opts) -> std::string {
	return "";
}

auto zpt::DefaultMutationEmitter::on(zpt::mutation::listener _listener, zpt::json _opts) -> std::string {
	return "";
}

auto zpt::DefaultMutationEmitter::off(zpt::mutation::operation _operation, std::string _callback_id) -> void {
}

auto zpt::DefaultMutationEmitter::off(std::string _callback_id) -> void {
}		

auto zpt::DefaultMutationEmitter::route(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts) -> zpt::json {
	return zpt::undefined;
}

auto zpt::DefaultMutationEmitter::sync_route(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts) -> zpt::json {
	return zpt::undefined;
}

auto zpt::DefaultMutationEmitter::trigger(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts) -> zpt::json {
	return zpt::undefined;
}

zpt::MutationListener::MutationListener(std::string _data_class_ns) : __namespace(_data_class_ns) {
}

zpt::MutationListener::~MutationListener() {
}

auto zpt::MutationListener::ns() -> std::string {
	return this->__namespace;
}

auto zpt::MutationListener::inserted(std::string _data_class_ns, zpt::json _record, zpt::MutationEmitterPtr _emitter) -> void {	
}

auto zpt::MutationListener::removed(std::string _data_class_ns, zpt::json _record, zpt::MutationEmitterPtr _emitter) -> void {
}

auto zpt::MutationListener::updated(std::string _data_class_ns, zpt::json _record, zpt::MutationEmitterPtr _emitter) -> void {
}

auto zpt::MutationListener::replaced(std::string _data_class_ns, zpt::json _record, zpt::MutationEmitterPtr _emitter) -> void {
}

auto zpt::MutationListener::connected(std::string _data_class_ns, zpt::json _record, zpt::MutationEmitterPtr _emitter) -> void {
}

auto zpt::MutationListener::reconnected(std::string _data_class_ns, zpt::json _record, zpt::MutationEmitterPtr _emitter) -> void {
}
