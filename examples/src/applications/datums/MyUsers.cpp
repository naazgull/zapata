#include <zapata/applications/datums/MyUsers.h>

auto zpt::apps::datums::MyUsers::get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
_r_data = _c->get("MyUsers", _topic);

	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
_r_data = _c->query("MyUsers", _filter, _filter);

	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}


