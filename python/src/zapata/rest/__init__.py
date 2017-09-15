import zpt

METHODS = ['get', 'post', 'put', 'patch', 'delete', 'head']

class RestHandler(object):

    def __init__(self):
        self.web_endpoint = None
        self.data_layer_endpoint = None
        self.methods_allowed = METHODS
        self.unauthorized_status = 401
        self.unauthorized_code = 1019
        self.unauthorized_text = 'unauthorized access to this resource'

    def __getattribute__(self, name):

        try:
            return object.__getattribute__(self, name)
        except Exception as e:

            if name in self.methods_allowed:
                return self.dispatch
            elif name in ['{}_callback'.format(method) for method in self.methods_allowed]:
                return self.default_callback

        super(RestHandler, self).__getattribute__(name)

    def __setattribute__(self, name, value):

        if name == 'methods_allowed':
            return [method for method in self.__methods_allowed if method in METHODS]

        super(RestHandler, self).__setattribute__(name, value)
        
    def authorize(self, method, topic, envelope, context):

        identity = zpt.authorize(self.web_endpoint, envelope)
        
        if not identity:
            zpt.reply(envelope, {
                'status': self.unauthorized_status,
                'payload': {
                    'code': self.unauthorized_code,
                    'text': self.unauthorized_text
                }
            })
            return None

        return identity

    @staticmethod
    def default_callback(method, topic, reply, context):
        zpt.reply(context, reply)
        
    def dispatch(self, method, topic, envelope, context):
        
        identity = self.authorize(method, topic, envelope, context)

        if not identity:
            return # cancel the dispatch

        zpt.route(
            method,
            self.data_layer_endpoint,
            {'headers': zpt.auth_headers(identity), 'params' : envelope.get('params') },
            {'context': envelope },
            getattr(self, '{}_callback'.format(method))
        )
