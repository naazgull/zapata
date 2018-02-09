#include <zapata/applications/datums/MyApplications.h>

auto zpt::apps::datums::MyApplications::get(std::string _topic,
					    zpt::ev::emitter _emitter,
					    zpt::json _identity,
					    zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
	_r_data = _c->get("MyApplications", _topic, {"href", _topic});

	zpt::json _d_channels =
	    _emitter->route(zpt::ev::Get,
			    zpt::path::join({zpt::array, "v2", "datums", "channel-subscriptions"}),
			    {"headers",
			     zpt::rest::authorization::headers(_identity["access_token"]),
			     "params",
			     (_envelope["params"] + zpt::json({"app_id", std::string(_r_data["id"])}))})["payload"];
	_r_data << "channels"
		<< (_d_channels["elements"]->type() == zpt::JSArray ? _d_channels["elements"]
								    : zpt::json({zpt::array, _d_channels}));

	return _r_data;
}

auto zpt::apps::datums::MyApplications::query(std::string _topic,
					      zpt::json _filter,
					      zpt::ev::emitter _emitter,
					      zpt::json _identity,
					      zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
	_r_data = _c->query("MyApplications", _filter, _filter + zpt::json({"href", _topic}));

	if (_r_data["elements"]->type() == zpt::JSArray) {
		for (auto _d_element : _r_data["elements"]->arr()) {
			zpt::json _d_channels = _emitter->route(
			    zpt::ev::Get,
			    zpt::path::join({zpt::array, "v2", "datums", "channel-subscriptions"}),
			    {"headers",
			     zpt::rest::authorization::headers(_identity["access_token"]),
			     "params",
			     (_envelope["params"] + zpt::json({"app_id", std::string(_d_element["id"])}))})["payload"];
			_d_element << "channels" << (_d_channels["elements"]->type() == zpt::JSArray
							 ? _d_channels["elements"]
							 : zpt::json({zpt::array, _d_channels}));
		}
	}

	return _r_data;
}

auto zpt::apps::datums::MyApplications::insert(std::string _topic,
					       zpt::json _document,
					       zpt::ev::emitter _emitter,
					       zpt::json _identity,
					       zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");

	_document << "created" << zpt::json::date() << "updated" << zpt::json::date();

	zpt::json _r_parent = zpt::json::object();
	if (_document["admin"]->ok())
		_r_parent << "admin" << _document["admin"];
	if (_document["app_secret"]->ok())
		_r_parent << "app_secret" << _document["app_secret"];
	if (_document["cloud_secret"]->ok())
		_r_parent << "cloud_secret" << _document["cloud_secret"];
	if (_document["device_secret"]->ok())
		_r_parent << "device_secret" << _document["device_secret"];
	if (_document["icon"]->ok())
		_r_parent << "icon" << _document["icon"];
	if (_document["image"]->ok())
		_r_parent << "image" << _document["image"];
	if (_document["name"]->ok())
		_r_parent << "name" << _document["name"];
	if (_document["namespace"]->ok())
		_r_parent << "namespace" << _document["namespace"];
	if (_document["token"]->ok())
		_r_parent << "token" << _document["token"];
	if (_document["users"]->ok())
		_r_parent << "users" << _document["users"];

	std::string _r_id = _c->insert("MyApplications", _topic, _document - _r_parent, {"href", _topic});
	_r_data = {"href", (_topic + std::string("/") + _r_id)};
	_r_parent << "id" << _r_id;
	zpt::apps::datums::Applications::insert(_topic, _r_parent, _emitter, _identity, _envelope);

	_r_extends = true;

	if (!_r_extends)
		_emitter->mutations()->route(zpt::mutation::Insert,
					     _topic,
					     {"performative", "insert", "href", _document["href"], "new", _document});
	return _r_data;
}

auto zpt::apps::datums::MyApplications::save(std::string _topic,
					     zpt::json _document,
					     zpt::ev::emitter _emitter,
					     zpt::json _identity,
					     zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");

	_document << "updated" << zpt::json::date();

	zpt::json _r_parent = zpt::json::object();
	if (_document["admin"]->ok())
		_r_parent << "admin" << _document["admin"];
	if (_document["app_secret"]->ok())
		_r_parent << "app_secret" << _document["app_secret"];
	if (_document["cloud_secret"]->ok())
		_r_parent << "cloud_secret" << _document["cloud_secret"];
	if (_document["device_secret"]->ok())
		_r_parent << "device_secret" << _document["device_secret"];
	if (_document["icon"]->ok())
		_r_parent << "icon" << _document["icon"];
	if (_document["image"]->ok())
		_r_parent << "image" << _document["image"];
	if (_document["name"]->ok())
		_r_parent << "name" << _document["name"];
	if (_document["namespace"]->ok())
		_r_parent << "namespace" << _document["namespace"];
	if (_document["token"]->ok())
		_r_parent << "token" << _document["token"];
	if (_document["users"]->ok())
		_r_parent << "users" << _document["users"];
	_r_data = {
	    "href", _topic, "n_updated", _c->save("MyApplications", _topic, _document - _r_parent, {"href", _topic})};
	zpt::apps::datums::Applications::save(_topic, _r_parent, _emitter, _identity, _envelope);

	_r_extends = true;

	if (!_r_extends)
		_emitter->mutations()->route(
		    zpt::mutation::Replace, _topic, {"performative", "save", "href", _topic, "new", _document});
	return _r_data;
}

auto zpt::apps::datums::MyApplications::set(std::string _topic,
					    zpt::json _document,
					    zpt::ev::emitter _emitter,
					    zpt::json _identity,
					    zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");

	_document << "updated" << zpt::json::date();

	zpt::json _r_parent = zpt::json::object();
	if (_document["admin"]->ok())
		_r_parent << "admin" << _document["admin"];
	if (_document["app_secret"]->ok())
		_r_parent << "app_secret" << _document["app_secret"];
	if (_document["cloud_secret"]->ok())
		_r_parent << "cloud_secret" << _document["cloud_secret"];
	if (_document["device_secret"]->ok())
		_r_parent << "device_secret" << _document["device_secret"];
	if (_document["icon"]->ok())
		_r_parent << "icon" << _document["icon"];
	if (_document["image"]->ok())
		_r_parent << "image" << _document["image"];
	if (_document["name"]->ok())
		_r_parent << "name" << _document["name"];
	if (_document["namespace"]->ok())
		_r_parent << "namespace" << _document["namespace"];
	if (_document["token"]->ok())
		_r_parent << "token" << _document["token"];
	if (_document["users"]->ok())
		_r_parent << "users" << _document["users"];
	_r_data = {
	    "href", _topic, "n_updated", _c->set("MyApplications", _topic, _document - _r_parent, {"href", _topic})};
	zpt::apps::datums::Applications::set(_topic, _r_parent, _emitter, _identity, _envelope);

	_r_extends = true;

	if (!_r_extends)
		_emitter->mutations()->route(
		    zpt::mutation::Update, _topic, {"performative", "set", "href", _topic, "changes", _document});
	return _r_data;
}

auto zpt::apps::datums::MyApplications::set(std::string _topic,
					    zpt::json _document,
					    zpt::json _filter,
					    zpt::ev::emitter _emitter,
					    zpt::json _identity,
					    zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");

	_document << "updated" << zpt::json::date();
	zpt::json _r_elements =
	    zpt::apps::datums::MyApplications::query(_topic, _filter, _emitter, _identity, _envelope);
	for (auto _r_element : _r_elements["elements"]->arr()) {
		zpt::apps::datums::MyApplications::set(
		    _r_element["href"]->str(), _document, _emitter, _identity, _envelope);
	}
	_r_data = {"href", _topic, "n_updated", _r_elements["size"]};

	_r_extends = true;

	if (!_r_extends)
		_emitter->mutations()->route(
		    zpt::mutation::Update,
		    _topic,
		    {"performative", "set", "href", _topic, "changes", _document, "filter", _filter});
	return _r_data;
}

auto zpt::apps::datums::MyApplications::remove(std::string _topic,
					       zpt::ev::emitter _emitter,
					       zpt::json _identity,
					       zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
	_r_data = {"href", _topic, "n_updated", _c->remove("MyApplications", _topic, {"href", _topic})};
	zpt::apps::datums::Applications::remove(_topic, _emitter, _identity, _envelope);

	_r_extends = true;

	if (!_r_extends)
		_emitter->mutations()->route(zpt::mutation::Remove, _topic, {"performative", "remove", "href", _topic});
	return _r_data;
}

auto zpt::apps::datums::MyApplications::remove(std::string _topic,
					       zpt::json _filter,
					       zpt::ev::emitter _emitter,
					       zpt::json _identity,
					       zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	bool _r_extends = false;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
	zpt::json _r_elements =
	    zpt::apps::datums::MyApplications::query(_topic, _filter, _emitter, _identity, _envelope);
	for (auto _r_element : _r_elements["elements"]->arr()) {
		zpt::apps::datums::MyApplications::remove(_r_element["href"]->str(), _emitter, _identity, _envelope);
	}
	_r_data = {"href", _topic, "n_deleted", _r_elements["size"]};

	_r_extends = true;

	if (!_r_extends)
		_emitter->mutations()->route(
		    zpt::mutation::Remove, _topic, {"performative", "remove", "href", _topic, "filter", _filter});
	return _r_data;
}
