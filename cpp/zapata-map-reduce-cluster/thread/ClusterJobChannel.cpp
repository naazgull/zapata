#include <thread/ClusterJobChannel.h>

zapata::ClusterJobChannel::ClusterJobChannel(string _address, uint16_t _port) {
	this->__stream.open(_address, _port);
}

zapata::ClusterJobChannel::~ClusterJobChannel() {
}

void zapata::ClusterJobChannel::notify(void* _in, void* _out) {
	HTTPReq* _req = (HTTPReq*) _in;
	this->__stream << _req << flush;

	HTTPRep* _rep = (HTTPRep*) _out;
	zapata::fromstream(this->__stream, *_rep);
}
