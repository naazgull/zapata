/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

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

void zapata::fromstream(istream& _in, HTTPReq& _out) {
	zapata::HTTPParser _p;
	_p.switchRoots(&_out);
	_p.switchStreams(_in);
	_p.parse();
}

void zapata::fromstream(istream& _in, HTTPRep& _out) {
	zapata::HTTPParser _p;
	_p.switchRoots(NULL, &_out);
	_p.switchStreams(_in);
	_p.parse();
}

void zapata::fromstream(istream& _in, HTTPElement** _out, zapata::HTTPType* _type )  {
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

void zapata::tostr(string& _out, HTTPElement& _in)  {
	_in.stringify(_out);
}

void zapata::tostr(string& _out, HTTPRep& _in)  {
	_in->stringify(_out);
}

void zapata::tostr(string& _out, HTTPReq& _in)  {
	_in->stringify(_out);
}
