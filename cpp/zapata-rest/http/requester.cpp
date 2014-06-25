#include <http/requester.h>

void zapata::send(zapata::HTTPReq& _in, zapata::HTTPRep& _out, bool _ssl) {
	string _address(_in->header("Host"));
	unsigned int _port = _ssl ? 443 : 80;
	size_t _idx = -1;

	if ((_idx = _address.find(":")) != string::npos) {
		string _ports(_address.substr(_idx + 1));
		zapata::fromstr(_ports, &_port);
		_address.assign(_address.substr(_idx));
	}

	if (_ssl) {
		zapata::sslsocketstream _s;
		_s.open(_address, _port);
		cout << _in  << endl << flush;
		_s << _in << flush;

		zapata::fromstream(_s, _out);
		_s.close();
	}
	else {
		zapata::socketstream _s;
		_s.open(_address, _port);
		cout << _in  << endl << flush;
		_s << _in << flush;

		zapata::fromstream(_s, _out);
		_s.close();
	}
}
