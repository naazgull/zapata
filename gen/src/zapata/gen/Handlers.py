import zpt

$[resource.handler.get]$[resource.handler.post]$[resource.handler.put]$[resource.handler.patch]$[resource.handler.delete]$[resource.handler.head]
zpt.on('$[resource.topic.regex]',
        {
                $[resource.handler.get.name]$[resource.handler.post.name]$[resource.handler.put.name]$[resource.handler.patch.name]$[resource.handler.delete.name]$[resource.handler.head.name]
        },
        $[resource.opts]
)
