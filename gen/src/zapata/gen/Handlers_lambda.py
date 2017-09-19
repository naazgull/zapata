import zpt

class $[resource.handler.class.name](object) :

    def __init__(self) :
        pass

        
$[resource.handler.get]$[resource.handler.post]$[resource.handler.put]$[resource.handler.patch]$[resource.handler.delete]$[resource.handler.head]
# If want to pass an instance of $[resource.handler.class.name]
# to handlers, as the 'context' parameter, create an instance
# and pass it on as the third argument of 'zpt.on'
#
# context = $[resource.handler.class.name]()
#
zpt.on('$[resource.topic.regex]',
    {
        $[resource.handler.get.name]$[resource.handler.post.name]$[resource.handler.put.name]$[resource.handler.patch.name]$[resource.handler.delete.name]$[resource.handler.head.name]
    },
    $[resource.opts]
    #, context
)
