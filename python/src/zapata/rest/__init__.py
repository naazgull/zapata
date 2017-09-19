import zpt
import re

PERFORMATIVES = ['get', 'post', 'put', 'patch', 'delete', 'head']
PARAM_PATTERN = re.compile(r'(\{[^\}]+\})')

class RestHandler(object):

    def __init__(self):
        self.resource_topic = None
        self.datum_topic = None
        self.datum_topic_params = None
        self.allowed_performatives = PERFORMATIVES
        self.unauthorized_status = 401
        self.unauthorized_code = 1019
        self.unauthorized_text = 'unauthorized access to this resource'

    def __getattribute__(self, name):

        try:
            return object.__getattribute__(self, name)
        except Exception as e:

            if name in self.allowed_performatives:
                return self.dispatch
            elif name in ['{}_callback'.format(method) for method in self.allowed_performatives]:
                return self.default_callback

        super(RestHandler, self).__getattribute__(name)

    def __setattribute__(self, name, value):

        if name == 'allowed_performatives':
            return [method for method in self.__allowed_performatives if method in PERFORMATIVES]

        super(RestHandler, self).__setattribute__(name, value)
        
    def authorize(self, method, topic, envelope, context):

        identity = zpt.authorize(self.resource_topic, envelope)
        
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

    def path_discover(self, template_path, original_path, endpoint):

        template_path_parts = template_path.split('/')
        original_path_parts = original_path.split('/')

        if len(template_path_parts) != len(original_path_parts):
            raise Exception('The template topic and topic pattern received doesn\'t match.')

        groups = PARAM_PATTERN.findall(template_path)
        params = {}

        for group in groups:
            params[re.sub(r'[\{\}]', '', group)] = original_path_parts[template_path_parts.index(group)]

        return (endpoint.format(**params), params)

    def variable_resolver(self, data_template, variables):

        for key in data_template.keys():

            value = data_template[key]

            if type(value) is str:
                data_template[key] = value.format(**variables) if value else None

        return data_template if data_template else {}

    def dispatch(self, method, topic, envelope, context):
        
        identity = self.authorize(method, topic, envelope, context)

        if not identity:
            return # cancel the dispatch

        endpoint, variables = self.path_discover(self.resource_topic, topic, self.datum_topic)
        extra_params = self.variable_resolver(self.datum_params, variables)

        zpt.route(
            method,
            endpoint,
            {
                'headers': zpt.auth_headers(identity),
                'params': zpt.merge(envelope.get('params'), extra_params) if envelope.get('params') else extra_params, # TODO: remove the if condition after merge function is working with a nil object
                'payload': zpt.merge(envelope.get('payload'), extra_params) if envelope.get('payload') else extra_params # TODO: remove the if condition after merge function is working with a nil object
            },
            {'context': envelope },
            getattr(self, '{}_callback'.format(method))
        )
