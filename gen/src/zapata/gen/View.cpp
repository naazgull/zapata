$[resource.path.h]

auto $[namespace]::$[datum.name]::get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity) -> zpt::json {
	zpt::connector _c = _emitter->connector("$[datum.method.get.client]");
	zpt::json _r_data = _c->get("$[datum.collection]", _topic);
	$[datum.relations.get]
	return _r_data;
}

auto $[namespace]::$[datum.name]::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity) -> zpt::json {
	zpt::connector _c = _emitter->connector("$[datum.method.query.client]");
	zpt::json _r_data = _c->query("$[datum.collection]", _filter, _filter);
	$[datum.relations.query]
	return _r_data;
}

auto $[namespace]::$[datum.name]::insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity) -> zpt::json {
	zpt::connector _c = _emitter->connector("$[datum.method.insert.client]");

	_document <<
	"created" << zpt::json::date() <<
	"updated" << zpt::json::date();
	
	zpt::json _r_data = _c->insert("$[datum.collection]", _topic, _document);
	$[datum.relations.insert]
	return _r_data;
}

auto $[namespace]::$[datum.name]::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity) -> zpt::json {
	zpt::connector _c = _emitter->connector("$[datum.method.save.client]");

	_document <<
	"updated" << zpt::json::date();
	
	zpt::json _r_data = _c->save("$[datum.collection]", _topic, _document);
	$[datum.relations.save]
	return _r_data;
}

auto $[namespace]::$[datum.name]::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity) -> zpt::json {
	zpt::connector _c = _emitter->connector("$[datum.method.set.client]");

	_document <<
	"updated" << zpt::json::date();
	
	zpt::json _r_data = _c->set("$[datum.collection]", _topic, _document);
	$[datum.relations.set]
	return _r_data;
}

auto $[namespace]::$[datum.name]::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity) -> zpt::json {
	zpt::connector _c = _emitter->connector("$[datum.method.set.client]");

	_document <<
	"updated" << zpt::json::date();
	
	zpt::json _r_data = _c->set("$[datum.collection]", _filter, _document);
	$[datum.relations.set]
	return _r_data;
}

auto $[namespace]::$[datum.name]::remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity) -> zpt::json {
	zpt::connector _c = _emitter->connector("$[datum.method.remove.client]");
	zpt::json _r_data = _c->remove("$[datum.collection]", _topic);
	$[datum.relations.remove]
	return _r_data;
}

auto $[namespace]::$[datum.name]::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity) -> zpt::json {
	zpt::connector _c = _emitter->connector("$[datum.method.remove.client]");
	zpt::json _r_data = _c->remove("$[datum.collection]", _filter, _filter);
	$[datum.relations.remove]
	return _r_data;
}

