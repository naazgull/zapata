#include <zapata/applications/collections/roles.h>

auto
zpt::apps::collections::roles::restify(zpt::ev::emitter _emitter) -> void {
    _emitter->on(
      "^/v2/datum/roles$",
      { { zpt::ev::Get,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity =
                zpt::rest::authorization::validate("/v2/datum/roles", _envelope, _emitter);

              zpt::json _r_body;
              /* ---> YOUR CODE HERE <---*/
              return { "status", (_r_body->ok() ? 200 : 204), "payload", _r_body };
          } },
        { zpt::ev::Post,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              assertz_timestamp(_envelope["payload"], "created", 412);
              _envelope["payload"] << "created" << zpt::json::date();
              assertz_mandatory(_envelope["payload"], "type", 412);
              assertz_ascii(_envelope["payload"], "type", 412);
              assertz_timestamp(_envelope["payload"], "updated", 412);
              _envelope["payload"] << "updated" << zpt::json::date();

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity =
                zpt::rest::authorization::validate("/v2/datum/roles", _envelope, _emitter);

              zpt::json _r_body;
              /* ---> YOUR CODE HERE <---*/
              return { "status", 201, "payload", _r_body };
          } }

        ,
        { zpt::ev::Patch,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              assertz_timestamp(_envelope["payload"], "created", 412);
              assertz_ascii(_envelope["payload"], "type", 412);
              assertz_timestamp(_envelope["payload"], "updated", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity =
                zpt::rest::authorization::validate("/v2/datum/roles", _envelope, _emitter);

              zpt::json _r_body;
              /* ---> YOUR CODE HERE <---*/
              return { "status", 200, "payload", _r_body };
          } },
        { zpt::ev::Delete,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity =
                zpt::rest::authorization::validate("/v2/datum/roles", _envelope, _emitter);

              zpt::json _r_body;
              /* ---> YOUR CODE HERE <---*/
              return { "status", 200, "payload", _r_body };
          } },
        { zpt::ev::Head,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity =
                zpt::rest::authorization::validate("/v2/datum/roles", _envelope, _emitter);

              zpt::json _r_body;
              /* ---> YOUR CODE HERE <---*/
              return { "headers",
                       { "Content-Length", std::string(_r_body).length() },
                       "status",
                       (_r_body->ok() ? 200 : 204) };
          } }

      },
      { "0mq", true, "amqp", true, "http", true, "mqtt", true });
}
