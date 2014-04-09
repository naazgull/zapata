#include <parsers/http.h>

void zapata::fromstr(string& _in, HTTPReq& _out) {
	istringstream _ss;
	_ss.str(_in);
	zapata::HTTPParser _p;
	_p.switchRoots(&_out);
	_p.switchStreams(_ss);
	_p.parse();

}

void zapata::fromstr(string& _in, HTTPRep& _out) {
	istringstream _ss;
	_ss.str(_in);
	zapata::HTTPParser _p;
	_p.switchRoots(NULL, &_out);
	_p.switchStreams(_ss);
	_p.parse();
}

void zapata::fromstr(string& _in, HTTPElement** _out, zapata::HTTPType* _type )  {
	istringstream _ss;
	_ss.str(_in);

	zapata::HTTPParser _p;
	HTTPReq _o;
	HTTPRep _a;
	_p.switchRoots(&_o, &_a);
	_p.switchStreams(_ss);
	_p.parse();

	if (_p.d_scanner.__root_type == zapata::HTTPRequest) {
		*_type = zapata::HTTPRequest;
		*_out = _o.get();
	}
	else {
		*_type = zapata::HTTPReply;
		*_out = _a.get();
	}
}

void zapata::fromfile(ifstream& _in, HTTPReq& _out) {
	if (_in.is_open()) {
		zapata::HTTPParser _p;
		_p.switchRoots(&_out);
		_p.switchStreams(_in);
		_p.parse();
	}
}

void zapata::fromfile(ifstream& _in, HTTPRep& _out) {
	if (_in.is_open()) {
		zapata::HTTPParser _p;
		_p.switchRoots(NULL, &_out);
		_p.switchStreams(_in);
		_p.parse();
	}
}

void zapata::fromfile(ifstream& _in, HTTPElement** _out, zapata::HTTPType* _type )  {
	if (_in.is_open()) {
		zapata::HTTPParser _p;
		HTTPReq _o;
		HTTPRep _a;
		_p.switchRoots(&_o, &_a);
		_p.switchStreams(_in);
		_p.parse();

		if (_p.d_scanner.__root_type == zapata::HTTPRequest) {
			*_type = zapata::HTTPRequest;
			*_out = _o.get();
		}
		else {
			*_type = zapata::HTTPReply;
			*_out = _a.get();
		}
	}
}

void zapata::tostr(string& _out, HTTPElement& _in)  {
	_in.stringify(_out);
}
