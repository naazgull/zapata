$[mutation.path.self.h]

auto $[namespace]::mutations::$[mutation.name]::mutify(zpt::mutation::emitter _emitter) -> void {
$[mutation.handler.begin]
_emitter->on("$[mutation.topic.regex]",
{
$[mutation.handler.insert]
$[mutation.handler.update]
$[mutation.handler.remove]
$[mutation.handler.replace]
},
$[mutation.opts]
);
$[mutation.handler.end]
}
