$[mutation.path.self.h]

auto $[namespace]::mutations::$[mutation.name]::mutify(zpt::mutation::emitter _emitter) -> void {
zpt::mutation::callback _h_on_change;
 
_emitter->on("$[mutation.topic.self.regex]",
{
$[mutation.handler.self.insert]
$[mutation.handler.self.update]
$[mutation.handler.self.remove]
$[mutation.handler.self.replace]
}
);

$[mutation.handler.begin]
_emitter->on("$[mutation.topic.regex]",
{
$[mutation.handler.insert]
$[mutation.handler.update]
$[mutation.handler.remove]
$[mutation.handler.replace]
}
);
$[mutation.handler.end]
}
