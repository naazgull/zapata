#include <zapata/uri/URIParser.h>

zpt::URIParser::URIParser(std::istream& _in, std::ostream& _out) {
    this->d_scanner.switchStreams(_in, _out);
}

zpt::URIParser::~URIParser() {}

void
zpt::URIParser::switchRoots(zpt::json& _root) {
    this->d_scanner.switchRoots(_root);
}

void
zpt::URIParser::switchStreams(std::istream& _in, std::ostream& _out) {
    this->d_scanner.switchStreams(_in, _out);
}

auto
zpt::uri::parse(std::string _in) -> zpt::json {
    std::istringstream _iss;
    _iss.str(_in);
    return zpt::uri::parse(_iss);
}

auto
zpt::uri::parse(std::istream& _in) -> zpt::json {
    static thread_local zpt::URIParser _thread_local_parser;
    zpt::json _root = zpt::json::object();
    _thread_local_parser.switchRoots(_root);
    _thread_local_parser.switchStreams(_in);
    _thread_local_parser.parse();
    _root >> "__aux";
    return _root;
}
