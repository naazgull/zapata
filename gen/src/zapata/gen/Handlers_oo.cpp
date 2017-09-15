$[resource.path.self.h]

$[namespace.begin]
class $[resource.class] : public zpt::ev::listener {
public:
	$[resource.class](std::string _topic) : zpt::ev::listener(_topic) {
	}
	virtual ~$[resource.class]() {
	}

$[resource.handler.get]
$[resource.handler.post]
$[resource.handler.put]
$[resource.handler.patch]
$[resource.handler.delete]
$[resource.handler.head]
$[resource.handler.reply]
	
}
$[namespace.end]

auto $[namespace]::$[resource.type]s::$[resource.name]::restify(zpt::ev::emitter _emitter) -> void {
$[namespace]::$[resource.class] _handler("$[resource.topic.regex]");
_emitter->on(_handler, $[resource.opts]);
}
