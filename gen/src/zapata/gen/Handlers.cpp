$[resource.path.self.h]

auto $[namespace]::$[resource.type]s::$[resource.name]::restify(zpt::ev::emitter _emitter) -> void {
_emitter->on("$[resource.topic.regex]",
{
$[resource.handler.get]
$[resource.handler.post]
$[resource.handler.put]
$[resource.handler.patch]
$[resource.handler.delete]
$[resource.handler.head]
},
$[resource.opts]
);
}
