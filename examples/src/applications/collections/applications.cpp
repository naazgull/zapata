#include <zapata/applications/collections/applications.h>

auto zpt::apps::collections::applications::restify(zpt::ev::emitter _emitter) -> void {
	_emitter->on("^/v2/datums/applications$",
		     {{zpt::ev::Get,
		       [](zpt::ev::performative _performative,
			  std::string _topic,
			  zpt::json _envelope,
			  zpt::ev::emitter _emitter) -> zpt::json {

			       zpt::json _t_split = zpt::split(_topic, "/");
			       zpt::json _identity =
				   zpt::rest::authorization::validate("/v2/datums/applications", _envelope, _emitter);

			       zpt::json _r_body;
			       _r_body = zpt::apps::datums::Applications::query(
				   _topic, _envelope["params"], _emitter, _identity, _envelope);

			       return {"status", (_r_body->ok() ? 200 : 204), "payload", _r_body};

		       }},
		      {zpt::ev::Post,
		       [](zpt::ev::performative _performative,
			  std::string _topic,
			  zpt::json _envelope,
			  zpt::ev::emitter _emitter) -> zpt::json {
			       assertz_object(_envelope["payload"], "admin", 412);
			       _envelope["payload"] >> "app_secret";
			       _envelope["payload"] << "app_secret" << zpt::generate::r_key(24);
			       _envelope["payload"] >> "cloud_secret";
			       _envelope["payload"] << "cloud_secret" << zpt::generate::r_key(24);
			       _envelope["payload"] >> "device_secret";
			       _envelope["payload"] << "device_secret" << zpt::generate::r_key(24);
			       assertz_uri(_envelope["payload"], "icon", 412);
			       assertz_uri(_envelope["payload"], "image", 412);
			       assertz_mandatory(_envelope["payload"], "name", 412);
			       assertz_utf8(_envelope["payload"], "name", 412);
			       assertz_mandatory(_envelope["payload"], "namespace", 412);
			       assertz_ascii(_envelope["payload"], "namespace", 412);
			       assertz_object(_envelope["payload"], "token", 412);
			       assertz_array(_envelope["payload"], "users", 412);

			       zpt::json _t_split = zpt::split(_topic, "/");
			       zpt::json _identity =
				   zpt::rest::authorization::validate("/v2/datums/applications", _envelope, _emitter);

			       zpt::json _r_body;
			       _r_body = zpt::apps::datums::Applications::insert(
				   _topic, _envelope["payload"], _emitter, _identity, _envelope);

			       return {"status", 201, "payload", _r_body};

		       }}

		      ,
		      {zpt::ev::Patch,
		       [](zpt::ev::performative _performative,
			  std::string _topic,
			  zpt::json _envelope,
			  zpt::ev::emitter _emitter) -> zpt::json {
			       assertz_object(_envelope["payload"], "admin", 412);
			       _envelope["payload"] >> "app_secret";
			       _envelope["payload"] >> "cloud_secret";
			       _envelope["payload"] >> "device_secret";
			       assertz_uri(_envelope["payload"], "icon", 412);
			       assertz_uri(_envelope["payload"], "image", 412);
			       assertz_utf8(_envelope["payload"], "name", 412);
			       assertz_ascii(_envelope["payload"], "namespace", 412);
			       assertz_object(_envelope["payload"], "token", 412);
			       assertz_array(_envelope["payload"], "users", 412);

			       zpt::json _t_split = zpt::split(_topic, "/");
			       zpt::json _identity =
				   zpt::rest::authorization::validate("/v2/datums/applications", _envelope, _emitter);

			       zpt::json _r_body;
			       _r_body = zpt::apps::datums::Applications::set(
				   _topic, _envelope["payload"], _envelope["params"], _emitter, _identity, _envelope);

			       return {"status", 200, "payload", _r_body};

		       }},
		      {zpt::ev::Delete,
		       [](zpt::ev::performative _performative,
			  std::string _topic,
			  zpt::json _envelope,
			  zpt::ev::emitter _emitter) -> zpt::json {

			       zpt::json _t_split = zpt::split(_topic, "/");
			       zpt::json _identity =
				   zpt::rest::authorization::validate("/v2/datums/applications", _envelope, _emitter);

			       zpt::json _r_body;
			       _r_body = zpt::apps::datums::Applications::remove(
				   _topic, _envelope["params"], _emitter, _identity, _envelope);

			       return {"status", 200, "payload", _r_body};

		       }},
		      {zpt::ev::Head,
		       [](zpt::ev::performative _performative,
			  std::string _topic,
			  zpt::json _envelope,
			  zpt::ev::emitter _emitter) -> zpt::json {

			       zpt::json _t_split = zpt::split(_topic, "/");
			       zpt::json _identity =
				   zpt::rest::authorization::validate("/v2/datums/applications", _envelope, _emitter);

			       zpt::json _r_body;
			       _r_body = zpt::apps::datums::Applications::query(
				   _topic, _envelope["params"], _emitter, _identity, _envelope);

			       return {"headers",
				       {"Content-Length", std::string(_r_body).length()},
				       "status",
				       (_r_body->ok() ? 200 : 204)};

		       }}

		     },
		     {"0mq", true, "amqp", true, "http", true, "mqtt", true});
}
