#include <api/RESTPool.h>

zapata::RESTPool::RESTPool() {
}

zapata::RESTPool::~RESTPool() {
}

void zapata::RESTPool::add(RESTResource* _res) {
	this->__resources.push_back(_res);
}

void zapata::RESTPool::process(HTTPReq& _req, HTTPRep& _rep) {
	for (vector<RESTResource*>::iterator _i = this->__resources.begin(); _i != this->__resources.end(); _i++) {
		if ((*_i)->matches(_req->url())) {
			switch(_req->method()) {
				case zapata::HTTPGet : {
					(*_i)->get(_req, _rep);
					break;
				}
				case zapata::HTTPPut : {
					(*_i)->put(_req, _rep);
					break;
				}
				case zapata::HTTPPost : {
					(*_i)->post(_req, _rep);
					break;
				}
				case zapata::HTTPDelete : {
					(*_i)->remove(_req, _rep);
					break;
				}
				case zapata::HTTPHead : {
					(*_i)->head(_req, _rep);
					break;
				}
				case zapata::HTTPTrace : {
					(*_i)->trace(_req, _rep);
					break;
				}
				case zapata::HTTPOptions : {
					(*_i)->options(_req, _rep);
					break;
				}
				case zapata::HTTPPatch : {
					(*_i)->patch(_req, _rep);
					break;
				}
				case zapata::HTTPConnect : {
					(*_i)->connect(_req, _rep);
					break;
				}
			}
		}
	}
}

/*extern "C" void populate(RESTPool& _pool) {
}*/

