#include <zapata/dom.h>

zpt::json _document = R"({
    "stand-alone" : {
        "enabled" : false,
        "host" : "localhost",
        "domain" : "localhost",
        "rest" : {
            "version" : "v3",
            "credentials" : { "client_id" : "{client-id}", "client_secret" : "{client-secret}", "server" : "{oauth2-server-address}", "grant" : "{type-of-grant}", "scope" : "{scope}" },
            "modules" : [
                "example-lib"
            ]
        },
        "mqtt" : { "connect" : "mqtts://localhost:1883" },
        "http" : { "bind" : "http://localhost:9000" },
        "zmq" : [{ "bind" : "@tcp://*:*", "type" : "rep" }],
        "log" : { "level" : 8 }
    },
    "router" : {
        "enabled" : false,
        "host" : "localhost",
        "domain" : "localhost",
        "rest" : {
            "version" : "v3",
            "credentials" : { "client_id" : "{client-id}", "client_secret" : "{client-secret}", "server" : "{oauth2-server-address}", "grant" : "{type-of-grant}", "scope" : "{scope}" }
        },
        "http" : { "bind" : "http://localhost:9000" },
        "zmq" : [{ "bind" : "@tcp://*:*,@tcp://*:9001", "type" : "router/dealer" }],
        "log" : { "level" : 8 }
    },
    "worker" : {
        "enabled" : false,
        "spawn" : 2,
        "host" : "localhost",
        "domain" : "localhost",
        "rest" : {
            "version" : "v3",
            "credentials" : { "client_id" : "{client-id}", "client_secret" : "{client-secret}", "server" : "{oauth2-server-address}", "grant" : "{type-of-grant}", "scope" : "{scope}" },
            "modules" : [
                "example-lib"
            ]
        },
        "http" : { "bind" : "http://localhost:9000" },
        "zmq" : [{ "bind" : ">tcp://*:9001", "type" : "rep" }],
        "log" : { "level" : 8 }
    }
})"_JSON;

auto
main(int argc, char* argv[]) -> int {
    zpt::dom::engine _dom;
    _dom.start_threads();

    _dom.add_listener(
      0, "/router/rest/credentials", [](zpt::pipeline::event<zpt::dom::element>& _event) {
          auto& _element = _event->content();
          std::cout << "----------------------------------------------------" << std::endl
                    << "xpath: " << _element.xpath() << std::endl
                    << "name: " << _element.name() << std::endl
                    << "content: " << _element.content() << std::endl
                    << "parent: " << _element.parent() << std::endl
                    << std::flush;
      });
    _dom.add_listener(
      0, "/router/rest/version", [](zpt::pipeline::event<zpt::dom::element>& _event) {
          auto& _element = _event->content();
          std::cout << "----------------------------------------------------" << std::endl
                    << "xpath: " << _element.xpath() << std::endl
                    << "name: " << _element.name() << std::endl
                    << "content: " << _element.content() << std::endl
                    << "parent: " << _element.parent() << std::endl
                    << std::flush;
      });
    _dom.add_listener(
      0, "/stand-alone/zmq/{:([^/]+):}/bind", [](zpt::pipeline::event<zpt::dom::element>& _event) {
          auto _element = _event->content();
          std::cout << "----------------------------------------------------" << std::endl
                    << "xpath: " << _element.xpath() << std::endl
                    << "name: " << _element.name() << std::endl
                    << "content: " << _element.content() << std::endl
                    << "parent: " << _element.parent() << std::endl
                    << std::flush;
      });
    _dom.add_listener(0, "/worker/http/bind", [](zpt::pipeline::event<zpt::dom::element>& _event) {
        auto& _element = _event->content();
        std::cout << "----------------------------------------------------" << std::endl
                  << "xpath: " << _element.xpath() << std::endl
                  << "name: " << _element.name() << std::endl
                  << "content: " << _element.content() << std::endl
                  << "parent: " << _element.parent() << std::endl
                  << std::flush;
    });
    _dom.add_listener(0, "/worker/zmq/0/bind", [](zpt::pipeline::event<zpt::dom::element>& _event) {
        auto _element = _event->content();
        std::cout << "----------------------------------------------------" << std::endl
                  << "xpath: " << _element.xpath() << std::endl
                  << "name: " << _element.name() << std::endl
                  << "content: " << _element.content() << std::endl
                  << "parent: " << _element.parent() << std::endl
                  << std::flush;
    });
    _dom.traverse(_document);
    _dom.shutdown();
    return 0;
}
