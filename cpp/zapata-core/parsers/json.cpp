#include <parsers/json.h>

void zapata::fromstr(string& _in, JSONObj& _out) {
	istringstream _ss;
	_ss.str(_in);
	zapata::JSONParser _p;
	_p.switchRoots(&_out);
	_p.switchStreams(_ss);
	_p.parse();

}

void zapata::fromstr(string& _in, JSONArr& _out) {
	istringstream _ss;
	_ss.str(_in);
	zapata::JSONParser _p;
	_p.switchRoots(NULL, &_out);
	_p.switchStreams(_ss);
	_p.parse();
}

void zapata::fromstr(string& _in, JSONElement** _out, zapata::JSONType* _type )  {
	istringstream _ss;
	_ss.str(_in);

	zapata::JSONParser _p;
	JSONObj _o;
	JSONArr _a;
	_p.switchRoots(&_o, &_a);
	_p.switchStreams(_ss);
	_p.parse();

	if (_p.d_scanner.__root_type == zapata::JSObject) {
		*_type = zapata::JSObject;
		*_out = _o.get();
	}
	else {
		*_type = zapata::JSArray;
		*_out = _a.get();
	}
}

void zapata::fromfile(ifstream& _in, JSONObj& _out) {
	if (_in.is_open()) {
		zapata::JSONParser _p;
		_p.switchRoots(&_out);
		_p.switchStreams(_in);
		_p.parse();
	}
}

void zapata::fromfile(ifstream& _in, JSONArr& _out) {
	if (_in.is_open()) {
		zapata::JSONParser _p;
		_p.switchRoots(NULL, &_out);
		_p.switchStreams(_in);
		_p.parse();
	}
}

void zapata::fromfile(ifstream& _in, JSONElement** _out, zapata::JSONType* _type )  {
	if (_in.is_open()) {
		zapata::JSONParser _p;
		JSONObj _o;
		JSONArr _a;
		_p.switchRoots(&_o, &_a);
		_p.switchStreams(_in);
		_p.parse();

		if (_p.d_scanner.__root_type == zapata::JSObject) {
			*_type = zapata::JSObject;
			*_out = _o.get();
		}
		else {
			*_type = zapata::JSArray;
			*_out = _a.get();
		}
	}
}

void zapata::tostr(string& _out, JSONElement& _in)  {
	_in.stringify(_out, _in.flags());
}

void zapata::tostr(string& _out, JSONObj& _in)  {
	_in->stringify(_out, _in->flags());
}

void zapata::tostr(string& _out, JSONArr& _in)  {
	_in->stringify(_out, _in->flags());
}
