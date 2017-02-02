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

#pragma once

#include <zapata/base.h>
#include <zapata/json.h>
#include <regex>
#include <string>
#include <map>
#include <memory>
#include <ossp/uuid++.hh>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class MutationEmitter;
	class MutationListener;
	class EventEmitter;
	class EventListener;
	class Bridge;
	class BridgePtr;

	typedef std::shared_ptr< zpt::MutationEmitter > MutationEmitterPtr;
	typedef std::shared_ptr< zpt::MutationListener > MutationListenerPtr;
	typedef std::weak_ptr< zpt::EventEmitter > EventEmitterWPtr;
	typedef std::shared_ptr< zpt::EventEmitter > EventEmitterPtr;
	typedef std::shared_ptr< zpt::EventListener > EventListenerPtr;

	typedef BridgePtr bridge;

	namespace ev {
		extern std::string* __default_authorization;

		typedef zpt::EventEmitterPtr emitter;
		typedef zpt::EventListenerPtr listener;

		typedef std::function<zpt::json (zpt::ev::performative, std::string, zpt::json, zpt::ev::emitter)> Handler;
		typedef std::function<zpt::json (zpt::ev::performative, std::string, zpt::json, zpt::ev::emitter)> handler;
		typedef Handler Callback;
		typedef handler callback;
		typedef std::map< std::string, std::pair<std::regex, std::vector< zpt::ev::handler> > > HandlerStack;
		typedef std::map< std::string, zpt::ev::handler > ReplyHandlerStack;

		auto split(std::string _url, zpt::json _orphans) -> zpt::json;
		auto join(zpt::json _info, std::size_t _orphans) -> std::string;

		auto set_default_authorization(std::string _default_authorization) -> void;
		auto get_default_authorization() -> std::string;

		auto init_request(std::string _cid = "") -> zpt::json;
		auto init_reply(std::string _cid = "") -> zpt::json;
	}

	namespace mutation {
		typedef zpt::MutationEmitterPtr emitter;
		typedef zpt::MutationListenerPtr listener;

		typedef std::function<void (zpt::mutation::operation, std::string, zpt::json, zpt::mutation::emitter)> Handler;
		typedef std::function<void (zpt::mutation::operation, std::string, zpt::json, zpt::mutation::emitter)> handler;
		typedef Handler Callback;
		typedef handler callback;
		typedef std::map< std::string, pair<std::regex, std::vector< zpt::mutation::handler > > > HandlerStack;
		typedef std::map< std::string, zpt::mutation::handler > ReplyHandlerStack;
	}

	class Connector {
	public:
		Connector();
		virtual ~Connector();
		
		virtual auto name() -> std::string = 0;
		virtual auto options() -> zpt::json = 0;
		virtual auto events(zpt::ev::emitter _emitter) -> void = 0;
		virtual auto events() -> zpt::ev::emitter = 0;
		virtual auto mutations(zpt::mutation::emitter _emitter) -> void = 0;
		virtual auto mutations() -> zpt::mutation::emitter = 0;

		virtual auto connect() -> void;
		virtual auto reconnect() -> void;

		virtual auto insert(std::string _collection, std::string _id_prefix, zpt::json _record, zpt::json _opts = zpt::undefined) -> std::string;
		virtual auto save(std::string _collection, std::string _id, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
		virtual auto set(std::string _collection, std::string _id, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
		virtual auto set(std::string _collection, zpt::json _query, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
		virtual auto unset(std::string _collection, std::string _id, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
		virtual auto unset(std::string _collection, zpt::json _query, zpt::json _record, zpt::json _opts = zpt::undefined) -> int;
		virtual auto remove(std::string _collection, std::string _id, zpt::json _opts = zpt::undefined) -> int;
		virtual auto remove(std::string _collection, zpt::json _query, zpt::json _opts = zpt::undefined) -> int;
		virtual auto get(std::string _collection, std::string _id, zpt::json _opts = zpt::undefined) -> zpt::json;
		virtual auto query(std::string _collection, std::string _query, zpt::json _opts = zpt::undefined) -> zpt::json;
		virtual auto query(std::string _collection, zpt::json _query, zpt::json _opts = zpt::undefined) -> zpt::json;
		virtual auto all(std::string _collection, zpt::json _opts = zpt::undefined) -> zpt::json;

	protected:
		virtual auto connection() -> zpt::json;
		virtual auto connection(zpt::json _conn_conf) -> void;

	private:
		zpt::json __connection;
	};
	typedef std::shared_ptr<zpt::Connector> ConnectorPtr;
	typedef ConnectorPtr connector;

	class BridgePtr : public std::shared_ptr< zpt::Bridge > {
	public:
		BridgePtr(zpt::Bridge* _target);
		BridgePtr();
		
		template< typename B >
		static inline auto instance() -> zpt::bridge {
			return B::instance();
		};
		
		template< typename B >
		static inline auto boot(zpt::json _options, zpt::ev::emitter _emitter) -> void {
			B::boot(_options);
			zpt::bridge::instance< B >()->emitter(_emitter);
		};
	};
	
	class Bridge {
	public:
		Bridge(zpt::json _options);
		virtual ~Bridge();
		
		virtual auto options() -> zpt::json;
		virtual auto name() -> std::string = 0;
		virtual auto events(zpt::ev::emitter _emitter) -> void = 0;
		virtual auto events() -> zpt::ev::emitter = 0;
		virtual auto mutations(zpt::mutation::emitter _emitter) -> void = 0;
		virtual auto mutations() -> zpt::mutation::emitter = 0;
		virtual auto initialize() -> void = 0;
		virtual auto self() const -> zpt::bridge = 0;

		template< typename T >
		inline auto eval(std::string _expr) -> decltype(((T*) T::instance().get())->eval(_expr)) {
			return ((T*) T::instance().get())->eval(_expr);
		};

		template< typename T >
		inline auto from(T _o) -> zpt::json {
			return _o->tojson();
		};

		template< typename T >
		inline auto to(zpt::json _o) -> T {
			return T::fromjson(_o);
		};

		template< typename D >
		inline auto data() -> D {
			return D::data(this->self());
		};

	private:
		zpt::json __options;
	};

	class MutationEmitter {
	public:
		MutationEmitter();
		MutationEmitter(zpt::json _options);
		virtual ~MutationEmitter();
		
		virtual auto options() -> zpt::json;
		virtual auto self() const -> zpt::mutation::emitter;
		virtual auto version() -> std::string = 0;
		
		virtual auto on(zpt::mutation::operation _operation, std::string _data_class_ns,  zpt::mutation::Handler _handler, zpt::json _opts = zpt::undefined) -> std::string = 0;
		virtual auto on(std::string _data_class_ns,  std::map< zpt::mutation::operation, zpt::mutation::Handler > _handlers, zpt::json _opts = zpt::undefined) -> std::string = 0;
		virtual auto on(zpt::mutation::listener _listener, zpt::json _opts = zpt::undefined) -> std::string = 0;
		virtual auto off(zpt::mutation::operation _operation, std::string _callback_id) -> void = 0;
		virtual auto off(std::string _callback_id) -> void = 0;
		
		virtual auto trigger(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record) -> zpt::json = 0;
		virtual auto route(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record) -> zpt::json = 0;
		
		virtual auto connector(std::string _name, zpt::connector _connector) -> void final;
		virtual auto connector(std::string _name) -> zpt::connector final;

	private:
		zpt::json __options;
		zpt::mutation::emitter __self;
		std::map<std::string, zpt::connector> __connector;
	};

	class MutationListener {
	public:
		MutationListener(std::string _data_class_ns);
		virtual ~MutationListener();

		virtual std::string ns() final;
		
		virtual auto inserted(std::string _data_class_ns, zpt::json _record, zpt::mutation::emitter _emitter) -> void;
		virtual auto removed(std::string _data_class_ns, zpt::json _record, zpt::mutation::emitter _emitter) -> void;
		virtual auto updated(std::string _data_class_ns, zpt::json _record, zpt::mutation::emitter _emitter) -> void;
		virtual auto replaced(std::string _data_class_ns, zpt::json _record, zpt::mutation::emitter _emitter) -> void;

	private:
		std::string __namespace;
	};
	
	class EventEmitter {
	public:
		EventEmitter();
		EventEmitter(zpt::json _options);
		virtual ~EventEmitter();
		
		virtual auto options() -> zpt::json;
		virtual auto self() const -> zpt::ev::emitter;
		virtual auto mutations() -> zpt::mutation::emitter;
		virtual auto version() -> std::string = 0;
		
		virtual auto on(zpt::ev::performative _method, std::string _regex,  zpt::ev::Handler _handler, zpt::json _opts = zpt::undefined) -> std::string = 0;
		virtual auto on(std::string _regex,  std::map< zpt::ev::performative, zpt::ev::Handler > _handlers, zpt::json _opts = zpt::undefined) -> std::string = 0;
		virtual auto on(zpt::ev::listener _listener, zpt::json _opts = zpt::undefined) -> std::string = 0;
		virtual auto off(zpt::ev::performative _method, std::string _callback_id) -> void = 0;
		virtual auto off(std::string _callback_id) -> void = 0;
		
		virtual auto trigger(zpt::ev::performative _method, std::string _resource, zpt::json _payload) -> zpt::json = 0;
		virtual auto route(zpt::ev::performative _method, std::string _resource, zpt::json _payload) -> zpt::json = 0;
		
		virtual auto connector(std::string _name, zpt::connector _connector) -> void final;
		virtual auto connector(std::map<std::string, zpt::connector> _connectors) -> void final;
		virtual auto connector(std::string _name) -> zpt::connector final;

	protected:
		virtual auto mutations(zpt::mutation::emitter _emitter) -> void;
		
	private:
		zpt::json __options;
		zpt::ev::emitter __self;
		zpt::mutation::emitter __mutant;
	};

	class EventListener {
	public:
		EventListener(std::string _regex);
		virtual ~EventListener();

		virtual std::string regex() final;
		
		virtual auto get(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json;
		virtual auto put(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json;
		virtual auto post(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json;
		virtual auto del(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json;
		virtual auto head(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json;
		virtual auto options(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json;
		virtual auto patch(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json;
		virtual auto reply(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json;

	private:
		std::string __regex;
	};

}

