import zpt
from zpt.rest import RestHandler

class $[resource.handler.class.name](RestHandler) :

    def __init__(self):
        super($[resource.handler.class.name], self).__init__()
        self.resource_topic = '$[resource.topic]'
        $[resource.topic.relay]
        self.allowed_performatives = $[resource.allowed.performatives]

        
handler = $[resource.handler.class.name]()
zpt.on('$[resource.topic.regex]', handler, $[resource.opts])
