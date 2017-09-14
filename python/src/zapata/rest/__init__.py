import zpt

class RestHandler(object):

    def __init__(self):
        self.get_callback = self.__callback
        self.post_callback = self.__callback
        self.put_callback = self.__callback
        self.patch_callback = self.__callback
        self.delete_callback = self.__callback
        self.head_callback = self.__callback

        self.web_endpoint = ''
        self.data_layer_endpoint = ''
        self.methods_allowed = ['get', 'post', 'put', 'patch', 'delete', 'head']
        self.unauthorized_status = 401
        self.unauthorized_code = 1019
        self.unauthorized_text = 'unauthorized access to this resource'
        
    def authorize(self, method, performative, topic, envelope, context):

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

    def __callback(self, performative, topic, t_reply, context):
        zpt.reply(context, t_reply)
        
    def dispatch(self, method, performative, topic, envelope, context):

        identity = self.authorize()

        if not identity:
            return # cancel the dispatch

        zpt.route(
            method,
            self.data_layer_endpoint,
            {'headers': zpt.auth_headers(identity), 'params' : envelope.get('params') },
            {'context': envelope },
            getattr(self, '{}_callback'.format(method))
        )

    def get(self, performative, topic, envelope, context):
        self.dispatch('get', performative, topic, envelope, context)

    def post(performative, topic, envelope, context):
        self.dispatch('post', performative, topic, envelope, context)

    def put(performative, topic, envelope, context):
        self.dispatch('put', performative, topic, envelope, context)

    def patch(performative, topic, envelope, context):
        self.dispatch('patch', performative, topic, envelope, context) 

    def delete(performative, topic, envelope, context):
        self.dispatch('delete', performative, topic, envelope, context)

    def head(performative, topic, envelope, context):
        self.dispatch('head', performative, topic, envelope, context)
