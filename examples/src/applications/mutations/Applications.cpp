#include <zapata/applications/mutations/Applications.h>

auto zpt::apps::mutations::Applications::mutify(zpt::mutation::emitter _emitter) -> void {
	zpt::mutation::callback _h_on_change;

	_emitter->on(
	    "^/Applications$",
	    {{zpt::mutation::Insert,
	      [](zpt::mutation::operation _performative,
		 std::string _resource,
		 zpt::json _envelope,
		 zpt::mutation::emitter _emitter) -> void {
		      zpt::json _c_names = {zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps"};
		      for (auto _c_name : _c_names->arr()) {
			      zpt::connector _c = _emitter->connector(_c_name->str());
			      zpt::json _r_data = _envelope["payload"]["new"];
			      zpt::json _r_admin = _emitter->events()->route(
				  zpt::ev::Get,
				  zpt::path::join({zpt::array, "v2", "datums", "user-applications"}),
				  {"params", {"app_id", std::string(_r_data["id"]), "role", "m/admin/i"}})["payload"];
			      _r_data << "admin"
				      << (_r_admin["elements"]->type() == zpt::JSArray ? _r_admin["elements"][0]
										       : _r_admin);
			      zpt::json _r_users = _emitter->events()->route(
				  zpt::ev::Get,
				  zpt::path::join({zpt::array, "v2", "datums", "users"}),
				  {"params", {"app_id", std::string(_r_data["id"])}})["payload"];
			      _r_data << "users" << (_r_users["elements"]->type() == zpt::JSArray
							 ? _r_users["elements"]
							 : zpt::json({zpt::array, _r_users}));
			      _c->insert("Applications",
					 _envelope["payload"]["href"]->str(),
					 _r_data,
					 {"mutated-event", true});
		      }
	      }}

	     ,
	     {zpt::mutation::Update,
	      [](zpt::mutation::operation _performative,
		 std::string _resource,
		 zpt::json _envelope,
		 zpt::mutation::emitter _emitter) -> void {
		      zpt::json _c_names = {zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps"};
		      for (auto _c_name : _c_names->arr()) {
			      zpt::connector _c = _emitter->connector(_c_name->str());
			      if (_envelope["payload"]["filter"]->ok()) {
				      _c->set("Applications",
					      _envelope["payload"]["filter"],
					      _envelope["payload"]["changes"],
					      {"mutated-event", true});
			      } else {
				      _c->set("Applications",
					      _envelope["payload"]["href"]->str(),
					      _envelope["payload"]["changes"],
					      {"mutated-event", true});
			      }
		      }
	      }}

	     ,
	     {zpt::mutation::Remove,
	      [](zpt::mutation::operation _performative,
		 std::string _resource,
		 zpt::json _envelope,
		 zpt::mutation::emitter _emitter) -> void {
		      zpt::json _c_names = {zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps"};
		      for (auto _c_name : _c_names->arr()) {
			      zpt::connector _c = _emitter->connector(_c_name->str());
			      if (_envelope["payload"]["filter"]->ok()) {
				      _c->remove(
					  "Applications", _envelope["payload"]["filter"], {"mutated-event", true});
			      } else {
				      _c->remove(
					  "Applications", _envelope["payload"]["href"]->str(), {"mutated-event", true});
			      }
		      }
	      }}

	     ,
	     {zpt::mutation::Replace,
	      [](zpt::mutation::operation _performative,
		 std::string _resource,
		 zpt::json _envelope,
		 zpt::mutation::emitter _emitter) -> void {
		      zpt::json _c_names = {zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps"};
		      for (auto _c_name : _c_names->arr()) {
			      zpt::connector _c = _emitter->connector(_c_name->str());
			      zpt::json _r_data = _envelope["payload"]["new"];
			      zpt::json _r_admin = _emitter->events()->route(
				  zpt::ev::Get,
				  zpt::path::join({zpt::array, "v2", "datums", "user-applications"}),
				  {"params", {"app_id", std::string(_r_data["id"]), "role", "m/admin/i"}})["payload"];
			      _r_data << "admin"
				      << (_r_admin["elements"]->type() == zpt::JSArray ? _r_admin["elements"][0]
										       : _r_admin);
			      zpt::json _r_users = _emitter->events()->route(
				  zpt::ev::Get,
				  zpt::path::join({zpt::array, "v2", "datums", "users"}),
				  {"params", {"app_id", std::string(_r_data["id"])}})["payload"];
			      _r_data << "users" << (_r_users["elements"]->type() == zpt::JSArray
							 ? _r_users["elements"]
							 : zpt::json({zpt::array, _r_users}));
			      _c->save("Applications",
				       _envelope["payload"]["href"]->str(),
				       _r_data,
				       {"mutated-event", true});
		      }
	      }}

	    });

	_emitter->on(
	    "^/v2/datums/user-applications$",
	    {{zpt::mutation::Insert,
	      (_h_on_change = [](zpt::mutation::operation _performative,
				 std::string _resource,
				 zpt::json _envelope,
				 zpt::mutation::emitter _emitter) -> void {
		      zpt::json _r_base = _emitter->events()->route(
			  zpt::ev::Get,
			  _envelope["payload"]["href"]->str(),
			  (_envelope["payload"]["filter"]->ok() ? zpt::json({"params", _envelope["payload"]["filter"]})
								: zpt::undefined))["payload"];
		      if (_r_base["elements"]->type() != zpt::JSArray) {
			      _r_base = {"elements", {zpt::array, _r_base}};
		      }
		      zpt::json _c_names = {zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps"};
		      for (auto _c_name : _c_names->arr()) {
			      zpt::connector _c = _emitter->connector(_c_name->str());
			      for (auto _r_element : _r_base["elements"]->arr()) {
				      zpt::json _r_targets =
					  _c->query("Applications", {"id", std::string(_r_element["app_id"])});
				      for (auto _r_data : _r_targets["elements"]->arr()) {
					      zpt::json _r_admin = _emitter->events()->route(
						  zpt::ev::Get,
						  zpt::path::join({zpt::array, "v2", "datums", "user-applications"}),
						  {"params",
						   {"app_id",
						    std::string(_r_data["id"]),
						    "role",
						    "m/admin/i"}})["payload"];
					      _c->set("Applications",
						      _r_data["href"]->str(),
						      {"admin",
						       (_r_admin["elements"]->type() == zpt::JSArray
							    ? _r_admin["elements"][0]
							    : _r_admin)},
						      {"href", _r_data["href"]});
				      }
			      }
		      }
	      })}

	     ,
	     {zpt::mutation::Update, _h_on_change}

	     ,
	     {zpt::mutation::Remove,
	      [](zpt::mutation::operation _performative,
		 std::string _resource,
		 zpt::json _envelope,
		 zpt::mutation::emitter _emitter) -> void {
		      if (_envelope["payload"]["filter"]->ok())
			      return;
		      zpt::json _c_names = {zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps"};
		      for (auto _c_name : _c_names->arr()) {
			      zpt::connector _c = _emitter->connector(_c_name->str());
			      zpt::json _r_targets =
				  _c->query("Applications", {"admin", {"href", _envelope["payload"]["href"]}});
			      for (auto _r_data : _r_targets["elements"]->arr()) {
				      zpt::json _r_admin = _emitter->events()->route(
					  zpt::ev::Get,
					  zpt::path::join({zpt::array, "v2", "datums", "user-applications"}),
					  {"params",
					   {"app_id", std::string(_r_data["id"]), "role", "m/admin/i"}})["payload"];
				      _c->set("Applications",
					      _r_data["href"]->str(),
					      {"admin",
					       (_r_admin["elements"]->type() == zpt::JSArray ? _r_admin["elements"][0]
											     : _r_admin)},
					      {"href", _r_data["href"]});
			      }
		      }
	      }}

	     ,
	     {zpt::mutation::Replace, _h_on_change}

	    });

	_emitter->on(
	    "^/v2/datums/users$",
	    {{zpt::mutation::Insert,
	      (_h_on_change = [](zpt::mutation::operation _performative,
				 std::string _resource,
				 zpt::json _envelope,
				 zpt::mutation::emitter _emitter) -> void {
		      zpt::json _r_base = _emitter->events()->route(
			  zpt::ev::Get,
			  _envelope["payload"]["href"]->str(),
			  (_envelope["payload"]["filter"]->ok() ? zpt::json({"params", _envelope["payload"]["filter"]})
								: zpt::undefined))["payload"];
		      if (_r_base["elements"]->type() != zpt::JSArray) {
			      _r_base = {"elements", {zpt::array, _r_base}};
		      }
		      zpt::json _c_names = {zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps"};
		      for (auto _c_name : _c_names->arr()) {
			      zpt::connector _c = _emitter->connector(_c_name->str());
			      for (auto _r_element : _r_base["elements"]->arr()) {
				      zpt::json _r_targets =
					  _c->query("Applications", {"id", std::string(_r_element["app_id"])});
				      for (auto _r_data : _r_targets["elements"]->arr()) {
					      zpt::json _r_users = _emitter->events()->route(
						  zpt::ev::Get,
						  zpt::path::join({zpt::array, "v2", "datums", "users"}),
						  {"params", {"app_id", std::string(_r_data["id"])}})["payload"];
					      _c->set("Applications",
						      _r_data["href"]->str(),
						      {"users",
						       (_r_users["elements"]->type() == zpt::JSArray
							    ? _r_users["elements"]
							    : zpt::json({zpt::array, _r_users}))},
						      {"href", _r_data["href"]});
				      }
			      }
		      }
	      })}

	     ,
	     {zpt::mutation::Update, _h_on_change}

	     ,
	     {zpt::mutation::Remove,
	      [](zpt::mutation::operation _performative,
		 std::string _resource,
		 zpt::json _envelope,
		 zpt::mutation::emitter _emitter) -> void {
		      if (_envelope["payload"]["filter"]->ok())
			      return;
		      zpt::json _c_names = {zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps"};
		      for (auto _c_name : _c_names->arr()) {
			      zpt::connector _c = _emitter->connector(_c_name->str());
			      zpt::json _r_targets =
				  _c->query("Applications", {"users", {"href", _envelope["payload"]["href"]}});
			      for (auto _r_data : _r_targets["elements"]->arr()) {
				      zpt::json _r_users = _emitter->events()->route(
					  zpt::ev::Get,
					  zpt::path::join({zpt::array, "v2", "datums", "users"}),
					  {"params", {"app_id", std::string(_r_data["id"])}})["payload"];
				      _c->set("Applications",
					      _r_data["href"]->str(),
					      {"users",
					       (_r_users["elements"]->type() == zpt::JSArray
						    ? _r_users["elements"]
						    : zpt::json({zpt::array, _r_users}))},
					      {"href", _r_data["href"]});
			      }
		      }
	      }}

	     ,
	     {zpt::mutation::Replace, _h_on_change}

	    });
}
