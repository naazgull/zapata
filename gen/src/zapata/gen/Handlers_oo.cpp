$[resource.path.self.h]

$[namespaces.begin]
class $[resource.handler.class.name] : public zpt::EventListener{
public :
$[resource.handler.class.name](std::string _topic) : zpt::EventListener(_topic){}
virtual ~$[resource.handler.class.name](){}

$[resource.handler.get]
$[resource.handler.post]
$[resource.handler.put]
$[resource.handler.patch]
$[resource.handler.delete]
$[resource.handler.head]
$[resource.handler.reply]
};
$[namespaces.end]

auto $[namespace]::$[resource.type]s::$[resource.name]::restify(zpt::ev::emitter _emitter) -> void {
zpt::ev::listener _handler(new $[namespace] ::$[resource.handler.class.name]("$[resource.topic.regex]"));
emitter->on(_handler, $[resource.opts]);
}
