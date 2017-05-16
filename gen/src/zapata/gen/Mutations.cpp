$[mutation.path.self.h]

auto $[namespace]::mutations::$[mutation.name]::mutify(zpt::ev::emitter _emitter) -> void {
zpt::callback _h_on_change;
 
$[mutation.self.handler.begin]
_emitter->on("$[mutation.topic.self.regex]",
{
{ zpt::ev::Reply,
[] (zpt::ev::performative _perfomative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {
zpt::json _t_split = zpt::split(_topic, "/");
std::string _tv_operation = std::string(_t_split[2]);
$[mutation.handler.self.insert]
$[mutation.handler.self.update]
$[mutation.handler.self.remove]
$[mutation.handler.self.replace]
}
}
}
);
$[mutation.self.handler.end]

$[mutation.handler.begin]
_emitter->on("$[mutation.topic.regex]",
{
{ zpt::ev::Reply,
[] (zpt::ev::performative _perfomative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {
zpt::json _t_split = zpt::split(_topic, "/");
std::string _tv_operation = std::string(_t_split[2]);
$[mutation.handler.insert]
$[mutation.handler.update]
$[mutation.handler.remove]
$[mutation.handler.replace]
}
}
}
);
$[mutation.handler.end]
}
