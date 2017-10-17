import zpt
import re
from ast import literal_eval
import builtins
import importlib
from functools import reduce

PERFORMATIVES = ['get', 'post', 'put', 'patch', 'delete', 'remove', 'head']
PARAM_PATTERN = re.compile(r'(\{[^\}]+\})')
VARIABLE_PATTERN = re.compile(r'\{(?P<variable>[^\}:]+):?(?P<type>[^\}:]+)?:?(?P<extra>[^\}]+)?\}')
RULES_ALLOWED_CONDITIONALS = {
    '$eq': '==',
    '$neq': '!=',
    '$gt': '>',
    '$gte': '>=',
    '$lt': '<',
    '$lte': '<=',
    '$in': 'in',
    '$nin': 'not in'
}


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
            elif name in PERFORMATIVES:
                return self.performative_not_accepted
            elif name in ['{}_callback'.format(performative) for performative in self.allowed_performatives]:
                return RestHandler.default_callback

        super(RestHandler, self).__getattribute__(name)

    def __setattribute__(self, name, value):

        if name == 'allowed_performatives':
            return [performative for performative in self.__allowed_performatives if performative in PERFORMATIVES]

        super(RestHandler, self).__setattribute__(name, value)
        
    def authorize(self, performative, topic, envelope, context):

        identity = zpt.authorize(self.resource_topic, envelope)
        
        if not identity:
            return RestHandler.reply(envelope, status=self.unauthorized_status, payload={
                'code': self.unauthorized_code,
                'text': self.unauthorized_text
            })

        return identity

    @staticmethod
    def default_callback(performative, topic, reply, context):
        return zpt.reply(context, reply)

    def path_discover(self, template_path, original_path, endpoint):

        template_path_parts = template_path.split('/')
        original_path_parts = original_path.split('/')

        if len(template_path_parts) != len(original_path_parts):
            raise Exception('The template topic and topic pattern received doesn\'t match.')

        groups = PARAM_PATTERN.findall(template_path)
        params = {}

        for group in groups:
            params[re.sub(r'[\{\}]', '', group)] = original_path_parts[
                template_path_parts.index(group)
            ]

        return (endpoint.format(**params), params)
    
    def deep_get(self, _dict, keys, default=None):

        if not isinstance(keys, list):
            raise Exception('Keys must be list of strings')

        def _reducer(d, key):
            if isinstance(d, dict):
                return d.get(key, default)
            elif isinstance(d, list):
                try:
                    return d[int(key)]
                except Exception as e:
                    pass
            return default

        return reduce(_reducer, keys, _dict)

    def try_to_cast(self, _object, _type):
        try:
            if _type in ['dict', 'list']:
                return literal_eval(_object)
        
            cls = getattr(builtins, _type)
            return cls(_object)
        except Exception as e:
            return _object

    def variables_resolver(self, _object, _variables):

        if isinstance(_object, str):

            for item in re.findall(VARIABLE_PATTERN, _object):

                _key, _type, extra = item
                _s_key = _key.split('.')

                _value = self.deep_get(
                    _variables,
                    _s_key if isinstance(_s_key,  list) else [_s_key],
                    default=''
                )

                _object = self.try_to_cast(
                    re.sub(r'\{' + _key + '(:[^\}]+)?\}', str(_value), _object),
                    _type if _type else 'str',
                )

            return _object
        
        elif isinstance(_object, dict):
            for key, value in _object.items():
                _object[key] = self.variables_resolver(value, _variables)
                                 
        elif isinstance(_object, list):
            for i in range(len(_object)):
                _object[i] = self.variables_resolver(_object[i], _variables)

        return _object

    def variables_extractor(self, _object, result={}):

        if isinstance(_object, str):
            
            variables = {}
            
            for item in re.findall(VARIABLE_PATTERN, _object):

                _name, _type, _extra = item
                variables.update({_name: '{_type}|{_extra}'.format(
                    _type=_type if _type else 'string',
                    _extra=_extra if _extra else 'required'
                )})
                
            return variables
                                 
        elif isinstance(_object, dict):

            for key, value in _object.items():
                 result.update(self.variables_extractor(value, result=result))
                                 
        elif isinstance(_object, list):
                                 
            for item in _object:
                result.update(self.variables_extractor(item, result=result))

        return result

    def performative_not_accepted(self, performative, topic, envelope, context=None):
        return RestHandler.reply(envelope, status=405, payload={
            'code': 1300,
            'text': 'Performative is not accepted for the given resource'
        })

    def dispatch(self, performative, topic, envelope, context=None):

        if self.datum_topic is None:
            return self.performative_not_accepted(performative, topic, envelope, context)
    
        identity = self.authorize(performative, topic, envelope, context)

        if not identity:
            return # cancel the dispatch

        endpoint, variables = self.path_discover(self.resource_topic, topic, self.datum_topic)
        extra_params = self.variables_resolver(self.datum_topic_params, variables)

        params = zpt.merge(
            envelope.get('params'),
            extra_params
        ) if envelope.get('params') else extra_params

        payload = zpt.merge(
            envelope.get('payload'),
            extra_params
        ) if envelope.get('payload') else extra_params

        return self.datum_request(
            performative,
            endpoint,
            envelope,
            identity,
            params=params,
            payload=payload
        )

    def datum_request(self, performative, endpoint, envelope, identity, params = None, payload = None, callback = None):
        return zpt.route(
            performative,
            endpoint,
            {
                'headers': zpt.auth_headers(identity),
                'params': params, # TODO: remove the if condition after merge function is working with a nil object
                'payload': payload # TODO: remove the if condition after merge function is working with a nil object
            },
            {'context': envelope},
            getattr(self, '{}_callback'.format(performative)) if not callback else callback
        )

    @staticmethod
    def reply(envelope, **kwargs):
        return zpt.reply(envelope, kwargs)

    def assert_rules(self, rules):

        if not isinstance(rules, list):
            raise Exception('The Rules must be a list')

        for rule in rules:

            for _cond, _test in rule.items():

                _op = RULES_ALLOWED_CONDITIONALS.get(_cond)

                if not _op:
                    raise Exception('Operator not allowed. Check the allowed list: {}'.format(
                        ', '.join(RULES_ALLOWED_CONDITIONALS.keys())
                    ))

                if not isinstance(_test, list) or len(_test) != 2:
                    raise Exception('Test values malformatted. It must to be a list and the list size equal 2')
                
                _expression = 'True if {left} {op} {right} else False'.format(
                    left=self.auto_quote(_test[0]),
                    op=RULES_ALLOWED_CONDITIONALS.get(_cond),
                    right=self.auto_quote(_test[1])
                )

                passed = eval(_expression)

                if not passed:
                    return False

        return True

    def auto_quote(self, _object):

        if type(_object) in [int, list]:
            return _object
        else:
            return '"{}"'.format(re.escape(_object))

