#include <zapata/catalog.h>

auto factory(zpt::json _argv, std::uint16_t, zpt::context) -> zpt::json {
    zlog(_argv(1), zpt::info);
    return zpt::undefined;
}

auto main(int, char**) -> int {
    zpt::catalog<std::string, zpt::json> _catalog{ "catalog" };
    _catalog.clear();

    zpt::lambda::add("factory", 2, factory);

    _catalog.add(
      "/users",
      "<self>",
      1,
      { "host", "localhost", "port", 8080, "callback", zpt::json::lambda("factory", 2) });
    _catalog.add(
      "/users/{}",
      "<self>",
      2,
      { "host", "localhost", "port", 8080, "callback", zpt::json::lambda("factory", 2) });
    _catalog.add(
      "/users/n@zgul.me",
      "<self>",
      3,
      { "host", "localhost", "port", 8080, "callback", zpt::json::lambda("factory", 2) });
    _catalog.add(
      "/users/{}/info",
      "<self>",
      4,
      { "host", "localhost", "port", 8080, "callback", zpt::json::lambda("factory", 2) });

    for (auto [_, _key, _record] : _catalog.search("/users/n@zgul.me/info")) {
        zpt::json::parse_json_str(_record("metadata")->string())("callback")
          ->lambda()({ zpt::array, _key, _record }, zpt::context{ nullptr });
    }
    for (auto [_, _key, _record] : _catalog.search("/users/n@zgul.me")) {
        zpt::json::parse_json_str(_record("metadata")->string())("callback")
          ->lambda()({ zpt::array, _key, _record }, zpt::context{ nullptr });
    }
    return 0;
}
