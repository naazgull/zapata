// #include <zapata/events/pipeline.h>

// auto
// zpt::emitter() -> zpt::ev::emitter_factory {
//     return zpt::EventEmitterFactory::instance();
// }

// auto
// zpt::uri::get_simplified_topics(std::string _pattern) -> zpt::json {
//     zpt::json _aliases = zpt::split(_pattern, "|");
//     zpt::json _topics = zpt::json::array();
//     char _op = '\0';
//     for (auto _alias : _aliases->arr()) {
//         std::string _return;
//         short _state = 0;
//         bool _regex = false;
//         bool _escaped = false;
//         for (auto _c : _alias->str()) {
//             switch (_c) {
//                 case '/': {
//                     if (_state == 0) {
//                         if (_regex) {
//                             if (_return.back() != '/') {
//                                 _return.push_back('/');
//                             }
//                             _return.push_back(_op);
//                             _regex = false;
//                         }
//                         _return.push_back(_c);
//                         _op = '\0';
//                     }
//                     else {
//                         _op = '+';
//                     }
//                     break;
//                 }
//                 case ')':
//                 case ']': {
//                     if (!_escaped) {
//                         _state--;
//                     }
//                     else {
//                         _escaped = false;
//                     }
//                     _regex = true;
//                     break;
//                 }
//                 case '(':
//                 case '[': {
//                     if (!_escaped) {
//                         _state++;
//                     }
//                     else {
//                         _escaped = false;
//                     }
//                     _regex = true;
//                     break;
//                 }
//                 case '{':
//                 case '}': {
//                     _op = '+';
//                     _regex = true;
//                     break;
//                 }
//                 case '+': {
//                     if (_op == '\0')
//                         _op = '*';
//                     _regex = true;
//                     break;
//                 }
//                 case '*': {
//                     _op = '*';
//                     _regex = true;
//                     break;
//                 }
//                 case '$':
//                 case '^': {
//                     break;
//                 }
//                 case '\\': {
//                     _escaped = !_escaped;
//                     break;
//                 }
//                 default: {
//                     if (_state == 0) {
//                         _return.push_back(_c);
//                     }
//                 }
//             }
//         }
//         if (_regex) {
//             if (_return.back() != '/') {
//                 _return.push_back('/');
//             }
//             _return.push_back(_op);
//         }
//         _topics << _return;
//     }
//     return _topics;
// }
// ;
extern "C" auto
zpt_events() -> int {
    return 1;
}
