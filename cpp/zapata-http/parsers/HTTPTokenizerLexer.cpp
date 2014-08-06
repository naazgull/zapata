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

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <parsers/HTTPTokenizerLexer.h>

zapata::HTTPTokenizerLexer::HTTPTokenizerLexer(std::istream &_in, std::ostream &_out, zapata::HTTPReq* _rootreq, zapata::HTTPRep* _rootrep) :
	zapata::HTTPLexer(_in, _out) {
}

zapata::HTTPTokenizerLexer::~HTTPTokenizerLexer() {
}

void zapata::HTTPTokenizerLexer::switchRoots(HTTPReq* _rootreq, HTTPRep* _rootrep) {
	this->__root_req = _rootreq;
	this->__root_rep = _rootrep;
}

void zapata::HTTPTokenizerLexer::justLeave() {
	this->leave(-1);
}

void zapata::HTTPTokenizerLexer::init(zapata::HTTPType _in_type) {
	this->__root_type = _in_type;
	switch (_in_type) {
		case zapata::HTTPRequest : {
			zapata::HTTPMethod _m;
			string _ms(this->matched());
			zapata::fromstr(_ms, &_m);
			(*this->__root_req)->method(_m);
			break;
		}
		case zapata::HTTPReply : {
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::body() {
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			(*this->__root_req)->body(this->matched());
			break;
		}
		case zapata::HTTPReply : {
			(*this->__root_rep)->body(this->matched());
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::url() {
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			(*this->__root_req)->url(this->matched());
			break;
		}
		case zapata::HTTPReply : {
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::status() {
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			break;
		}
		case zapata::HTTPReply : {
			int _status = 0;
			string _statusstr(this->matched());
			zapata::fromstr(_statusstr, &_status);
			(*this->__root_rep)->status((zapata::HTTPStatus) _status);
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::add() {
	string _s(this->matched());
	zapata::trim(_s);
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			(*this->__root_req) >> zapata::params;
			(*this->__root_req) << _s;
			break;
		}
		case zapata::HTTPReply : {
			(*this->__root_rep) >> zapata::params;
			(*this->__root_rep) << _s;
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::name() {
	string _s(this->matched());
	zapata::trim(_s);
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			(*this->__root_req) << zapata::params;
			(*this->__root_req) << _s;
			break;
		}
		case zapata::HTTPReply : {
			(*this->__root_rep) << zapata::params;
			(*this->__root_rep) << _s;
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::value() {
	string _s(this->matched());
	zapata::trim(_s);
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			(*this->__root_req) << zapata::params;
			(*this->__root_req) << _s;
			break;
		}
		case zapata::HTTPReply : {
			(*this->__root_rep) << zapata::params;
			(*this->__root_rep) << _s;
			break;
		}
	}
}

