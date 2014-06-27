#include <api/codes_rest.h>
#include <api/codes_users.h>
#include <base/assert.h>
#include <base/smart_ptr.h>
#include <base/str_map.h>
#include <db/convert_mongo.h>
#include <exceptions/AssertionException.h>
#include <http/HTTPObj.h>
#include <http/params.h>
#include <json/JSONObj.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <mongo/client/connpool.h>
#include <mongo/client/dbclientinterface.h>
#include <parsers/json.h>
#include <resource/RESTResource.h>
#include <resource/UsersDocument.h>
#include <string>

zapata::UsersDocument::UsersDocument() :
	zapata::RESTDocument("^/users/([^/]+)$") {
}

zapata::UsersDocument::~UsersDocument() {
}

void zapata::UsersDocument::get(HTTPReq& _req, HTTPRep& _rep) {
	mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) this->configuration()["zapata"]["mongodb"]["address"]);
	string _collection((string) this->configuration()["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), "." + ((string) this->configuration()["zapata_users"]["mongodb"]["collection"]));

	zapata::JSONObj _params;
	zapata::fromparams(_req, _params, zapata::RESTfulDocument);

	zapata::JSONObj _out;
	try {
		zapata::torestdocument(_conn, _collection, _params, _out);
	}
	catch (mongo::exception& _e) {
		_conn->done();
		delete _conn;
		assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
	}
	_conn->done();
	delete _conn;

	assertz(_out->size() != 0, "The requested user was not found", zapata::HTTP404, zapata::ERRUserNotFound);

	string _text;
	zapata::tostr(_text, _out);
	_rep->status(zapata::HTTP200);
	_rep->body(_text);
	_rep << "Content-Type" << "application/json" << "Content-Length" << _text.length();
}

void zapata::UsersDocument::put(HTTPReq& _req, HTTPRep& _rep) {
	string _body = _req->body();
	assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

	string _content_type = _req->header("Content-Type");
	assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

	zapata::JSONObj _record;
	zapata::fromstr(_body, _record);

	mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) this->configuration()["zapata"]["mongodb"]["address"]);
	string _collection((string) this->configuration()["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), "." + ((string) this->configuration()["zapata_users"]["mongodb"]["collection"]));

	string __id(_req->url());

	bool _exists = false;
	try {
		mongo::BSONObjBuilder _id_bo;
		_id_bo.append("_id", __id);

		mongo::BSONObjBuilder _bo;
		zapata::tomongo(_record, _bo);

		(*_conn)->update(_collection, _id_bo.obj(),  _bo.obj());

		_exists = (*_conn)->getLastError().length() == 0;
	}
	catch (mongo::exception& _e) {
		_conn->done();
		delete _conn;
		assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
	}

	_conn->done();
	delete _conn;

	 assertz(_exists, "The requested user was not found", zapata::HTTP404, zapata::ERRUserNotFound);

	zapata::JSONObj _rep_body;
	_rep_body << "href" << __id;
	string _text;
	zapata::tostr(_text, _rep_body);
	_rep->status(zapata::HTTP200);
	_rep << "Location" << __id << "Content-Length" << (long) _text.length();
	_rep->body(_text);
}

void zapata::UsersDocument::remove(HTTPReq& _req, HTTPRep& _rep) {
	mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) this->configuration()["zapata"]["mongodb"]["address"]);
	string _collection((string) this->configuration()["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), "." + ((string) this->configuration()["zapata_users"]["mongodb"]["collection"]));

	zapata::JSONObj _params;
	zapata::fromparams(_req, _params, zapata::RESTfulDocument);

	zapata::JSONObj _out;
	try {
		zapata::torestdocument(_conn, _collection, _params, _out);
		(*_conn)->remove(_collection, QUERY("_id" << _req->url()), true);
	}
	catch (mongo::exception& _e) {
		_conn->done();
		delete _conn;
		assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
	}
	_conn->done();
	delete _conn;

	string _text;
	zapata::tostr(_text, _out);
	_rep->status(zapata::HTTP200);
	_rep->body(_text);
	_rep << "Content-Type" << "application/json" << "Content-Length" << _text.length();
}

void zapata::UsersDocument::head(HTTPReq& _req, HTTPRep& _rep) {
	mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) this->configuration()["zapata"]["mongodb"]["address"]);
	string _collection((string) this->configuration()["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), "." + ((string) this->configuration()["zapata_users"]["mongodb"]["collection"]));

	zapata::JSONObj _params;
	zapata::fromparams(_req, _params, zapata::RESTfulDocument);

	zapata::JSONObj _out;
	try {
		zapata::torestdocument(_conn, _collection, _params, _out);
	}
	catch (mongo::exception& _e) {
		_conn->done();
		delete _conn;
		assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
	}
	_conn->done();
	delete _conn;

	assertz(_out->size() != 0, "The requested user was not found", zapata::HTTP404, zapata::ERRUserNotFound);

	string _text;
	zapata::tostr(_text, _out);
	_rep->status(zapata::HTTP200);
	_rep << "Content-Type" << "application/json" << "Content-Length" << _text.length();
}
