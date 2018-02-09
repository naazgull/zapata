#include <zapata/applications/datums/ResourceOwners.h>

auto zpt::apps::datums::ResourceOwners::get(std::string _topic,
					    zpt::ev::emitter _emitter,
					    zpt::json _identity,
					    zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
	_r_data = _c->get("ResourceOwners", _topic, {"href", _topic});

	return _r_data;
}

auto zpt::apps::datums::ResourceOwners::query(std::string _topic,
					      zpt::json _filter,
					      zpt::ev::emitter _emitter,
					      zpt::json _identity,
					      zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
	_r_data = _c->query("ResourceOwners", _filter, _filter + zpt::json({"href", _topic}));

	return _r_data;
}

auto zpt::apps::datums::ResourceOwners::insert(std::string _topic,
					       zpt::json _document,
					       zpt::ev::emitter _emitter,
					       zpt::json _identity,
					       zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.pgsql.zpt.apps");

	_document << "created" << zpt::json::date() << "updated" << zpt::json::date();

	_r_data = {"href",
		   (_topic + std::string("/") + _c->insert("ResourceOwners", _topic, _document, {"href", _topic}))};

	if (!_r_extends)
		_emitter->mutations()->route(zpt::mutation::Insert,
					     _topic,
					     {"performative", "insert", "href", _document["href"], "new", _document});
	return _r_data;
}

auto zpt::apps::datums::ResourceOwners::save(std::string _topic,
					     zpt::json _document,
					     zpt::ev::emitter _emitter,
					     zpt::json _identity,
					     zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.pgsql.zpt.apps");

	_document << "updated" << zpt::json::date();

	_r_data = {"href", _topic, "n_updated", _c->save("ResourceOwners", _topic, _document, {"href", _topic})};

	if (!_r_extends)
		_emitter->mutations()->route(
		    zpt::mutation::Replace, _topic, {"performative", "save", "href", _topic, "new", _document});
	return _r_data;
}

auto zpt::apps::datums::ResourceOwners::set(std::string _topic,
					    zpt::json _document,
					    zpt::ev::emitter _emitter,
					    zpt::json _identity,
					    zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.pgsql.zpt.apps");

	_document << "updated" << zpt::json::date();

	_r_data = {"href", _topic, "n_updated", _c->set("ResourceOwners", _topic, _document, {"href", _topic})};

	if (!_r_extends)
		_emitter->mutations()->route(
		    zpt::mutation::Update, _topic, {"performative", "set", "href", _topic, "changes", _document});
	return _r_data;
}

auto zpt::apps::datums::ResourceOwners::set(std::string _topic,
					    zpt::json _document,
					    zpt::json _filter,
					    zpt::ev::emitter _emitter,
					    zpt::json _identity,
					    zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.pgsql.zpt.apps");

	_document << "updated" << zpt::json::date();

	_r_data = {"href", _topic, "n_updated", _c->set("ResourceOwners", _filter, _document, {"href", _topic})};

	if (!_r_extends)
		_emitter->mutations()->route(
		    zpt::mutation::Update,
		    _topic,
		    {"performative", "set", "href", _topic, "changes", _document, "filter", _filter});
	return _r_data;
}

auto zpt::apps::datums::ResourceOwners::remove(std::string _topic,
					       zpt::ev::emitter _emitter,
					       zpt::json _identity,
					       zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.pgsql.zpt.apps");

	_r_data = {"href", _topic, "n_deleted", _c->remove("ResourceOwners", _topic, {"href", _topic})};

	if (!_r_extends)
		_emitter->mutations()->route(zpt::mutation::Remove, _topic, {"performative", "remove", "href", _topic});
	return _r_data;
}

auto zpt::apps::datums::ResourceOwners::remove(std::string _topic,
					       zpt::json _filter,
					       zpt::ev::emitter _emitter,
					       zpt::json _identity,
					       zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.pgsql.zpt.apps");

	_r_data = {
	    "href", _topic, "n_deleted", _c->remove("ResourceOwners", _filter, _filter + zpt::json({"href", _topic}))};

	if (!_r_extends)
		_emitter->mutations()->route(
		    zpt::mutation::Remove, _topic, {"performative", "remove", "href", _topic, "filter", _filter});
	return _r_data;
}
