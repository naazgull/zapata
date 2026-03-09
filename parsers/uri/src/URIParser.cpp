#include <zapata/uri/URIParser.h>

zpt::URIParser::URIParser(std::istream& _in, std::ostream& _out) {
    this->d_scanner.switchStreams(_in, _out);
}

zpt::URIParser::~URIParser() {}

auto zpt::URIParser::switchRoots(zpt::json& _root) -> void { this->d_scanner.switchRoots(_root); }

auto zpt::URIParser::switchStreams(std::istream& _in, std::ostream& _out) -> void {
    this->d_scanner.switchStreams(_in, _out);
}

auto zpt::URIParser::clear() -> void { *this->d_scanner = zpt::undefined; }
