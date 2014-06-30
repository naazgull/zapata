#include <http/params.h>

#define __SPECIAL_PARAMS string("orderBy pageSize pageStartIndex fields embed ")

void zapata::fromparams(zapata::HTTPReq& _in, zapata::JSONObj& _out, zapata::RESTfulType _resource_type, bool _regexp) {
	if (_resource_type != zapata::RESTfulResource) {
		switch(_resource_type) {
			case zapata::RESTfulDocument : {
				_out << "_id" << _in->url();;
				break;
			}
			default : { // collection/store
				string _uri(_in->url());
				_uri.insert(0, "^");
				_uri.insert(_uri.length(), "/([^/]+)$");
				zapata::replace(_uri, "/", "\\\\/");
				_uri.insert(0, "m/");
				_uri.insert(_uri.length(), "/i");
				_out << "_id" << _uri;
				break;
			}
		}
	}
	for (HTTPReqRef::iterator _i = _in->params().begin(); _i != _in->params().end(); _i++) {
		string _value(_i->second->data());
		string _key(_i->first.data());
		zapata::url_decode(_value);
		if (_regexp && __SPECIAL_PARAMS.find(_key + string(" ")) == string::npos) {
			zapata::replace(_value, "/", "\\\\/");
			zapata::replace(_value, " ", "(.*)");
			_value.insert(0, "m/");
			_value.insert(_value.length(), "/i");
		}
		_out << _i->first << _value;
	}
}
