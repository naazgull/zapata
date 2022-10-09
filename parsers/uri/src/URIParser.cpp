#include <zapata/uri/URIParser.h>

zpt::URIParser::URIParser(std::istream& _in, std::ostream& _out) { this->d_scanner.switchStreams(_in, _out); }

zpt::URIParser::~URIParser() {}

void
zpt::URIParser::switchRoots(zpt::json& _root) {
    this->d_scanner.switchRoots(_root);
}

void
zpt::URIParser::switchStreams(std::istream& _in, std::ostream& _out) {
    this->d_scanner.switchStreams(_in, _out);
}
