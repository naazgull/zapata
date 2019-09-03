$[data.path.self.h]

  auto $[namespace] ::datums::$[datum.name] ::get(std::string _topic,
                                                  zpt::ev::emitter _emitter,
                                                  zpt::json _identity,
                                                  zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    $[datum.extends.get] return _r_data;
}

auto $[namespace] ::datums::$[datum.name] ::query(std::string _topic,
                                                  zpt::json _filter,
                                                  zpt::ev::emitter _emitter,
                                                  zpt::json _identity,
                                                  zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    $[datum.extends.query] return _r_data;
}

auto $[namespace] ::datums::$[datum.name] ::insert(std::string _topic,
                                                   zpt::json _document,
                                                   zpt::ev::emitter _emitter,
                                                   zpt::json _identity,
                                                   zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    $[datum.extends.insert] return _r_data;
}

auto $[namespace] ::datums::$[datum.name] ::save(std::string _topic,
                                                 zpt::json _document,
                                                 zpt::ev::emitter _emitter,
                                                 zpt::json _identity,
                                                 zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    $[datum.extends.save] return _r_data;
}

auto $[namespace] ::datums::$[datum.name] ::set(std::string _topic,
                                                zpt::json _document,
                                                zpt::ev::emitter _emitter,
                                                zpt::json _identity,
                                                zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    $[datum.extends.set.topic] return _r_data;
}

auto $[namespace] ::datums::$[datum.name] ::set(std::string _topic,
                                                zpt::json _document,
                                                zpt::json _filter,
                                                zpt::ev::emitter _emitter,
                                                zpt::json _identity,
                                                zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    $[datum.extends.set.pattern] return _r_data;
}

auto $[namespace] ::datums::$[datum.name] ::remove(std::string _topic,
                                                   zpt::ev::emitter _emitter,
                                                   zpt::json _identity,
                                                   zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    $[datum.extends.remove.topic] return _r_data;
}

auto $[namespace] ::datums::$[datum.name] ::remove(std::string _topic,
                                                   zpt::json _filter,
                                                   zpt::ev::emitter _emitter,
                                                   zpt::json _identity,
                                                   zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    $[datum.extends.remove.pattern] return _r_data;
}
