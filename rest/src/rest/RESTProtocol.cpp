#include <zapata/rest/RESTProtocol.h>

zpt::RESTListener::RESTListener(std::string _regex) : zpt::EventListener(_regex) {}

zpt::RESTListener::~RESTListener() {}

auto zpt::RESTListener::get(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::RESTListener::put(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::RESTListener::post(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::RESTListener::del(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::RESTListener::head(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::RESTListener::options(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	if (_envelope["headers"]["Origin"]->ok()) {
		_emitter->reply(
		    _envelope,
		    {"status", 413, "headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]))});
	}
	string _origin = _envelope["headers"]["Origin"];
	_emitter->reply(_envelope,
			{"status",
			 200,
			 "headers",
			 (zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) +
			  zpt::json({"Access-Control-Allow-Origin",
				     _envelope["headers"]["Origin"],
				     "Access-Control-Allow-Methods",
				     "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY",
				     "Access-Control-Allow-Headers",
				     ACCESS_CONTROL_HEADERS,
				     "Access-Control-Expose-Headers",
				     ACCESS_CONTROL_HEADERS,
				     "Access-Control-Max-Age",
				     "1728000"}))});
}

auto zpt::RESTListener::patch(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::RESTListener::reply(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {}
