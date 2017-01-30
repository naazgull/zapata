$[resource.path.self.h]

auto $[namespace]::restify(zpt::ev::emitter _emitter) -> void {
$[resource.registry.begin]
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
$[resource.registry.end]
}
