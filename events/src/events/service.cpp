// #include <zapata/events/pipeline.h>

// zpt::service_graph::service_graph()
//   : __root("(.*)") {}

// zpt::service_graph::~service_graph() {}

// zpt::service_graph::node::node(zpt::json _splited, std::regex& _original_topic) {
//     for ([ _, __, _alias ] : _splited) {
//         this->insert(_alias);
//     }
// }

// zpt::service_graph::node::node(std::string _topic)
//   : node{ zpt::regex::split(_topic), std::regex{ _topic } } {}

// zpt::service_graph::node::~node() {}

// auto
// zpt::service_graph::node::find(zpt::json _topic) -> zpt::service_graph::node& {}

// auto
// zpt::service_graph::node::insert(zpt::json _topic) -> zpt::service_graph::node& {
//     for ([ _, __, _part ] : _topic) {
//     }
// }

// auto
// zpt::service_graph::insert(std::string _topic, const zpt::service_graph::node& _service)
//   -> zpt::service_graph& {
//     zpt::json _topics = zpt::uri::get_simplified_topics(_topic);
//     for (auto [_, __, _topic] : _topics) {
//         this->insert(zpt::path::split(std::string(_topic)), _service);
//     }
//     return (*this);
// }

// auto
// zpt::service_graph::merge(const zpt::service_graph::node& _other) -> void {
//     auto [_self_resolver, _self_callbacks, _self_children] = this->__root;
//     auto [_other_resolver, _other_callbacks, _other_children] = _other;
//     _self_callbacks.insert(_self_callbacks.end(), _other_callbacks.begin(),
//     _other_callbacks.end()); _self_children.insert(_self_children.end(), _other_children.begin(),
//     _other_children.end());
// }

// auto
// zpt::service_graph::insert(zpt::json _topic, const zpt::service_graph::node& _service) -> void {
//     if (_topic->size() == 0) {
//         if (std::get<1>(this->__service).size() != 0) {
//             if (std::get<1>(_service).size() != 0) {
//                 this->merge(_service);
//             }
//             return;
//         }

//         zpt::json _containers = std::get<0>(this->__service)["peers"];
//         if (_containers->is_array()) {
//             zpt::json _new_container = std::get<0>(_service);
//             if (_new_container->is_object()) {
//                 bool _found = false;
//                 for (auto _c : _containers->arr()) {
//                     if (_new_container["connect"] == _c["connect"]) {
//                         _c << "uuid" << _new_container["uuid"];
//                         _found = true;
//                         break;
//                     }
//                 }
//                 if (!_found) {
//                     _containers << _new_container;
//                 }
//             }
//         }
//         else {
//             zpt::json _s_data = std::get<0>(_service)->clone();
//             std::get<0>(_service)->obj()->clear();
//             std::get<0>(_service) << "peers" << zpt::json{ zpt::array, _s_data } << "next" << 0;
//             this->__service = _service;
//         }
//         return;
//     }

//     std::string _resolver = std::string(_topic[0]);
//     auto _child = this->__children.find(_resolver);
//     if (_child == this->__children.end()) {
//         this->__children.insert(std::make_pair(
//           _resolver.data(),
//           zpt::ev::graph(new zpt::service_graph(
//             _resolver, std::make_tuple(zpt::undefined, zpt::ev::handlers(),
//             std::regex(".*"))))));
//         _child = this->__children.find(_resolver);
//     }

//     _topic->arr()->erase(_topic->arr()->begin());
//     _child->second->insert(_topic, _service);
// }

// auto
// zpt::service_graph::find(std::string _topic, zpt::performative _performative)
//   -> zpt::service_graph::node {
//     zpt::json _splited = zpt::path::split(_topic);
//     if (std::string(_splited[0]).find(":") != std::string::npos) {
//         if (!zpt::test::uri(_topic)) {
//             return std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"));
//         }
//         zpt::json _uri = zpt::uri::parse(_topic);
//         std::string _connect =
//           std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["authority"]);
//         return std::make_tuple(
//           zpt::json{ "peers",
//                      { zpt::array,
//                        { "topic",
//                          _uri["path"],
//                          "type",
//                          (_uri["scheme"] == zpt::json::string("tcp") ?
//                          zpt::json::string("dealer")
//                                                                      : _uri["scheme"]),
//                          "connect",
//                          _connect,
//                          "regex",
//                          ".*" } },
//                      "next",
//                      0 },
//           zpt::ev::handlers(),
//           std::regex(".*"));
//     }
//     return this->find(_topic, _splited, _performative);
// }

// auto
// zpt::service_graph::find(std::string _topic, zpt::json _splited, zpt::performative _performative)
//   -> zpt::service_graph::node {
//     zpt::json _containers = std::get<0>(this->__service);
//     if (_containers->is_object()) {
//         if (std::regex_match(_topic, std::get<2>(this->__service))) {
//             if (_performative == zpt::ev::Connect) {
//                 return this->__service;
//             }

//             for (auto _c : _containers["peers"]->arr()) {
//                 if (_c["performatives"][zpt::ev::to_str(_performative)]->ok()) {
//                     return this->__service;
//                 }
//             }
//         }
//     }

//     if (_splited->arr()->size() == 0) {
//         return std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"));
//     }

//     auto _child = this->__children.find(std::string(_splited[0]));
//     if (_child != this->__children.end()) {
//         zpt::json _less = _splited->clone();
//         _less->arr()->erase(_less->arr()->begin());
//         zpt::service_graph::node _result = _child->second->find(_topic, _less, _performative);
//         if (std::get<0>(_result)->ok()) {
//             return _result;
//         }
//     }

//     _child = this->__children.find("+");
//     if (_child != this->__children.end()) {
//         zpt::json _less = _splited->clone();
//         _less->arr()->erase(_less->arr()->begin());
//         zpt::service_graph::node _result = _child->second->find(_topic, _less, _performative);
//         if (std::get<0>(_result)->ok()) {
//             return _result;
//         }
//     }

//     _child = this->__children.find("*");
//     if (_child != this->__children.end()) {
//         zpt::json _less = _splited->clone();
//         do {
//             _less->arr()->erase(_less->arr()->begin());
//             zpt::service_graph::node _result = _child->second->find(_topic, _less,
//             _performative); if (std::get<0>(_result)->ok()) {
//                 return _result;
//             }
//         } while (_less->arr()->size() != 0);
//     }

//     return std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"));
// }

// auto
// zpt::service_graph::remove(std::string _uuid) -> void {
//     zpt::json _containers = std::get<0>(this->__service);

//     if (_containers->is_object()) {
//         for (size_t _idx = 0; _idx != _containers["peers"]->arr()->size(); _idx++) {
//             if (_containers["peers"][_idx]["uuid"] == zpt::json::string(_uuid)) {
//                 _containers["peers"] >> _idx;
//                 break;
//             }
//         }
//         if (_containers["peers"]->arr()->size() == 0) {
//             std::get<0>(this->__service) = zpt::undefined;
//         }
//     }

//     for (auto _child : this->__children) {
//         _child.second->remove(_uuid);
//     }
// }

// auto
// zpt::service_graph::list(std::string _uuid) -> zpt::json {
//     zpt::json _containers = std::get<0>(this->__service);
//     zpt::json _return;

//     if (_containers->is_object()) {
//         _return = zpt::json::array();
//         for (auto _container : _containers["peers"]->arr()) {
//             if (!_container["performatives"]["REPLY"]->ok() &&
//                 (_uuid.length() == 0 || _container["uuid"] == zpt::json::string(_uuid))) {
//                 _return << _container;
//             }
//         }
//     }
//     else {
//         _return = zpt::json::array();
//     }

//     for (auto _child : this->__children) {
//         _return = _return + _child.second->list(_uuid);
//     }

//     return _return;
// }

// auto
// zpt::service_graph::pretty(std::string _tabs, bool _last) -> std::string {
//     std::string _return;
//     if (this->__resolver != "") {
//         _return = (_tabs + (_tabs != "" ? std::string(!_last ? "├─ " : "└─ ") : std::string("─
//         ")) +
//                    this->__resolver +
//                    (std::get<0>(this->__service)->is_object()
//                       ? std::string(" (") +
//                           std::to_string(std::get<0>(this->__service)["peers"]->arr()->size()) +
//                           std::string(")")
//                       : std::string("")) +
//                    std::string("\n"));
//     }

//     size_t _idx = 0;
//     for (auto _child : this->__children) {
//         _return += _child.second->pretty(
//           (this->__resolver != "" ? _tabs + std::string(!_last ? "│\t" : "\t") :
//           std::string("")), _idx == this->__children.size() - 1);
//         _idx++;
//     }
//     return _return;
// }

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
