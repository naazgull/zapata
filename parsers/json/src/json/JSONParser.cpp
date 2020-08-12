#include <zapata/json/JSONParser.h>

zpt::JSONParser::JSONParser(std::istream& _in, std::ostream& _out) {
    this->d_scanner.switchStreams(_in, _out);
}

zpt::JSONParser::~JSONParser() {}

void
zpt::JSONParser::switchRoots(zpt::json& _root) {
    this->d_scanner.switchRoots(_root);
}

void
zpt::JSONParser::switchStreams(std::istream& _in, std::ostream& _out) {
    this->d_scanner.switchStreams(_in, _out);
}
