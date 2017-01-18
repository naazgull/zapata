/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

auto zpt::Connector::connect(zpt::json _opts) -> void {
	this->mutations()->trigger(zpt::mutation::Connect, this->name(), { "node", { "ip", "127.0.0.1" } });	
}

auto zpt::Connector::reconnect() -> void {
	this->mutations()->trigger(zpt::mutation::Reconnect, this->name(), { "node", { "ip", "127.0.0.1" } });	
}

auto zpt::Connector::insert(std::string _collection, std::string _id_prefix, zpt::json _record, zpt::json _opts) -> std::string {
	assertz(_record["_id"]->ok(), "required fields: '_id'", 412, 0);
	this->mutations()->trigger(zpt::mutation::Insert, std::string("/") + _collection, { "lambda", "insert", "id", _record["_id"], "new", _record });
	return std::string(_record["id"]);
}

auto zpt::Connector::save(std::string _collection, std::string _id, zpt::json _record, zpt::json _opts) -> int {
	this->mutations()->trigger(zpt::mutation::Replace, std::string("/") + _collection + _id, { "lambda", "save", "id", _id, "new", _record });
	return 0;
}

auto zpt::Connector::set(std::string _collection, std::string _id, zpt::json _record, zpt::json _opts) -> int {
	this->mutations()->trigger(zpt::mutation::Update, std::string("/") + _collection + _id, { "lambda", "set", "id", _id, "changes", _record });
	return 0;
}

auto zpt::Connector::set(std::string _collection, zpt::json _pattern, zpt::json _record, zpt::json _opts) -> int {
	this->mutations()->trigger(zpt::mutation::Update, std::string("/") + _collection, { "lambda", "set", "pattern", _pattern, "changes", _record });
	return 0;
}

auto zpt::Connector::unset(std::string _collection, std::string _id, zpt::json _record, zpt::json _opts) -> int {
	this->mutations()->trigger(zpt::mutation::Update, std::string("/") + _collection + _id, { "lambda", "unset", "id", _id, "changes", _record });
	return 0;
}

auto zpt::Connector::unset(std::string _collection, zpt::json _pattern, zpt::json _record, zpt::json _opts) -> int {
	this->mutations()->trigger(zpt::mutation::Update, std::string("/") + _collection, { "lambda", "unset", "pattern", _pattern, "changes", _record });
	return 0;
}

auto zpt::Connector::remove(std::string _collection, std::string _id, zpt::json _opts) -> int {
	this->mutations()->trigger(zpt::mutation::Remove, std::string("/") + _collection + _id, { "lambda", "remove", "id", _id });
	return 0;
}

auto zpt::Connector::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
	this->mutations()->trigger(zpt::mutation::Remove, std::string("/") + _collection, { "lambda", "remove", "pattern", _pattern });
	return 0;
}

auto zpt::Connector::get(std::string _collection, std::string _id, zpt::json _opts) -> zpt::json {
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

zpt::MutationEmitter::MutationEmitter() : __self( this ) {
}

zpt::MutationEmitter::MutationEmitter(zpt::json _options) :  __options( _options), __self( this ) {
}

zpt::MutationEmitter::~MutationEmitter() {
}

auto zpt::MutationEmitter::options() -> zpt::json {
	return this->__options;
}
					 
auto zpt::MutationEmitter::self() -> zpt::mutation::emitter {
	return this->__self;
}

auto zpt::MutationEmitter::connector(std::string _name, zpt::connector _connector) -> void {
	auto _found = this->__connector.find(_name);
	if (_found == this->__connector.end()) {
		_connector->mutations(this->__self);
		this->__connector.insert(make_pair(_name, _connector));
	}
}

auto zpt::MutationEmitter::connector(std::string _name) -> zpt::connector {
	auto _found = this->__connector.find(_name);
	if (_found == this->__connector.end()) {
		return zpt::connector(nullptr);
	}
	return _found->second;
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
