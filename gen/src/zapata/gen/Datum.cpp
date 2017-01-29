$[resource.path.h]

auto $[namespace]::$[datum.name]::get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.get]
	$[datum.relations.get]
	return _r_data;
}

auto $[namespace]::$[datum.name]::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.query]
	$[datum.relations.query]
	return _r_data;
}

auto $[namespace]::$[datum.name]::insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.insert]
	$[datum.relations.insert]
	return _r_data;
}

auto $[namespace]::$[datum.name]::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.save]
	$[datum.relations.save]
	return _r_data;
}

auto $[namespace]::$[datum.name]::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.set.topic]
	$[datum.relations.set]
	return _r_data;
}

auto $[namespace]::$[datum.name]::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.set.pattern]
	$[datum.relations.set]
	return _r_data;
}

auto $[namespace]::$[datum.name]::remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.remove.topic]
	$[datum.relations.remove]
	return _r_data;
}

auto $[namespace]::$[datum.name]::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	$[datum.extends.remove.pattern]
	$[datum.relations.remove]
	return _r_data;
}

