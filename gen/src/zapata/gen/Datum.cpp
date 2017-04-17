$[data.path.self.h]

auto $[namespace]::datums::$[datum.name]::get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.get]
	$[datum.relations.get]
	return _r_data;
}

auto $[namespace]::datums::$[datum.name]::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.query]
	$[datum.relations.query]
	return _r_data;
}

auto $[namespace]::datums::$[datum.name]::insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.insert]
	$[datum.relations.insert]
	if (!_envelope["params"]["extension"]->ok() && _r_data["id"]->type() == zpt::JSString) _emitter->mutations()->route(zpt::mutation::Insert, _topic, { "headers", _envelope["headers"], "performative", "insert", "href", _document["href"], "new", _document });
	return _r_data;
}

auto $[namespace]::datums::$[datum.name]::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.save]
	$[datum.relations.save]
	if (!_envelope["params"]["extension"]->ok() && int(_r_data["n_updated"]) != 0) _emitter->mutations()->route(zpt::mutation::Replace, _topic, { "headers", _envelope["headers"], "performative", "save", "href", _topic, "new", _document });
	return _r_data;
}

auto $[namespace]::datums::$[datum.name]::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.set.topic]
	$[datum.relations.set]
	if (!_envelope["params"]["extension"]->ok() && int(_r_data["n_updated"]) != 0) _emitter->mutations()->route(zpt::mutation::Update, _topic, { "headers", _envelope["headers"], "performative", "set", "href", _topic, "changes", _document });
	return _r_data;
}

auto $[namespace]::datums::$[datum.name]::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.set.pattern]
	$[datum.relations.set]
	if (!_envelope["params"]["extension"]->ok() && int(_r_data["n_updated"]) != 0) _emitter->mutations()->route(zpt::mutation::Update, _topic, { "headers", _envelope["headers"], "performative", "set", "href", _topic, "changes", _document, "filter", _filter });
	return _r_data;
}

auto $[namespace]::datums::$[datum.name]::remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.remove.topic]
	$[datum.relations.remove]
	if (!_envelope["params"]["extension"]->ok() && int(_r_data["n_deleted"]) != 0) _emitter->mutations()->route(zpt::mutation::Remove, _topic, { "headers", _envelope["headers"], "performative", "remove", "href", _topic });
	return _r_data;
}

auto $[namespace]::datums::$[datum.name]::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.remove.pattern]
	$[datum.relations.remove]
	if (!_envelope["params"]["extension"]->ok() && int(_r_data["n_deleted"]) != 0) _emitter->mutations()->route(zpt::mutation::Remove, _topic, { "headers", _envelope["headers"], "performative", "remove", "href", _topic, "filter", _filter });
	return _r_data;
}
