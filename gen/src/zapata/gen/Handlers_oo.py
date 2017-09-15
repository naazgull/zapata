import zpt
from zpt.rest import RestHandler

class $[resource.handler.class.name](RestHandler) :

    def __init__(self):
        super(RecipesCollection, self).__init__()
        self.web_endpoint = '^/v3/recipes$'
        self.data_layer_endpoint = '/v3/data-layer/recipes'
        self.methods_allowed = ['get', 'post']

handler = RecipesCollection()
zpt.on(handler, {'http': True, '0mq': True, 'amqp': True})


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
