#include <http/params.h>

void zapata::fromparams(zapata::HTTPReq& _in, zapata::JSONObj& _out, zapata::RESTfulType _resource_type) {
	if (_resource_type != zapata::RESTfulResource) {
		switch(_resource_type) {
			case zapata::RESTfulDocument : {
				_out << "_id" << _in->url();;
				break;
			}
			default : { // collection/store
				string _uri(_in->url());
				_uri.insert(0, "^");
				_uri.insert(_uri.length(), "/[^/]+$");
				_out << "_id" << _uri;
				break;
			}
		}
	}
	for (HTTPReqRef::iterator _i = _in->params().begin(); _i != _in->params().end(); _i++) {
		_out << _i->first << *(_i->second);
	}
}
